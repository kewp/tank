
Readme.txt for Tank Sample game, written as a demo by Ed Martin, 

FILES

	The tank game is made up of the following source files (explained below):		GameConstants.cp		main.cp											MainGameRoutines.cp		TankClass.cp		TurretClass.cp		TubeClass.cp
		BulletClass.cp			Collisions.cp		Control.cp		GLConstants.cp		GLTextures.cp		GLGameObjectDrawFunctions.cp		GLGeneral.cp

	
RUNNING PROCEDURE

	The typical running procedure in main.cp goes something like this
	
		ACTION							USING A ROUTINE IN:

		OpenGL GLUT is initiated (graphics, mouse, keyboard)  	main.cp, 
									Control.cp
		The Game variables and objects are initiated		MainGameRoutines.cp
		The Game loop is initiated				MainGameRoutines.cp
		The OpenGL loop is initiated				GLTextures.cp

	The Game then loops forever in the Main Game Loop, found in MainGameRoutines.cp:

 		ACTION							USING A ROUTINE IN:

		Process new events (new bullets, map triggers, etc.)	MainGameRoutines.cp
		Update Animation timers					/
		Update Player (movement, health, etc) 			TankClass.cp
		Update Active Turrets					TurretClass.cp
		Update Active Tubes					TubeClass.cp
		Update Active Bullets					BulletClass.cp
		Check for Collisions					Collisions.cp
		Position Camera						GLGeneral.cp
		Draw Scene						GLGeneral.cp USING GLGameObjectDrawFunctions USING GLConstants

	(A more detailed description can be found in DESCRIPTION OF FILES - MainGameRoutines.cp below)

OTHER NOTES
	
	Not included are the necessary OpenGL packages ( GLUT/glut.h )
	
	I've been writing this for the past 2 months and as yet have only compiled and tested it on a 2.16GHz processor, 1Gb RAM MacBook.
	I don't see any reason that it shouldn't compile effortlessly on a PC (he says with a sort of naive optimism). 
	
	The timers are based on a per second basis, and should thus run at the same speed on each system, as long as the FRAME_RATE variable 
	is aptly set, but again this hasn't been tested on other systems as yet.	

THINGS I SHOULD PROBABLY WARN YOU ABOUT

	It was made initially for screenshots and videos, and isn't currently running in full screen.
	
	The GLUT Mouse function is based on the screen coordinates of the mouse, so the mouse gets stuck at the edges of the screen 
	(this means that the tank turret also stops moving right or left at the edge of the screen)
	
	Pointers are currently not being used because I've been having a problem initializing arrays of them (causing bad memory usage).

	Not being able to pass arguments to the OpenGL Display function, many variables are global.
		
	If a map object is directly behind the player between the camera and the player, the object is still drawn, albeit blocking the view

	That said, the current version is bug free.

DESCRIPTION OF FILES 

	More detailed comments can be found in the files themselves, the following comes from the first few lines of each file:
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			GameConstants.cp													
////	Author:			Ed Martin
////	Description:		Contains every tweakable gameplay variable, constants, and the unescapably global variables (see note)
////		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		////	File:			main.cp													
////	Author:			Ed Martin////	Description:		tank sample game main function									
////////	Contents:		////					1) General include files (math, stdio, stdlib, GameConstants.cp)////					2) Event Handler////					3) Game include files and game object constructors////					4) Initialize OpenGL////					5) main()//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
////	File:			MainGameRoutines.cp
////	Author:			Ed Martin
////	Description:		Holds the Main Game routines - the event processor, the game initializers and the main game loop
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
////							a) Update Animation timers
////							b) Position Camera		
////							c) Draw Scene
////
////						MODE 3) TUBE_APROACHING		- Camera circling player while boss approaches
////							a) Process new events (new bullets, map triggers, etc.)
////							b) Update Animation timers
////							c) Update Active Tubes
////							d) Update Active Bullets
////							e) Check for Collisions and update Player only if in the way of the oncoming train
////							f) Position Camera - Gradually make player face forward (so that he notices the turrets firing at him)
////							g) Draw Scene
////
////						MODE 4) NORMAL GAMEPLAY LOOP :
////							a) Process new events (new bullets, map triggers, etc.)
////							b) Update Animation timers
////							c) Update Player (movement, health, etc) 
////							d) Update Active Turrets
////							e) Update Active Tubes
////							f) Update Active Bullets
////							g) Check for Collisions
////							h) Position Camera
////							i) Draw Scene
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			TankClass.cp			
////	Author:			Ed Martin
////	Description:		Holds the TankClass Object								
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
////		
////	File:			TurretClass.cp											
////	Author:			Ed Martin
////	Description:		Holds the TurretClass Object
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
////		
////	File:			TubeClass.cp												
////	Author:			Ed Martin
////	Description:		Holds the TubeClass Objects
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
////		
////	File:			BulletClass.cp					
////	Author:			Ed Martin
////	Description:		Holds the BulletClass Object
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
////		
////	File:			Collisions.cpp											
////	Author:			Ed Martin
////	Description:		Holds the collision detection functions and responses		
////
////	Contents:		
////					A)  CheckPlayerMapCollisions			
////					
////						1) First the four corners of the tank are found using trig 
////						   (A and B - Front left and right. C and D - Back left and right).
////						2) The points that are checked for collisions are CollisionResolution many points along 
////						   the front and back bumper - 
////						   Half from A to B (the front bumper), and half from C to D (the back bumper).
////						   - These checking points are [checkx, checkz].
////						   - Checking points are compared with only the 25 surrounding Map coordinates for collisions.
////						3) if the Map coordinate of [checkx, checkz] is a wall or tower, then
////						   - move tank back to where it was before the collision and reverse speed.
////						   - bounce tank back from object in opposite direction of collision.
////						4) If the Map coordinate of [checkx, checkz] is an event trigger, then trigger the event.
////						
////					B)  CheckBulletMapCollisions
////
////						1) For each frame, collisions are checked for each active bullet individually for 
////						   10 points between it's previous position and it's current one by using the bullets 
////						   speed vector [dx,dy,dz]. 
////						   These checking points are [checkx, checky, checkz].
////						2) The corresponding map coordinate of each checking point [checkx, checkz] is [checki, checkj].
////						3) Depending on the value of the Map at [checki, checkj], the correct action is taken (explode,etc).
////
////					C)  CheckBulletPlayerCollisions
////
////						To simplify the problem, each active bullet's position is found relative to the tank's forward facing
////						direction. 
////						- AuxXSQUARED holds how far in front of the tank a bullet is (in the tank's forward direction).
////						- AuxZSQUARED holds how far right of the tank a bullet is (perpendicular to the tank's forward direction).
////
////						To do this:
////						1) The distance between the tank and the bullet is found.
////						2) the angle between the tank and the bullet is found.
////						3) Trig is used to create AuxXSQUARED and AuxZSQUARED.
////						4) A collision can now be accurately detected if AuxX and AuxZ fall within the tank's width and length.
////						5) On a collision, the correct action is taken.
////
////					D)  CheckBulletEnemyCollisions
////
////						1) For each frame, collisions are checked for each active bullet individually for 
////						   10 points between it's previous position and it's current one by using the bullets 
////						   speed vector [dx,dy,dz]. 
////						   These checking points are [checkx, checky, checkz].
////						2) The distance between Turret and Bullet for each checking point is calculated.
////						3) The turrets are spherical, so if the distance between the bullet and turret are less than the
////						   the radius of the turret and the radius of the bullet (or bullet explosion), then the
////						   take correct action is taken.
////
////					E)  CheckBulletTubeCollisions
////
////						1) For each frame, collisions are checked for each active bullet individually for 
////						   10 points between it's previous position and it's current one by using the bullets 
////						   speed vector [dx,dy,dz]. 
////						   These checking points are [checkx, checky, checkz].
////						2) The corresponding map coordinate of each checking point [checkx, checkz] is [checki, checkj].
////						3) If the Tube is Active, the Map at [checki, checkj] is a Railsleeper, and the bullet is 
////						   between the ends of the train and below the height of the train, explode
////						4) If the bullet is in one of the three windows and the tube is vulnerable, damage the tube
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			Control.cp												
////	Author:			Ed Martin
////	Description:		Holds the Keyboard and mouse handlers that in turn call routines for TankClass instance Player1		
////		
////					1) InGameKeystrokeUp	-	Capture presses of keys during game play (note the PAUSED and TOP_VIEW)
////					2) InGameKeystrokeUp	-	Capture release of keys during game play (note the PAUSED and TOP_VIEW)
////					3) InGameMouseMove		-	Used in the game to move the tank turret
////					4) InGameMouseButtons	-	Used for start menu only
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			GLConstants.cp												
////	Author:			Ed Martin
////	Description:		The Data for the draw functions in GLGameObjectDrawFunctions.cp, and the text Map used in the game		
////		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			GLTextures.cp												
////	Author:			Silicon Graphics, Ed Martin
////	Description:		Texture loading code. Came with the Xcode free samples
////				Can only work from 256x256 pixel sgi images.
////
////	Contents:		
////					1) Section written entirely by Silicon Graphics
////
////					2) Functions used by tank to load samples (written by Ed Martin)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			GLGameObjectDrwFunctions.cp										
////	Author:			Ed Martin
////	Description:		Draw Functions for the graphic objects (mostly GL_QUAD_STRIPs)							
////
////	Contents:		
////					void DrawHealthBar					(float camera_x, float camera_y, float camera_z, float lives, float health_ratio)
////					void DrawSky						(float camera_x, float camera_y, float camera_z)
////					void DrawStartScreenWallMounting			(float x, float y, float z)
////					void DrawGround						(float x, float y, float z)
////					void DrawWall						(float x, float y, float z)
////					void DrawTower						(float x, float y, float z)
////					void DrawArch						(float x, float y, float z, float length)
////					void DrawRail						(float x, float y, float z)
////					void DrawRailSleeper					(float x, float y, float z)
////					void DrawRapidFire					(float x, float y, float z, float random, float ObjectSpin)
////					void DrawDualFire					(float x, float y, float z, float random, float ObjectSpin)
////					void DrawMedPack					(float x, float y, float z, float random, float ObjectSpin)
////		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			GLGeneral.cp												
////	Author:			Ed Martin
////	Description:		Holds the OpenGL Drawing, camera positioning and window resizing routines					
////	
////	Contents:		
////					1) Map Drawing Function
////					2) Camera Positioning Function
////					3) Window Reshape Function
////					4) Main OpenGL callback display function
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
