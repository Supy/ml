#include "CMinesweeper.h"

//-----------------------------------constructor-------------------------
//
//-----------------------------------------------------------------------
CMinesweeper::CMinesweeper():
                             m_dRotation(RandFloat()*CParams::dTwoPi),
							 m_dMinesGathered(0),
							 m_dRocksGathered(0),
                             m_leftTrack(0.8),
                             m_rightTrack(0.8),
                             m_dFitness(0),
							 m_dScale(CParams::iSweeperScale),
                             m_iClosestMineOrRock(0)
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
	m_dRocksGathered = 0;
	//RocksGathered

	//and the rotation
	m_dRotation = RandFloat()*CParams::dTwoPi;

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
//	So given a force for each track we calculate the resultant rotation 
//	and acceleration and apply to current velocity vector.
//
//-----------------------------------------------------------------------
bool CMinesweeper::Update(vector<CCollisionObject> &objects)
{
	// Vector which stores Neural Network inputs
	vector<double> neuralNetworkInputs;	

	//get vector to closest mine
	SVector2D vClosestMineOrRock = GetClosestMineOrRock(objects);
	//normalise it
	Vec2DNormalize(vClosestMineOrRock);
	
	// Vector to closest mine added as input to ANN
	neuralNetworkInputs.push_back(vClosestMineOrRock.x);
	neuralNetworkInputs.push_back(vClosestMineOrRock.y);

	// Vector to vehicles lookAt added as input to ANN
	neuralNetworkInputs.push_back(m_vLookAt.x);
	neuralNetworkInputs.push_back(m_vLookAt.y);

	// Get ANNs Output by giving the ANN the NeuralNetworkInputs
	vector<double> neuralNetworkOutput = m_vANN.CalculateOutput(neuralNetworkInputs);

	// The output from the ANN is assigned to the left and right track of the sweeper respectively
	m_leftTrack = neuralNetworkOutput[0];
	m_rightTrack = neuralNetworkOutput[1];

	//calculate steering forces
	double RotForce = m_leftTrack - m_rightTrack;

	//clamp rotation
	Clamp(RotForce, -CParams::dMaxTurnRate, CParams::dMaxTurnRate);
	m_dRotation += RotForce;

	// Speed calculated from the combination of the force applied to the left and right track
	m_dSpeed = (m_leftTrack + m_rightTrack);

	//update Look At 
	m_vLookAt.x = -sin(m_dRotation);
	m_vLookAt.y = cos(m_dRotation);

	//update position
	SVector2D oldPosition = m_vPosition;
	m_vPosition += (m_vLookAt * m_dSpeed);

	// The Fitness score is adjusted for the sweeper (positive bonus) if the turn (RotForce) is less than the rotationThreshold
	// This reduces the sweepers from spiraling out of control 
	// If the fitness was incremented for each frame that the sweeper does not collide with a mine
	// The sweeper would maximize its fitness by spiraling (rotating in a circular motion)
	// Thus this is a reward (increment of the sweepers fitness) for not turning too drastically.
	const float rotationThreshold = 0.06f;   
   
	if (fabs(RotForce) < rotationThreshold)   
	{	  
		IncrementFitness();  
	}   
   
	//wrap around window limits
	if (m_vPosition.x > CParams::WindowWidth) m_vPosition.x = 0;
	if (m_vPosition.x < 0) m_vPosition.x = CParams::WindowWidth;
	if (m_vPosition.y > CParams::WindowHeight) m_vPosition.y = 0;
	if (m_vPosition.y < 0) m_vPosition.y = CParams::WindowHeight;

	return true;
}


//----------------------GetClosestObject()---------------------------------
//
//	returns the vector from the sweeper to the closest mine or rock (closest one)
//
//-----------------------------------------------------------------------
SVector2D CMinesweeper::GetClosestMineOrRock(vector<CCollisionObject> &objects)
{
	double			closest_so_far = 99999;

	SVector2D		vClosestObject(0, 0);

	//cycle through objects to find closet mine or rock to find closest
	for (int i=0; i<(int)objects.size(); i++)
	{
		if (objects[i].getType()==CCollisionObject::Mine || objects[i].getType()==CCollisionObject::Rock){
		double len_to_object = Vec2DLength(objects[i].getPosition() - m_vPosition);

		if (len_to_object < closest_so_far)
		{
			closest_so_far	= len_to_object;
			
			vClosestObject	= m_vPosition - objects[i].getPosition();

			m_iClosestMineOrRock = i;
		}
		}
	}

	return vClosestObject;
}
//----------------------------- CheckForMine -----------------------------
//
//  this function checks for collision with its closest mine or rock (calculated
//  earlier and stored in m_iClosestMineOrRock)
//-----------------------------------------------------------------------
int CMinesweeper::CheckForMineOrRock(vector<CCollisionObject> &objects, double size)
{
	SVector2D DistToObject = m_vPosition - objects[m_iClosestMineOrRock].getPosition();
		
	if (Vec2DLength(DistToObject) < (size + 5))
	{
			return m_iClosestMineOrRock;
	}

  return -1;
}


		
