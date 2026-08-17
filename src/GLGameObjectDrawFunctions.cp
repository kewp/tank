//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			GLGameObjectDrwFunctions.cp																															
////	Author:			Ed Martin
////	Description:	Draw Functions for the graphic objects (mostly GL_QUAD_STRIPs)																
////
////	Contents:		
////					void DrawHealthBar					(float camera_x, float camera_y, float camera_z, float lives, float health_ratio)
////					void DrawSky						(float camera_x, float camera_y, float camera_z)
////					void DrawStartScreenWallMounting	(float x, float y, float z)
////					void DrawGround						(float x, float y, float z)
////					void DrawWall						(float x, float y, float z)
////					void DrawTower						(float x, float y, float z)
////					void DrawArch						(float x, float y, float z, float length)
////					void DrawRail						(float x, float y, float z)
////					void DrawRailSleeper				(float x, float y, float z)
////					void DrawRapidFire					(float x, float y, float z, float random, float ObjectSpin)
////					void DrawDualFire					(float x, float y, float z, float random, float ObjectSpin)
////					void DrawMedPack					(float x, float y, float z, float random, float ObjectSpin)
////		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GL
	#define GL
	#include "Platform.h"
	#include "GLConstants.cp"
#endif

#include "GameConstants.cp"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawHealthBar					(float camera_x, float camera_y, float camera_z, float lives, float health_ratio)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawHealthBar(float camera_x, float camera_y, float camera_z, float lives, float health_ratio)
{
	glTranslatef( camera_x, camera_y, camera_z);
		glRotated(180-CurrentCameraDirectionRightLeft,0,1,0);
				
				if (health_ratio < 0) health_ratio = 0;		
				glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, steel1);
				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
				glBegin(GL_QUAD_STRIP);
					glVertex3f( +1.08,	1.61, +2.6);
					glVertex3f( +1.08,	1.32, +2.6);
					
					glVertex3f( 1.065,	1.63, 2.6);
					glVertex3f( 1.07,	1.3, 2.6);
	
					glVertex3f( +0,		1.63, +2.6);
					glVertex3f( +0,		1.3, +2.6);
				
					glVertex3f( -1.065,	1.63, 2.6);
					glVertex3f( -1.07,	1.3, 2.6);

					glVertex3f( -1.08,	1.61, 2.6);
					glVertex3f( -1.08,	1.32, 2.6);
				glEnd();
				glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, green1);
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

				glBegin(GL_QUAD_STRIP);
					glVertex3f( 0+.405,	1.58, +2.59);
					glVertex3f( +.405,	1.35, +2.59);
					glVertex3f( 0.405 - 1.465 * health_ratio,	1.58, 2.6);
					glVertex3f( 0.405 - 1.465 * health_ratio,	1.35, 2.6);
				glEnd();

				
				// DRAW THE PLAYERS LIVES
				glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, minitank1);
				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
				if (lives >= 1)
				{
					glTranslatef(0.70, -0.37, 0);
						glBegin(GL_QUAD_STRIP);
							glVertex3f( 0.32,	1.55, 2.5);
							glVertex3f( 0.02,	1.55, 2.5);
							glVertex3f( 0.32,	2.0, 2.5);
							glVertex3f( 0.02,	2.0, 2.5);
						glEnd();
					glTranslatef(-0.70, +0.37, 0);				
				}
				if (lives >= 2)
				{	
					glTranslatef(0.385, -0.37, 0);
						glBegin(GL_QUAD_STRIP);
							glVertex3f( 0.32,	1.55, 2.5);
							glVertex3f( 0.02,	1.55, 2.5);
							glVertex3f( 0.32,	2.0, 2.5);
							glVertex3f( 0.02,	2.0, 2.5);
						glEnd();
					glTranslatef(-.385, +0.37, 0);				
				}
		glRotated(-180+CurrentCameraDirectionRightLeft,0,1,0);
		glTranslatef( -camera_x, -camera_y, -camera_z); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawSky						(float camera_x, float camera_y, float camera_z)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSky(float camera_x, float camera_y, float camera_z)
{
	glEnable(GL_TEXTURE_2D);  
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);			
	glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky1);
	glTranslatef( camera_x, camera_y, camera_z); //(Player1.Getx(), 0, Player1.Getz());
		glRotated(180-CurrentCameraDirectionRightLeft,0,1,0);		
			if (RunMode == TRANSITION) glutSolidSphere(200,40,40);
			else glutSolidSphere(200,30,20);
		glRotated(-180+CurrentCameraDirectionRightLeft,0,1,0);
	glTranslatef( -camera_x, -camera_y, -camera_z); //(Player1.Getx(), 0, Player1.Getz());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawStartScreenWallMounting	(float x, float y, float z)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawStartScreenWallMounting(float x, float y, float z)
{
	glTranslatef(x,y,z);
		glRotated(-90, 0,1,0);
			glEnable(GL_TEXTURE_2D);  
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);			
			glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, start1);
			glBegin(GL_QUAD_STRIP);
				glVertex3f( 0,		1, 0);
				glVertex3f( 1,		1, 0);
				glVertex3f( 0,		2, 0);
				glVertex3f( 1,		2, 0);
			glEnd();
			glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, start2);
			glBegin(GL_QUAD_STRIP);
				glVertex3f( 1,		1, 0);
				glVertex3f( 2,		1, 0);
				glVertex3f( 1,		2, 0);
				glVertex3f( 2,		2, 0);
			glEnd();
			glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, start3);
			glBegin(GL_QUAD_STRIP);
				glVertex3f( 0,		0, 0);
				glVertex3f( 1,		0, 0);
				glVertex3f( 0,		1, 0);
				glVertex3f( 1,		1, 0);
			glEnd();
			glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, start4);
			glBegin(GL_QUAD_STRIP);
				glVertex3f( 1,		0, 0);
				glVertex3f( 2,		0, 0);
				glVertex3f( 1,		1, 0);
				glVertex3f( 2,		1, 0);
			glEnd();
		glRotated(90, 0,1,0);
	glTranslatef(-x,-y,-z);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawGround						(float x, float y, float z)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawGround(float x, float y, float z)
{
	glTranslatef(  MAP_BLOCK_SIZE*GLfloat(x),  MAP_BLOCK_SIZE*GLfloat(0),  MAP_BLOCK_SIZE*GLfloat(z));
		glRotated(90, 1,0,0);
			glBegin(GL_QUAD_STRIP);
				glVertex3f( 0, 0, 0);
				glVertex3f( ground[3], 0, 0);
				glVertex3f( 0, ground[3], 0);
				glVertex3f(  ground[3],  ground[3], 0);
			glEnd();
		glRotated(-90, 1,0,0);
	glTranslatef(  -MAP_BLOCK_SIZE*GLfloat(x),  -MAP_BLOCK_SIZE*GLfloat(y),  -MAP_BLOCK_SIZE*GLfloat(z));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawWall						(float x, float y, float z)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawWall(float x, float y, float z)
{
	int i;
	glTranslatef(MAP_BLOCK_SIZE * GLfloat(x+0.5), 0, +MAP_BLOCK_SIZE*GLfloat(z+0.5));
		glBegin(GL_QUAD_STRIP);
			for (i=0; i<=7; i++) glVertex3f( wall[3*i], wall[3*i+1] , wall[3*i+2]); 
		glEnd();			
		glRotated(90,0,1,0);
			glBegin(GL_QUAD_STRIP);
				for (i=0; i<=7; i++) glVertex3f( wall[3*i], wall[3*i+1] , wall[3*i+2]); 
			glEnd();	
		glRotated(-90,0,1,0);
		glTranslatef(0, 1.5 * MAP_BLOCK_SIZE,0);
			glRotated(90, 1,0,0);
				glBegin(GL_QUAD_STRIP);
					glVertex3f( 0.5 * MAP_BLOCK_SIZE,	0.5 * MAP_BLOCK_SIZE, 0);
					glVertex3f( -0.5 * MAP_BLOCK_SIZE,	0.5 * MAP_BLOCK_SIZE, 0);
					glVertex3f( 0.5 * MAP_BLOCK_SIZE,	-0.5 * MAP_BLOCK_SIZE,		0);
					glVertex3f( -0.5 * MAP_BLOCK_SIZE,	-0.5 * MAP_BLOCK_SIZE, 0);
				glEnd();
			glRotated(-90, 1,0,0);
		glTranslatef(0,- 1.5 * MAP_BLOCK_SIZE,0);
	glTranslatef(-MAP_BLOCK_SIZE*GLfloat(x+0.5), 0, -MAP_BLOCK_SIZE*GLfloat(z+0.5));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawTower						(float x, float y, float z)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawTower(float x, float y, float z)
{
	int i;
	glTranslatef(MAP_BLOCK_SIZE * GLfloat(x+0.5), 0, +MAP_BLOCK_SIZE*GLfloat(z+0.5));
		glBegin(GL_QUAD_STRIP);
			for (i=0; i<=7; i++) glVertex3f( tower[3*i], tower[3*i+1] , tower[3*i+2]); 
		glEnd();			
		glRotated(90,0,1,0);
			glBegin(GL_QUAD_STRIP);
				for (i=0; i<=7; i++) glVertex3f( tower[3*i], tower[3*i+1] , tower[3*i+2]); 
			glEnd();	
		glRotated(-90,0,1,0);
	glTranslatef(-MAP_BLOCK_SIZE*GLfloat(x+0.5), 0, -MAP_BLOCK_SIZE*GLfloat(z+0.5));	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawArch						(float x, float y, float z, float length)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawArch(float x, float y, float z, float length)
{
	z = z - 0.05;
	length = length + 0.1;
	int i;
	glBegin(GL_QUAD_STRIP);
		glColor3f(0.7, 0, 0);
		for (i=0; i<28; i++) glVertex3f( arch1[3*i] + MAP_BLOCK_SIZE*GLfloat(x), 2*arch1[3*i+1] , arch1[3*i+2]+MAP_BLOCK_SIZE*GLfloat(z)); 
	glEnd();
			
	glBegin(GL_QUAD_STRIP);
		glColor3f(0.7, 0, 0);
		for (i=0; i<28; i++) glVertex3f( arch1[3*i] + MAP_BLOCK_SIZE*GLfloat(x), 2*arch1[3*i+1] , arch1[3*i+2]+MAP_BLOCK_SIZE*GLfloat(z) + length*MAP_BLOCK_SIZE); 
	glEnd();
									
	glBegin(GL_QUAD_STRIP);
		for (i=0; i<14; i++)
		{				
			glColor3f(0.8, 0, 0);
			glVertex3f( arch1[2*3*i] + MAP_BLOCK_SIZE*GLfloat(x), 2*arch1[2*3*i+1] , arch1[2*3*i+2]+MAP_BLOCK_SIZE*GLfloat(z)); 
			glVertex3f( arch1[2*3*i] + MAP_BLOCK_SIZE*GLfloat(x), 2*arch1[2*3*i+1] , arch1[2*3*i+2]+MAP_BLOCK_SIZE*GLfloat(z) + length*MAP_BLOCK_SIZE); 
		}
	glEnd();
				
	glBegin(GL_QUAD_STRIP);
		for (i=0; i<14; i++)
		{				
			glColor3f(0.6, 0.0, 0.0);
			glVertex3f( arch1[2*3*i + 3] + MAP_BLOCK_SIZE*GLfloat(x), 2*arch1[2*3*i+1 + 3] , 2*arch1[2*3*i+2 + 3]+MAP_BLOCK_SIZE*GLfloat(z)); 
			glVertex3f( arch1[2*3*i + 3] + MAP_BLOCK_SIZE*GLfloat(x), 2*arch1[2*3*i+1 + 3] , 2*arch1[2*3*i+2 + 3]+MAP_BLOCK_SIZE*GLfloat(z) + length*MAP_BLOCK_SIZE); 
		}
	glEnd();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawRail						(float x, float y, float z)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawRail(float x, float y, float z)
{
	int i;
	glBegin(GL_QUAD_STRIP);
		glColor3f(0.7, 0.4, 0.0);
		for (i=0; i<16; i++) glVertex3f( rail1[3*i] + MAP_BLOCK_SIZE*GLfloat(x), rail1[3*i+1] , rail1[3*i+2]+MAP_BLOCK_SIZE*GLfloat(z)); 
	glEnd();
	glBegin(GL_QUAD_STRIP);
		glColor3f(0.8, 0.8, 0.8);
		for (i=0; i<16; i++) glVertex3f( rail2[3*i] + MAP_BLOCK_SIZE*GLfloat(x), rail2[3*i+1] , rail2[3*i+2]+MAP_BLOCK_SIZE*GLfloat(z)); 
	glEnd();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawRailSleeper				(float x, float y, float z)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawRailSleeper(float x, float y, float z)
{
	int i;
	glBegin(GL_QUAD_STRIP);
		glColor3f(0.7, 0.4, 0.0);
		for (i=0; i<16; i++) glVertex3f( rail1[3*i] + MAP_BLOCK_SIZE*GLfloat(x), rail1[3*i+1] , rail1[3*i+2]+MAP_BLOCK_SIZE*GLfloat(z)); 
	glEnd();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawRapidFire					(float x, float y, float z, float random, float ObjectSpin)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawRapidFire(float x,float y, float z, float random, float ObjectSpin)
{
	int i;
	glBegin(GL_QUAD_STRIP);
		glColor3f(0.5 + 0.5 * random, 0.5 + 0.5 * random, 0.5 + 0.5 * random);
		for (i=0; i<4; i++) glVertex3f( ground[3*i] + MAP_BLOCK_SIZE*GLfloat(x), ground[3*i+1] , ground[3*i+2]+MAP_BLOCK_SIZE*GLfloat(z)); 
	glEnd();
	glTranslatef(MAP_BLOCK_SIZE*GLfloat(x) + 0.5 * MAP_BLOCK_SIZE, 1.2*MAP_BLOCK_SIZE, +MAP_BLOCK_SIZE*GLfloat(z) +  0.5 * MAP_BLOCK_SIZE);
		glRotated(ObjectSpin, 0, 1, 0);
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.8, 0, 0);
				for (i=0; i<10; i++) glVertex3f( rapidfire[3*i], rapidfire[3*i+1], rapidfire[3*i+2] + 0.4 * MAP_BLOCK_SIZE); 
			glEnd();
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.8, 0, 0);
				for (i=0; i<10; i++) glVertex3f( rapidfire[3*i], rapidfire[3*i+1], rapidfire[3*i+2] - 0.4 * MAP_BLOCK_SIZE); 
			glEnd();
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.6, 0, 0);
				for (i=0; i<5; i++) 
				{
					glVertex3f( rapidfire[2*3*i], rapidfire[2*3*i+1], rapidfire[2*3*i+2] - 0.4*MAP_BLOCK_SIZE); 
					glVertex3f( rapidfire[2*3*i], rapidfire[2*3*i+1], rapidfire[2*3*i+2] + 0.4*MAP_BLOCK_SIZE); 
				}
			glEnd();
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.4, 0, 0);
				for (i=0; i<5; i++) 
				{
					glVertex3f( rapidfire[2*3*i + 3], rapidfire[2*3*i+1 + 3], rapidfire[2*3*i+2 + 3] - 0.4*MAP_BLOCK_SIZE); 
					glVertex3f( rapidfire[2*3*i + 3], rapidfire[2*3*i+1 + 3], rapidfire[2*3*i+2 + 3] + 0.4*MAP_BLOCK_SIZE); 
				}
			glEnd();
		glRotated(-ObjectSpin, 0, 1, 0);
	glTranslatef(-MAP_BLOCK_SIZE*GLfloat(x) - 0.5 * MAP_BLOCK_SIZE, -1.2*MAP_BLOCK_SIZE, -MAP_BLOCK_SIZE*GLfloat(z) -  0.5 * MAP_BLOCK_SIZE);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawDualFire					(float x, float y, float z, float random, float ObjectSpin)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawDualFire (float x,float y, float z, float random, float ObjectSpin)
{
	int i;
	glBegin(GL_QUAD_STRIP);
		glColor3f(0.5 + 0.5 * random, 0.5 + 0.5 * random, 0.5 + 0.5 * random);
		for (i=0; i<4; i++) glVertex3f( ground[3*i] + MAP_BLOCK_SIZE*GLfloat(x), ground[3*i+1] , ground[3*i+2]+MAP_BLOCK_SIZE*GLfloat(z)); 
	glEnd();
	glTranslatef(MAP_BLOCK_SIZE*GLfloat(x) + 0.5 * MAP_BLOCK_SIZE, 1.2*MAP_BLOCK_SIZE, +MAP_BLOCK_SIZE*GLfloat(z) +  0.5 * MAP_BLOCK_SIZE);
		glRotated(ObjectSpin, 0, 1, 0);
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.8, 0, 0);
				for (i=0; i<8; i++) glVertex3f( dualfire[3*i], dualfire[3*i+1], dualfire[3*i+2] +  0.4*MAP_BLOCK_SIZE); 
			glEnd();
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.8, 0, 0);
				for (i=0; i<8; i++) glVertex3f( dualfire[3*i], dualfire[3*i+1], dualfire[3*i+2] - 0.4*MAP_BLOCK_SIZE); 
			glEnd();
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.6, 0, 0);
				for (i=0; i<4; i++) 
				{
					glVertex3f( dualfire[2*3*i], dualfire[2*3*i+1], dualfire[2*3*i+2] - 0.4*MAP_BLOCK_SIZE); 
					glVertex3f( dualfire[2*3*i], dualfire[2*3*i+1], dualfire[2*3*i+2] + 0.4*MAP_BLOCK_SIZE); 
				}
			glEnd();
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.4, 0, 0);
				for (i=0; i<4; i++) 
				{
					glVertex3f( dualfire[2*3*i + 3], dualfire[2*3*i+1 + 3], dualfire[2*3*i+2 + 3] - 0.4*MAP_BLOCK_SIZE); 
					glVertex3f( dualfire[2*3*i + 3], dualfire[2*3*i+1 + 3], dualfire[2*3*i+2 + 3] + 0.4*MAP_BLOCK_SIZE); 
				}
			glEnd();
		glRotated(-ObjectSpin, 0, 1, 0);
	glTranslatef(-MAP_BLOCK_SIZE*GLfloat(x) - 0.5 * MAP_BLOCK_SIZE, -1.2*MAP_BLOCK_SIZE, -MAP_BLOCK_SIZE*GLfloat(z) -  0.5 * MAP_BLOCK_SIZE);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					void DrawMedPack					(float x, float y, float z, float random, float ObjectSpin)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawMedPack(float x,float y, float z, float random, float ObjectSpin)
{
	int i;
	int FRONTBACK;
	glBegin(GL_QUAD_STRIP);
		glColor3f(0.5 + 0.5 * random, 0.5 + 0.5 * random, 0.5 + 0.5 * random);
		for (i=0; i<4; i++) glVertex3f( ground[3*i] + MAP_BLOCK_SIZE*GLfloat(x), ground[3*i+1] , ground[3*i+2]+MAP_BLOCK_SIZE*GLfloat(z)); 
	glEnd();
	glTranslatef(MAP_BLOCK_SIZE*GLfloat(x) + 0.5 * MAP_BLOCK_SIZE, 1.2*MAP_BLOCK_SIZE, +MAP_BLOCK_SIZE*GLfloat(z) +  0.5 * MAP_BLOCK_SIZE);
		glRotated(ObjectSpin, 0, 1, 0);
			for (FRONTBACK = -1; FRONTBACK<=1; FRONTBACK+=2)
			{
				glBegin(GL_POLYGON);
					glColor3f(0.6, 0, 0);
					for (i=0; i<14; i++) glVertex3f( medpack[3*i], medpack[3*i+1], medpack[3*i+2] + FRONTBACK * 0.4 * MAP_BLOCK_SIZE); 
					for (i=0; i<14; i++) glVertex3f(- medpack[3*i], medpack[3*i+1], medpack[3*i+2] + FRONTBACK * 0.4 * MAP_BLOCK_SIZE); 
				glEnd();
			}						
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.4, 0, 0);
				for (i=0; i<14; i++) 
				{
					glVertex3f( medpack[3*i], medpack[3*i+1], medpack[3*i+2] + 0.4 * MAP_BLOCK_SIZE); 
					glVertex3f( medpack[3*i], medpack[3*i+1], medpack[3*i+2] - 0.4 * MAP_BLOCK_SIZE); 
				}
				for (i=0; i<14; i++) 
				{
					glVertex3f(- medpack[3*i], medpack[3*i+1], medpack[3*i+2] + 0.4 * MAP_BLOCK_SIZE); 
					glVertex3f(- medpack[3*i], medpack[3*i+1], medpack[3*i+2] - 0.4 * MAP_BLOCK_SIZE); 
				}
			glEnd();
		glRotated(-ObjectSpin, 0, 1, 0);
	glTranslatef(-MAP_BLOCK_SIZE*GLfloat(x) - 0.5 * MAP_BLOCK_SIZE, -1.2*MAP_BLOCK_SIZE, -MAP_BLOCK_SIZE*GLfloat(z) -  0.5 * MAP_BLOCK_SIZE);
}

