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
#include "utils.h"
#include "C2DMatrix.h"
#include "SVector2D.h"
#include "CParams.h"
#include "CCollisionObject.h"
#include "CNode.h"
#include "CMlp.h"

using namespace std;


class CMinesweeper
{

private:
	//its position in the world
	SVector2D		m_vPosition;

	//direction sweeper is facing
	SVector2D		m_vLookAt;

	//its rotation (surprise surprise)
	double			m_dRotation;

	double			m_dSpeed;
	
	//the number of Mines gathered by the sweeper 
	double			m_dMinesGathered;

	double			m_dSuperMinesGathered;

	//the scale of the sweeper when drawn
	double			m_dScale;

  //index position of closest mine
  int         m_iClosestMine;

  int		m_iClosestSuperMine;

  bool		minesForwardLeft;
  bool		minesForwardRight;
  bool		minesLeft;
  bool		minesRight;

  bool active;
  

public:


	CMinesweeper();
	
	//updates the information from the sweepers enviroment
	bool			Update(vector<CCollisionObject> &objects, vector<CMinesweeper> &tanks, CMlp &mlp);

	//used to transform the sweepers vertices prior to rendering
	void			WorldTransform(vector<SPoint> &sweeper);

	vector<int> GetNearbySupermines(vector<CCollisionObject> &objects, double range);

	vector<int> GetNearbySweepers(vector<CMinesweeper> &tanks, double range);

	//returns a vector to the closest object
	SVector2D	GetClosestMine(vector<CCollisionObject> &objects);
	SVector2D   GetClosestSuperMine(vector<CCollisionObject> &objects);

	//checks to see if the minesweeper has 'collected' a mine
	int       CheckForMine(vector<CCollisionObject> &objects, double size);

	int			CheckForSuperMine(vector<CCollisionObject> &objects, double size);

	bool		CheckCollides(CCollisionObject &object, double size);

	void			Reset(vector<CCollisionObject> &objects);
  

	//-------------------accessor functions
	SVector2D	Position()const{return m_vPosition;}

	// mines
	void		IncrementMinesGathered(){++m_dMinesGathered;}
	double		MinesGathered()const{return m_dMinesGathered;}

	void		SetInactive();
	bool		IsActive();


  
};


#endif

	
	