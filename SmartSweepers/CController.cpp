#include "CController.h"


//these hold the geometry of the sweepers and the mines
const int	 NumSweeperVerts = 16;
const SPoint sweeper[NumSweeperVerts] = {SPoint(-1, -1),
                                         SPoint(-1, 1),
                                         SPoint(-0.5, 1),
                                         SPoint(-0.5, -1),

                                         SPoint(0.5, -1),
                                         SPoint(1, -1),
                                         SPoint(1, 1),
                                         SPoint(0.5, 1),
                                         
                                         SPoint(-0.5, -0.5),
                                         SPoint(0.5, -0.5),

                                         SPoint(-0.5, 0.5),
                                         SPoint(-0.25, 0.5),
                                         SPoint(-0.25, 1.75),
                                         SPoint(0.25, 1.75),
                                         SPoint(0.25, 0.5),
                                         SPoint(0.5, 0.5)};



const int NumMineVerts = 4;
const SPoint mine[NumMineVerts] = {SPoint(-1, -1),
                                   SPoint(-1, 1),
                                   SPoint(1, 1),
                                   SPoint(1, -1)};

double totalMinesGathered;
double avgMinesGathered;

bool isFirstTick = true;



//---------------------------------------constructor---------------------
//
//	initilaize the sweepers and their brains
//
//-----------------------------------------------------------------------
CController::CController(HWND hwndMain): m_NumSweepers(CParams::iNumSweepers),
										                     m_bFastRender(false),
										                     m_iTicks(0),
										                     m_NumMines(CParams::iNumMines),
															 m_NumSuperMines(CParams::iNumSuperMines),
															 m_NumRocks(CParams::iNumRocks),
										                     m_hwndMain(hwndMain),
										                     m_iIterations(0),
                                         cxClient(CParams::WindowWidth),
                                         cyClient(CParams::WindowHeight)
{
	//let's create the mine sweepers
	for (int i=0; i<m_NumSweepers; ++i)
	{
		m_vecSweepers.push_back(CMinesweeper());
	}

	// Setup our MLP network. Training can take a few seconds.
	mlp.Train();

	//initialize mines in random positions within the application window
	for (int i=0; i<m_NumMines; ++i)
	{
		m_vecObjects.push_back(CCollisionObject(CCollisionObject::Mine, SVector2D(RandFloat() * cxClient,
                                   RandFloat() * cyClient)));
	}
	for (int i=0; i<m_NumSuperMines; ++i)
	{
		m_vecObjects.push_back(CCollisionObject(CCollisionObject::SuperMine, SVector2D(RandFloat() * cxClient,
                                   RandFloat() * cyClient)));
	}
	for (int i=0; i<m_NumRocks; ++i)
	{
		m_vecObjects.push_back(CCollisionObject(CCollisionObject::Rock, SVector2D(RandFloat() * cxClient,
                                   RandFloat() * cyClient)));
	}

	//create a pen for the graph drawing
	m_BluePen  = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	m_RedPen   = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	m_GreenPen = CreatePen(PS_SOLID, 1, RGB(0, 150, 0));

	m_OldPen	= NULL;

	//fill the vertex buffers
	for (int i=0; i<NumSweeperVerts; ++i)
	{
		m_SweeperVB.push_back(sweeper[i]);
	}

	for (int i=0; i<NumMineVerts; ++i)
	{
		m_MineVB.push_back(mine[i]);
	}

}


//--------------------------------------destructor-------------------------------------
//
//--------------------------------------------------------------------------------------
CController::~CController()
{
	DeleteObject(m_BluePen);
	DeleteObject(m_RedPen);
	DeleteObject(m_GreenPen);
	DeleteObject(m_OldPen);
}


//---------------------WorldTransform--------------------------------
//
//	sets up the translation matrices for the mines and applies the
//	world transform to each vertex in the vertex buffer passed to this
//	method.
//-------------------------------------------------------------------
void CController::WorldTransform(vector<SPoint> &VBuffer, SVector2D vPos)
{
	//create the world transformation matrix
	C2DMatrix matTransform;
	
	//scale
	matTransform.Scale(CParams::dMineScale, CParams::dMineScale);
	
	//translate
	matTransform.Translate(vPos.x, vPos.y);

	//transform the ships vertices
	matTransform.TransformSPoints(VBuffer);
}


//-------------------------------------Update-----------------------------
//
//	This is the main workhorse. The entire simulation is controlled from here.
//
//	The comments should explain what is going on adequately.
//-------------------------------------------------------------------------
bool CController::Update()
{
	//run the sweepers through CParams::iNumTicks amount of cycles. During
	//this loop each sweeper is constantly updated with the appropriate
	//information from its surroundings. The output from the learning algorithm is obtained
	//and the sweeper is moved. If it encounters a mine its MinesGathered is
	//updated appropriately,
	if (m_iTicks++ < CParams::iNumTicks)
	{
		for (int i=0; i<m_NumSweepers; ++i)
		{
			//update the position
			if (!m_vecSweepers[i].Update(m_vecObjects, mlp))
			{
				//error in processing the learning algorithm
				MessageBox(m_hwndMain, "An error occured while processing!", "Error", MB_OK);

				return false;
			}
				
			//see if it's found a mine
			int GrabHit = m_vecSweepers[i].CheckForMine(m_vecObjects,
													CParams::dMineScale);

			if (GrabHit >= 0)
			{
				if(m_vecObjects[GrabHit].getType()==CCollisionObject::Mine)
				{
					//we have discovered a mine so increase MinesGathered
					if(!isFirstTick)
						m_vecSweepers[i].IncrementMinesGathered();

					//mine found so replace the mine with another at a random 
					//position
					bool hit = false;
					do{
						hit = false;
						m_vecObjects[GrabHit] = CCollisionObject(m_vecObjects[GrabHit].getType(),SVector2D(RandFloat() * cxClient,
												RandFloat() * cyClient));

						for (int i=0; i<m_NumSweepers; ++i)
						{
							if(m_vecSweepers[i].CheckCollides(m_vecObjects[GrabHit], CParams::dMineScale)){
								hit = true;
								break;
							}
						}
					}while(hit);
				}
			}
		}
	}
	//Time to update the sweepers for the next iteration
	else
	{
		//update the stats to be used in our stat window
		//TODO: at the moment this is set to 0 for all sweepers by default.
		//		You should apply meaningful stats from your sweepers here.

		
		totalMinesGathered = 0;
		for (int i=0; i<m_NumSweepers; ++i)
		{
			totalMinesGathered += m_vecSweepers[i].MinesGathered();	
		}

		avgMinesGathered = totalMinesGathered / m_NumSweepers;

		m_vecAvMinesGathered.push_back(avgMinesGathered);
		m_vecMostMinesGathered.push_back(totalMinesGathered);

		//increment the iteration counter
		++m_iIterations;

		//reset cycles
		m_iTicks = 0;
	
		//reset the sweepers positions etc
		for (int i=0; i<m_NumSweepers; ++i)
		{
			m_vecSweepers[i].Reset();
		}
	}

	if(isFirstTick)
		isFirstTick = false;

	return true;
}

//------------------------------------Render()--------------------------------------
//
//----------------------------------------------------------------------------------
void CController::Render(HDC surface)
{
	//render the stats
	string s = "Iteration:          " + itos(m_iIterations);
	TextOut(surface, 5, 0, s.c_str(), s.size());

	//do not render if running at accelerated speed
	if (!m_bFastRender)
	{
		//keep a record of the old pen
		m_OldPen = (HPEN)SelectObject(surface, m_GreenPen);
		
		//render the mines
		for (int i=0; i<m_NumMines+m_NumSuperMines+m_NumRocks; ++i)
		{
			if ( m_vecObjects[i].getType() == CCollisionObject::Mine)
			{
				SelectObject(surface, m_GreenPen);
			}
			else if ( m_vecObjects[i].getType() == CCollisionObject::Rock)
			{
				SelectObject(surface, m_BluePen );
			}
			else if ( m_vecObjects[i].getType() == CCollisionObject::SuperMine)
			{
				SelectObject(surface, m_RedPen);
			}
			//grab the vertices for the mine shape
			vector<SPoint> mineVB = m_MineVB;

			WorldTransform(mineVB, m_vecObjects[i].getPosition());

			//draw the mines
			MoveToEx(surface, (int)mineVB[0].x, (int)mineVB[0].y, NULL);

			for (int vert=1; vert<mineVB.size(); ++vert)
			{
				LineTo(surface, (int)mineVB[vert].x, (int)mineVB[vert].y);
			}

			LineTo(surface, (int)mineVB[0].x, (int)mineVB[0].y);
			
		}
       		
		//we want some sweepers displayed in red
		SelectObject(surface, m_RedPen);

		//render the sweepers
		for (int i=0; i<m_NumSweepers; i++)
		{
			if (i == CParams::iNumElite)
			{
				SelectObject(surface, m_OldPen);
			}
      
			//grab the sweeper vertices
			vector<SPoint> sweeperVB = m_SweeperVB;

			//transform the vertex buffer
			m_vecSweepers[i].WorldTransform(sweeperVB);

			//draw the sweeper left track
			MoveToEx(surface, (int)sweeperVB[0].x, (int)sweeperVB[0].y, NULL);

			for (int vert=1; vert<4; ++vert)
			{
				LineTo(surface, (int)sweeperVB[vert].x, (int)sweeperVB[vert].y);
			}

			LineTo(surface, (int)sweeperVB[0].x, (int)sweeperVB[0].y);

			//draw the sweeper right track
			MoveToEx(surface, (int)sweeperVB[4].x, (int)sweeperVB[4].y, NULL);

			for (int vert=5; vert<8; ++vert)
			{
				LineTo(surface, (int)sweeperVB[vert].x, (int)sweeperVB[vert].y);
			}

			LineTo(surface, (int)sweeperVB[4].x, (int)sweeperVB[4].y);

			MoveToEx(surface, (int)sweeperVB[8].x, (int)sweeperVB[8].y, NULL);
			LineTo(surface, (int)sweeperVB[9].x, (int)sweeperVB[9].y);

			MoveToEx(surface, (int)sweeperVB[10].x, (int)sweeperVB[10].y, NULL);

			for (int vert=11; vert<16; ++vert)
			{
				LineTo(surface, (int)sweeperVB[vert].x, (int)sweeperVB[vert].y);
			}

		}

		//put the old pen back
		SelectObject(surface, m_OldPen);

	}//end if

  else
  {
    PlotStats(surface);
  }

}
//--------------------------PlotStats-------------------------------------
//
//  Given a surface to draw on this function displays stats and a crude
//  graph showing best and average MinesGathered
//------------------------------------------------------------------------
void CController::PlotStats(HDC surface)
{
	//TODO: at the moment this is set to 0 by default.
	//		You should plot meaningful stats from your sweepers here.
    string s = "Total MinesGathered:       " + ftos(totalMinesGathered);
	TextOut(surface, 5, 20, s.c_str(), s.size());

     s = "Average MinesGathered: " + ftos(avgMinesGathered);
	TextOut(surface, 5, 40, s.c_str(), s.size());
    
    //render the graph
    float HSlice = (float)cxClient/(m_iIterations+1);
	float VSlice = (float)cyClient/((1)*2);

    //plot the graph for the best MinesGathered
    float x = 0;
    
    m_OldPen = (HPEN)SelectObject(surface, m_RedPen);

    MoveToEx(surface, 0, cyClient, NULL);
    
    for (int i=0; i<m_vecMostMinesGathered.size(); ++i)
    {
       LineTo(surface, x, cyClient - VSlice*m_vecMostMinesGathered[i]);

       x += HSlice;
    }

    //plot the graph for the average MinesGathered
    x = 0;

    SelectObject(surface, m_BluePen);

    MoveToEx(surface, 0, cyClient, NULL);
    
    for (int i=0; i<m_vecAvMinesGathered.size(); ++i)
    {
       LineTo(surface, (int)x, (int)(cyClient - VSlice*m_vecAvMinesGathered[i]));

       x += HSlice;
    }

    //replace the old pen
    SelectObject(surface, m_OldPen);
}

