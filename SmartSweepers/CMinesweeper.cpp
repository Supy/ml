#include "CMinesweeper.h"

//-----------------------------------constructor-------------------------
//
//-----------------------------------------------------------------------
CMinesweeper::CMinesweeper():
                             m_dRotation(RandFloat()*CParams::dTwoPi),
                             m_dMinesGathered(0),
							 m_dScale(CParams::iSweeperScale),
                             m_iClosestMine(0),
							 minesForwardLeft(false),
							 minesForwardRight(false),
							 minesLeft(false),
							 minesRight(false)
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
void CMinesweeper::Reset()
{
	//reset the sweepers positions
	m_vPosition = SVector2D((RandFloat() * CParams::WindowWidth), 
					                (RandFloat() * CParams::WindowHeight));
	
	//and the MinesGathered
	m_dMinesGathered = 0;

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
bool CMinesweeper::Update(vector<CCollisionObject> &objects, CMlp &mlp)
{
	
	//get vector to closest mine
	SVector2D vClosestMine = GetClosestMine(objects);
	//normalise it
	Vec2DNormalize(vClosestMine);

	minesLeft = minesRight = minesForwardLeft = minesForwardRight = false;

	// We want more than the closest mine. We want all mines within a certain radius to make a better decision
	// on which direction to turn.
	vector<int> nearbyObjectIndices = GetNearbyMines(objects, CParams::dMineScale + 20);

	for(int i=0; i < nearbyObjectIndices.size(); i++){
		SVector2D direction =   objects[nearbyObjectIndices[i]].getPosition() - m_vPosition;
		double angle = Vec2DAngle(m_vLookAt, direction) * 180 / CParams::dPi;

		// Which quadrant does the mine fall into
		if(angle >= -90.0 && angle < -35.0)
			minesLeft = true;
		else if(angle >= -35.0 && angle < 0.0)
			minesForwardLeft = true;
		else if(angle >= 0.0 && angle < 35.0)
			minesForwardRight = true;
		else if(angle >= 35.0 && angle <= 90.0)
			minesRight = true;
	}

	// Set the inputs to the MLP of where the mines are located.
	mlp.SetNodeInput(0, minesLeft);
	mlp.SetNodeInput(1, minesForwardLeft);
	mlp.SetNodeInput(2, minesForwardRight);
	mlp.SetNodeInput(3, minesRight);

	// Our MLP outputs steering directions in the range 0.1-0.9. First we standardize it to 0-1.0.
	double output = (mlp.GetOutput(0) - 0.1) / 0.8;
	
	// Next we need to use this output in the form -0.3-0.3.
	double convertedRotation = (output * CParams::dMaxTurnRate * 2.0) - CParams::dMaxTurnRate;
	
	//TODO: calculate the steering forces here, it is set to 0 for now...
	double RotForce = convertedRotation;
	
	//clamp rotation
	Clamp(RotForce, -CParams::dMaxTurnRate, CParams::dMaxTurnRate);

	m_dRotation += RotForce;

	// Grab our calculated speed.
	m_dSpeed = mlp.GetOutput(1);	

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
vector<int> CMinesweeper::GetNearbyMines(vector<CCollisionObject> &objects, double range)
{
	double rsquared = range*range;
	SVector2D currentdelta;

	vector<int> nearbyObjects;
	//cycle through mines to find closest
	for (int i=0; i<objects.size(); i++)
	{
		currentdelta = objects[i].getPosition() - m_vPosition;
		double len_to_object = currentdelta.x * currentdelta.x + currentdelta.y * currentdelta.y;

		if (len_to_object <= range)
		{
			nearbyObjects.push_back(i);	
		}
	}

	return nearbyObjects;
}

//----------------------GetClosestObject()---------------------------------
//
//	returns the vector from the sweeper to the closest mine
//
//-----------------------------------------------------------------------
SVector2D CMinesweeper::GetClosestMine(vector<CCollisionObject> &objects)
{
	double			closest_so_far = 99999;

	SVector2D		vClosestObject(0, 0);

	SVector2D		cmv(0,0);

	//cycle through mines to find closest
	for (int i=0; i<objects.size(); i++)
	{
		cmv = objects[i].getPosition() - m_vPosition;
		double len_to_object = cmv.x * cmv.x + cmv.y * cmv.y;

		if (len_to_object < closest_so_far)
		{
			closest_so_far	= len_to_object;
			
			vClosestObject	= objects[i].getPosition() - m_vPosition;

      m_iClosestMine = i;
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
		
	if (Vec2DLength(DistToObject) < (size + 5))
	{
			return m_iClosestMine;
	}

  return -1;
}

bool CMinesweeper::CheckCollides(CCollisionObject &object, double size){
	SVector2D DistToObject = m_vPosition - object.getPosition();
		
	if (Vec2DLength(DistToObject) < (size + 5))
	{
			return true;
	}

	return false;
}

		
