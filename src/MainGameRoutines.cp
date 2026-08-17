//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			MainGameRoutines.cp																															
////	Author:			Ed Martin
////	Description:	Holds the Main Game routines - the event processor, the game initializers and the main game loop
////						
////					1) Event Processor
////						a) New Bullet Event
////						b) New Map Event
////						c) New Turret Event
////
////					2) Setup Difficulty Levels
////
////					3) Setup Game (Reset)
////						
////					4) the Main Game loop Function 
////						The game can always only be 4 in running modes, 3 of which are 
////						transitions between scenes. The last loop mode is the gameplay loop.
////						
////						MODE 1) RunMode == STARTSCREEN  - Camera fixed at startscreen texture on wall
////						        a) Initiate Animation timers
////						        b) Position Camera
////						        c) Draw Scene
////
////						MODE 2) RunMode == TRANSITION	- Camera Moving from startscreen to beginning of gameplay
////								a) Update Animation timers
////								b) Position Camera		
////								c) Draw Scene
////
////						MODE 3) TUBE_APROACHING			- Camera circling player while boss approaches
////								a) Process new events (new bullets, map triggers, etc.)
////								b) Update Animation timers
////								c) Update Active Tubes
////								d) Update Active Bullets
////								e) Check for Collisions and update Player only if in the way of the oncoming train
////								f) Position Camera - Gradually make player face forward (so that he notices the turrets firing at him)
////								g) Draw Scene
////
////						MODE 4) NORMAL GAMEPLAY LOOP :
////								a) Process new events (new bullets, map triggers, etc.)
////								b) Update Animation timers
////								c) Update Player (movement, health, etc) 
////								d) Update Active Turrets
////								e) Update Active Tubes
////								f) Update Active Bullets
////								g) Check for Collisions
////								h) Position Camera
////								i) Draw Scene
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameConstants.cp"

#ifndef EVENTS
	#define EVENTS	
	float events[0];
    void Event(int description, float a, float b, float c, float d, float e, float f, float g, float h, float i) {}
#endif

#ifndef GL
	#define GL
	#include "Platform.h"
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

#ifndef GLGENERAL
	#define GLGENERAL
	#include "GLGeneral.cp"
#endif

#include "Collisions.cp"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////						4) Event Processor
////
////						The first field of event[e*10] contains the description float. if this is non zero, 
////						ie NEW_BULLET_EVENT, NEW_MAP_EVENT or NEW_TURRET_EVENT, the neccessary action is taken,
////						then the description field is reset to zero, to make space for future events
////
////							a) New Bullet Event
////							b) New Map Event
////							c) New Turret Event
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CheckEvents()
{
	
	int x,z,b,e;
	
	for (e = 0; e < MAX_EVENTS; e++)
	{

//// a) New Bullet Event
//      When an object fires a bullet or shrapnel is created, a NEW_BULLET event is made, 
//      here the bullet/shrapnel is initiated:
		
		if (events[e*10] == NEW_BULLET_EVENT) 
			for (b = 0; b < MAX_BULLETS; b++)
				if (Bullet[b].GetActive()==0)
				{
					Bullet[b].InitBullet(events[e*10+1], events[e*10+2], events[e*10+3], 
										events[e*10+4], events[e*10+5], events[e*10+6],
										events[e*10+7], events[e*10+8], events[e*10+9]);
					if (b >= MAX_BULLETS - 1) printf("maxed out on bullets!?!");
					b = MAX_BULLETS +1;
			
					// The event has been processed, therefore events[e*10] is cleared, to make space for new events:
					
					events[e*10] = 0;
				}
		
//// b) New Map Event
//   When the tank drives over certain points in the map, an event is triggered (wake up turrets, initiate boss, etc):

		if (events[e*10] == NEW_MAP_EVENT) 
		{	
			// Since the event has been triggered, it is removed from the map (so that it isn't triggered again)
			if  ( (int(events[e*10 + 1]) == RAPID_FIRE) || (int(events[e*10 + 1]) == DUAL_FIRE) || (int(events[e*10 + 1]) == MED_PACK) ) 
			{
				Map[int(events[e*10 + 2])] = GROUND;
			}
			else for (x = 0; x <= MAP_X; x++)	for (z = 0; z <= MAP_Z; z++) if (Map[x+ MAP_X*z] == int(events[e*10 + 1])) Map[x+ MAP_X*z] = GROUND;
			
			switch(int(events[e*10 + 1]))
			{
				case 8:
					ObjectiveCameraDistance *= 0.9;
					Turret[14].InitTurret(MAP_BLOCK_SIZE * 5,		MAP_BLOCK_SIZE * 5,		MAP_BLOCK_SIZE * 4);
					Turret[14].SetState(1);					
				break;
	
				case 2:
					ObjectiveCameraDistance /= 0.9;
					Turret[0].InitTurret(MAP_BLOCK_SIZE * 0.5,		MAP_BLOCK_SIZE * 6,		MAP_BLOCK_SIZE * 30.5);
					Turret[0].SetState(1);					
					break;
					
				case 3:
					Turret[1].InitTurret(MAP_BLOCK_SIZE * 19.5,		8*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 30.5);
					Turret[1].SetState(1);
					break;
				case 4:
					Turret[6].InitTurret(MAP_BLOCK_SIZE * 45.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 18.5);
					  Turret[7].InitTurret(MAP_BLOCK_SIZE * 53.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 22.5);
						Turret[8].InitTurret(MAP_BLOCK_SIZE * 59.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 26.5);
					  Turret[9].InitTurret(MAP_BLOCK_SIZE * 53.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 30.5);
					Turret[10].InitTurret(MAP_BLOCK_SIZE * 45.5,	7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 34.5);
				
					Turret[6].SetState(1);
					  Turret[7].SetState(1);
						Turret[8].SetState(1);
					  Turret[9].SetState(1);
					Turret[10].SetState(1);
					break;

				case 5:
				
					Turret[2].InitTurret(MAP_BLOCK_SIZE * 3.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 54.5);
					Turret[3].InitTurret(MAP_BLOCK_SIZE * 23.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 54.5);
					Turret[2].SetState(1);
					Turret[3].SetState(1);
					break;				
					
	 			case 6:
					Turret[4].InitTurret(MAP_BLOCK_SIZE * 2.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 62.5);
					Turret[5].InitTurret(MAP_BLOCK_SIZE * 5.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 69.5);
					Turret[6].InitTurret(MAP_BLOCK_SIZE * 9.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 72.5);
					Turret[7].InitTurret(MAP_BLOCK_SIZE * 15.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 72.5);

					Turret[4].SetState(1);
					Turret[5].SetState(1);
					Turret[6].SetState(1);
					Turret[7].SetState(1);
					break;		
					
				case 7:	
					Turret[8].InitTurret(MAP_BLOCK_SIZE * 2.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 100.5);
					Turret[9].InitTurret(MAP_BLOCK_SIZE * 8.5,		3*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 104.5);
					Turret[10].InitTurret(MAP_BLOCK_SIZE * 14.5,	3*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 104.5);
					Turret[11].InitTurret(MAP_BLOCK_SIZE * 22.5,	7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 100.5);
					Turret[8].SetState(1);
					Turret[9].SetState(1);
					Turret[10].SetState(1);
					Turret[11].SetState(1);
					
					Turret[12].InitTurret(MAP_BLOCK_SIZE * 2.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 82.5);
					Turret[13].InitTurret(MAP_BLOCK_SIZE * 22.5,		7*MAP_BLOCK_SIZE,		MAP_BLOCK_SIZE * 82.5);
					Turret[12].SetState(1);
					Turret[13].SetState(1);
					break;
							
				// The player wakes the boss..
				case 9:
					RunMode = TUBE;
					CameraShakeTime = 2*MAX_CAMERA_SHAKE_TIME;
					Tube[0].InitTube( (MAP_BLOCK_SIZE) * 41.5, 0, (MAP_BLOCK_SIZE) *  101.5);
					Tube[0].SetState( TUBE_STATE_APROACHING);
					Tube[1].InitTube( (MAP_BLOCK_SIZE) * 60.5, 0, (MAP_BLOCK_SIZE) *  101.5);
					Tube[1].SetState( TUBE_STATE_APROACHING);
					
				// The player picks up a Rapid fire
				case RAPID_FIRE:
					Player1.SetType(RAPID_FIRE);
					break;
					
				// The player picks up a Dual fire
				case DUAL_FIRE:	
					Player1.SetType(DUAL_FIRE);
					break;
				
				// The player picks up a Med Pack
				case MED_PACK:
					Player1.DecreaseHealth(- 0.6 * PLAYER_MAX_HEALTH);
					break;
			}
			
			// The event has been processed, therefore events[e*10] is cleared, to make space for new events:
			
			events[e*10] = 0;
		}

//// c) New Turret Event
//   If the tube is active, it will initiate 5 new turrets and a random object (health, rapidfire, etc) at one point in it's cycle, 
//   causing a NEW_TURRET_EVENT:
		
		if (events[e*10] == NEW_TURRET_EVENT) 
		{
			Turret[5].InitTurret( 44.5*MAP_BLOCK_SIZE, 2.5*MAP_BLOCK_SIZE, 88.5*MAP_BLOCK_SIZE);//events[e*10 + 1], events[e*10 + 2], events[e*10 + 3]);
			Turret[6].InitTurret( 47.5*MAP_BLOCK_SIZE,	6*MAP_BLOCK_SIZE, 87*MAP_BLOCK_SIZE);//events[e*10 + 1], events[e*10 + 2], events[e*10 + 3]);
			Turret[7].InitTurret( 51*MAP_BLOCK_SIZE,	10*MAP_BLOCK_SIZE, 86*MAP_BLOCK_SIZE);//events[e*10 + 1], events[e*10 + 2], events[e*10 + 3]);
			Turret[8].InitTurret( 54.5*MAP_BLOCK_SIZE,	6*MAP_BLOCK_SIZE, 87*MAP_BLOCK_SIZE);//events[e*10 + 1], events[e*10 + 2], events[e*10 + 3]);
			Turret[9].InitTurret( 57.5*MAP_BLOCK_SIZE, 2.5*MAP_BLOCK_SIZE, 88.5*MAP_BLOCK_SIZE);//events[e*10 + 1], events[e*10 + 2], events[e*10 + 3]);
			Turret[5].SetState(1);
			Turret[6].SetState(1);
			Turret[7].SetState(1);
			Turret[8].SetState(1);
			Turret[9].SetState(1);
			
			int r = 3*(float(random()) / float(RAND_MAX));
					Map[51 + MAP_X * 95] = RAPID_FIRE + r;
					
			// The event has been processed, therefore events[e*10] is cleared, to make space for new events:
			
			events[e*10] = 0;
		}
	}
}
	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Setup Difficulty Levels
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Setup_Dificultylevels()
{
	// Setup difficulty levels depending on what the player has chosen from the start screen
	if (Dificulty_Level == EASY)
	{
		PLAYER_MAX_HEALTH = 100;
		Player1.SetHealth(PLAYER_MAX_HEALTH);

		TURRET_MAX_HEALTH = 20;
	
		ENEMY_MAX_TIME_TO_FIRE =		1 * FRAME_RATE;						// [frames]						=	[seconds] * ([frames] / [second])

		TUBE_APROACHING_TIME =			2.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_NEW_TURRETS_TIME =			3.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_AIMING_TIME =				2.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_OPENING_DOORS_TIME =		0.3 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_OPEN_FIRE_TIME =			4.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_CLOSING_DOORS_TIME =		0.3 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
	
		PLAYER_BULLET_SPEED =			500 * MAP_BLOCK_SIZE / FRAME_RATE;	// [map units]	/ [frame]		=	([map units] / [second]) / ([frames] / [second])
		ENEMY_BULLET_SPEED =			30 * MAP_BLOCK_SIZE / FRAME_RATE;	// [map units]	/ [frame]		=	([map units] / [second]) / ([frames] / [second])
	}
	else if (Dificulty_Level == MEDIUM)
	{
		PLAYER_MAX_HEALTH = 40;
		Player1.SetHealth(PLAYER_MAX_HEALTH);

		TURRET_MAX_HEALTH = 20;
	
		ENEMY_MAX_TIME_TO_FIRE =		0.6 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])

		TUBE_APROACHING_TIME =			2.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_NEW_TURRETS_TIME =			3.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_AIMING_TIME =				2.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_OPENING_DOORS_TIME =		0.3 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_OPEN_FIRE_TIME =			1.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_CLOSING_DOORS_TIME =		0.3 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
	
		PLAYER_BULLET_SPEED =			500 * MAP_BLOCK_SIZE / FRAME_RATE;	// [map units]	/ [frame]		=	([map units] / [second]) / ([frames] / [second])
		ENEMY_BULLET_SPEED =			40 * MAP_BLOCK_SIZE / FRAME_RATE;	// [map units]	/ [frame]		=	([map units] / [second]) / ([frames] / [second])
	}
	else if (Dificulty_Level == HARD)
	{
		PLAYER_MAX_HEALTH = 35;
		Player1.SetHealth(PLAYER_MAX_HEALTH);

		TURRET_MAX_HEALTH = 60;
	
		ENEMY_MAX_TIME_TO_FIRE =		0.3 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])

		TUBE_APROACHING_TIME =			2.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_NEW_TURRETS_TIME =			3.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_AIMING_TIME =				2.0 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_OPENING_DOORS_TIME =		0.3 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_OPEN_FIRE_TIME =			0.5 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
		TUBE_CLOSING_DOORS_TIME =		0.3 * FRAME_RATE;					// [frames]						=	[seconds] * ([frames] / [second])
	
		PLAYER_BULLET_SPEED =			500 * MAP_BLOCK_SIZE / FRAME_RATE;	// [map units]	/ [frame]		=	([map units] / [second]) / ([frames] / [second])
		ENEMY_BULLET_SPEED =			60 * MAP_BLOCK_SIZE / FRAME_RATE;	// [map units]	/ [frame]		=	([map units] / [second]) / ([frames] / [second])
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////						3) Setup Game (Reset)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GameInit() 
{	
	// Initiate Bullets to InActive
	int b;
	for (b = 0; b< MAX_BULLETS; b++) Bullet[b].SetState(INACTIVE);
	
	// Initiate Turrets to InActive
	int t;
	for (t = 0; t< MAX_TURRETS; t++) Turret[t].SetState(INACTIVE);
	for (t = 0; t< 2; t++) Tube[t].SetState(INACTIVE);
	
	// Empty all events
	int i,j;
	for (i = 0; i<8; i++) 
		for (j = 0; j<10; j++) 
			events[i*10 + j] = 0;
	
	// Copy the Original Map into the working Map (reset Map)
	for (i = 0; i < MAP_X; i++)
		for(j = 0; j < MAP_Z; j++)
			Map[i+ MAP_X*j] = OriginalMap[i+ MAP_X*j];
	
	// Setup Difficulty Levels
	Setup_Dificultylevels();
	
	// Setup Player
	Player1.SetLives(2);
	Player1.SetturretUpDown(10);
	Player1.SetturretRightLeft(270);
	Player1.SettrackDirection(235);
	
	Player1.Setx((MAP_BLOCK_SIZE) * 54);
	Player1.Setz((MAP_BLOCK_SIZE) * 5.5);

	// Setup Camera Position
	ObjectiveCameraDistance = 3 * MAP_BLOCK_SIZE;

	// Reset Transition counters (so they are reset if a transitional mode is initiated)
	TransitionCameraAngle = 0;
	TransitionCameraDistance = 0;
	TransitionCounter = 0;
	StartCameraDistance = 0;
	
	camera_x = 140;
	camera_y = 140;
	camera_z = 140;
	
	// for Boss Debug Mode:
	//Player1.Setx((MAP_BLOCK_SIZE) * 54);
	//Player1.Setz((MAP_BLOCK_SIZE) * 100);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					4) the Main Game loop Function 
////						The game can always only be 4 in running modes, 3 of which are 
////						transitions between scenes. The last loop mode is the gameplay loop.
////						
////						MODE 1) RunMode == STARTSCREEN  - Camera fixed at startscreen texture on wall
////						        a) Initiate Animation timers
////						        b) Position Camera
////						        c) Draw Scene
////
////						MODE 2) RunMode == TRANSITION	- Camera Moving from startscreen to beginning of gameplay
////								a) Update Animation timers
////								b) Position Camera		
////								c) Draw Scene
////
////						MODE 3) TUBE_APROACHING			- Camera circling player while boss approaches
////								a) Process new events (new bullets, map triggers, etc.)
////								b) Update Animation timers
////								c) Update Active Tubes
////								d) Update Active Bullets
////								e) Check for Collisions and update Player only if in the way of the oncoming train
////								f) Position Camera - Gradually make player face forward (so that he notices the turrets firing at him)
////								g) Draw Scene
////
////						MODE 4) NORMAL GAMEPLAY LOOP :
////								a) Process new events (new bullets, map triggers, etc.)
////								b) Update Animation timers
////								c) Update Player (movement, health, etc) 
////								d) Update Active Turrets
////								e) Update Active Tubes
////								f) Update Active Bullets
////								g) Check for Collisions
////								h) Position Camera
////								i) Draw Scene
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainGameLoop(int value)
{
	int b,t;	
	
//// MODE 1) RunMode == STARTSCREEN, Camera fixed at beginning start texture on wall

	if (RunMode==STARTSCREEN)
	{		

//// a) Initiate Animation timers

		StartCameraDistance = 0;
		CurrentCameraDistance = 4;

//// b) Position Camera
	
		lineOfSight_x = 1; 
		lineOfSight_y = 0;
		lineOfSight_z = 0; 
	
		camera_x = (1.3615384)*(MAP_BLOCK_SIZE);
		camera_y = 6 * MAP_BLOCK_SIZE;
		camera_z = 4 * MAP_BLOCK_SIZE;
		
		glLoadIdentity();
		gluLookAt(	camera_x,				camera_y,				camera_z, 
					camera_x+lineOfSight_x, camera_y+lineOfSight_y, camera_z+lineOfSight_z,
					0, 1, 0
					);

//// c) Draw Scene

		display();
	}
	
//// MODE 2) RunMode == TRANSITION, Camera Moving from startscreen to beginning of gameplay

	else if (RunMode==TRANSITION)
	{
	
//// a) Update Animation timers
		
		StartCameraDistance -=2;
		if (StartCameraDistance < -181) 
		{
			GameInit();
			RunMode = NORMAL;
		}
		CurrentCameraDirectionRightLeft = 360 - 90 - 135 - 2*StartCameraDistance;
		
//// b) Position Camera		
		
		glLoadIdentity();
		lineOfSight_x = 1 + cos(0.8*StartCameraDistance *2*M_PI/ 360);
		lineOfSight_y = 0 + 0.7*StartCameraDistance/180;
		lineOfSight_z = 0 + sin(2*StartCameraDistance *2*M_PI/ 360);

		camera_x = 1 * MAP_BLOCK_SIZE + 0.37*StartCameraDistance * cos(-StartCameraDistance *2*M_PI/ 360);//0.96153*MAP_BLOCK_SIZE;
		camera_y = 6 * MAP_BLOCK_SIZE + 0.45 *StartCameraDistance * sin(StartCameraDistance *2*M_PI/ 360);;
		camera_z = 4 * MAP_BLOCK_SIZE + 0.2 *StartCameraDistance * sin(2*StartCameraDistance *2*M_PI/ 360);
	
		gluLookAt(	camera_x,				camera_y,				camera_z, 
					camera_x+lineOfSight_x, camera_y+lineOfSight_y, camera_z+lineOfSight_z,
					0,						1,						0);

//// c) Draw Scene

		display();
	}
	
//// MODE 3) TUBE_APROACHING, Camera circling player while boss approaches
	
	else if ((Tube[0].GetState() == TUBE_STATE_APROACHING) || (Tube[1].GetState() == TUBE_STATE_APROACHING))
	{
	
//// a) Process new events (new bullets, map triggers, etc.)

		CheckEvents();

//// b) Update Animation timers
		
		TransitionCounter++;
		if (CameraShakeTime>0) CameraShakeTime--;
		ObjectSpin += 1;

//// c) Update Active Tubes

		Tube[0].Update(Player1.Getx(), Player1.Getz());
		Tube[1].Update(Player1.Getx(), Player1.Getz());		

//// d) Update Active Bullets

		for ( b = 0; b < MAX_BULLETS; b++) 
			if (Bullet[b].GetActive() != 0)	Bullet[b].Update();

//// e) Check for Collisions and update Player only if in the way of the oncoming train

		if ( 
			 ( Tube[1].Getz() -   7 * MAP_BLOCK_SIZE <= Player1.Getz() + 1 ) 
			 &&
			 ( (Tube[0].Getx() + 1.5 * MAP_BLOCK_SIZE >= Player1.Getx()) ||
			   (Tube[1].Getx() - 1.5 * MAP_BLOCK_SIZE <= Player1.Getx()) )	
		   ) 
		{
			if (Player1.GetState() == ACTIVE) Player1.DecreaseHealth(2*PLAYER_MAX_HEALTH);
			CameraShakeTime  = 2.5 * MAX_CAMERA_SHAKE_TIME;
			Player1.Update();
		}

//// f) Position Camera - Gradually make player face forward (so that he notices the turrets firing at him)

		if (Player1.GetturretRightLeft() < 180) Player1.SetturretRightLeft(Player1.GetturretRightLeft() + 180 / TUBE_APROACHING_TIME);
		if (Player1.GetturretRightLeft() > 180) Player1.SetturretRightLeft(Player1.GetturretRightLeft() - 180 / TUBE_APROACHING_TIME);
		
		// - Change camera angle from players current turret direction to facing turrets smoothly:
		CurrentCameraDirectionRightLeft = TransitionCameraAngle =   ( 90 - Player1.GetturretRightLeft()) * (1 - TransitionCounter / TUBE_APROACHING_TIME)
																	  + (-90 + 360 )					     * (    TransitionCounter / TUBE_APROACHING_TIME);
																	  
		// - Camera distance from player moves in the shape of the first 160 degrees of a sign wave:
		TransitionCameraDistance = 1 * MAP_BLOCK_SIZE + 5 * MAP_BLOCK_SIZE * sin(160 * TransitionCounter / TUBE_APROACHING_TIME * 2 * M_PI / 360);

		lineOfSight_x = cos(TransitionCameraAngle *2*M_PI/ 360);
		lineOfSight_y = 0;
		lineOfSight_z = sin(TransitionCameraAngle *2*M_PI/ 360);

		camera_x = Player1.Getx() - TransitionCameraDistance * cos(TransitionCameraAngle *2*M_PI/ 360)
				   + MAX_CAMERA_SHAKE_DISTANCE * pow(CameraShakeTime / MAX_CAMERA_SHAKE_TIME,2) * (2 * float(random()) / float(RAND_MAX) - 1);
		camera_y = TransitionCameraDistance
					+ MAX_CAMERA_SHAKE_DISTANCE * pow(CameraShakeTime / MAX_CAMERA_SHAKE_TIME,2) * (2 * float(random()) / float(RAND_MAX) - 1);
		camera_z = Player1.Getz() - TransitionCameraDistance * sin(TransitionCameraAngle *2*M_PI/ 360)
					+ MAX_CAMERA_SHAKE_DISTANCE * pow(CameraShakeTime / MAX_CAMERA_SHAKE_TIME,2) * (2 * float(random()) / float(RAND_MAX) - 1);;

		glLoadIdentity();
		gluLookAt(	camera_x,				camera_y,				camera_z, 
					camera_x+lineOfSight_x, camera_y+lineOfSight_y, camera_z+lineOfSight_z,
					0,						1,						0);
//// g) Draw Scene
		
		display();
	}
	
//// MODE 4) GENERAL GAMEPLAY LOOP :

	else 
	{
		if (RunMode != PAUSED) 
		{

//// a) Process new events (new bullets, map triggers, etc.)
	
			CheckEvents();
	
//// b) Update Animation timers

			if (CameraShakeTime>0) CameraShakeTime--;
			ObjectSpin += 1;

//// c) Update Player (movement, health, etc) 

			Player1.Update();

//// If player is alive:

			if ((Player1.GetHealth() > 0) && (RunMode != END))
			{
			
//// d) Update Active Turrets
			
				for ( t = 0; t < MAX_TURRETS; t++) 
					if ((Turret[t].GetState() != INACTIVE) && (Turret[t].GetState() != SMOKING))	
						Turret[t].Update(Player1.Getx(), Player1.Getz());
			}
				
//// e) Update Active Tubes

			for ( t = 0; t <= 1; t++) 
				if ((Tube[t].GetState() != INACTIVE) && (Tube[t].GetState() != SMOKING) && (Tube[t].GetState() > 0))	
					Tube[t].Update(Player1.Getx(), Player1.Getz());
			
			// kill both tubes if one dies:
			if (Tube[0].GetState()==SMOKING) Tube[1].SetState(SMOKING);
			if (Tube[1].GetState()==SMOKING) Tube[0].SetState(SMOKING);
			
//// f) Update Active Bullets

			for ( b = 0; b < MAX_BULLETS; b++) 
				if (Bullet[b].GetActive() != 0)	Bullet[b].Update();
	
//// g) Check for Collisions
			
			// Check Player / Map Collisions
			Player1 =	CheckPlayerMapCollisions( Map, Player1, Tube );	
			
			// Check Bullet / Map Collisions
			CheckBulletMapCollisions( Map, Bullet);	
						
			// Check Bullet / Player Collisions
			Player1 =	CheckBulletPlayerCollisions( Player1, Bullet );	

			// Check Bullet / Enemy Collisions
			for (t=0; t< MAX_TURRETS; t++) 
			Turret[t] = CheckBulletEnemyCollisions( Turret[t], Bullet );		
				
			// Check Bullet / Tube Collisions
			if ((Tube[0].GetState() != INACTIVE) || (Tube[1].GetState() != INACTIVE)) 
			    for (t=0; t<=1; t++) 
			Tube[t] = CheckBulletTubeCollisions( Bullet, Tube[t] );
						
		}
		
//// h) Position Camera

		positionCamera(Player1);
		
//// i) Draw Scene
	
		display();
	}
	
	// loop
	// [2026 port] In the browser the loop is driven by requestAnimationFrame through a fixed-timestep
	// accumulator in tankmain.cp, so this self-rearming timer must not fire as well. See WebFrame().
#ifndef __EMSCRIPTEN__
	glutTimerFunc( TankTimerInterval() , MainGameLoop, 1);
#endif
}
