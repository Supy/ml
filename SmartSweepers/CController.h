#ifndef CCONTROLLER_H
#define CCONTROLLER_H

//------------------------------------------------------------------------
//
//	Name: CController.h
//
//  Author: Mat Buckland 2002
//
//  Desc: Controller class for the 'Smart Sweeper' example 
//
//------------------------------------------------------------------------
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <windows.h>
#include "CCollisionObject.h"
#include "CMinesweeper.h"
#include "utils.h"
#include "C2DMatrix.h"
#include "SVector2D.h"
#include "CParams.h"
#include "CMlp.h"

using namespace std;



class CController
{

private:

	//and the minesweepers
    vector<CMinesweeper> m_vecSweepers;

	//and the mines
	vector<CCollisionObject>	   m_vecObjects;
	
	int					         m_NumSweepers;

	int					         m_NumMines;
	int					         m_NumSuperMines;

	//vertex buffer for the sweeper shape's vertices
	vector<SPoint>		   m_SweeperVB;

	//vertex buffer for the mine shape's vertices
	vector<SPoint>		   m_MineVB;

	//stores the average MinesGathered per iteration for use 
	//in graphing.
	vector<double>		   m_vecAvMinesGathered;

	//stores the most MinesGathered per iteration
	vector<double>		   m_vecMaxMinesGathered;


	//stores the number of sweepers active at the end of each iteration
	//in graphing.
	vector<double>		   m_vecSweepersActive;


	// render/update state
	bool isFirstTick;
	bool hasTrained;
	bool hasRendered;

	// graph drawing things
	// mines
	int lastminecmciteration;
	double maxmaxmines;
	//SuperMines 
	int lastSuperMinecmciteration;
	double maxmaxSuperMines;
	//Avg
	int lastAvgMinecmciteration;
	double maxAvgMines;
	
	double mineSpawnThreshold;


	//pens we use for the stats
	HPEN				m_RedPen;
	HPEN				m_BluePen;
	HPEN				m_GreenPen;
	HPEN				m_OldPen;
	HPEN				m_BlackPen;
	HPEN				m_PinkPen;
	HPEN				m_BrownPen;

	HBRUSH				m_WhiteFill;
	HBRUSH				m_GreyFill;
	HBRUSH				m_RedFill;
	
	//handle to the application window
	HWND				m_hwndMain;

	//toggles the speed at which the simulation runs
	bool				m_bFastRender;
	
	//cycles per iteration
	int					m_iTicks;

	//iteration counter
	int					m_iIterations;

	//window dimensions
	int         cxClient, cyClient;

	//this function plots a graph of the average and best MinesGathered
	//over the course of a run
	void   PlotStats(HDC surface);

	// multi layer perceptron
	CMlp mlp;

	void		ResetEnvironment();

	int totalSuperMinesCollected;
	int totalMinesCollected;
	int maxMinesCollected;


public:

	CController(HWND hwndMain);
	~CController();

	void		Render(HDC surface);

	void		WorldTransform(vector<SPoint> &VBuffer, SVector2D vPos);
	
	bool		Update();


	//accessor methods
	bool		FastRender()const	  {return m_bFastRender;}
	void		FastRender(bool arg){m_bFastRender = arg;}
	void		FastRenderToggle()  {m_bFastRender = !m_bFastRender;}

};


#endif
	
