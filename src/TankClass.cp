//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			TankClass.cp																															
////	Author:			Ed Martin
////	Description:	Holds the TankClass Object														
////
////	Contents:		
////					1) Private variables
////						a) Position, Velocity, Turret and Track Direction, Health, Lives
////						b) State and turret cannon type
////						c) Gameplay counters
////						d) Animation counters
////						e) Key and Mouse Control variables
////
////					2) Constructor, Destructor and Restart / Init
////
////					3) Set Functions
////						a) Position, Velocity, Turret and Track Direction, Health, Lives
////						b) State and turret cannon type
////						c) Key and Mouse Control variables
////
////					4) Get Functions
////						a) Position, Velocity, Turret and Track Direction, Health, Lives
////						b) State and turret cannon type
////						c) Key and Mouse Control variables
////
////					5) Update Function
////						a) Update Animation and Gameplay counters
////						b) Check if the Player has died and exploded. If lives are left then restart, else go to start screen.
////						c) Update the players motion if still alive, depending on state of keyboard and mouse
////							- Forward and Backward motion of tank tracks (dependant on KeyUp and KeyDown)
////							- Left and Right motion of tank tracks (dependant on KeyLeft and KeyRight)
////							- Left, Right, Up and Down motion of the Turret (dependant on MouseY and MouseX and MOUSE_SENSETIVITY)
////							- Shoot in direction of turret (dependant on KeyShoot and if the timeToFire counter has finished its
////							  decremental count from maxtimetofire to zero)
////
////					6) Draw Function
////						a) If tank is flashing (freshly born, picked up an object or hit by a bullet), then randomise the additive 
////						   whiteness colour ExtraWhiteness
////						b) Draw the 2 side tracks
////						c) Draw the main tank body rectangle
////						d) Draw the turret tower
////						e) Draw the bubble of the turret tower
////						f) Draw the turret barrels depending on turretType
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "GameConstants.cp"

#ifndef GL
	#define GL
	#include "Platform.h"
	#include "GLConstants.cp"
	#include "GLGameObjectDrawFunctions.cp"
	#include "GLTextures.cp"
#endif

#ifndef EVENTS
	#define EVENTS
	float events[10*40];
    void Event(int description, float a, float b, float c, float d, float e, float f, float g, float h, float i) {}
#endif

class TankClass
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					1) Private variables
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	private:
	
//// a) Position, Velocity, Turret and Track Direction, Health, Lives
				float x, z;
				float speed, turnspeed;
				float trackDirection, turretRightLeft, turretUpDown;
				int health, lives;
	
//// b) State and turret cannon type
				int state, turretType;

//// c) Gameplay counters
				float timeToFire, maxtimetofire;

//// d) Animation counters
				int explosionTime;
				float timeToFlash;	

//// e) Key and Mouse Control variables
				int KeyUp, KeyDown, KeyLeft, KeyRight, KeyShoot;
				int MouseX, MouseY;
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Constructor, Destructor and Restart / Init
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	public:	
				TankClass					(float newX, 
											 float newZ					);
				virtual ~TankClass();
				
				void  ReStart				(float newX, 
											 float newZ					);
				
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					3) Set Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret and Track Direction, Health, Lives

				void  Setx					(float	newX				);
				void  Setz					(float	newZ				);
				void  SetSpeed				(float	newSpeed			);
				void  SetturretRightLeft	(float	newTurretRightLeft	);
				void  SetturretUpDown		(float	newTurretUpDown		);
				void  SettrackDirection		(float	newTrackDirection	);
				void  SetHealth				(int	newHealth			);
				void  DecreaseHealth		(int	newHealth			);	
				void  SetLives				(int	newLives			);
				
//// b) State and turret cannon type

				void  SetType				(int	newType);
				
//// c) Key and Mouse Control variables

				void  SetKeyUp				(int	newState			);
				void  SetKeyDown			(int	newState			);
				void  SetKeyLeft			(int	newState			);	
				void  SetKeyRight			(int	newState			);
				void  SetKeyShoot			(int	newState			);
					
				void  SetMouseButton		(int	newState			);
				void  SetMousemove			(int	newX, 
											 int	newY				);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) Get Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret and Track Direction, Health, Lives

				float Getx					(void);
				float Getz					(void);
				float GetSpeed				(void);
				float GetturretRightLeft	(void);
				float GetturretUpDown		(void);
				float GettrackDirection		(void);
				int	  GetHealth				(void);
				int	  GetLives				(void);
				
//// b) State and turret cannon type

				int GetState				(void);
				
//// c) Key and Mouse Control variables

				int   GetKeyUp				(void);
				int   GetKeyDown			(void);
				int   GetKeyLeft			(void);
				int   GetKeyRight			(void);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					5) Update Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				void  Update();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					6) Draw Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				void  DrawTank();
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Constructor, Destructor and Restart / Init
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TankClass::TankClass(float newX, float newZ) 
{	
	// Position, Velocity, Health, Lives
	x =					newX;
	z =					newZ;
	speed =				0;
	turnspeed =			0;		
	health =			0;
	lives =				2;
	trackDirection =	270 -45;
	turretRightLeft =	0;
	turretUpDown =		0;
	
	// State and turret cannon type
	state =				ACTIVE;
	turretType =		SINGLE_FIRE;

	// Gameplay counters
	timeToFire =		0;
	maxtimetofire =		PLAYER_MAX_TIME_TO_FIRE;

	// Animation counters
	timeToFlash =		0;
	explosionTime =		0;

	// Key and Mouse Control variables
	KeyUp = KeyDown = KeyLeft = KeyRight = KeyShoot = 0;
}

TankClass::~TankClass()
{}

void TankClass::ReStart(float newX, float newZ)
{
	// Position, Velocity, Health, Lives
	x =					newX;
	z =					newZ;
	speed =				0;
	turnspeed =			0;		
	health =			PLAYER_MAX_HEALTH;
	trackDirection =	235;
	turretRightLeft =	270;
	turretUpDown =		0;
	
	// State and turret cannon type
	state =				ACTIVE;
	turretType =		SINGLE_FIRE;

	// Gameplay counters
	timeToFire =		0;
	maxtimetofire =		PLAYER_MAX_TIME_TO_FIRE;

	// Animation counters
	timeToFlash =		0;
	explosionTime =		0;

	// Key and Mouse Control variables
	KeyUp = KeyDown = KeyLeft = KeyRight = KeyShoot = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					3) Set Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret and Track Direction, Health, Lives

void TankClass::Setx				(float newX)				{	x = newX;								}
void TankClass::Setz				(float newZ)				{	z = newZ;								}

void TankClass::SetSpeed			(float newSpeed)			{	speed = newSpeed;						}

void TankClass::SettrackDirection	(float newtrackDirection)	{	trackDirection = newtrackDirection;		}
void TankClass::SetturretRightLeft	(float newturretRightLeft)	{	turretRightLeft = newturretRightLeft;	}											
void TankClass::SetturretUpDown		(float newturretUpDown)		{	
																	turretUpDown = newturretUpDown;	
																	if (turretUpDown < 0) turretUpDown = 0;
																	if (turretUpDown > 90) turretUpDown = 90;
																}
																
void TankClass::SetHealth			(int   newHealth)			{	health = newHealth;						}

void TankClass::DecreaseHealth		(int   newHealth)			{	
																	health -= newHealth;
																	timeToFlash = PLAYER_MAX_TIME_TO_FLASH;
																	if (health <= 0)	
																	{
																		state = EXPLODING;
																		health = 0;
																		timeToFlash = 2 * PLAYER_MAX_TIME_TO_FLASH;

																	}
																	else if (health > PLAYER_MAX_HEALTH) 
																		health = PLAYER_MAX_HEALTH;
																}
						
void TankClass::SetLives			(int   newLives)			{	lives = newLives;						}
	
//// b) State and turret cannon type

void TankClass::SetType				(int   newType)				{	
																	if (turretType == SINGLE_FIRE) 
																		turretType = newType;	
																	else if ( ((turretType == RAPID_FIRE) && (newType == DUAL_FIRE)) ||
																			  ((turretType == DUAL_FIRE)  && (newType == RAPID_FIRE)) )
																		turretType = RAPID_DUAL_FIRE;
	
																	if (newType == RAPID_FIRE) maxtimetofire = maxtimetofire/2;
																	timeToFlash = 2 * PLAYER_MAX_TIME_TO_FLASH;
																}
																
//// c) Key and Mouse Control variables

void TankClass::SetKeyUp			(int   newState)			{	KeyUp = newState;						}
void TankClass::SetKeyDown			(int   newState)			{	KeyDown = newState;						}	
void TankClass::SetKeyLeft			(int   newState)			{	KeyLeft = newState;						}
void TankClass::SetKeyRight			(int   newState)			{	KeyRight = newState;					}
void TankClass::SetKeyShoot			(int   newState)			{	KeyShoot = newState;					}
void TankClass::SetMouseButton		(int   newState)			{	KeyShoot = newState;					}
void TankClass::SetMousemove		(int   newX, 
									 int   newY)				{   
																	MouseX = newX;	
																	MouseY = newY;							
																}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) Get Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret and Track Direction, Health, Lives

	float TankClass::Getx(void)					{	return x;				}
	float TankClass::Getz(void)					{	return z;				}
	float TankClass::GetSpeed(void)				{	return speed;			}
	float TankClass::GetturretRightLeft(void)	{	return turretRightLeft;	}
	float TankClass::GetturretUpDown(void)		{	return turretUpDown;	}
	float TankClass::GettrackDirection(void)	{	return trackDirection;	}
	int	  TankClass::GetHealth(void)			{	return health;			}
	int	  TankClass::GetLives(void)				{	return lives;			}

//// b) State and turret cannon type

	int TankClass::GetState(void)				{	return state;			}

//// c) Key and Mouse Control variables

	int TankClass::GetKeyUp(void)				{	return KeyUp;			}
	int TankClass::GetKeyDown(void)				{	return KeyDown;			}
	int TankClass::GetKeyLeft(void)				{	return KeyLeft;			}
	int TankClass::GetKeyRight(void)			{	return KeyRight;		}
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					5) Update Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TankClass::Update()
{

//// a) Update Animation and Gameplay counters

	if (timeToFlash>=0)		timeToFlash--;
	if (timeToFire>=0)		timeToFire --;
	if (state == EXPLODING) explosionTime ++;
	
//// b) Check if the Player has died and exploded. If lives are left then restart, else go to start screen.

	if (explosionTime > ENEMY_MAX_EXPLOSION_TIME)
	{
		lives--;				
		if (lives >=0 ) 
		{
			if (RunMode == TUBE)	
				ReStart((MAP_BLOCK_SIZE) * 51, (MAP_BLOCK_SIZE) * 111);
			else 
				ReStart((MAP_BLOCK_SIZE) * 27, (MAP_BLOCK_SIZE) * 4.5);
		}
		else RunMode = STARTSCREEN;
	}
	
//// c) Update the players motion if still alive, depending on state of keyboard and mouse
////    - Forward and Backward motion of tank tracks (dependant on KeyUp and KeyDown)

	if ((KeyUp==1) && (health > 0))				
	{	
		if (speed < TOP_SPEED)	 speed += 0.3*TOP_ACCELERATION;
		if (speed < TOP_SPEED*0.5) speed += 0.7*TOP_ACCELERATION;
		if (speed < 0)
		{
			speed += 2*TOP_ACCELERATION;
			if (speed >0) speed = 0;
		}
	}
	
	if ((KeyDown==1) && (health > 0))						
	{	
		if (speed > -TOP_SPEED)	  speed -= 0.3*TOP_ACCELERATION;
		if (speed > -TOP_SPEED*0.5) speed -= 0.7*TOP_ACCELERATION;
		if (speed > 0) 
		{
			speed -= 2*TOP_ACCELERATION;
			if (speed < 0 ) speed = 0;
		}
		
	}
	
	if ((KeyDown==0) && (KeyUp==0))
	{
		if (speed > 0 ) 
		{
			speed -= 2*TOP_ACCELERATION;
			if (speed < 0.1*TOP_SPEED ) speed = 0;
		}
		if (speed < 0 ) 
		{
			speed += 2*TOP_ACCELERATION;
			if (speed > -0.1*TOP_SPEED) speed = 0;
		}
	}
	
	x = x + sin( 2 * M_PI * trackDirection / 360) * speed;
	z = z + cos( 2 * M_PI * trackDirection / 360) * speed;
	
//// - Left and Right motion of tank tracks (dependant on KeyLeft and KeyRight)

	if		(KeyLeft == 1)		turnspeed = TOP_TURN_SPEED;
	else if (KeyRight==1)		turnspeed = -TOP_TURN_SPEED;
	else						turnspeed = 0;

	trackDirection = trackDirection + turnspeed;

//// - Left, Right, Up and Down motion of the Turret (dependant on MouseY and MouseX and MOUSE_SENSETIVITY)
		
	if (MouseY!=0)
	{
		SetturretUpDown( turretUpDown + MouseY * MOUSE_SENSETIVITY);
		MouseY = 0;
	}
		
	if (MouseX!=0)
	{
		SetturretRightLeft( turretRightLeft + MouseX * MOUSE_SENSETIVITY);
		MouseX = 0;
	}

		
//// - Shoot in direction of turret (dependant on KeyShoot and if the timeToFire counter has finished it's decremental count from maxtimetofire to zero)
		
	if ((KeyShoot == 1) && (timeToFire<0) && (health>0)) 
	{
		float rl = 2 * M_PI * GetturretRightLeft() / 360 ;
		float ud = 2 * M_PI * GetturretUpDown() / 360 ;
	
		if (timeToFire<=0)
		{	
			if ((turretType == SINGLE_FIRE) || (turretType == RAPID_FIRE))
			{
				// For a single bullet, a NEW_BULLET_EVENT is called using the following format:
				// Event(NEW_BULLET_EVENT,	x_initial,														y_initial,								z_initial,
				//							x_velocity,														y_velocity,								z_velocity,
				//							y_acceleration,													bullet speed,							type of bullet
				
				Event(	NEW_BULLET_EVENT,	x + sin(rl) * cos(ud) * tankbarrellength,						0.7 + tankbarrellength * sin(ud),		z + cos(rl) * cos(ud) * tankbarrellength, 
											PLAYER_BULLET_SPEED * cos(ud) * sin(rl),						PLAYER_BULLET_SPEED * sin(ud),			PLAYER_BULLET_SPEED * cos(ud) * cos(rl),	
											-PLAYER_BULLET_SPEED/50,										PLAYER_BULLET_SPEED,					PLAYER_BULLET);
			}
			else if ((turretType == DUAL_FIRE) || (turretType == RAPID_DUAL_FIRE))
			{
				// For a DUAL_FIRE bullet, two NEW_BULLET_EVENTs are called, one for the left barrel and one for the right, again using the following format:
				// Event(NEW_BULLET_EVENT,	x_initial,														y_initial,								z_initial,
				//							x_velocity,														y_velocity,								z_velocity,
				//							y_acceleration,													bullet speed,							type of bullet

				Event(	NEW_BULLET_EVENT,	x + sin(rl) * cos(ud) * tankbarrellength  + cos(rl) * 0.3,		0.7 + tankbarrellength * sin(ud),		z + cos(rl) * cos(ud) * tankbarrellength - sin(rl)*0.3, 
											PLAYER_BULLET_SPEED * cos(ud) * sin(rl),						PLAYER_BULLET_SPEED * sin(ud),			PLAYER_BULLET_SPEED * cos(ud) * cos(rl),	
											-PLAYER_BULLET_SPEED/50,										PLAYER_BULLET_SPEED,					PLAYER_BULLET);
											
				Event(	NEW_BULLET_EVENT,	x + sin(rl) * cos(ud) * tankbarrellength - cos(rl) * 0.3,		0.7 + tankbarrellength * sin(ud),		z + cos(rl) * cos(ud) * tankbarrellength + sin(rl)*0.3, 
											PLAYER_BULLET_SPEED * cos(ud) * sin(rl),						PLAYER_BULLET_SPEED * sin(ud),			PLAYER_BULLET_SPEED * cos(ud) * cos(rl),	
											-PLAYER_BULLET_SPEED/50,										PLAYER_BULLET_SPEED,					PLAYER_BULLET);				
			}
			timeToFire = maxtimetofire;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					6) Draw Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TankClass::DrawTank()
{ 
	
//// a) If tank is flashing (freshly born, picked up an object or hit by a bullet), then randomise the additive whiteness colour ExtraWhiteness

	float ExtraWhiteness;
	if (timeToFlash > 0) 
		ExtraWhiteness = timeToFlash * (0.5 + 0.5 * (float(random()) / float(RAND_MAX)) )/ PLAYER_MAX_TIME_TO_FLASH; 
	else ExtraWhiteness = 0;
		
	int i;
		
	glTranslated(x,0,z);
		glRotated(trackDirection, 0, 1, 0);
			
//// b) Draw the 2 side tracks
			
			for (GLfloat RIGHTorLEFT = -1; RIGHTorLEFT<= 1; RIGHTorLEFT = RIGHTorLEFT + 2)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, wall2);	
				glEnable(GL_TEXTURE_2D);
				glBegin(GL_QUAD_STRIP);
					glColor3f(0.0 + 1.0 * ExtraWhiteness, 0.0 + 1.0 * ExtraWhiteness, 0.6 + 0.4 * ExtraWhiteness); 
					for (i=0; i<18; i++)		
					{	
						glVertex3f( RIGHTorLEFT * tanktracks[3*i] , tanktracks[3*i+1] , tanktracks[3*i+2]); 
					}
				glEnd();
					
				glBegin(GL_QUAD_STRIP);
					glColor3f(0.0 + 1.0 * ExtraWhiteness, 0.0 + 1.0 * ExtraWhiteness, 0.5 + 0.5 * ExtraWhiteness); 
					for (i=0; i<18; i++)		
					{	
						glVertex3f( RIGHTorLEFT * tanktracks[3*i] , tanktracks[3*i+1] * 0.8 + 0.05, tanktracks[3*i+2] * 0.9); 
					}
				glEnd();
				
				glBegin(GL_QUAD_STRIP);
					glColor3f(0.0 + 1.0 * ExtraWhiteness, 0.0 + 1.0 * ExtraWhiteness, 0.2 + 0.8 * ExtraWhiteness); 
					for (i=1; i<18; i = i + 2)		
					{	
						glVertex3f( RIGHTorLEFT * tanktracks[3*i] , tanktracks[3*i+1] , tanktracks[3*i+2]); 
						glVertex3f( RIGHTorLEFT * tanktracks[3*i] , tanktracks[3*i+1] * 0.8 + 0.05, tanktracks[3*i+2] * 0.9); 
					}
				glEnd();
				glBegin(GL_QUAD_STRIP);
					glColor3f(0.0 + 1.0 * ExtraWhiteness, 0.0 + 1.0 * ExtraWhiteness, 0.2 + 0.8 * ExtraWhiteness); 
					for (i=0; i<18; i = i + 2)		
					{	
						glVertex3f( RIGHTorLEFT * tanktracks[3*i] , tanktracks[3*i+1] , tanktracks[3*i+2]); 
						glVertex3f( RIGHTorLEFT * tanktracks[3*i] , tanktracks[3*i+1] * 0.8 + 0.05, tanktracks[3*i+2] * 0.9); 
					}
				glEnd();
						
//// c) Draw the main tank body rectangle

				glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, steel2);	
				glEnable(GL_TEXTURE_2D);
				glBegin(GL_QUAD_STRIP);
					glColor3f(0.4 + 0.6 * ExtraWhiteness, 0.4 + 0.6 * ExtraWhiteness, 0.7 + 0.3 * ExtraWhiteness); 
					for (i=0; i<10; i++)		
					{	
						glVertex3f( tankbody[3*i] , tankbody[3*i+1] , tankbody[3*i+2]); 
					}	
				glEnd();
			}
		glRotated(-trackDirection, 0, 1, 0);
	glTranslated(-x, 0, -z);
	
//// d) Draw the turret tower

	glDisable(GL_TEXTURE_2D);
	glTranslated(x, 0, z);
		glRotated(trackDirection, 0, 1, 0);
			glBegin(GL_QUAD_STRIP);
				glColor3f(0.0 + 0.6 * ExtraWhiteness, 0.0 + 0.6 * ExtraWhiteness, 0.4 + 0.6 * ExtraWhiteness); 
				for (i=0; i<16; i++)		
				{	
					glVertex3f( tanktower[3*i] , tanktower[3*i+1] , tanktower[3*i+2]); 
				}	
			glEnd();
		glRotated(-trackDirection, 0, 1, 0);
	glTranslated(-x, 0, -z);

//// e) Draw the bubble of the turret tower

	glEnable(GL_TEXTURE_2D);
	glTranslated( x - 0.2 * sin(2 * M_PI / 360 * trackDirection), 0.7, z - 0.2 * cos(2 * M_PI / 360 * trackDirection));
		glRotated( turretRightLeft + 90, 0, 1, 0);
			glutSolidSphere( 0.39, 20, 50);  
		glRotated( -turretRightLeft - 90, 0, 1, 0);	
	glTranslated( -x + 0.2 * sin(2 * M_PI / 360 * trackDirection), -0.7, -z + 0.2 * cos(2 * M_PI / 360 * trackDirection));

//// f) Draw the turret barrels depending on turretType

	glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, steel1);	
	glEnable(GL_TEXTURE_2D);
	glTranslated( x - 0.2 * sin(2 * M_PI / 360 * trackDirection), 0.8, z - 0.2 * cos(2 * M_PI / 360 * trackDirection));
		glRotated(turretRightLeft,0,1,0);
			glRotated(turretUpDown,-1,0,0);
					
				// N is the number of points in the circles that make up the cyllindrical barrel
				
				int N = 90;
				float angle, tempX, tempY;
					
				float silenserlength;
				if ((turretType == SINGLE_FIRE) || (turretType == DUAL_FIRE)) 
				{	silenserlength = 0.2;	}
				else  
				{	silenserlength = 0.4;	}
										
				// If required draw 1 turret barrel

				if ((turretType == SINGLE_FIRE) || (turretType == RAPID_FIRE)) 
				{
					glBegin(GL_QUAD_STRIP);
						glColor3f(0.6 + 0.4 * ExtraWhiteness, 0.6 + 0.4 * ExtraWhiteness, 0.6 + 0.4 * ExtraWhiteness); 
						for (i=0; i<=N; i++)		
						{	
							angle =2*M_PI / N + 2*M_PI/N * i;
							
							tempX = sin( angle ) * 0.2;
							tempY = cos( angle ) * 0.2;
				
							glVertex3f( 0.6* tempX, 0.6* tempY, 0);
							glVertex3f( tempX,			tempY,			tankbarrellength * ( 1 - 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire)));
						}
					glEnd();						
					glDisable(GL_TEXTURE_2D);
					glBegin(GL_QUAD_STRIP);
						glColor3f(0.0 + 0.4 * ExtraWhiteness, 0.0 + 0.4 * ExtraWhiteness, 0.2 + 0.8 * ExtraWhiteness); 
						for (i=0; i<=N; i++)		
						{	
							angle = 2 * M_PI / N + 2 * M_PI/N * i;
					
							tempX = sin( angle ) * 0.2;
							tempY = cos( angle ) * 0.2;
			
							glVertex3f( 1.1 * tempX,	1.3 * tempY,	tankbarrellength * ( 0.95 - 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire) - silenserlength) );
							glVertex3f( 1.4 * tempX,	1.4 * tempY,	tankbarrellength * ( 0.95 - 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire)				 ) );
						}
					glEnd();
				}

				// If required draw 2 turret barrels

				else if ((turretType == DUAL_FIRE) || (turretType == RAPID_DUAL_FIRE))
				{
					glTranslated(0.16,0,0);
						glBegin(GL_QUAD_STRIP);
							glColor3f(0.6 + 0.4 * ExtraWhiteness, 0.6 + 0.4 * ExtraWhiteness, 0.6 + 0.4 * ExtraWhiteness); 
							for (i=0; i<=N; i++)		
							{	
								angle =2*M_PI / N + 2*M_PI/N * i;
					
								tempX = sin( angle ) * 0.18;
								tempY = cos( angle ) * 0.18;
			
								glVertex3f( 0.7* tempX, 0.7* tempY, 0);
								glVertex3f( tempX, tempY, tankbarrellength * ( 1 - 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire)));
							}
						glEnd();							
						glDisable(GL_TEXTURE_2D);
						glBegin(GL_QUAD_STRIP);
							glColor3f(0.0 + 0.4 * ExtraWhiteness, 0.0 + 0.4 * ExtraWhiteness, 0.2 + 0.8 * ExtraWhiteness); 
							for (i=0; i<=N; i++)		
							{	
								angle = 2 * M_PI / N + 2 * M_PI/N * i;
						
								tempX = sin( angle ) * 0.18;
								tempY = cos( angle ) * 0.18;
			
								glVertex3f( 1.1 * tempX,	1.3 * tempY,	tankbarrellength * (0.95 - 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire) - silenserlength) );
								glVertex3f( 1.4 * tempX,	1.4 * tempY,	tankbarrellength * (0.95 - 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire)					) );
							}	
						glEnd();
					glTranslated(-0.32, 0, 0);
						glEnable(GL_TEXTURE_2D);
						glBegin(GL_QUAD_STRIP);
							glColor3f(0.6 + 0.4 * ExtraWhiteness, 0.6 + 0.4 * ExtraWhiteness, 0.6 + 0.4 * ExtraWhiteness); 
							for (i=0; i<=N; i++)		
							{	
								angle =2*M_PI / N + 2*M_PI/N * i;
				
								tempX = sin( angle ) * 0.18;
								tempY = cos( angle ) * 0.18;
				
								glVertex3f( 0.7 * tempX,	0.7 * tempY,	0);
								glVertex3f( tempX,			tempY,			tankbarrellength * ( 1 - 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire)));
							}
						glEnd();
							
						glDisable(GL_TEXTURE_2D);
						glBegin(GL_QUAD_STRIP);
							glColor3f(0.0 + 0.4 * ExtraWhiteness, 0.0 + 0.4 * ExtraWhiteness, 0.2 + 0.8 * ExtraWhiteness); 
							for (i=0; i<=N; i++)		
							{	
								angle = 2 * M_PI / N + 2 * M_PI/N * i;
							
								tempX = sin( angle ) * 0.18;	
								tempY = cos( angle ) * 0.18;
			
								glVertex3f( 1.1 * tempX,	1.3 * tempY,	tankbarrellength * ( 0.95 - 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire) - silenserlength ));
								glVertex3f( 1.4 * tempX,	1.4 * tempY,	tankbarrellength * ( 0.95 - 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire)));
							}
						glEnd();
					glTranslated(0.16, 0, 0);
				}
			glRotated(-turretUpDown, -1, 0,	0);
		glRotated(-turretRightLeft, 0, 1, 0);
	glTranslated(-x + 0.2 * sin(2 * M_PI / 360 * trackDirection), - 0.8, - z + 0.2 * cos(2 * M_PI / 360 * trackDirection));		
	glDisable(GL_TEXTURE_2D);
}