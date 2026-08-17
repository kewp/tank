//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			TubeClass.cp																															
////	Author:			Ed Martin
////	Description:	Holds the TubeClass Objects
////
////	Contents:		
////					1) Private variables
////						a) Position, Velocity, Turret and Track Direction, Health, Lives
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
////						b) Step through the the Tube's State cycle:
////						   - TUBE_STATE_APROACHING	- Tube aproaches arena while camera circles player. Player is disabled.
////						   - TUBE_STATE_NEW_TURRETS	- New Turrets are setup to fire at player, A random object is also put 
////													into play (RAPID_FIRE, HEALTH_PACK etc).
////						   - TUBE_STATE_AIMING		- Tube aims at Player.
////						   - TUBE_STATE_OPENING_DOORS	- Tube opens doors and windows
////						   - TUBE_STATE_OPEN_FIRE		- Tube opens fire and is vulnerable
////						   - TUBE_STATE_CLOSING_DOORS	- Tube closes doors and windows - the cycle starts again...
////						c) Update the tube's motion. Unless it is aproaching, the tube always blocks the player
////
////
////					6) Draw Function
////						a) If tube is flashing (freshly born or hit by a bullet), then randomise the additive whiteness colour ExtraWhiteness
////						b) Draw the Tube Insignia on the side of the tube
////						c) Draw the Front and back of the tube (where the driver would sit)
////						d) Draw the Floor
////						e) Draw the Wheels
////						f) Draw the White Shell, if tube is not exploding
////						g) Draw the blue strip at the base
////						h) Draw the black strip on the top
////						i) Draw the doors using the animation variable doorOpenRatio
////						j) Draw the door windows using the animation variable doorOpenRatio
////						k) Draw the main windows using the animation variable doorOpenRatio
////						l) draw the multicoloured weak spot behind the windows
////						m) draw the Gun turrets inside the Tube
////						n) if the tube is exploding, draw the explosion
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <math.h>

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
	float events[0];
    void Event(int description, float a, float b, float c, float d, float e, float f, float g, float h, float i) {}
#endif

class TubeClass
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					1) Private variables
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	private:

//// a) Position, Velocity, Turret Direction, Health

				float x_current, x_objective;
				float y_current, y_objective;
				float z_current, z_objective;
				float turretRightLeft, turretUpDown;
				int   health;

//// b) State and turret cannon type

				int   state;

//// c) Gameplay counters

				float timeToFire, maxtimeToFire;

//// d) Animation counters

				float explosionTime;
				int   timeToFlash, maxtimeToFlash;
				int   cycle_timer;
				int   health_blink, health_blink_count;	
				float doorOpenAngle, doorOpenRatio;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Constructor, Destructor and Restart / Init
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	public:	
				TubeClass				(void);
				virtual ~TubeClass();
				void InitTube			(float Turretx, 
										 float Turrety, 
										 float Turretz);
									
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					3) Set Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret and Track Direction, Health, Lives

				void  DecreaseHealth	(int   newHealth);	

//// b) State and turret cannon type

				void  SetState			(int   newState);
				
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) Get Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret and Track Direction, Health, Lives

				float Getx				(void);
				float Gety				(void);
				float Getz				(void);

//// b) State and turret cannon type

				int  GetState			(void);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					5) Update Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				void Update(float Playerx, float Playerz);	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					6) Draw Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				void DrawTube(void);

};
	
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Constructor, Destructor and Restart / Init
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TubeClass::TubeClass(void) {}
TubeClass::~TubeClass(void) {}
	
void TubeClass::InitTube(float Turretx, float Turrety, float Turretz)
{
	doorOpenRatio = 0;
	x_objective = Turretx;
	y_objective = Turrety;
	z_objective = Turretz;
		
	health = 100 * TURRET_MAX_HEALTH;
	timeToFire = ENEMY_MAX_TIME_TO_FIRE;
	timeToFlash = ENEMY_MAX_TIME_TO_FLASH;
	turretRightLeft  = 180;
	turretUpDown = -20;
	explosionTime = 0;
	cycle_timer = 0;
	state = TUBE_STATE_APROACHING;
	x_current = Turretx;
	y_current = Turrety;
	z_current = Turretz + 1000 * MAP_BLOCK_SIZE;

	health_blink_count = 0;
	health_blink = 0;
	
	doorOpenAngle = -90;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					3) Set Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret and Track Direction, Health, Lives
	
void TubeClass::DecreaseHealth(int H)	
{
	CameraShakeTime = 0.5 * MAX_CAMERA_SHAKE_TIME;
	health -= H;
	timeToFlash = ENEMY_MAX_TIME_TO_FLASH;
	
	// if the tube is dead, then make it explode, create random shrapnel
	// and change the Runmode to END - the camera will circle the scene from above
	if (health <=0 )
	{
		state = EXPLODING;
		explosionTime = 0;
		timeToFlash = 0;
		CameraShakeTime = 2.5*MAX_CAMERA_SHAKE_TIME;
																
		float ud,rl;
		float r;
		int i;	
		for (i=0; i<10; i++)
		{
			r =  0.5 - float(random()) / float(RAND_MAX);
			for (int j = 1; j<10; j++)
			{
																
				ud =  M_PI * float(random()) / float(RAND_MAX);
				rl = 2 * M_PI * float(random()) / float(RAND_MAX);	
				
				// For a single bullet/shrapnel, a NEW_BULLET_EVENT is called using the following format:
				// Event(NEW_BULLET_EVENT,	x_initial,								y_initial,						z_initial,
				//							x_velocity,		
				//							y_velocity,		
				//							z_velocity,
				//							y_acceleration,							bullet speed,					type of bullet

				Event(	NEW_BULLET_EVENT,	x_current + sin(rl) * cos(ud) * 20,		y_current + 20 * sin(ud),		z_current + r * 10  + cos(rl) * cos(ud) * 20, 
											2*float(random()) / float(RAND_MAX) * ENEMY_BULLET_SPEED * cos(ud) * sin(rl),	
											2*float(random()) / float(RAND_MAX) * ENEMY_BULLET_SPEED * sin(ud),		
											2*float(random()) / float(RAND_MAX) * ENEMY_BULLET_SPEED * cos(ud) * cos(rl),	
											-ENEMY_BULLET_SPEED/50,					ENEMY_BULLET_SPEED,				SHRAPNEL);
			}
		}
			
		CurrentCameraDistance = 7 * MAP_BLOCK_SIZE;
		ObjectiveCameraDistance = 20 * MAP_BLOCK_SIZE;
		RunMode = END;															
	}
}

//// b) State and turret cannon type

void TubeClass::SetState(int newState)		{	state = newState;		}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) Get Functions
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// a) Position, Velocity, Turret and Track Direction, Health, Lives
	
float TubeClass::Getx(void)			{	return x_current;		}
float TubeClass::Gety(void)			{	return y_current;		}
float TubeClass::Getz(void)			{	return z_current;		}
	
//// b) State and turret cannon type

int  TubeClass::GetState(void)				{	return state;		}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					5) Update Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TubeClass::Update(float Playerx, float Playerz)	
{

//// a) Update Animation and Gameplay counters
		
	cycle_timer++;
	
	if (state == EXPLODING)							
		explosionTime++;
	if (explosionTime > ENEMY_MAX_EXPLOSION_TIME)	
		state = SMOKING;  
		
	timeToFlash--;
	if (timeToFlash<0) 
		timeToFlash = 0;
	
	// The tube flashes if it's health is < 80%, and flashes faster the weaker it is.
	if (health < 0.8 * 100 * TURRET_MAX_HEALTH)
	{
		health_blink_count += 1;
			
		if (health_blink_count > 50*health/100/TURRET_MAX_HEALTH + 10)
		{
				health_blink = 1;
				health_blink_count = 0;
		}
		if (health_blink_count > 50*health/100/TURRET_MAX_HEALTH  )
		{
				health_blink = 0;
		}
	}

//// b) Step through the the Tube's State cycle:
////    - TUBE_STATE_APROACHING		- Tube aproaches arena while camera circles player. Player is disabled.

	if (state == TUBE_STATE_APROACHING)
	{
		//if  (Playerz < 96 * MAP_BLOCK_SIZE)										z_objective = Playerz + 9;
		if ((Playerz < 102 * MAP_BLOCK_SIZE))										z_objective = Playerz + 2.5 * MAP_BLOCK_SIZE;
		if ((Playerz > 102 * MAP_BLOCK_SIZE) && (Playerz < 108 * MAP_BLOCK_SIZE))	z_objective = Playerz - 2.5 * MAP_BLOCK_SIZE;
		if  (Playerz > 108 * MAP_BLOCK_SIZE)										z_objective = Playerz - 9;
		if (cycle_timer > TUBE_APROACHING_TIME) {cycle_timer = 0; state++;}
	}

//// - TUBE_STATE_NEW_TURRETS		- New Turrets are setup to fire at player, A random object is also put into play (RAPID_FIRE, HEALTH_PACK etc).

	if (state == TUBE_STATE_NEW_TURRETS)
	{
		Event( NEW_TURRET_EVENT, 0,				 0,0,0,0,0,0,0,0);
		cycle_timer = 0; 
		state++;
	}

//// - TUBE_STATE_AIMING			- Tube aims at Player.
	
	if (x_current < Playerx) turretRightLeft = 90;
	else turretRightLeft = -90;
	if (state == TUBE_STATE_AIMING)
	{
		if  (Playerz < 96 * MAP_BLOCK_SIZE)											z_objective = Playerz + 9;
		if ((Playerz > 96 * MAP_BLOCK_SIZE) && (Playerz < 102 * MAP_BLOCK_SIZE))	z_objective = Playerz + 2.5 * MAP_BLOCK_SIZE;
		if ((Playerz > 102 * MAP_BLOCK_SIZE) && (Playerz < 108 * MAP_BLOCK_SIZE))	z_objective = Playerz - 2.5 * MAP_BLOCK_SIZE;
		if  (Playerz > 108 * MAP_BLOCK_SIZE)										z_objective = Playerz - 9;
		if (cycle_timer > TUBE_AIMING_TIME) {cycle_timer = 0; state++;}
	}
	
//// - TUBE_STATE_OPENING_DOORS	- Tube opens doors and windows
	
	if (state == TUBE_STATE_OPENING_DOORS)
	{
		doorOpenAngle = 270 + 180 * cycle_timer / TUBE_OPENING_DOORS_TIME;
		doorOpenRatio = 0.5 + 0.5 * sin(2*M_PI/360*doorOpenAngle);
		if (cycle_timer> TUBE_OPENING_DOORS_TIME) 
		{
			cycle_timer = 0; 
			state++;
		}
	}
	
//// - TUBE_OPEN_FIRE		- Tube opens fire and is vulnerable

	if (state == TUBE_STATE_OPEN_FIRE)
	{
		turretUpDown = 85 - 40 * cycle_timer / TUBE_OPEN_FIRE_TIME;
		timeToFire++;
			
		float rl, ud;
		rl = 2*M_PI/360*turretRightLeft;
		ud = 2*M_PI/360*turretUpDown;
		
		float y_acc = -0.03;
		if (Dificulty_Level == EASY) y_acc = -0.03 * ENEMY_BULLET_SPEED;
		else if (Dificulty_Level == MEDIUM) y_acc = -0.040 * ENEMY_BULLET_SPEED;
		else if (Dificulty_Level == HARD) y_acc = -0.05 * ENEMY_BULLET_SPEED;
			
		if (timeToFire > TUBE_OPEN_FIRE_TIME / 200)
		if(int(200 * cycle_timer / TUBE_OPEN_FIRE_TIME) == (200 * cycle_timer / TUBE_OPEN_FIRE_TIME))
		{
			timeToFire = 0;
			Event(	NEW_BULLET_EVENT,	x_current + sin(rl) * cos(ud) * 1 * MAP_BLOCK_SIZE,					(4 + 1 * sin(ud)) * MAP_BLOCK_SIZE,					z_current + cos(rl) * cos(ud) * 1 * MAP_BLOCK_SIZE + 7.1*MAP_BLOCK_SIZE, 
										ENEMY_BULLET_SPEED * cos(ud) * sin(rl),	ENEMY_BULLET_SPEED * sin(ud),			ENEMY_BULLET_SPEED * cos(ud) * cos(rl),	
										y_acc, ENEMY_BULLET_SPEED, ENEMY_BULLET);
			Event(	NEW_BULLET_EVENT,	x_current + sin(rl) * cos(ud) * 1 * MAP_BLOCK_SIZE,					(4 + 1 * sin(ud)) * MAP_BLOCK_SIZE,					z_current + cos(rl) * cos(ud) * 1 * MAP_BLOCK_SIZE + 2.5*MAP_BLOCK_SIZE, 
										ENEMY_BULLET_SPEED * cos(ud) * sin(rl),	ENEMY_BULLET_SPEED * sin(ud),			ENEMY_BULLET_SPEED * cos(ud) * cos(rl),	
										y_acc, ENEMY_BULLET_SPEED, ENEMY_BULLET);
			Event(	NEW_BULLET_EVENT,	x_current + sin(rl) * cos(ud) * 1 * MAP_BLOCK_SIZE,					(4 + 1 * sin(ud)) * MAP_BLOCK_SIZE,					z_current + cos(rl) * cos(ud) * 1 * MAP_BLOCK_SIZE - 7.1*MAP_BLOCK_SIZE, 
										ENEMY_BULLET_SPEED * cos(ud) * sin(rl),	ENEMY_BULLET_SPEED * sin(ud),			ENEMY_BULLET_SPEED * cos(ud) * cos(rl),	
										y_acc, ENEMY_BULLET_SPEED, ENEMY_BULLET);
			Event(	NEW_BULLET_EVENT,	x_current + sin(rl) * cos(ud) * 1 * MAP_BLOCK_SIZE,					(4 + 1 * sin(ud)) * MAP_BLOCK_SIZE,					z_current + cos(rl) * cos(ud) * 1 * MAP_BLOCK_SIZE - 2.5*MAP_BLOCK_SIZE, 
										ENEMY_BULLET_SPEED * cos(ud) * sin(rl),	ENEMY_BULLET_SPEED * sin(ud),			ENEMY_BULLET_SPEED * cos(ud) * cos(rl),	
										y_acc, ENEMY_BULLET_SPEED, ENEMY_BULLET);
		}
		if (cycle_timer > TUBE_OPEN_FIRE_TIME) {cycle_timer = 0; state++; turretUpDown = -20;}
	}
	
//// - TUBE_STATE_CLOSING_DOORS	- Tube closes doors and windows - the cycle starts again...
	
	if (state == TUBE_STATE_CLOSING_DOORS)
	{
		doorOpenAngle = 90 + 180 * cycle_timer / TUBE_OPENING_DOORS_TIME;
		doorOpenRatio = 0.5 + 0.5 * sin(2*M_PI/360*doorOpenAngle);
		if (cycle_timer > TUBE_CLOSING_DOORS_TIME)
		{
			cycle_timer = 0;
			state = TUBE_STATE_NEW_TURRETS;
		}
	}

//// c) Update the tube's motion. Unless it is aproaching, the tube always blocks the player

	x_current += (x_objective - x_current)/10;
	y_current += (y_objective - y_current)/10;
	z_current += (z_objective - z_current)/10;
	
	if (state != TUBE_STATE_APROACHING)
	{
		if (Playerz > z_current + 10*MAP_BLOCK_SIZE) z_objective = z_current = Playerz - 10;
		if (Playerz < z_current - 10*MAP_BLOCK_SIZE) z_objective = z_current = Playerz + 10;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					6) Draw Function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TubeClass::DrawTube(void)
{
		// TRAIN_Y_RESOLUTION is the y distance between points on the parabola that make up the body of the tube
		float TRAIN_Y_RESOLUTION = 0.1;
		
//// a) If tube is flashing (freshly born or hit by a bullet), then randomise the additive whiteness colour ExtraWhiteness

		float ExtraWhiteness;
		if (timeToFlash > 0)
			ExtraWhiteness = timeToFlash * (0.5 + 0.5 * (float(random()) / float(RAND_MAX)) )/ ENEMY_MAX_TIME_TO_FLASH; 
		else if (health_blink ==1)
			ExtraWhiteness = 0.5 + 0.5 * (float(random()) / float(RAND_MAX)); 
		else ExtraWhiteness = 0;
		
		float dy;
		float angle, tempX, tempY;

		float weakspot_color;
		int i;
		
		int FRONTBACK, CARRIAGE, LEFTRIGHT;
		if (state != SMOKING)
		{
			for (FRONTBACK = -1; FRONTBACK <= 1; FRONTBACK+=2)
			{
			
//// b) Draw the Tube Insignia on the side of the tube

				glDisable(GL_TEXTURE_2D);
				glTranslated(x_current + FRONTBACK * 1.25 * MAP_BLOCK_SIZE, y_current + 2.65 * MAP_BLOCK_SIZE , z_current);
					glRotated(90, 0, 1, 0);
					glRotated(-9*FRONTBACK,1,0,0);
						glColor3f(0.6 + 0.4 * ExtraWhiteness, + 1.0 * ExtraWhiteness, + 1.0 * ExtraWhiteness);
						glBegin(GL_QUAD_STRIP);
							for (i = 1; i<=21; i++)
							{
								glVertex3f( 0.3*MAP_BLOCK_SIZE*sin( 2*M_PI * i / 20), 
											0.4*MAP_BLOCK_SIZE*cos( 2*M_PI * i / 20), 
											0);
								glVertex3f( 0.2*MAP_BLOCK_SIZE*sin( 2*M_PI * i / 20), 
											0.23*MAP_BLOCK_SIZE*cos( 2*M_PI * i / 20), 
											0);
							}
						glEnd();
						glColor3f( 1 * ExtraWhiteness, + 1.0 * ExtraWhiteness, 0.4 + 0.6 * ExtraWhiteness);
						glBegin(GL_QUAD_STRIP);
							glVertex3f( 0.35*MAP_BLOCK_SIZE,	0.1*MAP_BLOCK_SIZE,		0);
							glVertex3f( -0.35*MAP_BLOCK_SIZE,	0.1*MAP_BLOCK_SIZE,		0);
							glVertex3f( 0.35*MAP_BLOCK_SIZE,	-0.1*MAP_BLOCK_SIZE,		0);
							glVertex3f( -0.35*MAP_BLOCK_SIZE,	-0.1*MAP_BLOCK_SIZE,		0);				
						glEnd();
					glRotated(9*FRONTBACK,1,0,0);
					glRotated(-90, 0, 1, 0);
				glTranslated(-x_current - FRONTBACK * 1.25 * MAP_BLOCK_SIZE, -y_current - 2.65 * MAP_BLOCK_SIZE , -z_current);

//// c) Draw the Front and back of the tube (where the driver would sit)

				for (LEFTRIGHT = -1;	LEFTRIGHT <= 1;	LEFTRIGHT+=2)
				{
					glDisable(GL_TEXTURE_2D);
					glColor3f(0.6 + 0.4 * ExtraWhiteness, + 1.0 * ExtraWhiteness, + 1.0 * ExtraWhiteness);
					glTranslated( x_current, y_current, z_current);
						glRotated(90,0,1,0);
						glBegin(GL_QUAD_STRIP);
							for (dy = 1 * MAP_BLOCK_SIZE; dy < 7 * MAP_BLOCK_SIZE; dy+= TRAIN_Y_RESOLUTION)
							{
								if (dy > 7 * MAP_BLOCK_SIZE) dy = 7 * MAP_BLOCK_SIZE;
						
								glVertex3f(	LEFTRIGHT * 7.5 * MAP_BLOCK_SIZE, 
											dy, 
											-FRONTBACK*1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy))	);

								glVertex3f(	LEFTRIGHT * (7.5 * MAP_BLOCK_SIZE + 1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy))),
											dy,
											0);
							}
						glEnd();
						glRotated(-90,0,1,0);
					glTranslated( -x_current,-y_current,-z_current);
				}
			
				for (CARRIAGE = -1; CARRIAGE <=1;	CARRIAGE++)
				{			
					glTranslated(x_current, y_current, z_current + CARRIAGE*5*MAP_BLOCK_SIZE);
						glRotated(90,0,1,0);

						glRotated(180*FRONTBACK, 0, 1, 0);
							
//// d) Draw the Floor

							glDisable(GL_TEXTURE_2D);
						glColor3f(0.3 + 0.7 * ExtraWhiteness,0.3 + 0.7 * ExtraWhiteness,0.3 + 0.7 * ExtraWhiteness);
							glBegin(GL_QUAD_STRIP);
								glVertex3f( -4*MAP_BLOCK_SIZE,	1 * MAP_BLOCK_SIZE,		0);
								glVertex3f( 4*MAP_BLOCK_SIZE,		1 * MAP_BLOCK_SIZE,		0);
								glVertex3f( -2.5*MAP_BLOCK_SIZE,	1 * MAP_BLOCK_SIZE,		FRONTBACK * 1.5 * MAP_BLOCK_SIZE);
								glVertex3f( 2.5*MAP_BLOCK_SIZE,		1 * MAP_BLOCK_SIZE,		FRONTBACK * 1.5 * MAP_BLOCK_SIZE);				
							glEnd();
	
//// e) Draw the Wheels

						glColor3f(0.2 + 0.2 * ExtraWhiteness,0.2 + 0.2 * ExtraWhiteness,0.2 + 0.2 * ExtraWhiteness);
							glBegin(GL_QUAD_STRIP);
								for (i = 0; i < 8; i++)
								{
									glVertex3f(TubeBase[ i*3 + 0], TubeBase[ i*3 + 1], FRONTBACK*TubeBase[ i*3 + 2]);
								}
							glEnd();
							
							for (LEFTRIGHT = -1; LEFTRIGHT<=1; LEFTRIGHT+=2)
							{
								glBegin(GL_POLYGON);
									glColor3f(0.1,0.1,0.1);
									for (i=0; i<=20; i++)		
									{	
										angle =2*M_PI / 20 + 2*M_PI/20 * i;
								
										tempX = sin( angle ) * 0.5*MAP_BLOCK_SIZE; 
										tempY = cos( angle ) * 0.5*MAP_BLOCK_SIZE;
						
										glVertex3f( tempX + 1.0*MAP_BLOCK_SIZE*LEFTRIGHT, 0.5 * MAP_BLOCK_SIZE  + tempY, FRONTBACK*1.3);//- 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire)));
									}
								glEnd();
							}
							
//// f) Draw the White Shell, if tube is not exploding

							if (state != EXPLODING)
							{
								glDisable(GL_TEXTURE_2D);
								glColor3f(0.8 + 0.2 * ExtraWhiteness,0.8 + 0.2 * ExtraWhiteness,0.8 + 0.2 * ExtraWhiteness);
								glBegin(GL_QUAD_STRIP);
									for (i = 0; i < 6; i+=2)
										for (dy = TubeFrame[i*2 + 1]; dy < TubeFrame[i*2 + 5]; dy+= TRAIN_Y_RESOLUTION)
									{
										glVertex3f(	TubeFrame[ i*2 + 0], 
													dy, 
													FRONTBACK*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy))	);
										glVertex3f(	TubeFrame[ i*2 + 2], 
													dy, 
													FRONTBACK*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy))	);
									}
								glEnd();
							}
					
//// g) Draw the blue strip at the base

							glBegin(GL_QUAD_STRIP);
								for (dy = TubeBlueStrip[0*2 + 1]; dy < TubeBlueStrip[2*2 + 1]; dy+= TRAIN_Y_RESOLUTION)
								{
									glColor3f(ExtraWhiteness,ExtraWhiteness,0.4 + 0.6 * ExtraWhiteness);
									glVertex3f(	TubeBlueStrip[ 0], 
												dy, 
												FRONTBACK*(1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy)))	);
									glVertex3f(	TubeBlueStrip[ 2], 
												dy, 
												FRONTBACK*(1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy)))	);
								}
							glEnd();
						
//// h) Draw the black strip on the top

							glBegin(GL_QUAD_STRIP);
								for (dy = TubeBlackStrip[0*2 + 1]; dy < TubeBlackStrip[2*2 + 1]; dy+= TRAIN_Y_RESOLUTION)
								{
									glColor3f(0.2 + 0.8 * ExtraWhiteness,0.2 + 0.8 * ExtraWhiteness,0.2 + 0.8 * ExtraWhiteness);
									glVertex3f(	TubeBlackStrip[ 0], 
												dy, 
												FRONTBACK*(1.05*sqrt( 0.45*(7.1 * MAP_BLOCK_SIZE - dy)))	);
									glVertex3f(	TubeBlackStrip[ 2], 
												dy, 
												FRONTBACK*(1.05*sqrt( 0.45*(7.1 * MAP_BLOCK_SIZE - dy)))	);
								}
							glEnd();
					
//// i) Draw the doors using the animation variable doorOpenRatio

							for (LEFTRIGHT = -1; LEFTRIGHT <= 1; LEFTRIGHT+=2)
							{
								glBegin(GL_QUAD_STRIP);
									for (dy = TubeDoor[0*2 + 1]; dy < TubeDoor[2*2 + 1]; dy+= TRAIN_Y_RESOLUTION)
									{
										glColor3f(0.7 + 0.3 * ExtraWhiteness,ExtraWhiteness,ExtraWhiteness);
										glVertex3f(	LEFTRIGHT*(TubeDoor[0] - doorOpenRatio), 
													dy, 
													FRONTBACK*(1.04*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy)))	);
										glVertex3f(	LEFTRIGHT*(TubeDoor[2] - doorOpenRatio), 
													dy, 
													FRONTBACK*(1.04*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy)))	);
									}
								glEnd();
							}
					
//// j) Draw the door windows using the animation variable doorOpenRatio

							glEnable(GL_TEXTURE_2D);
							for (LEFTRIGHT = -1; LEFTRIGHT <= 1; LEFTRIGHT+=2)
							{
								glBegin(GL_QUAD_STRIP);
									for (dy = TubeDoorWindow[0*2 + 1]; dy < TubeDoorWindow[2*2 + 1]; dy+= TRAIN_Y_RESOLUTION)
									{
										glVertex3f(	LEFTRIGHT*(TubeDoorWindow[0] - doorOpenRatio), 
													dy, 
													FRONTBACK*(1.05*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy)))	);
										glVertex3f(	LEFTRIGHT*(TubeDoorWindow[2] - doorOpenRatio), 
													dy, 
													FRONTBACK*(1.05*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy)))	);
									}
								glEnd();
							}

//// k) Draw the main windows using the animation variable doorOpenRatio

							glBegin(GL_QUAD_STRIP);
								for (dy = TubeWindow[0*2 + 1]; dy < TubeWindow[2*2 + 1]; dy+= TRAIN_Y_RESOLUTION)
								{
									glVertex3f(	TubeWindow[ 0], 
												dy + doorOpenRatio * 1.4*MAP_BLOCK_SIZE, 
												FRONTBACK*(-0.02 + 1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy - doorOpenRatio * 1.4 * MAP_BLOCK_SIZE)))	);
									glVertex3f(	TubeWindow[ 2], 
												dy + doorOpenRatio * 1.4*MAP_BLOCK_SIZE, 
												FRONTBACK*(-0.02 + 1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy - doorOpenRatio * 1.4 * MAP_BLOCK_SIZE)))	);
								}
							glEnd();
							glBegin(GL_QUAD_STRIP);
								for (dy = TubeWindow[0*2 + 1]; dy < TubeWindow[2*2 + 1]; dy+= TRAIN_Y_RESOLUTION)
								{
									glVertex3f(	TubeWindow[ 0], 
												dy + doorOpenRatio * 1.4*MAP_BLOCK_SIZE, 
												FRONTBACK*(+0.01 + 1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy - doorOpenRatio * 1.4*MAP_BLOCK_SIZE)))	);
									glVertex3f(	TubeWindow[ 2], 
												dy + doorOpenRatio * 1.4* MAP_BLOCK_SIZE, 
												FRONTBACK*(+0.01 + 1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy - doorOpenRatio * 1.4*MAP_BLOCK_SIZE)))	);
								}
							glEnd();
						
//// l) draw the multicoloured weak spot behind the windows

							glDisable(GL_TEXTURE_2D);
							glBegin(GL_QUAD_STRIP);	
							weakspot_color = float(random()) / float(RAND_MAX);
							glColor3f(weakspot_color, 0.2*weakspot_color, 0.2*weakspot_color);
								for (dy = TubeWindow[0*2 + 1]; dy < TubeWindow[2*2 + 1]; dy+= TRAIN_Y_RESOLUTION)
								{
									glVertex3f(	TubeWindow[ 0], 
												dy, 
												FRONTBACK*(1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy)))	);
									glVertex3f(	TubeWindow[ 2], 
												dy, 
												FRONTBACK*(1.02*sqrt( 0.45*(7 * MAP_BLOCK_SIZE - dy)))	);
								}
							glEnd();
						glRotated(-180*FRONTBACK, 0, 1, 0);
						glRotated(-90,0,1,0);

					glTranslated(-x_current, -y_current, -z_current - CARRIAGE*5*MAP_BLOCK_SIZE);
				}
			}
			
//// m) draw the Gun turrets inside the Tube

			int GUN_TURRET;
			for (GUN_TURRET = 0; GUN_TURRET < 4; GUN_TURRET++)
			{
				glDisable(GL_TEXTURE_2D);
				if (GUN_TURRET == 0) glTranslated(0,0,0.4*MAP_BLOCK_SIZE);
				if (GUN_TURRET == 3) glTranslated(0,0,-0.4*MAP_BLOCK_SIZE);
				
				
				// draw the turret that holds the barrel
				glTranslated(x_current, y_current, z_current - 7.5 * MAP_BLOCK_SIZE + 5 * MAP_BLOCK_SIZE * GUN_TURRET);
					glRotated(90,0,1,0);
						glBegin(GL_QUAD_STRIP);
							glColor3f(0.6,0.6,0.6);
							for (i=0; i<16; i++)		
							{	
								glVertex3f( TubeTurret[3*i] , TubeTurret[3*i+1] , TubeTurret[3*i+2]); 
							}		
						glEnd();
						glBegin(GL_QUAD_STRIP);
							glColor3f(0.5,0.5,0.5);
							for (i=0; i<8; i+=2)		
							{	
								glVertex3f( TubeTurret[3*i] , TubeTurret[3*i+1] , TubeTurret[3*i+2]); 
								glVertex3f( TubeTurret[3*i + 24] , TubeTurret[3*i+1+24] , TubeTurret[3*i+2+24]); 
							}	
						glEnd();
						glBegin(GL_QUAD_STRIP);
							glColor3f(0.5,0.5,0.5);
							for (i=1; i<8; i+=2)		
							{	
								glVertex3f( TubeTurret[3*i] , TubeTurret[3*i+1] , TubeTurret[3*i+2]); 
								glVertex3f( TubeTurret[3*i + 24] , TubeTurret[3*i+1+24] , TubeTurret[3*i+2+24]); 
							}	
						glEnd();
					glRotated(-90,0,1,0);
					
					// draw the barrel
					glTranslated(0,3.5*MAP_BLOCK_SIZE,0);
						glRotated(turretRightLeft,0,1,0);
							glRotated(turretUpDown,-1,0,0);
								glBegin(GL_QUAD_STRIP);
										glColor3f(0.1,0.1,0.1);
										for (i=0; i<=20; i++)		
										{	
											angle =2*M_PI / 20 + 2*M_PI/20 * i;
									
											tempX = sin( angle ) * 0.4;
											tempY = cos( angle ) * 0.4;
					
											glVertex3f( 0.6* tempX, 0.6* tempY, 0);
											glVertex3f( tempX, tempY, tankbarrellength*0.7 * ( 1 ));//- 0.4 * float(timetoFire * timeToFire / maxtimetofire / maxtimetofire)));
										}
								glEnd();
								glBegin(GL_POLYGON);
										glColor3f(0.1,0.1,0.1);
										for (i=0; i<=20; i++)		
										{	
											angle =2*M_PI / 20 + 2*M_PI/20 * i;
								
											tempX = sin( angle ) * 0.4;
											tempY = cos( angle ) * 0.4;
						
											glVertex3f( tempX, tempY, tankbarrellength*0.7 * ( 1 ));//- 0.4 * float(timeToFire * timeToFire / maxtimetofire / maxtimetofire)));
										}
								glEnd();
							glRotated(-turretUpDown,-1,0,0);
						glRotated(-turretRightLeft,0,1,0);
					glTranslated(0,-3.5*MAP_BLOCK_SIZE,0);
				
					if (GUN_TURRET == 0) glTranslated(0,0,-0.4*MAP_BLOCK_SIZE);
					if (GUN_TURRET == 3) glTranslated(0,0,0.4*MAP_BLOCK_SIZE);
				glTranslated(-x_current, -y_current, - z_current + 7.5 * MAP_BLOCK_SIZE - 5 * MAP_BLOCK_SIZE * GUN_TURRET);
			}
		}

//// n) if the tube is exploding, draw the explosion

		if (state == EXPLODING)
		{
			glColor3f(1,1,1);
			glTranslated(x_current, y_current, z_current - 3 * MAP_BLOCK_SIZE);
			for ( i=0; i< 4; i++)
			{	
				glutSolidSphere(ENEMY_MIN_EXPLOSION_RADIUS + (ENEMY_MAX_EXPLOSION_RADIUS * explosionTime/ENEMY_MAX_EXPLOSION_TIME),100,100);
				glTranslated(0, 0, 2*MAP_BLOCK_SIZE);
			}
			glTranslated(-x_current, -y_current, -z_current - 3 * MAP_BLOCK_SIZE);
		}
	}
	
	
