#include "CParams.h"


//because we will always be loading in the settings from an ini file
//we can just initialize everything to zero
double CParams::dPi                 = 3.14159265358979;
double CParams::dHalfPi             = dPi / 2;
double CParams::dTwoPi              = dPi * 2;
int CParams::WindowWidth            = 400;
int CParams::WindowHeight           = 400;
int CParams::iFramesPerSecond       = 0;
double CParams::dMaxTurnRate        = 0;
double CParams::dMaxSpeed           = 0;
int CParams::iSweeperScale          = 0;
int CParams::iNumSweepers           = 0;
int CParams::iNumMines              = 0;
int CParams::iNumSuperMines         = 0;
int CParams::iNumRocks              = 0;
int CParams::iNumTicks              = 0;
double CParams::dMineScale          = 0;
int CParams::iNumElite              = 0;
int CParams::iNumCopiesElite        = 0;

//this function loads in the parameters from a given file name. Returns
//false if there is a problem opening the file.
bool CParams::LoadInParameters(char* szFileName)
{
  ifstream grab(szFileName);

  //check file exists
  if (!grab)
  {
    return false;
  }

  //load in from the file
  char ParamDescription[40];

  grab >> ParamDescription;
  grab >> iFramesPerSecond;
  grab >> ParamDescription;
  grab >> dMaxTurnRate;
  grab >> ParamDescription;
  grab >> dMaxSpeed;
  grab >> ParamDescription;
  grab >> iSweeperScale;
  grab >> ParamDescription;
  grab >> iNumMines;
  grab >> ParamDescription;
  grab >> iNumSuperMines;
  grab >> ParamDescription;
  grab >> iNumRocks;
  grab >> ParamDescription;
  grab >> iNumSweepers;
  grab >> ParamDescription;
  grab >> iNumTicks;
  grab >> ParamDescription;
  grab >> dMineScale;
  grab >> ParamDescription;
  grab >> iNumElite;
  grab >> ParamDescription;
  grab >> iNumCopiesElite;


  return true;
}
 




  
  
