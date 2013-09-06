#ifndef CMINESWEEPER_H
#define CMINESWEEPER_H

//------------------------------------------------------------------------
//
//	Name: CMineSweeper.h
//
//  Author: Mat Buckland 2002
//
//  Desc: Class to create a minesweeper object 
//
//------------------------------------------------------------------------
#include <vector>
#include <math.h>
#include "CArtificalNeuralNet.h"
#include "utils.h"
#include "C2DMatrix.h"
#include "SVector2D.h"
#include "CParams.h"
#include "CCollisionObject.h"
using namespace std;


class CMinesweeper
{

private:

	//the ANN (Artificial Neural Network) of the minesweeper
	CArtificalNeuralNet m_vANN;

	//its position in the world
	SVector2D m_vPosition;

	//direction sweeper is facing
	SVector2D m_vLookAt;

	//its rotation (surprise surprise)
	double m_dRotation;

	double m_dSpeed;
	
	// Left and Right Track are outputed from the ANN
	double m_leftTrack;
	double m_rightTrack;

	// Sweepers fitness
	double m_dFitness;

	//the number of Mines gathered by the sweeper 
	double m_dMinesGathered;
	//the number of rocks driven over by the sweeper
	double m_dRocksGathered;

	//the scale of the sweeper when drawn
	double m_dScale;

	//index position of closest mine
	int m_iClosestMineOrRock;

	//index position of closest rock
	//int m_iClosestRock;

public:

	CMinesweeper();
	
	//updates the information from the sweepers enviroment
	bool Update(vector<CCollisionObject> &objects);

	//used to transform the sweepers vertices prior to rendering
	void WorldTransform(vector<SPoint> &sweeper);

	//returns a vector to the closest object
	SVector2D GetClosestMineOrRock(vector<CCollisionObject> &objects);

	//checks to see if the minesweeper has 'collected' a mine
	int CheckForMineOrRock(vector<CCollisionObject> &objects, double size);

	void Reset();
  

	//-------------------accessor functions
	SVector2D	Position()const{return m_vPosition;}

	void IncrementMinesGathered(){++m_dMinesGathered;}
	void IncrementRocksGathered(){++m_dRocksGathered;}
	double GetRotation(){return m_dRotation;}
	double const MinesGathered(){return m_dMinesGathered;}
	double const RocksGathered(){return m_dRocksGathered;}
  	void IncrementFitness(){m_dFitness+=0.8;}
	void DecrementFitness(){m_dFitness-=10;}
	double const GetFitness(){return m_dFitness;}
	void ReplaceWeightsInANN(vector<double> &w){m_vANN.ReplaceWeightsInANN(w);}
	int const GetNumberOfWeightsInANN(){return m_vANN.GetNumberOfWeightsNeeded();}
};


#endif	