//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			GLGeneral.cp																															
////	Author:			Ed Martin
////	Description:	Holds the OpenGL Drawing, camera positioning and window resizing routines													
////	
////	Contents:		
////					1) Map Drawing Function
////					2) Camera Positioning Function
////					3) Window Reshape Function
////					4) Main OpenGL callback display function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// [2026 port] removed: #include <Carbon/Carbon.h>  -- macOS-only and unused by this code
#include "Platform.h"

#include "GameConstants.cp"

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

#ifndef GL
	#define GL
	#include "Platform.h"
	#include "GLConstants.cp"
	#include "GLGameObjectDrawFunctions.cp"
	#include "GLTextures.cp"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					1) Map Drawing Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void drawmap(int RunMode,
			 int Map[], TankClass Player1, TubeClass Tube[],
			 GLsizei TextureWidth, GLsizei TextureHeight,
			 unsigned int *start1, unsigned int *start2, unsigned int *start3, unsigned int *start4,
			 unsigned int *sky1, unsigned int *steel, unsigned int *green1, unsigned int *minitank1,
			 unsigned int *sand1, unsigned int *wall1, unsigned int *wall2, 
			 float camera_x, float camera_y, float camera_z, float ObjectSpin)
{
	int x,z;	
	float r =  float(random()) / float(RAND_MAX);
	
	// Draw the Start screen on a wall in the Map
	DrawStartScreenWallMounting(2.9*MAP_BLOCK_SIZE, 7, 4.2);

	// Draw the Sky
	DrawSky(camera_x, camera_y, camera_z);
			
	// Draw the Health Bar
	float health_ratio = float(Player1.GetHealth()) / PLAYER_MAX_HEALTH;
	float lives = Player1.GetLives();
	if ((RunMode!= STARTSCREEN) && (RunMode != TRANSITION) && (Tube[1].GetState() != TUBE_STATE_APROACHING))	
	  DrawHealthBar(camera_x, camera_y, camera_z, lives, health_ratio);
	
	// Setup the Ground Textures and Draw the Ground
	glEnable(GL_TEXTURE_2D);  
	glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, sand1);	
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	for (x=0; x<MAP_X; x++)
	 for (z=0; z<MAP_Z; z++)
	  if ((Map[x+ MAP_X*z] == GROUND) || 
	      (Map[x+ MAP_X*z] == RAPID_FIRE) ||	(Map[x+ MAP_X*z] == DUAL_FIRE) || 
		  (Map[x+ MAP_X*z] == RAIL) ||		(Map[x+ MAP_X*z] == RAILSLEEPER) ||
		  ((Map[x+ MAP_X*z] > 0) && (Map[x+ MAP_X*z] < 10)) )
	DrawGround(x,y,z);
	
	// Setup the Wall Textures and Draw the Walls
	glEnable(GL_TEXTURE_2D);  
	glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, wall2);	
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	for (x=0; x<MAP_X; x++)
	 for (z=0; z<MAP_Z; z++)
	  if (Map[x+ MAP_X*z] == WALL)
	DrawWall(x,y,z);
	
	// Setup the Tower Textures and Draw the Towers
	glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, wall1);	
	for (x=0; x<MAP_X; x++)
	 for (z=0; z<MAP_Z; z++)
	  if (Map[x+ MAP_X*z] == TOWER) 
	DrawTower(x,y,z);
	
	// Draw all other Map objects (ones without textures)
	glDisable(GL_TEXTURE_2D);
	for (x=0; x<MAP_X; x++)
	 for (z=0; z<MAP_Z; z++)
	switch( Map[x+ MAP_X*z] )
	{
		case ARCH:			DrawArch			(x, y, z, 10			);	break;
		case RAIL:			DrawRail			(x, y, z				);	break;
		case RAILSLEEPER:	DrawRailSleeper		(x, y, z				);	break;
		case RAPID_FIRE:	DrawRapidFire		(x, y, z, r, ObjectSpin);	break;
		case DUAL_FIRE:		DrawDualFire		(x, y, z, r, ObjectSpin);	break;
		case MED_PACK:		DrawMedPack			(x, y, z, r, ObjectSpin);	break;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Camera Positioning Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void positionCamera(TankClass Player1)
{
	float new_camera_x, new_camera_y, new_camera_z;
	
	// Camera is based on the direction the players tank turret is facing unless the boss has been destroyed
	
	if (RunMode != END)
	{
		CurrentCameraDirectionRightLeft = 180 - Player1.GetturretRightLeft();
		if (CurrentCameraDirectionRightLeft > 360 ) CurrentCameraDirectionRightLeft -= 360;
		if (CurrentCameraDirectionRightLeft < 0 )	CurrentCameraDirectionRightLeft += 360;
	}
	else CurrentCameraDirectionRightLeft ++;
	
	// the camera's distance from the player (CurrentCameraDistance) gets 1/80th closer to ObjectiveCameraDistance every frame
	// this makes the camera following smoother:'
	
	if (CurrentCameraDistance != ObjectiveCameraDistance) CurrentCameraDistance -= (CurrentCameraDistance - ObjectiveCameraDistance)/80;
	
	// Based on camera's distance from the player, and camera angle (set above), the line of sight and camera position is set up
	
	lineOfSight_x =		sin( (2*M_PI/360) * (CurrentCameraDirectionRightLeft) );
	lineOfSight_y =		-0.2;
	lineOfSight_z =		-cos( (2*M_PI/360) * (CurrentCameraDirectionRightLeft) );
	
	new_camera_x =			Player1.Getx() - CurrentCameraDistance * lineOfSight_x 
							+ MAX_CAMERA_SHAKE_DISTANCE * pow(CameraShakeTime / MAX_CAMERA_SHAKE_TIME,2) * (2 * float(random()) / float(RAND_MAX) - 1);
	new_camera_y =			CurrentCameraDistance *1							
							+ MAX_CAMERA_SHAKE_DISTANCE * pow(CameraShakeTime / MAX_CAMERA_SHAKE_TIME,2) * (2 * float(random()) / float(RAND_MAX) - 1);
	new_camera_z =			Player1.Getz() + CurrentCameraDistance * -lineOfSight_z							
							+ MAX_CAMERA_SHAKE_DISTANCE * pow(CameraShakeTime / MAX_CAMERA_SHAKE_TIME,2) * (2 * float(random()) / float(RAND_MAX) - 1);

	// again the current camera position gets a fraction closer to the old camera position every frame, again for smoothness
	
	camera_x += -(camera_x - new_camera_x) /1.9;
	camera_y += -(camera_y - new_camera_y) /1.9;
	camera_z += -(camera_z - new_camera_z) /1.9;
	
	// One of the debug modes is TOP_VIEW, in which the tank is viewed from above, otherwise the normal camera view is setup
	
	glLoadIdentity();
	if (RunMode == TOP_VIEW)
	{	
		gluLookAt(	Player1.Getx(),			MAP_BLOCK_SIZE*3,		Player1.Getz(),			 
					Player1.Getx(),			0,						Player1.Getz(),
					0,						0,						1);
	}
	else
	{	
		gluLookAt(	camera_x,				camera_y,				camera_z, 
					camera_x+lineOfSight_x, camera_y+lineOfSight_y, camera_z+lineOfSight_z,
					0,						1,						0);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					3) Window Reshape Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void reshape(int width, int height)
{    
	glViewport(0, 0, width, height);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// [2026 port] Was: gluPerspective(90, (GLfloat)height / (GLfloat)height, ...) -- height/height,
	// so the aspect ratio was hard-wired to 1.0 and the scene was horizontally squashed in any
	// non-square window (including the shipped 1450x820 default). Should be width/height.
	gluPerspective(90, (GLfloat)width / (GLfloat)(height ? height : 1), 2.0, 900.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) Main OpenGL callback display function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void display()
{
	int b,t;

	// [2026 port] see SuppressDisplay in GameConstants.cp -- lets the browser frame driver
	// advance the simulation several steps and draw only once.
	if (SuppressDisplay) return;

	// Clear the buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw the Entire Map (there's room for optimisation here, although arguably unneccessary in this scale)
	drawmap(RunMode,
			Map, Player1, Tube,
			TextureWidth, TextureHeight,
			start1, start2, start3, start4,
			sky1, steel1, green1, minitank1,
			sand1, wall1, wall2, 
			camera_x, camera_y, camera_z, ObjectSpin);

	// Draw the Player
	if (Player1.GetState() == ACTIVE) Player1.DrawTank();
	
	// Draw the bullets
	for (b = 0; b < MAX_BULLETS; b++) Bullet[b].DrawBullet();
	
	// Draw the Active turrets
	glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky2);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);		
	for (t = 0; t < MAX_TURRETS; t++) if (Turret[t].GetState()!=INACTIVE) Turret[t].DrawTurret();
	
	// Draw the Active tubes
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);		
	for (t = 0; t <= 1; t++) if (Tube[t].GetState()!=INACTIVE) Tube[t].DrawTube();

	// [2026 port] no-op unless TANK_SHOT is set; see Platform.h
	TankMaybeScreenshot();

	glutSwapBuffers();
}
