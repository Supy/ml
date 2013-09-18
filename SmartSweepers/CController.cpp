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

	//initialize mines in random positions within the application window
	for (int i=0; i<m_NumMines; ++i)
	{
		m_vecObjects.push_back(CCollisionObject(CCollisionObject::Mine, SVector2D(RandFloat() * cxClient, RandFloat() * cyClient)));
	}
	for (int i=0; i<m_NumSuperMines; ++i)
	{
		m_vecObjects.push_back(CCollisionObject(CCollisionObject::SuperMine, SVector2D(RandFloat() * cxClient, RandFloat() * cyClient)));
	}
	for (int i=0; i<m_NumRocks; ++i)
	{
		m_vecObjects.push_back(CCollisionObject(CCollisionObject::Rock, SVector2D(RandFloat() * cxClient, RandFloat() * cyClient)));
	}

	//create a pen for the graph drawing
	m_BluePen  = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	m_RedPen   = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	m_GreenPen = CreatePen(PS_SOLID, 1, RGB(0, 150, 0));
	m_BlackPen = CreatePen(PS_SOLID, 1, RGB(0,0,0));
	m_PinkPen  = CreatePen(PS_SOLID, 1, RGB(255,140,140));
	m_BrownPen = CreatePen(PS_SOLID, 1, RGB(50,50,0));
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

	
	isFirstTick = true;
	hasTrained = false;
	hasRendered = false;

	lastminecmciteration = 0;
	maxmaxmines = 1;
	lastrockcmciteration = 0;
	maxmaxrocks = 1;
}


//--------------------------------------destructor-------------------------------------
//
//--------------------------------------------------------------------------------------
CController::~CController()
{
	DeleteObject(m_BluePen);
	DeleteObject(m_RedPen);
	DeleteObject(m_GreenPen);
	DeleteObject(m_BlackPen);
	DeleteObject(m_PinkPen);
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
	if (!hasTrained)
	{
		if (hasRendered)
		{
			mlp.Train();
			hasTrained = true;
		}

	}
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
				
			//see if it's found an object
			int GrabHit = m_vecSweepers[i].CheckForMine(m_vecObjects, CParams::dMineScale);
			if (GrabHit >= 0)
			{

				if(m_vecObjects[GrabHit].getType() == CCollisionObject::Mine)
				{
					//we have discovered a mine so increase MinesGathered
					if(!isFirstTick) m_vecSweepers[i].IncrementMinesGathered();

					// object hit, so respawn it 
					bool hit = false;
					do{
						hit = false;
						m_vecObjects[GrabHit] = CCollisionObject(m_vecObjects[GrabHit].getType(),SVector2D(RandFloat() * cxClient, RandFloat() * cyClient));

						for (int i=0; i<m_NumSweepers; ++i)
						{
							if(m_vecSweepers[i].CheckCollides(m_vecObjects[GrabHit], CParams::dMineScale))
							{
								hit = true;
								break;
							}
						}
					} while(hit);	

				}
				else if(m_vecObjects[GrabHit].getType() == CCollisionObject::Rock)
				{
					//we have discovered a mine so increase MinesGathered
					if(!isFirstTick) m_vecSweepers[i].IncrementRocksGathered();
				}
								
			
			}
		}
	}
	//Time to update the sweepers for the next iteration
	else
	{
		// Update MinesGathered average and max
		int max = 0;
		int total = 0;

		for (int i=0; i<m_NumSweepers; ++i)
		{
			int mg = m_vecSweepers[i].MinesGathered();				
			total += mg;
			if (mg > max) max = mg;
		}
		
		double average = ((double)total / m_NumSweepers);
				
		m_vecAvMinesGathered.push_back(average);
		m_vecMaxMinesGathered.push_back(max);

		// Update RocksGathered average and max

		max = 0;
		total = 0;

		for (int i=0; i<m_NumSweepers; ++i)
		{
			int rg = m_vecSweepers[i].RocksGathered();				
			total += rg;
			if (rg > max) max = rg;
		}
		
		average = ((double)total / m_NumSweepers);

		m_vecAvRocksGathered.push_back(average);
		m_vecMaxRocksGathered.push_back(max);


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

	if(isFirstTick) isFirstTick = false;

	return true;
}

//------------------------------------Render()--------------------------------------
//
//----------------------------------------------------------------------------------
void CController::Render(HDC surface)
{
	if ( hasTrained)
	{

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
					SelectObject(surface, m_PinkPen);
				}
				else if ( m_vecObjects[i].getType() == CCollisionObject::Rock)
				{
					SelectObject(surface, m_BrownPen );
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

				LineTo(surface, (int)mineVB[1].x, (int)mineVB[1].y);
				LineTo(surface, (int)mineVB[2].x, (int)mineVB[2].y);			
				LineTo(surface, (int)mineVB[3].x, (int)mineVB[3].y);					
				LineTo(surface, (int)mineVB[0].x, (int)mineVB[0].y);

				LineTo(surface, (int)mineVB[2].x, (int)mineVB[2].y);
				MoveToEx(surface, (int)mineVB[1].x, (int)mineVB[1].y, NULL);
				LineTo(surface, (int)mineVB[3].x, (int)mineVB[3].y);
			
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

		}
		else
		{
			PlotStats(surface);
		}

	
		//render the stats
		string s = "Iteration:          " + itos(m_iIterations);
		TextOut(surface, 5, 0, s.c_str(), s.size());
	
		string s2 = "Progress:   " + itos(m_iTicks) + "/" + itos(CParams::iNumTicks) + " ticks";
		TextOut(surface, 5, 20, s2.c_str(), s2.size());
	}
	else
	{

		string s = "Training...";
		TextOut(surface, 5, 0, s.c_str(), s.size());

		s = "This generally takes under 10 seconds.";
		TextOut(surface, 5, 20, s.c_str(), s.size());

		hasRendered = true;
	}
}
//--------------------------PlotStats-------------------------------------
//
//  Given a surface to draw on this function displays stats and a crude
//  graph showing best and average MinesGathered
//------------------------------------------------------------------------
void CController::PlotStats(HDC surface)
{

	int iterationcount = m_vecMaxMinesGathered.size();

	//		You should plot meaningful stats from your sweepers here.
	string temp;




	
    
	//=============== Iteration Graphs ==================================	
	

	

	// -------------- Mines Gathered ----------------------------
			
	std::string mostmstr = "Max Mines Collided:       ";
	std::string avgmstr =  "Average Mines Collided:  ";
	
	temp = mostmstr + ((iterationcount > 0) ? ftos(m_vecMaxMinesGathered.back()) : "-");
	TextOut(surface, 5, 60, temp.c_str(), temp.size());
		
	temp = avgmstr + ((iterationcount > 0) ? ftos(m_vecAvMinesGathered.back()) : "-");
	TextOut(surface, 5, 80, temp.c_str(), temp.size());
	
	int top = 100; int bottom = 200;
	int left = 5; int right = 395;

	int width = right-left;
	int displayit_start = (iterationcount > width) ? iterationcount - width : 0;

	// draw border
	SelectObject(surface, m_BlackPen);
	MoveToEx(surface, left-1, top-1, NULL);
	LineTo(surface, right+1, top-1);
	LineTo(surface, right+1, bottom+1);
	LineTo(surface, left-1, bottom+1);
	LineTo(surface, left-1, top-1);

	if (iterationcount > 0)
	{
		//run current max calculations if required
		if (iterationcount > lastminecmciteration)
		{			
			// calculate max
			auto maxmaxminesptr = std::max_element(m_vecMaxMinesGathered.begin()+displayit_start, m_vecMaxMinesGathered.end());
			maxmaxmines = *maxmaxminesptr;
			if (maxmaxmines == 0) maxmaxmines = 1;

			lastminecmciteration = iterationcount;
		}

		// calculate scale
		double scale = (bottom-top) / maxmaxmines;

		MoveToEx(surface, left, bottom-2, NULL);
		SelectObject(surface, m_RedPen);
	
		for (size_t i=displayit_start, l=0; i<m_vecMaxMinesGathered.size(); ++i, ++l)
		{
			MoveToEx(surface, left + l, bottom, NULL);
			LineTo(surface, left + l, bottom - m_vecMaxMinesGathered[i]*scale);
		}

		MoveToEx(surface, left, bottom-2, NULL);
		SelectObject(surface, m_BluePen);
		for (size_t i=displayit_start, l=0; i<m_vecAvMinesGathered.size(); ++i, ++l)
		{
		   LineTo(surface, left + l, bottom - m_vecAvMinesGathered[i]*scale);
		}
	}

	// -------------- Rocks Gathered ----------------------------
		
	std::string mostrstr = "Max Rocks Collided:       ";
	std::string avgrstr =  "Average Rocks Collided:  ";
	
	temp = mostrstr + ((iterationcount > 0) ? ftos(m_vecMaxRocksGathered.back()) : "-");
	TextOut(surface, 5, 220, temp.c_str(), temp.size());
		
	temp = avgrstr + ((iterationcount > 0) ? ftos(m_vecAvRocksGathered.back()) : "-");
	TextOut(surface, 5, 240, temp.c_str(), temp.size());

	top = 260; bottom = 360;
	left = 5; right = 395;

	width = right-left;
	displayit_start = (iterationcount > width) ? iterationcount - width : 0;

	// draw border
	SelectObject(surface, m_BlackPen);
	MoveToEx(surface, left-1, top-1, NULL);
	LineTo(surface, right+1, top-1);
	LineTo(surface, right+1, bottom+1);
	LineTo(surface, left-1, bottom+1);
	LineTo(surface, left-1, top-1);

	if (iterationcount > 0)
	{

		//run current max calculations if required
		if (iterationcount > lastrockcmciteration)
		{			
			// calculate max
			auto maxmaxrocksptr = std::max_element(m_vecMaxRocksGathered.begin()+displayit_start, m_vecMaxRocksGathered.end());
			maxmaxrocks = *maxmaxrocksptr;
			if (maxmaxrocks == 0) maxmaxrocks = 1;

			lastrockcmciteration = iterationcount;
		}	

		// calculate scale
		double scale = (bottom-top) / maxmaxrocks;

		MoveToEx(surface, left, bottom-2, NULL);
		SelectObject(surface, m_RedPen);
	
		for (size_t i=displayit_start, l=0; i<m_vecMaxRocksGathered.size(); ++i, ++l)
		{
			MoveToEx(surface, left + l, bottom, NULL);
			LineTo(surface, left + l, bottom - m_vecMaxRocksGathered[i]*scale);
		}

		MoveToEx(surface, left, bottom-2, NULL);
		SelectObject(surface, m_BluePen);
		for (size_t i=displayit_start, l=0; i<m_vecAvRocksGathered.size(); ++i, ++l)
		{
		   LineTo(surface, left + l, bottom - m_vecAvRocksGathered[i]*scale);
		}
	}
    //replace the old pen
    SelectObject(surface, m_OldPen);
}

