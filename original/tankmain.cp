//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			main.cp																															
////	Author:			Ed Martin
////	Description:	tank sample game main function																
////
////	Contents:		
////						1) General include files (math, stdio, stdlib, GameConstants.cp)
////						2) Event Handler
////						3) Game include files and game object constructors
////						4) Initialize OpenGL
////						5) main()
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////						1) General include files (math, stdio, stdlib, GameConstants.cp)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "GameConstants.cp"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////						2) Event Handler
////		
////						Event() is called for any inter object communication. Event() stores the event parameters in 
////						the array events[]. events[] are checked on every frame and processed with CheckEvents().
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef EVENTS
	#define EVENTS	
	int x;

	float events[10 * MAX_EVENTS];
    void Event(int description, float a, float b, float c, float d, float e, float f, float g, float h, float i)
	{

		for (x = 0; x<MAX_EVENTS; x++) if (events[x*10] ==0) 
		{
			events[x*10]   = description;
			events[x*10+1] = a;
			events[x*10+2] = b;
			events[x*10+3] = c;
			events[x*10+4] = d;
			events[x*10+5] = e;
			events[x*10+6] = f;
			events[x*10+7] = g;
			events[x*10+8] = h;
			events[x*10+9] = i;
			x = MAX_EVENTS; 
		}
	}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////						3) Game include files and game object constructors
////					
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GL
	#define GL
	#include <GLUT/glut.h>
	#include "GLConstants.cp"
	#include "GLGameObjectDrawFunctions.cp"
	#include "GLTextures.cp"
#endif

#ifndef OBJ	
	#define OBJ	
	#include "TankClass.cp"
	#include "TurretClass.cp"
	#include "TubeClass.cp"
	#include "BulletClass.cp"
	
	// Note: I undestand that it is generally bad practice to use global variables, unfortunately I haven't figured out how to get 
	// OpenGL's display function to accept arguments, and so until I do so, the following, with the game objects, must be global.
	// Also, as you can see from below, the use of pointers for the objects are currently under construction

	TankClass	Player1(0,0);
	TurretClass Turret[MAX_TURRETS];
	TubeClass	Tube[2];
	BulletClass Bullet[MAX_BULLETS];

	//TankClass	*Player1;
	//TurretClass *Turret[MAX_TURRETS];
	//TubeClass	*Tube[4];
	//BulletClass *Bullet[MAX_BULLETS];

#endif

#ifndef CONTROL
	#define CONTROL	
	#include "Control.cp"	
#endif

#ifndef GLGENERAL
	#define GLGENERAL
	#include "GLGeneral.cp"
#endif

#include "MainGameRoutines.cp"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////						4) Initialize OpenGL
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OpenGLInit(int argc, char* argv[])
{
	glutInit(&argc, argv);

	// Setup Window
	glutInitWindowPosition(0, 0); 
	glutInitWindowSize(1450,820);//(1450, 850);								// Large
	//glutInitWindowSize(int(480*1.618), 480);//(1450, 850);				// For Screenshots / Videos
	glutCreateWindow("tank");

	// Setup OpenGL general parameters
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glEnable(GL_SHADE_MODEL);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Setup redraw and reshape functions
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);

	// Setup the Keyboard / Mouse Handlers (found in Control.cp)
	glutKeyboardFunc(InGameKeystrokeDown);
	glutKeyboardUpFunc(InGameKeystrokeUp);
	glutPassiveMotionFunc(InGameMouseMove);
	glutMouseFunc(InGameMouseButtons);

	// Setup the Textures
	GLsizei sphereTexW, sphereTexH, padSphereTexW, padSphereTexH;
	int sphereTexComp;

	create_textures(&sphereTexW,	&sphereTexH,	&padSphereTexW,		&padSphereTexH,		&sphereTexComp);								

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	glEnable(GL_TEXTURE_2D);  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					5) main()
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{

	RunMode = STARTSCREEN;
	
	//TankClass Tank(0,0);
	//Player1 = &Tank;
	//Player1->ReStart(MAP_BLOCK_SIZE * 57, MAP_BLOCK_SIZE *  104.5);

	//TurretClass TurretData[MAX_TURRETS+2];
	//for (i = 0; i < MAX_TURRETS; i++) Turret[i] = &TurretData[i];
	
	//TubeClass	TubeData[2+1];
	//for (i = 0; i < 2+1; i++) Tube[i] = &TubeData[i];
	
	//BulletClass BulletData[MAX_BULLETS+2];
	//for (i=0; i < MAX_BULLETS; i++) Bullet[i] = &BulletData[i];
	
	OpenGLInit(argc, argv);
	GameInit();
	
	glutTimerFunc( ( 1000/FRAME_RATE ) , MainGameLoop, 1);
	glutMainLoop();
    
	return 0;
}
