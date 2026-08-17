//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			BulletClass.cp																															
////	Author:			Ed Martin
////	Description:	Holds the BulletClass Object
////
////	Contents:		
////					1) Private variables
////						a) Position, Velocity, Turret Direction, Health, Lives
////						b) State and bullet type
////						c) Animation counters
////
////					2) Constructor, Destructor and Restart / Init
////
////					3) Set Functions
////
////					4) Get Functions
////
////					5) Update Function
////						a) Update Animation and Gameplay counters
////						b) Update the bullet's motion
////
////					6) Draw Function
////					   If bullet is active :
////						  a) Draw either a PLAYER / ENEMY / SHRAPNEL bullet
////						  b) Draw bullet's shadow.
////
////					   If turret is Exploding : 
////						  c) Randomise 40 explosion points every 2nd frame
////						  d) Draw Red, Orange, then Yellow part of the explosion.
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameConstants.cp"

#ifndef GL
	#define GL
	#include <GLUT/glut.h>
	#include "GLConstants.cp"
	#include "GLGameObjectDrawFunctions.cp"
	#include "GLTextures.cp"
#endif

#ifndef EVENTS
	#define EVENTS	
	int x;
	float events[0];
    void Event(int description, float a, float b, float c, float d, float e, float f, float g, float h, float i) {}
#endif

class BulletClass
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					1) Private variables
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	private:
	
//// a) Position, Velocity, Turret Direction, Health, Lives

				float x,y,z;
				float dx,dy,dz;
				float ay;
				float Speed;

//// b) State and bullet type

				int   state, bulletType;

//// c) Animation counters
				
				float explosionTime;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Constructor / Destructor and Restart / Init
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	public:
				BulletClass();
				virtual ~BulletClass();
				
				void InitBullet(float newX, float newY, float newZ, float newDx, float newDy, float newDz, float newAy, float newSpeed, int newBulletType);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					3) Set Functions including Explode function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				
				void  SetState(int newState);
				void  Explode(float newX, float newY, float newZ);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) Get Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				float Getx(void);
				float Gety(void);
				float Getz(void);
				float Getdx(void);
				float Getdy(void);
				float Getdz(void);
				float GetSpeed(void);
				float GetexplosionRadius(void);
				int   GetActive();
				int   GetType();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					5) Update Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				void  Update(void);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					6) Draw Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				void  DrawBullet();
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Constructor / Destructor and Restart / Init
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BulletClass::BulletClass(){}
BulletClass::~BulletClass() {}	

void BulletClass::InitBullet(float newX, float newY, float newZ, float newDx, float newDy, float newDz, float newAy, float newSpeed, int newBulletType)
{
	x = newX;
	y = newY;
	z = newZ;
	dx = newDx;
	dy = newDy;
	dz = newDz;
	ay = newAy;
	Speed = newSpeed;
	state = ACTIVE;
	explosionTime = 0;
	bulletType = newBulletType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					3) Set Functions including Explode function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BulletClass::SetState(int newState)	{	state = newState;		}

void BulletClass::Explode(float newX, float newY, float newZ)
{
	x = newX;
	y = newY;
	z = newZ;
	dx = dy = dz = ay = 0;

	state = EXPLODING;
	explosionTime = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) Get Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float BulletClass::Getx(void)		{	return x;			}
float BulletClass::Gety(void)		{	return y;			}
float BulletClass::Getz(void)		{	return z;			}
float BulletClass::Getdx(void)		{	return dx;			}
float BulletClass::Getdy(void)		{	return dy;			}
float BulletClass::Getdz(void)		{	return dz;			}
float BulletClass::GetSpeed(void)	{	return Speed;		}

float BulletClass::GetexplosionRadius(void)	
{	
	if (bulletType==PLAYER_BULLET)		return MIN_EXPLOSION_RADIUS * (explosionTime / MAX_EXPLOSION_TIME); 
	else if (bulletType==ENEMY_BULLET)	return ENEMY_MIN_EXPLOSION_RADIUS * (explosionTime / ENEMY_MAX_EXPLOSION_TIME); 
	else return 0;
}

int BulletClass::GetActive()		{	return state;		}
int BulletClass::GetType()			{   return bulletType;	}
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					5) Update Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BulletClass::Update(void)
{

//// a) Update Animation and Gameplay counters

	if (explosionTime > MAX_EXPLOSION_TIME) state = INACTIVE;
	if (state == EXPLODING) explosionTime++;

//// b) Update the bullet's motion

	x += dx;
	dy+= ay;
	y += dy;
	z += dz;
			
}
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					6) Draw Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BulletClass::DrawBullet()
{
	int i;
	
//// If bullet is active :
//// a) Draw either a PLAYER / ENEMY / SHRAPNEL bullet

	if (state == ACTIVE)
	{
		glTranslated(x,y,z);
			if (bulletType == PLAYER_BULLET) 
			{
				glColor3f( 1,1,1);
				glutSolidSphere( PLAYER_BULLET_RADIUS, 10, 10);
			}
			else if (bulletType == ENEMY_BULLET)	
			{
				glColor3f( 0.4, 0.0, 0.0);	
				glutSolidSphere( ENEMY_BULLET_RADIUS, 10, 10);
			}
			else if (bulletType == SHRAPNEL)
			{
				float r = float(random()) / float(RAND_MAX);
				glColor3f( 0.1 + 0.3 * r, 0.1 + 0.3 * r, 0.1 + 0.3 * r);
				glBegin(GL_QUAD_STRIP);
					for( int sd = 1; sd<20; sd++)
					{
						glVertex3f( 0.2 * MAP_BLOCK_SIZE * (2 * float(random()) / float(RAND_MAX) - 1), 
									0.2 * MAP_BLOCK_SIZE * (2 * float(random()) / float(RAND_MAX) - 1), 
									0.2 * MAP_BLOCK_SIZE * (2 * float(random()) / float(RAND_MAX) - 1));
						glVertex3f( - 2.5 * MAP_BLOCK_SIZE * dx + 0.05 * MAP_BLOCK_SIZE * (2 * float(random()) / float(RAND_MAX) - 1),		 
									- 2.5 * MAP_BLOCK_SIZE * dy + 0.05 * MAP_BLOCK_SIZE * (2 * float(random()) / float(RAND_MAX) - 1),		 		
									- 2.5 * MAP_BLOCK_SIZE * dz + 0.05 * MAP_BLOCK_SIZE * (2 * float(random()) / float(RAND_MAX) - 1));
					}	
				glEnd();
			}	
		glTranslated(-x,-y,-z);
			
//// b) Draw bullet's shadow.

		glColor3f(0,0,0);
		glTranslated(x, 0.1, z);
			glBegin(GL_POLYGON);
				for (i=0; i<20; i++)
				{
					glVertex3f( PLAYER_BULLET_RADIUS * sin( 2 * M_PI * i/20), 0.1, PLAYER_BULLET_RADIUS * cos( 2 * M_PI * i/20));
				}
			glEnd();
		glTranslated(-x, -0.1, -z);
	}
	
//// If turret is Exploding : 

	else if (state == EXPLODING)
	{

//// c) Randomise 40 explosion points every 2nd frame

		static float yellow, red, sa[40], ca[40], sb[40], cb[40];
		float a,b, rndx, rndy, rndz;
		int i;
							
		if ( (int(explosionTime) % 2 == 0) || (explosionTime==0) ) for (i = 0; i<40; i++)
		{
			a = 2 * M_PI * float(random()) / float(RAND_MAX);
			b = 2 * M_PI * float(random()) / float(RAND_MAX);	
			sa[i] = sin(a);
			ca[i] = cos(a);
			sb[i] = sin(b);
			cb[i] = cos(b);
			yellow = float(random()) / float(RAND_MAX);
			red =  float(random()) / float(RAND_MAX);
		}

//// d) Draw Red, Orange, then Yellow part of the explosion.

		glTranslated(x,y,z);			
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.1 + 0.3*yellow + 0.4*red, 0.1 + 0.3*yellow, 0);
				glColor3f(0.1 + 0.3*yellow + 0.4*red, 0.1 + 0.3*yellow, 0);
				for (i = 0; i<40; i++)
				{
					rndx = sb[i] * ca[i];
					rndy = cb[i];
					rndz = sb[i] * sa[i];
					
					glVertex3f(	rndx*MIN_EXPLOSION_RADIUS + (MAX_EXPLOSION_RADIUS - MIN_EXPLOSION_RADIUS) * rndx * (explosionTime * explosionTime)/(MAX_EXPLOSION_TIME * MAX_EXPLOSION_TIME + 0.001),
								rndy*MIN_EXPLOSION_RADIUS + (MAX_EXPLOSION_RADIUS - MIN_EXPLOSION_RADIUS) * rndy * (explosionTime * explosionTime)/(MAX_EXPLOSION_TIME * MAX_EXPLOSION_TIME + 0.001),
								rndz*MIN_EXPLOSION_RADIUS + (MAX_EXPLOSION_RADIUS - MIN_EXPLOSION_RADIUS) * rndz * (explosionTime * explosionTime)/(MAX_EXPLOSION_TIME * MAX_EXPLOSION_TIME + 0.001)); 
				}	
				glColor3f(0.6 + 0.2*yellow + 0.2*red, 0.6 + 0.2*yellow, 0);
				for (i = 0; i<20; i++)
				{
					rndx = cb[i+2] * ca[i+2];
					rndy = sb[i+2];
					rndz = cb[i+2] * sa[i+2];
					glVertex3f( 0.7 * (rndx * MIN_EXPLOSION_RADIUS + (MAX_EXPLOSION_RADIUS - MIN_EXPLOSION_RADIUS) * rndx * (explosionTime * explosionTime)/(MAX_EXPLOSION_TIME * MAX_EXPLOSION_TIME + 0.001)),
								0.7 * (rndy * MIN_EXPLOSION_RADIUS + (MAX_EXPLOSION_RADIUS - MIN_EXPLOSION_RADIUS) * rndy * (explosionTime * explosionTime)/(MAX_EXPLOSION_TIME * MAX_EXPLOSION_TIME + 0.001)),
								0.7 * (rndz * MIN_EXPLOSION_RADIUS + (MAX_EXPLOSION_RADIUS - MIN_EXPLOSION_RADIUS) * rndz * (explosionTime * explosionTime)/(MAX_EXPLOSION_TIME * MAX_EXPLOSION_TIME + 0.001)) ); 
				}
				glColor3f(0.8 + 0.1*yellow + 0.1*red, 0.8 + 0.1*yellow, 0);
				for (i = 0; i<20; i++)
				{
					rndx = sb[i+6] * sa[i+6];
					rndy = cb[i+6];
					rndz = sb[i+6] * ca[i+6];
					glVertex3f( 0.3 * (rndx * MIN_EXPLOSION_RADIUS + (MAX_EXPLOSION_RADIUS - MIN_EXPLOSION_RADIUS) * rndx * (explosionTime * explosionTime)/(MAX_EXPLOSION_TIME * MAX_EXPLOSION_TIME + 0.001)),
								0.3 * (rndy * MIN_EXPLOSION_RADIUS + (MAX_EXPLOSION_RADIUS - MIN_EXPLOSION_RADIUS) * rndy * (explosionTime * explosionTime)/(MAX_EXPLOSION_TIME * MAX_EXPLOSION_TIME + 0.001)),
								0.3 * (rndz * MIN_EXPLOSION_RADIUS + (MAX_EXPLOSION_RADIUS - MIN_EXPLOSION_RADIUS) * rndz * (explosionTime * explosionTime)/(MAX_EXPLOSION_TIME * MAX_EXPLOSION_TIME + 0.001)) ); 											
				}
			glEnd();
		glTranslated(-x,-y,-z);
	}
}