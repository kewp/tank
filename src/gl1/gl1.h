//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////	File:			gl1/gl1.h
////	Author:			(added 2026 -- not part of Ed Martin's 2007 source)
////	Description:	A fixed-function OpenGL 1.x compatibility shim implemented on top of the
////					GLES2 / WebGL subset, so the 2007 game can run unchanged in a browser.
////
////					The game's calls are redirected here by macro at the bottom of this header,
////					which means NOT ONE LINE of game code has to change. Include this instead of
////					<GL/glut.h> and glBegin/glVertex3f/glRotated/glTexGeni all quietly become
////					shim calls.
////
////	What the game actually needs (from a full survey of the source -- see docs/ARCHITECTURE.md):
////
////					- immediate mode:  glBegin/glEnd/glVertex3f/glColor3f, GL_QUAD_STRIP + GL_POLYGON
////					- matrices:        glMatrixMode/glLoadIdentity/glTranslate/glRotated, no push/pop
////					- glu:             gluLookAt, gluPerspective
////					- texturing:       glTexImage2D, glTexEnvi(GL_DECAL), glTexGeni x3 modes
////					- geometry:        glutSolidSphere
////					- no lighting at all, which is why this is only ~700 lines
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TANK_GL1_H
#define TANK_GL1_H

//// ---- the real GL / GLUT headers, included BEFORE our macros exist ----------------------------
//// Order matters: if the macros below were already defined, they would rewrite the declarations
//// inside these headers and nothing would compile.

#if defined(__EMSCRIPTEN__)
	#include <GLES2/gl2.h>
	#include <GL/glut.h>
#elif defined(__APPLE__)
	#include <OpenGL/gl.h>
	#include <GLUT/glut.h>
#else
	#include <GL/gl.h>
	#include <GL/glut.h>
#endif

//// ---- legacy enums ----------------------------------------------------------------------------
//// GLES2 / WebGL deleted every one of these. The shim still needs them as its own vocabulary --
//// the game says GL_QUAD_STRIP and GL_SPHERE_MAP, and we translate. Values are the historical
//// OpenGL ones, so a desktop build (where the real headers define them) agrees exactly.

#ifndef GL_QUADS
	#define GL_QUADS              0x0007
#endif
#ifndef GL_QUAD_STRIP
	#define GL_QUAD_STRIP         0x0008
#endif
#ifndef GL_POLYGON
	#define GL_POLYGON            0x0009
#endif
#ifndef GL_MODELVIEW
	#define GL_MODELVIEW          0x1700
#endif
#ifndef GL_PROJECTION
	#define GL_PROJECTION         0x1701
#endif
#ifndef GL_S
	#define GL_S                  0x2000
#endif
#ifndef GL_T
	#define GL_T                  0x2001
#endif
#ifndef GL_TEXTURE_GEN_S
	#define GL_TEXTURE_GEN_S      0x0C60
#endif
#ifndef GL_TEXTURE_GEN_T
	#define GL_TEXTURE_GEN_T      0x0C61
#endif
#ifndef GL_TEXTURE_GEN_MODE
	#define GL_TEXTURE_GEN_MODE   0x2500
#endif
#ifndef GL_EYE_LINEAR
	#define GL_EYE_LINEAR         0x2400
#endif
#ifndef GL_OBJECT_LINEAR
	#define GL_OBJECT_LINEAR      0x2401
#endif
#ifndef GL_SPHERE_MAP
	#define GL_SPHERE_MAP         0x2402
#endif
#ifndef GL_TEXTURE_ENV
	#define GL_TEXTURE_ENV        0x2300
#endif
#ifndef GL_TEXTURE_ENV_MODE
	#define GL_TEXTURE_ENV_MODE   0x2200
#endif
#ifndef GL_DECAL
	#define GL_DECAL              0x2101
#endif
#ifndef GL_SHADE_MODEL
	#define GL_SHADE_MODEL        0x0B54
#endif
#ifndef GL_SMOOTH
	#define GL_SMOOTH             0x1D01
#endif
#ifndef GL_FLAT
	#define GL_FLAT               0x1D00
#endif

//// GLES2 has no GLdouble. The game uses glRotated/glTranslated throughout.
#if defined(__EMSCRIPTEN__)
	typedef double GLdouble;
#endif

#ifdef __cplusplus
extern "C" {
#endif

//// ---- lifecycle ------------------------------------------------------------------------------

void gl1_Init(void);        /* call once, after a GL context exists */
void gl1_Shutdown(void);
void gl1_Flush(void);       /* force the pending batch out; called automatically where needed */
void gl1_SwapBuffers(void); /* flush, then hand the frame to GLUT */
void gl1_ReportStats(void); /* draw calls / vertices for the last frame, to stderr */

//// ---- immediate mode -------------------------------------------------------------------------

void gl1_Begin(GLenum mode);
void gl1_End(void);
void gl1_Vertex3f(GLfloat x, GLfloat y, GLfloat z);
void gl1_Color3f(GLfloat r, GLfloat g, GLfloat b);
void gl1_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void gl1_Normal3f(GLfloat x, GLfloat y, GLfloat z);

//// ---- matrix stack ---------------------------------------------------------------------------

void gl1_MatrixMode(GLenum mode);
void gl1_LoadIdentity(void);
void gl1_PushMatrix(void);
void gl1_PopMatrix(void);
void gl1_Translatef(GLfloat x, GLfloat y, GLfloat z);
void gl1_Translated(GLdouble x, GLdouble y, GLdouble z);
void gl1_Rotatef(GLfloat a, GLfloat x, GLfloat y, GLfloat z);
void gl1_Rotated(GLdouble a, GLdouble x, GLdouble y, GLdouble z);
void gl1_Scaled(GLdouble x, GLdouble y, GLdouble z);

void gl1_uLookAt(GLdouble ex, GLdouble ey, GLdouble ez,
                 GLdouble cx, GLdouble cy, GLdouble cz,
                 GLdouble ux, GLdouble uy, GLdouble uz);
void gl1_uPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);

//// ---- state ----------------------------------------------------------------------------------

void gl1_Enable(GLenum cap);
void gl1_Disable(GLenum cap);
void gl1_Clear(GLbitfield mask);
void gl1_ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void gl1_Viewport(GLint x, GLint y, GLsizei w, GLsizei h);
void gl1_DepthFunc(GLenum f);
void gl1_ShadeModel(GLenum m);

//// ---- texturing ------------------------------------------------------------------------------
////
//// gl1_TexImage2D is the interesting one. The 2007 game has NO texture objects -- it re-uploads a
//// full 256x256 RGBA image every time it wants to switch texture, 15 times a frame (~3.9 MB/frame,
//// see docs/FINDINGS.md #11). Browsers will not tolerate that.
////
//// Rather than edit 15 call sites, this shim caches by pixel POINTER: the game hands back the same
//// 16 pointers forever, so the first call for a pointer uploads and creates a real texture object,
//// and every later call is just a glBindTexture. The upload problem disappears with zero changes to
//// game code.

void gl1_TexImage2D(GLenum target, GLint level, GLint internalFormat,
                    GLsizei width, GLsizei height, GLint border,
                    GLenum format, GLenum type, const void *pixels);
void gl1_TexParameteri(GLenum target, GLenum pname, GLint param);
void gl1_TexEnvi(GLenum target, GLenum pname, GLint param);
void gl1_TexGeni(GLenum coord, GLenum pname, GLint param);

//// ---- glut geometry --------------------------------------------------------------------------
//// Emscripten's GLUT has no solid-geometry helpers, and we need one that emits normals anyway
//// because GL_SPHERE_MAP texgen depends on them.

void gl1_SolidSphere(GLdouble radius, GLint slices, GLint stacks);

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	The interception layer.
////	gl1.cpp defines GL1_IMPLEMENTATION first, so it sees the real GL functions, not these.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GL1_IMPLEMENTATION

#define glBegin            gl1_Begin
#define glEnd              gl1_End
#define glVertex3f         gl1_Vertex3f
#define glColor3f          gl1_Color3f
#define glColor4f          gl1_Color4f
#define glNormal3f         gl1_Normal3f

#define glMatrixMode       gl1_MatrixMode
#define glLoadIdentity     gl1_LoadIdentity
#define glPushMatrix       gl1_PushMatrix
#define glPopMatrix        gl1_PopMatrix
#define glTranslatef       gl1_Translatef
#define glTranslated       gl1_Translated
#define glRotatef          gl1_Rotatef
#define glRotated          gl1_Rotated
#define glScaled           gl1_Scaled

#define gluLookAt          gl1_uLookAt
#define gluPerspective     gl1_uPerspective

#define glEnable           gl1_Enable
#define glDisable          gl1_Disable
#define glClear            gl1_Clear
#define glClearColor       gl1_ClearColor
#define glViewport         gl1_Viewport
#define glDepthFunc        gl1_DepthFunc
#define glShadeModel       gl1_ShadeModel

#define glTexImage2D       gl1_TexImage2D
#define glTexParameteri    gl1_TexParameteri
#define glTexEnvi          gl1_TexEnvi
#define glTexGeni          gl1_TexGeni

#define glutSolidSphere    gl1_SolidSphere
#define glutSwapBuffers    gl1_SwapBuffers

#endif // GL1_IMPLEMENTATION

#endif // TANK_GL1_H
