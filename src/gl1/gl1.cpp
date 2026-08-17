//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////	File:			gl1/gl1.cpp
////	Author:			(added 2026 -- not part of Ed Martin's 2007 source)
////	Description:	Implementation of the OpenGL 1.x -> GLES2/WebGL shim declared in gl1.h.
////
////	HOW IT WORKS
////
////	1) Vertices are transformed to EYE SPACE on the CPU, at glVertex3f() time.
////	   This is the central design decision and it buys three things at once:
////
////	     a) The only per-draw uniform left is the projection matrix, which changes about
////	        twice a frame. Everything else can be batched.
////	     b) GL_EYE_LINEAR and GL_SPHERE_MAP texgen need eye-space position and normal --
////	        which we now have in hand, for free, exactly where we need them.
////	     c) The 2007 code has NO glPushMatrix/glPopMatrix anywhere (every draw function
////	        manually inverts its own transform), so there is no matrix stack to respect.
////
////	2) Primitives are decomposed to independent GL_TRIANGLES and appended to one big batch,
////	   flushed only when the bound texture or the textured/untextured state changes. The game
////	   issues ~50,000 glBegin/glEnd pairs per frame; this collapses them into roughly a dozen
////	   draw calls. Winding is ignored because the game never enables face culling.
////
////	3) glTexImage2D is cached by pixel pointer, which silently fixes the game's habit of
////	   re-uploading 3.9 MB of texture every frame. See gl1_TexImage2D below.
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GL1_IMPLEMENTATION      /* see gl1.h -- keeps the interception macros out of this file */
#include "gl1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	1) Matrix maths -- 4x4, column-major doubles, same convention as real OpenGL (m[col*4+row])
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef double Mat[16];

/* Arrays are not copy-assignable, so the (unused by this game) matrix stack needs a wrapper. */
struct MatBox { double m[16]; };

static void matIdentity(Mat m)
{
	int i;
	for (i = 0; i < 16; i++) m[i] = 0.0;
	m[0] = m[5] = m[10] = m[15] = 1.0;
}

static void matCopy(Mat dst, const Mat src)
{
	int i;
	for (i = 0; i < 16; i++) dst[i] = src[i];
}

/* out = a * b  (out may alias a or b) */
static void matMul(Mat out, const Mat a, const Mat b)
{
	Mat t;
	int c, r, k;
	for (c = 0; c < 4; c++)
		for (r = 0; r < 4; r++)
		{
			double s = 0.0;
			for (k = 0; k < 4; k++) s += a[k*4 + r] * b[c*4 + k];
			t[c*4 + r] = s;
		}
	matCopy(out, t);
}

/* transform a point (w = 1) */
static void matXformPoint(const Mat m, double x, double y, double z, double *ox, double *oy, double *oz)
{
	*ox = m[0]*x + m[4]*y + m[8] *z + m[12];
	*oy = m[1]*x + m[5]*y + m[9] *z + m[13];
	*oz = m[2]*x + m[6]*y + m[10]*z + m[14];
}

/* transform a direction (w = 0). The game only ever translates and rotates -- never scales --
   so the upper 3x3 is orthonormal and doubles as the normal matrix. */
static void matXformDir(const Mat m, double x, double y, double z, double *ox, double *oy, double *oz)
{
	*ox = m[0]*x + m[4]*y + m[8] *z;
	*oy = m[1]*x + m[5]*y + m[9] *z;
	*oz = m[2]*x + m[6]*y + m[10]*z;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	2) Shim state
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Vertex
{
	GLfloat x, y, z;        /* eye space */
	GLfloat r, g, b, a;
	GLfloat s, t;
};

#define GL1_MAX_BATCH_VERTS  (96 * 1024)

static bool   gInited        = false;

/* matrices */
static Mat    gModelview;
static Mat    gProjection;
static GLenum gMatrixMode    = GL_MODELVIEW;
static std::vector<MatBox> *gStack = 0;      /* unused by this game, supported anyway */

/* current immediate-mode attributes */
static GLfloat gCurR = 1, gCurG = 1, gCurB = 1, gCurA = 1;
static GLfloat gCurNx = 0, gCurNy = 0, gCurNz = 1;   /* GL's default normal */

/* texgen */
static GLenum gTexGenSMode = GL_EYE_LINEAR;
static GLenum gTexGenTMode = GL_EYE_LINEAR;
static bool   gTexGenSOn   = false;
static bool   gTexGenTOn   = false;

/* texture / raster state */
static bool   gTextured     = false;
static GLuint gBoundTex     = 0;
static GLint  gMinFilter    = GL_NEAREST;
static GLint  gMagFilter    = GL_LINEAR;
static GLint  gWrapS        = GL_REPEAT;
static GLint  gWrapT        = GL_REPEAT;

/* primitive assembly */
static GLenum gPrimMode = 0;
static bool   gInBegin  = false;
static std::vector<Vertex> *gPrim  = 0;      /* vertices of the current glBegin/glEnd */
static std::vector<Vertex> *gBatch = 0;      /* accumulated triangles awaiting a draw call */

/* GL objects */
static GLuint gProgram = 0, gVBO = 0;
static GLint  gLocProj = -1, gLocTex = -1, gLocTextured = -1;
static GLint  gAttrPos = -1, gAttrCol = -1, gAttrTex = -1;

/* stats, printed by gl1_ReportStats() */
static long   gDrawCalls = 0, gVertsDrawn = 0, gBeginCount = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	3) Shaders
////
////	One program covers the whole game, because the game has no lighting -- just flat vertex
////	colour and an optional texture combined in GL_DECAL mode.
////
////	No #version directive: desktop GLSL then defaults to 110 and GLSL ES to 100, both of which
////	have attribute/varying/texture2D. The GL_ES guard supplies the precision qualifier that
////	GLSL ES requires and desktop GLSL rejects.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *kVertexShader =
	"attribute vec3 aPos;\n"
	"attribute vec4 aColor;\n"
	"attribute vec2 aTex;\n"
	"uniform mat4 uProj;\n"
	"varying vec4 vColor;\n"
	"varying vec2 vTex;\n"
	"void main() {\n"
	"    vColor = aColor;\n"
	"    vTex   = aTex;\n"
	"    gl_Position = uProj * vec4(aPos, 1.0);\n"
	"}\n";

static const char *kFragmentShader =
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"#endif\n"
	"varying vec4 vColor;\n"
	"varying vec2 vTex;\n"
	"uniform sampler2D uTex;\n"
	"uniform float uTextured;\n"
	"void main() {\n"
	"    if (uTextured > 0.5) {\n"
	"        vec4 t = texture2D(uTex, vTex);\n"
	/*       GL_DECAL for an RGBA texture:  C = Cf*(1-As) + Cs*As ,  A = Af                     */
	/*       Eight of the game's textures are 3-channel sources expanded with alpha = 255, for   */
	/*       which this correctly degrades to a straight replace.                                */
	"        gl_FragColor = vec4(mix(vColor.rgb, t.rgb, t.a), vColor.a);\n"
	"    } else {\n"
	"        gl_FragColor = vColor;\n"
	"    }\n"
	"}\n";

static GLuint compileShader(GLenum type, const char *src)
{
	GLuint sh = glCreateShader(type);
	GLint  ok = 0;
	glShaderSource(sh, 1, &src, 0);
	glCompileShader(sh);
	glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[2048];
		GLsizei n = 0;
		glGetShaderInfoLog(sh, (GLsizei)sizeof(log), &n, log);
		fprintf(stderr, "gl1: %s shader failed to compile:\n%s\n",
		        type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
		return 0;
	}
	return sh;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	4) Init
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ensureInit(void)
{
	if (gInited) return;
	gInited = true;

	gPrim  = new std::vector<Vertex>();
	gBatch = new std::vector<Vertex>();
	gStack = new std::vector<MatBox>();
	gPrim->reserve(4096);
	gBatch->reserve(GL1_MAX_BATCH_VERTS);

	matIdentity(gModelview);
	matIdentity(gProjection);

	GLuint vs = compileShader(GL_VERTEX_SHADER,   kVertexShader);
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);
	if (!vs || !fs) { fprintf(stderr, "gl1: fatal -- shader compilation failed\n"); return; }

	gProgram = glCreateProgram();
	glAttachShader(gProgram, vs);
	glAttachShader(gProgram, fs);
	glLinkProgram(gProgram);

	GLint linked = 0;
	glGetProgramiv(gProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		char log[2048];
		GLsizei n = 0;
		glGetProgramInfoLog(gProgram, (GLsizei)sizeof(log), &n, log);
		fprintf(stderr, "gl1: program failed to link:\n%s\n", log);
		return;
	}
	glDeleteShader(vs);
	glDeleteShader(fs);

	gAttrPos     = glGetAttribLocation(gProgram,  "aPos");
	gAttrCol     = glGetAttribLocation(gProgram,  "aColor");
	gAttrTex     = glGetAttribLocation(gProgram,  "aTex");
	gLocProj     = glGetUniformLocation(gProgram, "uProj");
	gLocTex      = glGetUniformLocation(gProgram, "uTex");
	gLocTextured = glGetUniformLocation(gProgram, "uTextured");

	glGenBuffers(1, &gVBO);

	glUseProgram(gProgram);
	glUniform1i(gLocTex, 0);
	glActiveTexture(GL_TEXTURE0);
}

void gl1_Init(void) { ensureInit(); }

void gl1_Shutdown(void)
{
	if (!gInited) return;
	if (gVBO)     glDeleteBuffers(1, &gVBO);
	if (gProgram) glDeleteProgram(gProgram);
	delete gPrim;  gPrim  = 0;
	delete gBatch; gBatch = 0;
	delete gStack; gStack = 0;
	gInited = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	5) The flush -- turn the accumulated triangles into one draw call
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void flush(void)
{
	if (!gInited || !gBatch || gBatch->empty() || !gProgram) return;

	GLfloat proj[16];
	int i;
	for (i = 0; i < 16; i++) proj[i] = (GLfloat)gProjection[i];

	glUseProgram(gProgram);
	glUniformMatrix4fv(gLocProj, 1, GL_FALSE, proj);
	glUniform1f(gLocTextured, gTextured ? 1.0f : 0.0f);

	if (gTextured)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBoundTex);
		glUniform1i(gLocTex, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gVBO);
	glBufferData(GL_ARRAY_BUFFER,
	             (GLsizeiptr)(gBatch->size() * sizeof(Vertex)),
	             &(*gBatch)[0], GL_STREAM_DRAW);

	const GLsizei stride = (GLsizei)sizeof(Vertex);
	if (gAttrPos >= 0)
	{
		glEnableVertexAttribArray((GLuint)gAttrPos);
		glVertexAttribPointer((GLuint)gAttrPos, 3, GL_FLOAT, GL_FALSE, stride, (const void *)0);
	}
	if (gAttrCol >= 0)
	{
		glEnableVertexAttribArray((GLuint)gAttrCol);
		glVertexAttribPointer((GLuint)gAttrCol, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(3 * sizeof(GLfloat)));
	}
	if (gAttrTex >= 0)
	{
		glEnableVertexAttribArray((GLuint)gAttrTex);
		glVertexAttribPointer((GLuint)gAttrTex, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(7 * sizeof(GLfloat)));
	}

	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)gBatch->size());

	gDrawCalls++;
	gVertsDrawn += (long)gBatch->size();
	gBatch->clear();
}

void gl1_Flush(void) { flush(); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	6) Texture coordinate generation
////
////	The game emits ZERO glTexCoord2f calls -- every texture coordinate in the entire game comes
////	from glTexGen. It also never calls glTexGenfv to set a custom plane, so every plane is the
////	OpenGL default, and the three modes collapse to the simple forms below.
////
////	  GL_OBJECT_LINEAR : default S plane (1,0,0,0), T plane (0,1,0,0)  ->  s = x_obj, t = y_obj
////	  GL_EYE_LINEAR    : same planes, applied in eye space            ->  s = x_eye, t = y_eye
////	  GL_SPHERE_MAP    : the standard OpenGL reflection formula
////
////	Note the game never calls glNormal3f either, so for all game geometry the current normal is
////	GL's default (0,0,1). That is not a bug to fix -- it is the behaviour to REPRODUCE, or the
////	boss's chrome comes out looking different. gl1_SolidSphere does emit real normals.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void computeTexCoord(double ox, double oy, double oz,
                            double ex, double ey, double ez,
                            GLfloat *outS, GLfloat *outT)
{
	double sphereS = 0.0, sphereT = 0.0;
	bool   needSphere = (gTexGenSMode == GL_SPHERE_MAP) || (gTexGenTMode == GL_SPHERE_MAP);

	(void)oz;

	if (needSphere)
	{
		double nx, ny, nz, len;
		double ux, uy, uz;
		double rx, ry, rz, ndotu, m;

		/* eye-space normal */
		matXformDir(gModelview, gCurNx, gCurNy, gCurNz, &nx, &ny, &nz);
		len = sqrt(nx*nx + ny*ny + nz*nz);
		if (len > 1e-12) { nx /= len; ny /= len; nz /= len; }

		/* unit vector from eye to vertex */
		len = sqrt(ex*ex + ey*ey + ez*ez);
		if (len > 1e-12) { ux = ex/len; uy = ey/len; uz = ez/len; }
		else             { ux = 0; uy = 0; uz = -1; }

		ndotu = nx*ux + ny*uy + nz*uz;
		rx = ux - 2.0*nx*ndotu;
		ry = uy - 2.0*ny*ndotu;
		rz = uz - 2.0*nz*ndotu;

		m = 2.0 * sqrt(rx*rx + ry*ry + (rz + 1.0)*(rz + 1.0));
		if (m < 1e-12) m = 1e-12;

		sphereS = rx/m + 0.5;
		sphereT = ry/m + 0.5;
	}

	switch (gTexGenSMode)
	{
		case GL_OBJECT_LINEAR: *outS = (GLfloat)ox;      break;
		case GL_EYE_LINEAR:    *outS = (GLfloat)ex;      break;
		case GL_SPHERE_MAP:    *outS = (GLfloat)sphereS; break;
		default:               *outS = 0.0f;             break;
	}
	switch (gTexGenTMode)
	{
		case GL_OBJECT_LINEAR: *outT = (GLfloat)oy;      break;
		case GL_EYE_LINEAR:    *outT = (GLfloat)ey;      break;
		case GL_SPHERE_MAP:    *outT = (GLfloat)sphereT; break;
		default:               *outT = 0.0f;             break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	7) Immediate mode
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void gl1_Begin(GLenum mode)
{
	ensureInit();
	gPrimMode = mode;
	gInBegin  = true;
	gBeginCount++;
	gPrim->clear();
}

void gl1_Color3f(GLfloat r, GLfloat g, GLfloat b)
{
	gCurR = r; gCurG = g; gCurB = b; gCurA = 1.0f;
}

void gl1_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	gCurR = r; gCurG = g; gCurB = b; gCurA = a;
}

void gl1_Normal3f(GLfloat x, GLfloat y, GLfloat z)
{
	gCurNx = x; gCurNy = y; gCurNz = z;
}

void gl1_Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	ensureInit();
	if (!gInBegin) return;

	double ex, ey, ez;
	Vertex v;

	/* the central move: transform to eye space here, on the CPU */
	matXformPoint(gModelview, x, y, z, &ex, &ey, &ez);

	v.x = (GLfloat)ex; v.y = (GLfloat)ey; v.z = (GLfloat)ez;
	v.r = gCurR; v.g = gCurG; v.b = gCurB; v.a = gCurA;

	if (gTexGenSOn || gTexGenTOn) computeTexCoord(x, y, z, ex, ey, ez, &v.s, &v.t);
	else                          { v.s = 0.0f; v.t = 0.0f; }

	gPrim->push_back(v);
}

/* append one triangle to the batch */
static inline void pushTri(const Vertex &a, const Vertex &b, const Vertex &c)
{
	gBatch->push_back(a);
	gBatch->push_back(b);
	gBatch->push_back(c);
}

void gl1_End(void)
{
	if (!gInBegin) return;
	gInBegin = false;

	const std::vector<Vertex> &p = *gPrim;
	const size_t n = p.size();
	size_t i;

	/* Decompose to independent triangles so that consecutive primitives can share one draw call.
	   Winding is irrelevant: the game never enables GL_CULL_FACE. */
	switch (gPrimMode)
	{
		case GL_QUAD_STRIP:          /* identical vertex ordering to GL_TRIANGLE_STRIP */
		case GL_TRIANGLE_STRIP:
			for (i = 0; i + 2 < n; i++) pushTri(p[i], p[i+1], p[i+2]);
			break;

		case GL_POLYGON:             /* the game's polygons are simple convex shapes */
		case GL_TRIANGLE_FAN:
			for (i = 1; i + 1 < n; i++) pushTri(p[0], p[i], p[i+1]);
			break;

		case GL_TRIANGLES:
			for (i = 0; i + 2 < n; i += 3) pushTri(p[i], p[i+1], p[i+2]);
			break;

		case GL_QUADS:
			for (i = 0; i + 3 < n; i += 4)
			{
				pushTri(p[i], p[i+1], p[i+2]);
				pushTri(p[i], p[i+2], p[i+3]);
			}
			break;

		default:
			break;
	}

	if (gBatch->size() >= GL1_MAX_BATCH_VERTS) flush();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	8) Matrices
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Mat *current(void)
{
	return (gMatrixMode == GL_PROJECTION) ? &gProjection : &gModelview;
}

/* the projection matrix is a uniform, so changing it has to break the batch */
static void touchProjection(void)
{
	if (gMatrixMode == GL_PROJECTION) flush();
}

void gl1_MatrixMode(GLenum mode) { ensureInit(); gMatrixMode = mode; }

void gl1_LoadIdentity(void)      { ensureInit(); touchProjection(); matIdentity(*current()); }

void gl1_PushMatrix(void)
{
	ensureInit();
	MatBox b;
	matCopy(b.m, *current());
	gStack->push_back(b);
}

void gl1_PopMatrix(void)
{
	ensureInit();
	if (gStack->empty()) return;
	touchProjection();
	matCopy(*current(), gStack->back().m);
	gStack->pop_back();
}

void gl1_Translated(GLdouble x, GLdouble y, GLdouble z)
{
	ensureInit();
	touchProjection();
	Mat t; matIdentity(t);
	t[12] = x; t[13] = y; t[14] = z;
	matMul(*current(), *current(), t);
}

void gl1_Translatef(GLfloat x, GLfloat y, GLfloat z)
{
	gl1_Translated((GLdouble)x, (GLdouble)y, (GLdouble)z);
}

void gl1_Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	ensureInit();
	touchProjection();

	double len = sqrt(x*x + y*y + z*z);
	if (len < 1e-12) return;
	x /= len; y /= len; z /= len;

	const double rad = angle * M_PI / 180.0;
	const double c = cos(rad), s = sin(rad), omc = 1.0 - c;

	Mat r; matIdentity(r);
	r[0]  = x*x*omc + c;     r[4]  = x*y*omc - z*s;  r[8]  = x*z*omc + y*s;
	r[1]  = y*x*omc + z*s;   r[5]  = y*y*omc + c;    r[9]  = y*z*omc - x*s;
	r[2]  = z*x*omc - y*s;   r[6]  = z*y*omc + x*s;  r[10] = z*z*omc + c;

	matMul(*current(), *current(), r);
}

void gl1_Rotatef(GLfloat a, GLfloat x, GLfloat y, GLfloat z)
{
	gl1_Rotated((GLdouble)a, (GLdouble)x, (GLdouble)y, (GLdouble)z);
}

void gl1_Scaled(GLdouble x, GLdouble y, GLdouble z)
{
	ensureInit();
	touchProjection();
	Mat s; matIdentity(s);
	s[0] = x; s[5] = y; s[10] = z;
	matMul(*current(), *current(), s);
}

void gl1_uPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	ensureInit();
	touchProjection();

	const double f = 1.0 / tan(fovy * M_PI / 360.0);
	Mat p;
	int i;
	for (i = 0; i < 16; i++) p[i] = 0.0;

	p[0]  = f / aspect;
	p[5]  = f;
	p[10] = (zFar + zNear) / (zNear - zFar);
	p[11] = -1.0;
	p[14] = (2.0 * zFar * zNear) / (zNear - zFar);

	if (getenv("GL1_DEBUG"))
		fprintf(stderr, "gl1: uPerspective fovy=%g aspect=%g near=%g far=%g -> p0=%g p5=%g mode=%s\n",
		        fovy, aspect, zNear, zFar, p[0], p[5],
		        gMatrixMode == GL_PROJECTION ? "PROJECTION" : "MODELVIEW");

	matMul(*current(), *current(), p);
}

void gl1_uLookAt(GLdouble ex, GLdouble ey, GLdouble ez,
                 GLdouble cx, GLdouble cy, GLdouble cz,
                 GLdouble ux, GLdouble uy, GLdouble uz)
{
	ensureInit();
	touchProjection();

	double fx = cx - ex, fy = cy - ey, fz = cz - ez;
	double len = sqrt(fx*fx + fy*fy + fz*fz);
	if (len < 1e-12) return;
	fx /= len; fy /= len; fz /= len;

	/* s = f x up */
	double sx = fy*uz - fz*uy;
	double sy = fz*ux - fx*uz;
	double sz = fx*uy - fy*ux;
	len = sqrt(sx*sx + sy*sy + sz*sz);
	if (len < 1e-12) return;
	sx /= len; sy /= len; sz /= len;

	/* u = s x f */
	const double upx = sy*fz - sz*fy;
	const double upy = sz*fx - sx*fz;
	const double upz = sx*fy - sy*fx;

	Mat l; matIdentity(l);
	l[0] = sx;   l[4] = sy;   l[8]  = sz;
	l[1] = upx;  l[5] = upy;  l[9]  = upz;
	l[2] = -fx;  l[6] = -fy;  l[10] = -fz;

	matMul(*current(), *current(), l);

	Mat t; matIdentity(t);
	t[12] = -ex; t[13] = -ey; t[14] = -ez;
	matMul(*current(), *current(), t);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	9) State
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void gl1_Enable(GLenum cap)
{
	ensureInit();
	switch (cap)
	{
		case GL_TEXTURE_2D:     if (!gTextured) { flush(); gTextured = true; }  break;
		case GL_TEXTURE_GEN_S:  gTexGenSOn = true;  break;
		case GL_TEXTURE_GEN_T:  gTexGenTOn = true;  break;
		case GL_DEPTH_TEST:     glEnable(GL_DEPTH_TEST); break;
		case GL_BLEND:          glEnable(GL_BLEND); break;
		case GL_CULL_FACE:      glEnable(GL_CULL_FACE); break;
		default:                /* GL_SHADE_MODEL and friends: not a real capability, ignore */ break;
	}
}

void gl1_Disable(GLenum cap)
{
	ensureInit();
	switch (cap)
	{
		case GL_TEXTURE_2D:     if (gTextured) { flush(); gTextured = false; }  break;
		case GL_TEXTURE_GEN_S:  gTexGenSOn = false; break;
		case GL_TEXTURE_GEN_T:  gTexGenTOn = false; break;
		case GL_DEPTH_TEST:     glDisable(GL_DEPTH_TEST); break;
		case GL_BLEND:          glDisable(GL_BLEND); break;
		case GL_CULL_FACE:      glDisable(GL_CULL_FACE); break;
		default:                break;
	}
}

void gl1_Clear(GLbitfield mask)
{
	ensureInit();
	flush();
	glClear(mask);
	gDrawCalls = 0; gVertsDrawn = 0; gBeginCount = 0;
}

void gl1_ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { glClearColor(r, g, b, a); }
void gl1_Viewport(GLint x, GLint y, GLsizei w, GLsizei h)       { ensureInit(); flush(); glViewport(x, y, w, h); }
void gl1_DepthFunc(GLenum f)                                    { glDepthFunc(f); }
void gl1_ShadeModel(GLenum m)                                   { (void)m; /* always smooth */ }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	10) Texturing
////
////	The pointer cache is the whole trick here. The 2007 code calls glTexImage2D 15 times a frame
////	to switch between 16 images that never change -- ~3.9 MB of upload per frame (FINDINGS #11).
////	Because it always passes the same 16 pixel pointers, caching on the pointer turns every call
////	after the first into a glBindTexture. No game code changes.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TexCacheEntry { const void *pixels; GLuint id; };

#define GL1_TEX_CACHE_MAX 64
static TexCacheEntry gTexCache[GL1_TEX_CACHE_MAX];
static int           gTexCacheCount = 0;

void gl1_TexImage2D(GLenum target, GLint level, GLint internalFormat,
                    GLsizei width, GLsizei height, GLint border,
                    GLenum format, GLenum type, const void *pixels)
{
	ensureInit();
	(void)internalFormat; (void)border;

	if (target != GL_TEXTURE_2D || level != 0 || pixels == 0) return;

	int i;
	for (i = 0; i < gTexCacheCount; i++)
	{
		if (gTexCache[i].pixels == pixels)
		{
			if (gBoundTex != gTexCache[i].id) { flush(); gBoundTex = gTexCache[i].id; }
			return;
		}
	}

	if (gTexCacheCount >= GL1_TEX_CACHE_MAX)
	{
		fprintf(stderr, "gl1: texture cache full (%d entries)\n", GL1_TEX_CACHE_MAX);
		return;
	}

	GLuint id = 0;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	/* GLES2 requires internalFormat == format; the game passes the legacy component count 4 */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, type, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gMinFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gMagFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     gWrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     gWrapT);

	gTexCache[gTexCacheCount].pixels = pixels;
	gTexCache[gTexCacheCount].id     = id;
	gTexCacheCount++;

	flush();
	gBoundTex = id;
}

void gl1_TexParameteri(GLenum target, GLenum pname, GLint param)
{
	(void)target;
	/* Recorded and applied at texture-creation time: in GLES2 these are per-object, but the game
	   sets them once at startup before any texture exists. */
	switch (pname)
	{
		case GL_TEXTURE_MIN_FILTER: gMinFilter = param; break;
		case GL_TEXTURE_MAG_FILTER: gMagFilter = param; break;
		case GL_TEXTURE_WRAP_S:     gWrapS     = param; break;
		case GL_TEXTURE_WRAP_T:     gWrapT     = param; break;
		default: break;
	}
}

void gl1_TexEnvi(GLenum target, GLenum pname, GLint param)
{
	/* The game sets GL_DECAL once and never changes it; the fragment shader implements it. */
	(void)target; (void)pname; (void)param;
}

void gl1_TexGeni(GLenum coord, GLenum pname, GLint param)
{
	ensureInit();
	if (pname != GL_TEXTURE_GEN_MODE) return;

	GLenum mode = (GLenum)param;
	if      (coord == GL_S) { if (gTexGenSMode != mode) { flush(); gTexGenSMode = mode; } }
	else if (coord == GL_T) { if (gTexGenTMode != mode) { flush(); gTexGenTMode = mode; } }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	11) glutSolidSphere
////
////	Emscripten's GLUT has no solid-geometry helpers. This matches GLUT's own orientation
////	(poles on the Z axis) and emits normals, which GL_SPHERE_MAP texgen depends on.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void gl1_SolidSphere(GLdouble radius, GLint slices, GLint stacks)
{
	ensureInit();
	if (slices < 3) slices = 3;
	if (stacks < 2) stacks = 2;

	int i, j;
	for (i = 0; i < stacks; i++)
	{
		const double rho0 = M_PI * (double)i       / (double)stacks;
		const double rho1 = M_PI * (double)(i + 1) / (double)stacks;

		gl1_Begin(GL_TRIANGLE_STRIP);
		for (j = 0; j <= slices; j++)
		{
			const double theta = 2.0 * M_PI * (double)j / (double)slices;
			const double ct = cos(theta), st = sin(theta);

			double nx = sin(rho0) * ct, ny = sin(rho0) * st, nz = cos(rho0);
			gl1_Normal3f((GLfloat)nx, (GLfloat)ny, (GLfloat)nz);
			gl1_Vertex3f((GLfloat)(radius*nx), (GLfloat)(radius*ny), (GLfloat)(radius*nz));

			nx = sin(rho1) * ct; ny = sin(rho1) * st; nz = cos(rho1);
			gl1_Normal3f((GLfloat)nx, (GLfloat)ny, (GLfloat)nz);
			gl1_Vertex3f((GLfloat)(radius*nx), (GLfloat)(radius*ny), (GLfloat)(radius*nz));
		}
		gl1_End();
	}

	/* leave the current normal where the fixed pipeline's default would be */
	gl1_Normal3f(0.0f, 0.0f, 1.0f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	12) Frame end
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void gl1_SwapBuffers(void)
{
	flush();
	glutSwapBuffers();
}

void gl1_ReportStats(void)
{
	fprintf(stderr, "gl1: %ld draw calls, %ld verts, %ld glBegin blocks\n",
	        gDrawCalls, gVertsDrawn, gBeginCount);
}
