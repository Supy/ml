#include "CMinesweeper.h"

//-----------------------------------constructor-------------------------
//
//-----------------------------------------------------------------------
CMinesweeper::CMinesweeper():
                             m_dRotation(RandFloat()*CParams::dTwoPi),
                             m_dMinesGathered(0),
							 m_dSuperMinesGathered(0),
							 m_dScale(CParams::iSweeperScale),
                             m_iClosestMine(0),
							 m_iClosestSuperMine(0),
							 minesForwardLeft(false),
							 minesForwardRight(false),
							 minesLeft(false),
							 minesRight(false),
							 active(true)
{
	//create a random start position
	m_vPosition = SVector2D((RandFloat() * CParams::WindowWidth), 
					                (RandFloat() * CParams::WindowHeight));
  
}

//-------------------------------------------Reset()--------------------
//
//	Resets the sweepers position, MinesGathered and rotation
//
//----------------------------------------------------------------------
void CMinesweeper::Reset(vector<CCollisionObject> &objects)
{
	double threshold = (CParams::dMineScale+5)*(CParams::dMineScale+5);
	boolean collision = false;
	do{
		//reset the sweepers positions but don't spawn on mines
		m_vPosition = SVector2D((RandFloat() * CParams::WindowWidth), 
										(RandFloat() * CParams::WindowHeight));
		collision = false;
		for(int i=0; i < objects.size(); i++){
			if(objects[i].getType() == CCollisionObject::SuperMine){
				if(Vec2DLengthSquared(m_vPosition-objects[i].getPosition()) <= threshold){
					collision = true;
					break;
				}
			}
		}
	}while(collision);

	//and the MinesGathered
	m_dMinesGathered = 0;
	m_dSuperMinesGathered = 0;

	//and the rotation
	m_dRotation = RandFloat()*CParams::dTwoPi;

	minesLeft = minesRight = minesForwardLeft = minesForwardRight = false;

	return;
}
//---------------------WorldTransform--------------------------------
//
//	sets up a translation matrix for the sweeper according to its
//  scale, rotation and position. Returns the transformed vertices.
//-------------------------------------------------------------------
void CMinesweeper::WorldTransform(vector<SPoint> &sweeper)
{
	//create the world transformation matrix
	C2DMatrix matTransform;
	
	//scale
	matTransform.Scale(m_dScale, m_dScale);
	
	//rotate
	matTransform.Rotate(m_dRotation);
	
	//and translate
	matTransform.Translate(m_vPosition.x, m_vPosition.y);
	
	//now transform the ships vertices
	matTransform.TransformSPoints(sweeper);
}

//-------------------------------Update()--------------------------------
//
//	First we take sensor readings. These are then fed into the learning algorithm
//
//	The inputs are:
//	
//	A vector to the closest mine (x, y)
//	The sweepers 'look at' vector (x, y)
//	So given a force we calculate the resultant rotation 
//	and acceleration. This is then applied to current velocity vector.
//
//-----------------------------------------------------------------------
bool CMinesweeper::Update(vector<CCollisionObject> &objects, vector<CMinesweeper> &tanks, CMlp &mlp)
{
	
	//get vector to closest mine
	SVector2D vClosestMine = GetClosestMine(objects);

	GetClosestSuperMine(objects);

	//normalise it
	Vec2DNormalize(vClosestMine);
	double angleToMine = Vec2DAngle(m_vLookAt, vClosestMine);
	double steering = 0.5;
	if(abs(angleToMine) > 0.5){
		if(angleToMine <= 0)
			steering = 0.0;
		else
			steering = 1.0;
	}


	minesLeft = minesRight = false;

	// We want more than the closest mine. We want all mines within a certain radius to make a better decision
	// on which direction to turn.
	vector<int> nearbyObjects = GetNearbySupermines(objects, CParams::dMineScale + 8);
	Vec2DNormalize(m_vLookAt);

	// Shift the point we measure angles from to the back of the sweeper
	// in order to reduce the area where the sweeper doesn't see stuff in front of it.
	SVector2D fakePosition = m_vPosition - (m_vLookAt*100);
	for(int i=0; i < nearbyObjects.size(); i++){

		if (objects[nearbyObjects[i]].getType() == CCollisionObject::ObjectType::SuperMine)
		{			
			SVector2D direction =   objects[nearbyObjects[i]].getPosition() - fakePosition;
			double angle = Vec2DAngle(m_vLookAt, direction) * 180 / CParams::dPi;

			// Which quadrant does the mine fall into
			if(angle >= -20.0 && angle < 0.0)
				minesLeft = true;
			else if(angle <= 20.0 && angle >= 0.0)
				minesRight = true;
		}
	}


	// Don't collide with other tanks either
	vector<int> nearbySweepers = GetNearbySweepers(tanks, CParams::iSweeperScale + 10);
	for(int i=0; i < nearbySweepers.size(); i++){		
		SVector2D direction =   tanks[nearbySweepers[i]].Position() - fakePosition;
		double angle = Vec2DAngle(m_vLookAt, direction) * 180 / CParams::dPi;

		// Which quadrant does the mine fall into
		if(angle >= -20.0 && angle < 0.0)
			minesLeft = true;
		else if(angle <= 20.0 && angle >= 0.0)
			minesRight = true;
	}



	// Set the inputs to the MLP of where the mines are located.
	mlp.SetNodeInput(0, minesLeft);
	mlp.SetNodeInput(1, minesRight);

	mlp.CalculateOutput();

	// Our MLP outputs steering directions in the range 0.1-0.9. First we standardize it to 0-1.0.
	double output = (mlp.GetOutput(0) - 0.1) / 0.8;
		
	if (output <= 0.499 || output >= 0.501) steering = output;

	// Next we need to use this output in the form -0.3-0.3.
	double convertedRotation = (steering * CParams::dMaxTurnRate * 2.0) - CParams::dMaxTurnRate;
	
	//TODO: calculate the steering forces here, it is set to 0 for now...
	double RotForce = convertedRotation;
	
	//clamp rotation
	Clamp(RotForce, -CParams::dMaxTurnRate, CParams::dMaxTurnRate);

	m_dRotation += RotForce;

	// Grab our calculated speed.
	m_dSpeed = (((mlp.GetOutput(1) - 0.1) / 0.8) * CParams::dMaxSpeed);	

	//update Look At 
	m_vLookAt.x = -sin(m_dRotation);
	m_vLookAt.y = cos(m_dRotation);

	//update position
	m_vPosition += (m_vLookAt * m_dSpeed);

	
	//wrap around window limits
	if (m_vPosition.x > CParams::WindowWidth) m_vPosition.x = 0;
	if (m_vPosition.x < 0) m_vPosition.x = CParams::WindowWidth;
	if (m_vPosition.y > CParams::WindowHeight) m_vPosition.y = 0;
	if (m_vPosition.y < 0) m_vPosition.y = CParams::WindowHeight;

	return true;
}

// Returns a vector of object ID's who are within a given radius of the Minesweeper
vector<int> CMinesweeper::GetNearbySupermines(vector<CCollisionObject> &objects, double range)
{
	double rangesquared = range*range;

	vector<int> nearbyObjects;
	//cycle through mines to find closest
	for (int i=0; i<objects.size(); i++)
	{
		if(objects[i].getType() == CCollisionObject::SuperMine){
			double len_to_object = Vec2DLengthSquared(objects[i].getPosition() - m_vPosition);

			if (len_to_object <= rangesquared)
			{
				nearbyObjects.push_back(i);	
			}	
		}
	}
	return nearbyObjects;
}

vector<int> CMinesweeper::GetNearbySweepers(vector<CMinesweeper> &tanks, double range){
	vector<int> nearbyTanks;
	double rangesquared = range*range;
	for(int i=0; i < tanks.size(); i++){
		if(&tanks[i] == this)
			continue;

		double len_to_object = Vec2DLengthSquared(tanks[i].Position() - m_vPosition);

		if (len_to_object <= rangesquared)
		{
			nearbyTanks.push_back(i);	
		}
	}

	return nearbyTanks;
}


//----------------------GetClosestObject()---------------------------------
//
//	returns the vector from the sweeper to the closest mine
//
//-----------------------------------------------------------------------
SVector2D CMinesweeper::GetClosestMine(vector<CCollisionObject> &objects)
{
	double			closest_so_far = 999999999;

	SVector2D		vClosestObject(0, 0);

	//cycle through mines to find closest
	for (int i=0; i<objects.size(); i++)
	{
		if(objects[i].getType() == CCollisionObject::Mine){
			double len_to_object = Vec2DLengthSquared(objects[i].getPosition() - m_vPosition);

			if (len_to_object < closest_so_far)
			{
				closest_so_far	= len_to_object;
			
				vClosestObject	= objects[i].getPosition() - m_vPosition;

				m_iClosestMine = i;
			}
		}
	}

	return vClosestObject;
}

SVector2D CMinesweeper::GetClosestSuperMine(vector<CCollisionObject> &objects)
{
	double			closest_so_far = 999999999;

	SVector2D		vClosestObject(0, 0);

	//cycle through mines to find closest
	for (int i=0; i<objects.size(); i++)
	{
		if(objects[i].getType() == CCollisionObject::SuperMine){
			double len_to_object = Vec2DLengthSquared(objects[i].getPosition() - m_vPosition);

			if (len_to_object < closest_so_far)
			{
				closest_so_far	= len_to_object;
			
				vClosestObject	= objects[i].getPosition() - m_vPosition;

				m_iClosestSuperMine = i;
			}
		}
	}

	return vClosestObject;
}

//----------------------------- CheckForMine -----------------------------
//
//  this function checks for collision with its closest mine (calculated
//  earlier and stored in m_iClosestMine)
//-----------------------------------------------------------------------
int CMinesweeper::CheckForMine(vector<CCollisionObject> &objects, double size)
{
	SVector2D DistToObject = m_vPosition - objects[m_iClosestMine].getPosition();
	double threshold = (size+5)*(size+5);

	if (Vec2DLengthSquared(DistToObject) < threshold)
	{
			return m_iClosestMine;
	}

  return -1;
}

int CMinesweeper::CheckForSuperMine(vector<CCollisionObject> &objects, double size)
{
	SVector2D DistToObject = m_vPosition - objects[m_iClosestSuperMine].getPosition();
	double threshold = (size+5)*(size+5);

	if (Vec2DLengthSquared(DistToObject) < threshold)
	{
			return m_iClosestSuperMine;
	}

  return -1;
}

bool CMinesweeper::CheckCollides(CCollisionObject &object, double size){
	SVector2D DistToObject = m_vPosition - object.getPosition();
	double threshold = (size+5)*(size+5);

	if (Vec2DLengthSquared(DistToObject) < threshold)
	{
			return true;
	}

	return false;
}

void CMinesweeper::SetInactive(){
	active = false;
}

bool CMinesweeper::IsActive(){
	return active;
}

		
