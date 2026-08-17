//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////	File:			Platform.h
////	Author:			(added 2026 during the revival port -- not part of Ed Martin's 2007 source)
////	Description:	Single place where every platform-specific include and quirk is handled.
////					The 2007 source hard-coded <GLUT/glut.h> and <Carbon/Carbon.h> in 13 files;
////					all of those now include this header instead.
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TANK_PLATFORM_H
#define TANK_PLATFORM_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//// ---- GLUT ------------------------------------------------------------------------------------------------------------------------------
//// macOS puts GLUT in a framework (<GLUT/glut.h>); everybody else uses <GL/glut.h>.
//// Emscripten ships its own GLUT implementation that maps onto the browser canvas, also at <GL/glut.h>.

//// TANK_USE_GL1SHIM routes every fixed-function GL call through src/gl1/, which reimplements the
//// ~30 GL 1.x entry points this game uses on top of GLES2/WebGL. Required for the browser build;
//// also usable natively, where it is the reference the WASM build is diffed against.
//// Without it, the original path is used unchanged: real GLUT and the real fixed-function pipeline.

#if defined(TANK_USE_GL1SHIM)
	#include "gl1/gl1.h"
#elif defined(__EMSCRIPTEN__)
	#include <GL/glut.h>
#elif defined(__APPLE__)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

//// ---- M_PI ------------------------------------------------------------------------------------------------------------------------------
//// M_PI is not in the C or C++ standard. glibc/libc++ expose it by default, MSVC does not.

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

//// ---- random() --------------------------------------------------------------------------------------------------------------------------
//// The game calls random() and divides by RAND_MAX. random() is POSIX and absent on MSVC.
//// Mapping it to rand() keeps the random()/RAND_MAX ratio correct on every platform
//// (on Windows RAND_MAX is 32767, so calling the real random() there would produce ratios far above 1.0).

#if defined(_WIN32) && !defined(__CYGWIN__)
	#define random() (rand())
#endif

//// ---- frame capture ---------------------------------------------------------------------------
//// [2026 port] A verification hook, not a game feature. Set TANK_SHOT=<path> to dump one frame
//// as a binary PPM and exit; TANK_SHOT_FRAME picks which frame (default 90).
////
//// This is how the shim build is checked against the original fixed-function build: run both with
//// the same frame number and compare the images pixel by pixel. Without it there is no objective
//// way to tell whether the port still renders the same scene.
//// A no-op unless TANK_SHOT is set.

#ifndef __EMSCRIPTEN__

static void TankMaybeScreenshot(void)
{
	const char *path = getenv("TANK_SHOT");
	if (!path) return;

	static int frame = 0;
	const char *fs = getenv("TANK_SHOT_FRAME");
	const int target = fs ? atoi(fs) : 90;

	if (++frame != target) return;

#ifdef TANK_USE_GL1SHIM
	gl1_Flush();          /* the shim batches; get it on to the framebuffer before reading back */
#endif

	GLint vp[4] = {0,0,0,0};
	glGetIntegerv(GL_VIEWPORT, vp);        /* real pixels, unlike glutGet on a retina display */

	const int w = vp[2], h = vp[3];
	if (w <= 0 || h <= 0) return;

	unsigned char *px = (unsigned char *)malloc((size_t)w * h * 3);
	if (!px) return;

	glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px);

	FILE *f = fopen(path, "wb");
	if (f)
	{
		int y;
		fprintf(f, "P6\n%d %d\n255\n", w, h);
		for (y = h - 1; y >= 0; y--) fwrite(px + (size_t)y * w * 3, 1, (size_t)w * 3, f);
		fclose(f);
		fprintf(stderr, "wrote %s (%dx%d, frame %d)\n", path, w, h, target);
	}
	free(px);
	exit(0);
}

#else
	#define TankMaybeScreenshot() ((void)0)
#endif

#endif // TANK_PLATFORM_H
