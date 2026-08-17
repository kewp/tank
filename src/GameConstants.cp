//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			GameConstants.cp																															
////	Author:			Ed Martin
////	Description:	Contains every tweakable gameplay variable, constants, and the unescapably global variables (see note)
////		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Platform.h"

#ifndef GAME
	#define GAME

	// DEBUGGING CONSTANTS AND VARIABLES
	const int	NORMAL						  = 11;
	const int	STARTSCREEN					  = 12;
	const int	TRANSITION					  = 13;
	const int	TUBE						  = 14;
	const int	TOP_VIEW					  = 15;
	const int	PAUSED						  = 16;
	const int	RESET						  = 17;
	const int	END							  = 18;
	int			RunMode;
		
	const int	EASY						  = 1;
	const int	MEDIUM						  = 2;
	const int	HARD						  = 3;
	int			Dificulty_Level;
	
	float		MOUSE_SENSETIVITY			  = 1.5;

	// [2026 port] Global gameplay speed multiplier.
	//
	// Every velocity and duration below is expressed in frames at FRAME_RATE (80), which makes the
	// simulation framerate-independent -- but 80 steps a second is far more than the 2007 code ever
	// actually achieved. Even on modern hardware the original fixed-function path renders about 41
	// fps; on the MacBook this was written for it would have been half that or less. The game was
	// tuned by feel at that real rate, so running the simulation at its full nominal rate makes
	// everything frantic.
	//
	// This scales simulation steps per second, so movement, firing rates, animation and bullet
	// speed all slow down together and the game keeps its proportions. 1.0 is the nominal 80
	// steps/s. Adjust live with [ and ].
	float		GAME_SPEED					  = 0.5;
	
	// GAMEPLAY RELATED CONSTANTS;
	float		PLAYER_MAX_HEALTH			  = 100;
	float		TURRET_MAX_HEALTH			  = 40;
	const int	MAX_BULLETS					  = 400;
	const int	MAX_TURRETS					  = 20;
	
	// SPACE RELATED CONSTANTS
	#define		MAP_BLOCK_SIZE				    (1.3)
	const int   MAP_X						  = 66;									/*	Size of map grid (x)															*/
	const int   MAP_Z						  = 127;								/*	Size of map grid (z)															*/
	const float PLAYER_BULLET_RADIUS 		  = 0.3 * MAP_BLOCK_SIZE;				/*	[map units]																		*/
	const float MIN_EXPLOSION_RADIUS 		  = 1.2 * MAP_BLOCK_SIZE;				/*	[map units]																		*/
	const float	MAX_EXPLOSION_RADIUS 		  = 3.0 * MAP_BLOCK_SIZE;				/*	[map units]																		*/
	
	const float	ENEMY_BULLET_RADIUS 		  = 0.5 * MAP_BLOCK_SIZE;				/*	[map units]																		*/
	const float	ENEMY_MIN_EXPLOSION_RADIUS	  = 2.0 * MAP_BLOCK_SIZE;				/*	[map units]																		*/
	const float	ENEMY_MAX_EXPLOSION_RADIUS    = 14  * MAP_BLOCK_SIZE;				/*	[map units]																		*/
	const float	ENEMY_TURRET_SIZE			  = 1.2	* MAP_BLOCK_SIZE;				/*	[map units]																		*/

	const float	MAX_CAMERA_SHAKE_DISTANCE	  = 2.0 * MAP_BLOCK_SIZE;				/*	[map units]																		*/

	// TIME RELATED CONSTANTS
	#define		FRAME_RATE						(80)								/*	[frames] / [second]																*/

	// [2026 port] Milliseconds between simulation steps for the native GLUT timer, honouring
	// GAME_SPEED. The browser build ignores this and uses the accumulator in tankmain.cp instead.
	// (The 2007 code wrote 1000/FRAME_RATE inline, which integer-divides to 12 -- 83.3 steps a
	// second rather than the intended 80.)
	static int TankTimerInterval()
	{
		float s = GAME_SPEED;
		if (s < 0.05f) s = 0.05f;
		int ms = int(1000.0f / (float(FRAME_RATE) * s) + 0.5f);
		return (ms < 1) ? 1 : ms;
	}

	const float	ENEMY_MAX_TIME_TO_FLASH		  =	0.7 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	float		ENEMY_MAX_TIME_TO_FIRE 		  = 1.0 * FRAME_RATE;					/*	[frames]				=   [seconds] * ([frames] / [second])					*/
	const float	PLAYER_MAX_TIME_TO_FLASH 	  = 0.7 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	const float	PLAYER_MAX_TIME_TO_FIRE 	  = 0.1 * FRAME_RATE;					/*	[frames]				=   [seconds] * ([frames] / [second])					*/
	
	const float	MAX_EXPLOSION_TIME			  = 0.1 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	const float	ENEMY_MAX_EXPLOSION_TIME	  = 0.2 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	const float	MAX_CAMERA_SHAKE_TIME 		  = 0.3 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	
	float		TUBE_APROACHING_TIME 		  = 3.0 * FRAME_RATE;					/*	[frames]				=   [seconds] * ([frames] / [second])					*/
	float		TUBE_NEW_TURRETS_TIME 		  = 3.0 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	float		TUBE_AIMING_TIME 			  = 3.0 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	float		TUBE_OPENING_DOORS_TIME 	  = 3.0 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	float		TUBE_OPEN_FIRE_TIME			  = 3.0 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	float		TUBE_CLOSING_DOORS_TIME 	  = 3.0 * FRAME_RATE;					/*	[frames]				=	[seconds] * ([frames] / [second])					*/
	
	// SPACE & TIME RELATED CONSTANTS
	float		TOP_SPEED					  = 65 * MAP_BLOCK_SIZE / FRAME_RATE;	/*	[map units]	/ [frame]	=	([map units] / [second]) / ([frames] / [second])	*/
	float		TOP_ACCELERATION			  = 3  * MAP_BLOCK_SIZE / FRAME_RATE;	/*	[map units]	/ [frame]	=	([map units] / [second]) / ([frames] / [second])	*/
	float		TOP_TURN_SPEED				  = 1.7 * 360 / FRAME_RATE;				/*	[degrees]	/ [frame]	=	([degrees]   / [second]) / ([frames] / [second])	*/
	float		PLAYER_BULLET_SPEED			  = 500 * MAP_BLOCK_SIZE / FRAME_RATE;	/*	[map units]	/ [frame]	=	([map units] / [second]) / ([frames] / [second])	*/
	float		ENEMY_BULLET_SPEED			  = 40 * MAP_BLOCK_SIZE / FRAME_RATE;	/*	[map units]	/ [frame]	=	([map units] / [second]) / ([frames] / [second])	*/
	const int	OBJECT_SPIN_SPEED			  = 50 / FRAME_RATE;					/*	[degrees]	/ [frame]	=	([degrees]   / [second]) / ([frames] / [second])	*/
	
	// CONSTANTS USED FOR MAP OBJECTS
	// used in the code and in the map
	const int	GROUND						  = 20;		const int	X = 20;
	const int	WALL						  = 21;		const int	B = 21;
	const int	ARCH						  = 22;		const int	A = 22;
	const int	TOWER						  = 23;		const int	T = 23;
	const int	RAIL						  = 24;		const int	I = 24;
	const int	RAILSLEEPER					  = 25;		const int	H = 25;

	const int	SINGLE_FIRE					  = 26;
	const int	RAPID_FIRE					  = 27;		const int	R = 27;
	const int	DUAL_FIRE					  = 28;		const int	D = 28;
	const int	MED_PACK					  = 29;		const int	M = 29;
	const int	RAPID_DUAL_FIRE				  = 30;
		
	//CAMERA RELATED CONSTANTS (NO LONGER IN USE - now the camera follows the player, not the scene - much better)
	const int	N	=							361;	/* N,S,W,E: direction the camera faces. rotating from it's previous position in a CLOCKWISE direction. */
	const int	S	=							181;	
	const int	W	=							271;
	const int	E	=							91 ;
	const int	n	=							359;	/* n,s,w,e: direction the camera faces. rotating from it's previous position in an ANTI-CLOCKWISE direction. */
	const int	s	=							179;	
	const int	w	=							269;
	const int	e	=							89 ;
	#define		CONSTANT_ROTATION				(999)	
	/* CONSTANTROTATION keeps the camera rotating after player has died */

	// OBJECT RELATED CONSTANTS
	const int	INACTIVE					  = 0;
	const int	ACTIVE						  = 1;
	const int	EXPLODING					  = 2;
	const int	SMOKING						  = 3;
	
	const int	TUBE_STATE_APROACHING		  = 10;
	const int	TUBE_STATE_NEW_TURRETS		  = 11;
	const int	TUBE_STATE_AIMING			  = 12;
	const int	TUBE_STATE_OPENING_DOORS	  = 13;
	const int	TUBE_STATE_OPEN_FIRE		  = 14;
	const int	TUBE_STATE_CLOSING_DOORS	  = 15;
	
	const int	PLAYER_BULLET				  = 0;
	const int	ENEMY_BULLET				  = 1;
	const int	FIRE_BULLET					  = 2;
	const int	SHRAPNEL					  = 3;

	// EVENT RELATED CONSTANTS
	const int	NEW_TURRET_EVENT			  = 1;
	const int	NEW_BULLET_EVENT			  = 2;
	const int	NEW_MAP_EVENT				  = 3;
	const int	MAX_EVENTS					  = 100;	
	 
	// TEXTURE RELATED VARIABLES
	unsigned int *sky1, *sky2, *sand1, *wall1, *wall2, *steel1, *steel2, *green1, *red1, *orange1, *yellow1, *minitank1, *start1, *start2, *start3, *start4;
	GLsizei TextureWidth, TextureHeight;
	int y, iw, ih;

	// GLOBAL VARIABLES 
	// Note: I undestand that this is generally bad practice, but for the most part, I haven't figured out how to get OpenGL's 
	// display function to accept arguments, and so until I do so, the following, with the game objects, must be global
	
	// [2026 port] When set, display() returns without drawing. The browser frame driver
	// (WebFrame in tankmain.cp) uses it to run several simulation steps per animation frame while
	// rendering only the final state -- MainGameLoop draws on every step, which would otherwise
	// mean up to N full scene renders per frame.
	int SuppressDisplay = 0;

	float camera_x;
	float camera_y;
	float camera_z;

	float lineOfSight_x;
	float lineOfSight_y;
	float lineOfSight_z;
	
	float ObjectiveCameraDirectionRightLeft = 120;
	float CurrentCameraDirectionRightLeft = 120;
	float CurrentCameraDistance = -1 * MAP_BLOCK_SIZE;
	float ObjectiveCameraDistance = 3 * MAP_BLOCK_SIZE;
	float CameraShakeTime = 0;
	float ObjectSpin = 0;
	float ballx=0;
	float ballz=0;
	float TransitionCameraDistance;
	float TransitionCameraAngle;
	int TransitionCounter;
	float StartCameraDistance;

#endif
