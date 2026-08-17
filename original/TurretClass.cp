//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			TurretClass.cp																															
////	Author:			Ed Martin
////	Description:	Holds the TurretClass Object
////
////	Contents:		
////					1) Private variables
////						a) Position, Velocity, Turret Direction, Health, Lives
////						b) State and turret cannon type
////						c) Gameplay counters
////						d) Animation counters
////
////					2) Constructor, Destructor and Restart / Init
////
////					3) Set Functions
////						a) Position, Velocity, Turret Direction, Health
////						b) State and turret cannon type
////
////					4) Get Functions
////						a) Position, Velocity, Turret Direction, Health
////						b) State and turret cannon type
////
////					5) Update Function
////						a) Update Animation and Gameplay counters
////						b) If not exploding, aim, then if the timeToFire counter has finished its decremental count 
////						   from maxtimetofire to zero and the player is in range, Shoot in direction of Player.
////
////					6) Draw Function
////					   If turret is Active : 
////					      a) If turret is flashing (freshly born or hit by a bullet), then randomise the additive 
////						 	whiteness colour ExtraWhiteness
////					      b) Draw main turret Bubble
////						  c) Draw main turret barrel
////					      d) Draw 2 side barrels
////					      e) Draw Window
////
////					   If turret is Exploding : 
////						  f) Randomise 40 explosion points every 3rd frame
////						  g) Draw white shock wave ring
////						  h) Draw Red, Orange, Yellow part of explosion.
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

class TurretClass
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					1) Private variables
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	private:

//// a) Position, Velocity, Turret Direction, Health, Lives
				float x, y, z;
				float turretRightLeft, turretUpDown;
				int health;
	
//// b) State and turret cannon type
				int state;
				
//// c) Gameplay counters
				float timeToFire, maxtimeToFire;

//// d) Animation counters
				int explosionTime;
				float timeToFlash;
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Constructor / Destructor and Restart / Init
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	public:	
				TurretClass(void);
				virtual ~TurretClass();
	 
				void  InitTurret(float Turretx, float Turrety, float Turretz);	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					3) Set Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret Direction, Health

				void  SetturretRightLeft	(float TankturretRightLeft	);
				void  SetturretUpDown		(float TankturretUpDown		);	
				void  DecreaseHealth		(int   newHealth			);							

//// b) State and turret cannon type

				void SetState				(int   newState				);		

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) Get Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret Direction, Health

				float Getx();			
				float Gety();			
				float Getz();			
				float GetturretRightLeft();	
				float GetturretUpDown();		
	
//// b) State and turret cannon type

				int	  GetState();				
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					5) Update Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				void  Update(float Playerx, float Playerz);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					6) Draw Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
				void  DrawTurret(void); 
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Constructor, Destructor and Restart / Init
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TurretClass::TurretClass(void) {}

TurretClass::~TurretClass() {}

void TurretClass::InitTurret(float Turretx, float Turrety, float Turretz)
{
	x = Turretx;
	y = Turrety;
	z = Turretz;
	health = TURRET_MAX_HEALTH;
	timeToFire = ENEMY_MAX_TIME_TO_FIRE;
	timeToFlash = ENEMY_MAX_TIME_TO_FLASH;
	turretRightLeft  = 180;
	turretUpDown = 0;
	explosionTime = 0;
	state = 0;
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					3) Set Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret Direction, Health

void TurretClass::SetturretRightLeft	(float TankturretRightLeft)		{	turretRightLeft = TankturretRightLeft;	}
void TurretClass::SetturretUpDown		(float TankturretUpDown)		{	turretUpDown = TankturretUpDown;		}

void TurretClass::DecreaseHealth		(int newHealth)					
{	
	CameraShakeTime = 0.5 * MAX_CAMERA_SHAKE_TIME;
	health -= newHealth;
	timeToFlash = ENEMY_MAX_TIME_TO_FLASH;
																		
	// if the turret is dead, then make it explode and create random shrapnel
	if  (health <= 0)
	{
		state = EXPLODING;
		explosionTime = 0;
		CameraShakeTime = 1*MAX_CAMERA_SHAKE_TIME;
															
		float ud,rl;
		for (int i = 1; i<20; i++)
		{
			ud =  M_PI * float(random()) / float(RAND_MAX);
			rl = 2 * M_PI * float(random()) / float(RAND_MAX);	
																			
			// For a single bullet/shrapnel, a NEW_BULLET_EVENT is called using the following format:
			// Event(NEW_BULLET_EVENT,	x_initial,						y_initial,						z_initial,
			//							x_velocity,		
			//							y_velocity,		
			//							z_velocity,
			//							y_acceleration,					bullet speed,					type of bullet

			Event(	NEW_BULLET_EVENT,	x + sin(rl) * cos(ud) * 20,		y + 20 * sin(ud),				z + cos(rl) * cos(ud) * 20, 
										2*float(random()) / float(RAND_MAX) * ENEMY_BULLET_SPEED * cos(ud) * sin(rl),	
										2*float(random()) / float(RAND_MAX) * ENEMY_BULLET_SPEED * sin(ud),		
										2*float(random()) / float(RAND_MAX) * ENEMY_BULLET_SPEED * cos(ud) * cos(rl),	
										-ENEMY_BULLET_SPEED/50,			ENEMY_BULLET_SPEED,				SHRAPNEL);
		}
	}
}

//// b) State and turret cannon type

void TurretClass::SetState				(int   newState)				{	state = newState;						}
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) Get Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
//// a) Position, Velocity, Turret Direction, Health

float TurretClass::Getx()					{	return x;				}
float TurretClass::Gety()					{	return y;				}
float TurretClass::Getz()					{	return z;				}
float TurretClass::GetturretRightLeft()		{	return turretRightLeft;	}
float TurretClass::GetturretUpDown()		{	return turretUpDown;	}
int TurretClass::GetState()					{	return state;			}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					5) Update Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
void TurretClass::Update(float Playerx, float Playerz)	
{	

//// a) Update Animation and Gameplay counters

	if (timeToFire>=0)		timeToFire--;	
	if (timeToFlash>=0)		timeToFlash--;
		
	if (explosionTime > ENEMY_MAX_EXPLOSION_TIME)	state = SMOKING;    

	if (state == EXPLODING)							explosionTime++;

//// b) If not exploding, aim, then if the timeToFire counter has finished its decremental count 
////    from maxtimetofire to zero and the player is in range, Shoot in direction of Player.
	if (state == ACTIVE) 
	{
		turretRightLeft =  (360 / ( 2 * M_PI )) * atan( (Playerx - x) / (Playerz - z) );
		if (Playerz - z <= 0) turretRightLeft += 180;
	
		float xzDistToPlayer = sqrt((Playerz - z)*(Playerz - z) + (Playerx - x)*(Playerx - x)) + 0.00001;
		
		if ((timeToFire<0) && (xzDistToPlayer < 50 * MAP_BLOCK_SIZE)) 
		{	
			turretUpDown = -(360 / (2*M_PI)) * atan( y / xzDistToPlayer );
			float rl, ud;
			rl = 2*M_PI/360*turretRightLeft;
			ud = 2*M_PI/360*turretUpDown;
		
			// For a single bullet, a NEW_BULLET_EVENT is called using the following format:
			// Event(NEW_BULLET_EVENT,	x_initial,											y_initial,								z_initial,
			//							x_velocity,											y_velocity,								z_velocity,
			//							y_acceleration,										bullet speed,							type of bullet

			Event(	NEW_BULLET_EVENT,	x + sin(rl) * cos(ud) * 4 * MAP_BLOCK_SIZE,			y + 4 * MAP_BLOCK_SIZE * sin(ud),		z + cos(rl) * cos(ud) * 4 * MAP_BLOCK_SIZE, 
											ENEMY_BULLET_SPEED * cos(ud) * sin(rl),			ENEMY_BULLET_SPEED * sin(ud),			ENEMY_BULLET_SPEED * cos(ud) * cos(rl),	
											0,												ENEMY_BULLET_SPEED,						ENEMY_BULLET);
			timeToFire = ENEMY_MAX_TIME_TO_FIRE; 
		}
	}
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					6) Draw Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TurretClass::DrawTurret(void) 
{	
	
//// a) If turret is flashing (freshly born or hit by a bullet), then randomise the additive whiteness colour ExtraWhiteness

	float ExtraWhiteness;
	if (timeToFlash > 0) 
		ExtraWhiteness = timeToFlash * (0.5 + 0.5 * (float(random()) / float(RAND_MAX)) )/ ENEMY_MAX_TIME_TO_FLASH; 
	else ExtraWhiteness = 0;
		

//// If turret is Active : 

	if (state == ACTIVE)
	{

		glColor3f(0.3 + 0.7 * ExtraWhiteness, 0 + 1 * ExtraWhiteness, 0 + 1 * ExtraWhiteness);
		glTranslated(x,y,z);
			glRotated(turretRightLeft,0,1,0);
			glRotated(turretUpDown,-1,0,0);
				glDisable(GL_TEXTURE_2D);  

//// b) Draw main turret Bubble

				glutSolidSphere(ENEMY_TURRET_SIZE,10,5);

//// c) Draw main turret barrel

				int i,N=7;
				float angle = 0;
				float tempX, tempY;
				
				glBegin(GL_QUAD_STRIP);
					glColor3f(0.2 + 0.7 * ExtraWhiteness, 0 + 1 * ExtraWhiteness, 0 + 1 * ExtraWhiteness);
					for (i=0; i<=N; i++)		
					{	
						angle =2*M_PI / N + 2*M_PI/N * i;
						tempX = sin( angle ) * 0.4 * ENEMY_TURRET_SIZE;
						tempY = cos( angle ) * 0.3 * ENEMY_TURRET_SIZE  - 0.4 * ENEMY_TURRET_SIZE;
						glVertex3f( 0.6 * tempX, 0.6* tempY, 0);
						glVertex3f( tempX, tempY, 3 * ENEMY_TURRET_SIZE - 0.8 * ENEMY_TURRET_SIZE * float(timeToFire * timeToFire / ENEMY_MAX_TIME_TO_FIRE / ENEMY_MAX_TIME_TO_FIRE));
					}
				glEnd();

//// d) Draw 2 side barrels

				int LeftRightTurretSticks;
				for (LeftRightTurretSticks = 1; LeftRightTurretSticks > - 2; LeftRightTurretSticks -= 1)
				{
					glBegin(GL_QUAD_STRIP);
						glColor3f(0.2 + 0.7 * ExtraWhiteness, 0 + 1 * ExtraWhiteness, 0 + 1 * ExtraWhiteness);
						for (i=0; i<=N; i++)		
						{	
							angle =2*M_PI / N + 2*M_PI/N * i;
							tempX = sin( angle ) * 0.1 * ENEMY_TURRET_SIZE - 0.6 * ENEMY_TURRET_SIZE * LeftRightTurretSticks;
							tempY = cos( angle ) * 0.1 * ENEMY_TURRET_SIZE  - 0.2 * ENEMY_TURRET_SIZE;
							glVertex3f( 0.6 * tempX, 0.6* tempY, 0);
							glVertex3f( tempX, tempY, 3 * ENEMY_TURRET_SIZE - 0.8 * ENEMY_TURRET_SIZE * float(timeToFire * timeToFire / ENEMY_MAX_TIME_TO_FIRE / ENEMY_MAX_TIME_TO_FIRE));
						}
					glEnd();
				}
					
//// e) Draw Window

				glEnable(GL_TEXTURE_2D);  
				glBegin(GL_QUAD_STRIP);
					glColor3f(0.0 + 0.6 * ExtraWhiteness, 0.0 + 0.6 * ExtraWhiteness, 0.4 + 0.6 * ExtraWhiteness); 
					for (i=0; i<18; i++)		
					{	
						glVertex3f( turretwindow[3*i] , turretwindow[3*i+1] , turretwindow[3*i+2]); 
					}		
				glEnd();
			glRotated(-turretUpDown,-1,0,0);
			glRotated(-turretRightLeft,0,1,0);
		glTranslated(-x,-y,-z);
		glDisable(GL_TEXTURE_2D);  
	}

//// If turret is Exploding : 

	else if (state == EXPLODING)
	{ 
		int i;
							
//// f) Randomise 50 explosion points every 3th frame

		static float yellow, red, sa[50], ca[50], sb[50], cb[50];
		float a,b, rndx, rndy, rndz;
		
		if ( (int(explosionTime) % 3 == 0) || (explosionTime==0) ) 
		 for (i = 0; i<50; i++)
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
		
//// g) Draw white shock wave ring

		glTranslated(x,y,z);	
			glBegin(GL_QUAD_STRIP);
				glColor3f(1,1,1);
				for (i = 0; i < 41; i++)
				{
					glVertex3f(	1 * MAP_BLOCK_SIZE * sin(i*2*M_PI/40) * pow(explosionTime,1.3), 0,	1 * MAP_BLOCK_SIZE * cos(i*2*M_PI/40) * pow(explosionTime, 1.3));
					glVertex3f(	1.3 * MAP_BLOCK_SIZE * sin(i*2*M_PI/40) * pow(explosionTime,1.3),	0, 	2.4 * MAP_BLOCK_SIZE * cos(i*2*M_PI/40) * pow(explosionTime, 1.3));
				}
			glEnd();
			
//// h) Draw Red, Orange, then Yellow part of the explosion.

			glBegin(GL_QUAD_STRIP);
				glColor3f(0.1 + 0.3*yellow + 0.4*red, 0.1 + 0.3*yellow, 0);
				for (i = 0; i<49; i++)
				{
					rndx = sb[i] * ca[i];
					rndy = cb[i];
					rndz = sb[i] * sa[i];
					
					glVertex3f(  1 * (rndx*ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS - ENEMY_MIN_EXPLOSION_RADIUS) * rndx *  pow(explosionTime,2)/pow(ENEMY_MAX_EXPLOSION_TIME,2) + 0.001)	 ,
								0.5 * (rndy*ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS - ENEMY_MIN_EXPLOSION_RADIUS) * rndy *  pow(explosionTime,2)/pow(ENEMY_MAX_EXPLOSION_TIME,2)  + 0.001) ,
								1 * (rndz*ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS - ENEMY_MIN_EXPLOSION_RADIUS) * rndz * pow(explosionTime,2)/pow(ENEMY_MAX_EXPLOSION_TIME,2)  + 0.001) ); 
				}
				glColor3f(0.6 + 0.2*yellow + 0.2*red, 0.6 + 0.2*yellow, 0);
				for (i = 0; i<30; i++)
				{
					rndx = cb[i+2] * ca[i+2];
					rndy = sb[i+2];
					rndz = cb[i+2] * sa[i+2];
					glVertex3f(  1 * (0.7 * (rndx * ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS - ENEMY_MIN_EXPLOSION_RADIUS) * rndx * pow(explosionTime,2)/pow(ENEMY_MAX_EXPLOSION_TIME,2) + 0.001) ),
								0.5 * (0.7 * (rndy * ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS - ENEMY_MIN_EXPLOSION_RADIUS) * rndy * pow(explosionTime,2)/pow(ENEMY_MAX_EXPLOSION_TIME,2) + 0.001) ),
								1 * (0.7 * (rndz * ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS - ENEMY_MIN_EXPLOSION_RADIUS) * rndz * pow(explosionTime,2)/pow(ENEMY_MAX_EXPLOSION_TIME,2) + 0.001) )		);
				}
				glColor3f(0.8 + 0.1*yellow + 0.1*red, 0.8 + 0.1*yellow, 0);
				for (i = 0; i<20; i++)
				{
					rndx = sb[i+6] * sa[i+6];
					rndy = cb[i+6];
					rndz = sb[i+6] * ca[i+6];
					glVertex3f(  1 * (0.3 * (rndx * ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS - ENEMY_MIN_EXPLOSION_RADIUS) * rndx * pow(explosionTime,2)/pow(ENEMY_MAX_EXPLOSION_TIME,2) + 0.001) ),
								0.5 * (0.3 * (rndy * ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS - ENEMY_MIN_EXPLOSION_RADIUS) * rndy * pow(explosionTime,2)/pow(ENEMY_MAX_EXPLOSION_TIME,2) + 0.001) ),
								1 * (0.3 * (rndz * ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS - ENEMY_MIN_EXPLOSION_RADIUS) * rndz * pow(explosionTime,2)/pow(ENEMY_MAX_EXPLOSION_TIME,2) + 0.001) )		);
				}
			glEnd();
		glTranslated(-x,-y,-z);
	}
}