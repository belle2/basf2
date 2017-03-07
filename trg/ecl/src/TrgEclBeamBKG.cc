//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclBeamBKG.cc
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGECLCLUSTER_SHORT_NAMES
#include <framework/gearbox/Unit.h>


#include <trg/ecl/TrgEclBeamBKG.h>
#include <framework/logging/Logger.h>
#include "trg/ecl/TrgEclBeamBKG.h"

using namespace std;
using namespace Belle2;
//
//
//
TrgEclBeamBKG::TrgEclBeamBKG()
{
  for (int iii = 0; iii < 3; iii++) {
    for (int jjj = 0; jjj < 3; jjj++) {
      Quadrant[iii][jjj] = 0;
    }
  }
  _TCMap = new TrgEclMapping();



}

TrgEclBeamBKG::~TrgEclBeamBKG()
{

  delete _TCMap;
}
bool TrgEclBeamBKG::GetBeamBkg(std::vector<std::vector<double>> ThetaRingSum)
{

  for (int iFwd = 0 ; iFwd < 32 ; iFwd++) {
    if (ThetaRingSum[0][iFwd] > 0) {
      Quadrant[0][(int)(iFwd / 8)]++;
    }
  }
  for (int iBwd = 0 ; iBwd < 36 ; iBwd++) {
    if (ThetaRingSum[2][iBwd] > 0) {
      Quadrant[2][(int)(iBwd / 8)]++;
    }
  }
  for (int iBr = 0 ; iBr < 36 ; iBr++) {
    if (ThetaRingSum[1][iBr] > 0) {
      Quadrant[1][(int)(iBr / 9)]++;
    }
  }


  bool boolForward =
    ((Quadrant[0][0] && Quadrant[0][2]) ||
     (Quadrant[0][1] && Quadrant[0][3]));
  bool boolBarrel =
    ((Quadrant[1][0] && Quadrant[1][2]) ||
     (Quadrant[1][1] && Quadrant[1][3]));
  bool boolBeamBkgVeto = (boolForward || boolBarrel);


  return boolBeamBkgVeto;

}


//
//===<END>
//
