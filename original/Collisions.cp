//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			Collisions.cpp																															
////	Author:			Ed Martin
////	Description:	Holds the collision detection functions and responses																
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef EVENTS
	#define EVENTS
	float events[10*40];
    void Event(int description, float a, float b, float c, float d, float e, float f, float g, float h, float i) {}
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					A)  CheckPlayerMapCollisions			
////					
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TankClass CheckPlayerMapCollisions(int Map[], TankClass Player1, TubeClass Tube[])
{
	float tankwidth = 1.4;
	float tanklength = 1.4;	
	int i,j;

	float x,z,r;
	float sx,cx,sz,cz;
	float Ax,Bx,Cx,Dx,Az,Bz,Cz,Dz;
	float checkx,checkz;	
	int checki, checkj;
	
//// 1) First the four corners of the tank are found using trig 
////	(A and B - Front left and right. C and D - Back left and right).

	sx = sin(Player1.GettrackDirection() * 2 * M_PI / 360) * (tanklength/2);
	cx = cos(Player1.GettrackDirection() * 2 * M_PI / 360) * (tankwidth/2);
	sz = sin(Player1.GettrackDirection() * 2 * M_PI / 360) * (tankwidth/2);
	cz = cos(Player1.GettrackDirection() * 2 * M_PI / 360) * (tanklength/2);

	Ax = Player1.Getx() + sx + cx;			Az = Player1.Getz() + cz - sz;
	Bx = Player1.Getx() + sx - cx;			Bz = Player1.Getz() + sz + cz;
	Cx = Player1.Getx() - sx + cx;			Cz = Player1.Getz() - cz - sz;
	Dx = Player1.Getx() - sx - cx;			Dz = Player1.Getz() + sz - cz;

//// 2) The points that are checked for collisions are CollisionResolution many points along 
////    the front and back bumper - 
////    Half from A to B (the front bumper), and half from C to D (the back bumper).
////    - These checking points are [checkx, checkz].
////    - Checking points are compared with only the 25 surrounding Map coordinates for collisions,
////	  [checki, checkj] is the Map coordinate of the tanks position.

	int CollisionResolution = 16;
	checki = int(Player1.Getx()/MAP_BLOCK_SIZE);
	checkj = int(Player1.Getz()/MAP_BLOCK_SIZE);
	
	for (i = checki - 2; i <= checki + 2; i++)
	  for (j = checkj - 2; j <= checkj + 2; j++)
		if (Map[i+MAP_X*j] >= 2)
		  for (r = 0; r < CollisionResolution; r++)
	{
		if (r<CollisionResolution/2) 
		{
			checkx = Ax + (Bx - Ax)*(r)/(CollisionResolution/2-1);
			checkz = Az + (Bz - Az)*(r)/(CollisionResolution/2-1);
		}
		else
		{
			checkx  = Dx + (Cx - Dx)*(r - CollisionResolution/2)/(CollisionResolution/2-1);
			checkz  = Dz + (Cz - Dz)*(r - CollisionResolution/2)/(CollisionResolution/2-1);
		}
			
		if ((checkx > MAP_BLOCK_SIZE*i) && (checkx < MAP_BLOCK_SIZE*(i+1)) &&
			(checkz > MAP_BLOCK_SIZE*j) && (checkz < MAP_BLOCK_SIZE*(j+1)))
		{ 

//// 3) if the Map coordinate of [checkx, checkz] is a wall or tower, then...

			if (  (Map[i+MAP_X*j] == WALL) || (Map[i+MAP_X*j] == ARCH) || (Map[i+MAP_X*j] == TOWER) ||
				 (( (Tube[0].GetState() != INACTIVE) && (Tube[0].GetState() != SMOKING) ) && (Map[i+MAP_X*j] == RAIL)) 
				)
			{				
				
////	- move tank back to where it was before the collision and reverse speed
				//r = CollisionResolution + 1;
				Player1.Setx( Player1.Getx() - Player1.GetSpeed() * sin(2 * M_PI * Player1.GettrackDirection() / 360) );
				Player1.Setz( Player1.Getz() - Player1.GetSpeed() * cos(2 * M_PI * Player1.GettrackDirection() / 360) );
				Player1.SetSpeed( - Player1.GetSpeed());
				
////	- bounce tank back from object in opposite direction of collision.

				x = (Player1.Getx() - checkx);
				z = (Player1.Getz() - checkz);
				x = 0.3*MAP_BLOCK_SIZE * x / sqrt(x*x + z*z);
				z = 0.3*MAP_BLOCK_SIZE * z / sqrt(x*x + z*z);
				Player1.Setx(Player1.Getx() + x);
				Player1.Setz(Player1.Getz() + z);
			}

//// 4) If the Map coordinate of [checkx, checkz] is an event trigger (0-9,R,D,M,etc), then trigger the event.

			if (  ((Map[i+MAP_X*j] >= 1) && (Map[i+MAP_X*j] <= 9)) || 
				  (Map[i+MAP_X*j] == RAPID_FIRE) || (Map[i+MAP_X*j] == DUAL_FIRE) || (Map[i+MAP_X*j] == MED_PACK) )
			{
				Event(NEW_MAP_EVENT, Map[i+MAP_X*j], i+MAP_X*j, 0, 0,0,0, 0,0,0);
			}
										
			// Since a collision has been detected, leave loop
			r = CollisionResolution;
		}
	}
	return Player1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					B)  CheckBulletMapCollisions
////
////						1) For each frame, collisions are checked for each active bullet individually for 
////						   10 points between it's previous position and it's current one by using the bullets 
////						   speed vector [dx,dy,dz]. 
////						   These checking points are [checkx, checky, checkz].
////						2) The corresponding map coordinate of each checking point [checkx, checkz] is [checki, checkj]
////						3) Depending on the value of the Map at [checki, checkj], the correct action is taken (explode,etc)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CheckBulletMapCollisions(int Map[], BulletClass Bullet[])
{			
	float checkx, checky, checkz;
	int b,checki,checkj;
	float r;

//// 1) For each frame, collisions are checked for each active bullet individually for 
////	10 points between it's previous position and it's current one by using the bullets 
////	speed vector [dx,dy,dz]. 
////	These checking points are [checkx, checky, checkz].

	for ( b=0; b < MAX_BULLETS; b++)
	 if (Bullet[b].GetActive() == 1)
	  for (r=1 ; r >= 0; r-= 0.1)
	{	
		checkx = Bullet[b].Getx() - r * Bullet[b].Getdx();
		checky = Bullet[b].Gety() - r * Bullet[b].Getdy();
		checkz = Bullet[b].Getz() - r * Bullet[b].Getdz();

//// 2) The corresponding map coordinate of each checking point [checkx, checkz] is [checki, checkj]

		checki = int(checkx/MAP_BLOCK_SIZE);
		checkj = int(checkz/MAP_BLOCK_SIZE);
		
//// 3) Depending on the value of the Map at [checki, checkj], the correct action is taken (explode,etc), and stop checking (r=0)
		
		// if Bullet is out of sight, disable it quietly
		if ((Map[checki+MAP_X*checkj] == 0) && (checky<=-10)) 
		{
			Bullet[b].SetState(INACTIVE);
			r = 0;
		}
				
		// if Collision object is a ground block or railway, explode.
		if ( ((Map[checki+MAP_X*checkj] == GROUND)||(Map[checki+MAP_X*checkj] == RAIL)||(Map[checki+MAP_X*checkj] == RAILSLEEPER)) && (checky<= 0)) 
		{
			Bullet[b].Explode(checkx, checky, checkz);
			r = 0;
		}
				
		// if Collision object is a wall, explode
		if ((Map[checki+MAP_X*checkj] == WALL) && (checky<=wall[4] )) 
		{
			Bullet[b].Explode(checkx, checky, checkz);
			r = 0;
		}
			
		// if Collision object is a tower, explode depending on height.
		if ( (Map[checki+MAP_X*checkj] == TOWER) && (checky<=tower[4] - 0.5)) 
		{
			Bullet[b].Explode(checkx, checky, checkz);
			r = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					C)  CheckBulletPlayerCollisions
////
////						Like map collisions, bullet collisions only occur in the tank's main base rectangle
////						To simplify the problem, each active bullet's position is found relative to the tank's forward facing
////						direction. 
////						- AuxXSQUARED holds how far in front of the tank a bullet is (in the tank's direction)
////						- AuxZSQUARED holds how far right of the tank a bullet is (perpendicular to the tank's direction)
////
////						To do this:
////						1) The distance between the tank and the bullet is found
////						2) the angle between the tank and the bullet is found
////						3) Trig is used to create AuxXSQUARED and AuxZSQUARED
////						4) A collision can now be accurately detected if AuxX and AuxZ fall within the tank's width and length
////						5) On a collision, the correct action is taken
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TankClass CheckBulletPlayerCollisions(TankClass Player1, BulletClass Bullet[])
{
	float tankwidth = 1.4;
	float tanklength = 1.2;
	float xzDistanceToBulletSQUARED, alpha, AuxXSQUARED, AuxZSQUARED;
	int b;

	for ( b=0; b < MAX_BULLETS; b++)
	if ((Bullet[b].GetActive() == ACTIVE) && (Bullet[b].Gety() < 2))
	{
		
//// 1) The distance between the tank and the bullet is found

		xzDistanceToBulletSQUARED = (Bullet[b].Getx() - Player1.Getx())*(Bullet[b].Getx() - Player1.Getx()) + 
									(Bullet[b].Getz() - Player1.Getz())*(Bullet[b].Getz() - Player1.Getz());
									
//// 2) the angle between the tank and the bullet is found

		alpha = (360 / M_PI/ 2) *  atan( (Bullet[b].Getx() - Player1.Getx()) / (Bullet[b].Getz() - Player1.Getz()) );
		if (Bullet[b].Getz() - Player1.Getz() < 0) alpha += 180;
		if (alpha<0) alpha += 360;
		
		alpha -= Player1.GettrackDirection();
		
//// 3) Trig is used to create AuxXSQUARED and AuxZSQUARED

		AuxXSQUARED = cos(2 * M_PI / 360 * alpha) * xzDistanceToBulletSQUARED; 
		AuxZSQUARED = sin(2 * M_PI / 360 * alpha) * xzDistanceToBulletSQUARED;

//// 4) A collision can now be accurately detected if AuxX and AuxZ fall within the tank's width and length
//// 5) On a collision, the correct action is taken

		if ((AuxXSQUARED < tanklength*tanklength/2) && (AuxXSQUARED > - tanklength*tanklength/2) && 
			(AuxZSQUARED < tankwidth*tankwidth/2) && (AuxZSQUARED > -tankwidth*tankwidth/2))
		{
			Bullet[b].Explode(Bullet[b].Getx(), 0.7, Bullet[b].Getz());
			CameraShakeTime = MAX_CAMERA_SHAKE_TIME;
			if (RunMode != END) Player1.DecreaseHealth(10);
		}
	}
	return Player1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
////						   correct action is taken.
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TurretClass CheckBulletEnemyCollisions(TurretClass Turret, BulletClass Bullet[])
{	
	int b;
	float r;
	float checkx, checky, checkz, DistToBullet;
	
//// 1) For each frame, collisions are checked for each active bullet individually for 
////    10 points between it's previous position and it's current one by using the bullets 
////	speed vector [dx,dy,dz]. 
////    These checking points are [checkx, checky, checkz].

	for ( b=0; b < MAX_BULLETS; b++) if ((Bullet[b].GetActive() == ACTIVE) || (Bullet[b].GetActive() == EXPLODING))
		if (Turret.GetState() == ACTIVE)
			for (r = 0; r <= 1 ; r+=0.1) 
	{	
				checkx = Bullet[b].Getx() - r * Bullet[b].Getdx();
				checky = Bullet[b].Gety() - r * Bullet[b].Getdy();
				checkz = Bullet[b].Getz() - r * Bullet[b].Getdz();
				
//// 2) The distance between Turret and Bullet for each checking point is calculated.

				DistToBullet = pow( (Turret.Getx()-checkx) * (Turret.Getx()-checkx) +
									(Turret.Gety()-checky) * (Turret.Gety()-checky) +
									(Turret.Getz()-checkz) * (Turret.Getz()-checkz)   , 0.33333) ;

//// 3) The turrets are spherical, so if the distance between the bullet and turret are less than the
////    the radius of the turret and the radius of the bullet (or bullet explosion), then the
////    correct action is taken.

				if ( (Bullet[b].GetActive() == ACTIVE) && ( DistToBullet <  ENEMY_TURRET_SIZE) )
				{
					Bullet[b].Explode(checkx, checky, checkz);
					r = 1;
					Turret.DecreaseHealth(10);

				}
				if ( (Bullet[b].GetActive() == EXPLODING) && ( DistToBullet <  Bullet[b].GetexplosionRadius() + ENEMY_TURRET_SIZE) )// + MAP_BLOCK_SIZE/2)) )
				{
					r = 1;
					Turret.DecreaseHealth(1);
				}
	}
	return Turret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

TubeClass CheckBulletTubeCollisions(BulletClass Bullet[], TubeClass Tube)
{
	float checkx, checky, checkz;
	int b,checki,checkj;
	float r;

//// 1) For each frame, collisions are checked for each active bullet individually for 
////	10 points between it's previous position and it's current one by using the bullets 
////	speed vector [dx,dy,dz]. 
////	These checking points are [checkx, checky, checkz].

	for ( b=0; b < MAX_BULLETS; b++)
	 if (Bullet[b].GetActive() == 1)
	  for (r=1 ; r >= 0; r-= 0.1)
	{	
		checkx = Bullet[b].Getx() - r * Bullet[b].Getdx();
		checky = Bullet[b].Gety() - r * Bullet[b].Getdy();
		checkz = Bullet[b].Getz() - r * Bullet[b].Getdz();

//// 2) The corresponding map coordinate of each checking point [checkx, checkz] is [checki, checkj].

		checki = int(checkx/MAP_BLOCK_SIZE);
		checkj = int(checkz/MAP_BLOCK_SIZE);
						
//// 3) If the Tube is Active, the Map at [checki, checkj] is a Railsleeper, and the bullet is 
////    between the ends of the train and below the height of the train, explode

		if ( (Tube.GetState()!=INACTIVE) && (Bullet[b].GetType() == PLAYER_BULLET) && 
			((Map[checki+MAP_X*checkj] == RAIL) || (Map[checki+MAP_X*checkj] == RAILSLEEPER)) && 
			((checkx > Tube.Getx() - 1.5 * MAP_BLOCK_SIZE) && (checkx < Tube.Getx() + 1.5 * MAP_BLOCK_SIZE)) &&
			((checkz > Tube.Getz() - 8 * MAP_BLOCK_SIZE) && (checkz < Tube.Getz() + 8 * MAP_BLOCK_SIZE)) &&
			(checky<= 7 * MAP_BLOCK_SIZE))
		{
			Bullet[b].Explode(checkx, checky, checkz);	

//// 4) If the bullet is in one of the three windows and the tube is vulnerable, damage the tube

			if (	(	(checky > Tube.Gety() + TubeWindow[1]) && (checky <  Tube.Gety() + TubeWindow[5])	) 
					&&
					(	
						( 
							((checkz > Tube.Getz() + TubeWindow[0] + 5 * MAP_BLOCK_SIZE ) && (checkz < Tube.Getz() + TubeWindow[2] + 5 * MAP_BLOCK_SIZE) ) 
							||
							((checkz > Tube.Getz() + TubeWindow[0]) && (checkz < Tube.Getz() + TubeWindow[2]) ) 
							||
							((checkz > Tube.Getz() + TubeWindow[0] - 5 * MAP_BLOCK_SIZE) && (checkz < Tube.Getz() + TubeWindow[2] - 5 * MAP_BLOCK_SIZE) ) 	
						) 
					)
				)
			if (Tube.GetState()==TUBE_STATE_OPEN_FIRE)	
			{
				Tube.DecreaseHealth(10);
			}
		}
	}
	return Tube;
}
