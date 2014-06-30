//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclCluster.cc
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

#include <trg/ecl/TrgEclCluster.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
//
//
//
TrgEclCluster::TrgEclCluster()
{

  for (int iii = 0; iii < 3; iii++) {
    for (int jjj = 0; jjj < 4; jjj++) {
      _icnquadrant[iii][jjj] = 0;
    }
  }
}
//
//
//
int
TrgEclCluster::getICNQuadrant(int FwBrBwId, int QuadrantId)
{

  return _icnquadrant[FwBrBwId][QuadrantId];

}
//
//
//
int
TrgEclCluster::getICNFwBr(void)
{

  return _icnfwbrbw[0] + _icnfwbrbw[1];

}
//
//
//
int
TrgEclCluster::getICNSub(int FwBrBw)
{

  return _icnfwbrbw[FwBrBw];

}
//
//
//
void
TrgEclCluster::setICN(TrgEclFAM* obj_tchit, int HitTC[][20])
{

  _icnfwbrbw[0] = setForwardICN(obj_tchit, HitTC);
  _icnfwbrbw[1] = setBarrelICN(obj_tchit, HitTC);
  _icnfwbrbw[2] = setBackwardICN(obj_tchit);

  return;
}
//
//
//
bool
TrgEclCluster::getBeamBkgVeto(void)
{

  bool boolBeamBkgVeto = false;
  bool boolForward =
    ((_icnquadrant[0][0] && _icnquadrant[0][2]) ||
     (_icnquadrant[0][1] && _icnquadrant[0][3]));
  bool boolBarrel =
    ((_icnquadrant[1][0] && _icnquadrant[1][2]) ||
     (_icnquadrant[1][1] && _icnquadrant[1][3]));
  boolBeamBkgVeto = (boolForward || boolBarrel);

  return boolBeamBkgVeto;
}
//
//
//
int
TrgEclCluster::setBarrelICN(TrgEclFAM* obj_tchit, int HitTC[][20])
{

  int TCFire[432] = { 0 };

  for (int iTCId0 = 0; iTCId0 < 576 ; iTCId0++) {
    int TCId = iTCId0 + 1;
    int ntcoutput = obj_tchit->getTCNoOutput(TCId);
    for (int intcoutput = 0 ; intcoutput < ntcoutput ; intcoutput++) {
      if (HitTC[TCId][intcoutput] > 0) {
        if (TCId >= 81 && TCId <= 512 && obj_tchit->getTCEnergy(TCId, intcoutput) > 0.1) {

          TCFire[TCId - 81] = 1;
        }
      }
    }
  }
  int icn = 0;
  //
  //
  //
  int tc_upper = 0; // check upper TC
  int tc_right = 0; // check right TC
  int tc_lower = 0; // check lower TC
  int tc_lowerright = 0; // check lower and right TC
  for (int iii = 0 ; iii < 432 ; iii++) {
    int quad_sec = (iii / 108);
    if (TCFire[iii] != 1) { continue; }
    if (iii % 12 != 0) {
      // check TC except for most left column
      if (iii < 12) {
        tc_upper      = TCFire[iii + 420];
        tc_right      = TCFire[iii - 1];
        tc_lower      = TCFire[iii + 12];
        tc_lowerright = TCFire[iii + 11];
      } else if (iii < 420) {
        tc_upper      = TCFire[iii - 12];
        tc_right      = TCFire[iii - 1];
        tc_lower      = TCFire[iii + 12];
        tc_lowerright = TCFire[iii + 11];
      } else {
        tc_upper      = TCFire[iii - 12];
        tc_right      = TCFire[iii - 1];
        tc_lower      = TCFire[iii - 420];
        tc_lowerright = TCFire[iii - 421];
      }
    } else {
      // check TC in most left column
      if (iii <  12) { tc_upper = TCFire[iii + 420];}
      else if (iii < 420) { tc_upper = TCFire[iii - 12];}
      else              { tc_upper = TCFire[iii - 12];}
    }
    //
    if (iii % 12 != 0) {
      if (!(tc_upper || tc_right)) {
        if (!(tc_lower && tc_lowerright)) {
          icn++;
          _icnquadrant[1][quad_sec]++;
        }
      }
    } else {
      if (! tc_upper) {
        icn++;
        _icnquadrant[1][quad_sec]++;
      }
    }
  } // iii loop
  return icn;
}
//
//
//
int
TrgEclCluster::setForwardICN(TrgEclFAM* obj_tchit, int HitTC[][20])
{

  int TCFire[96] = { 0 };
  int icn = 0;

  for (int iTCId0 = 0; iTCId0 < 80 ; iTCId0++) {
    int TCId = iTCId0 + 1;
    int ntcoutput = obj_tchit->getTCNoOutput(TCId);
    for (int intcoutput = 0 ; intcoutput < ntcoutput ; intcoutput++) {
      if (HitTC[TCId][intcoutput] == 0) {continue;}
      if (TCId > 80 || obj_tchit->getTCEnergy(TCId, intcoutput) < 0.1) { continue; }

      //------------------------------------
      // To rearrange the hitted map
      //
      //   orignal       converted
      //  (<- Theta)    (<- Theta)
      //
      //   3  2  1       64 32  0
      //   4  5  -       65 33  1
      //   8  7  6  =>   66 34  2
      //   9 10  -       67 35  3
      //  ....           ...
      //  78 77 76       94 62 30
      //  79 80  -       95 63 31
      //
      // Here, TCId-1 becomes TCId=0 and 1.
      //------------------------------------
      int kkk = 0;
      if (iTCId0 % 5 == 0) {
        kkk = (iTCId0 / 5) * 2;
        TCFire[kkk]   = 1;
        TCFire[kkk + 1] = 1;
      } else {
        kkk = iTCId0 / 5;
        switch (iTCId0 % 5) {
          case 1 :
            TCFire[32 + 2 * kkk]   = 1; break;
          case 2 :
            TCFire[64 + 2 * kkk]   = 1; break;
          case 3 :
            TCFire[64 + 2 * kkk + 1] = 1; break;
          case 4 :
            TCFire[32 + 2 * kkk + 1] = 1; break;
          default:
            break;
        }
      }
    }

  }
  //
  // First calculate the middle part
  //
  for (int iii = 32 ; iii < 64 ; iii++) {
    if (TCFire[iii] != 1) { continue; }

    int quad_sec = (iii / 24);

    if (iii % 32 != 0) {
      // Change to consider only two layers
      // if( !(TCFire[iii-32] || TCFire[iii-1]) ){
      if (!(TCFire[iii - 1]))
        if (!(TCFire[iii + 32] && TCFire[iii + 31])) {
          icn++;
          _icnquadrant[0][quad_sec]++;
        }
    } else {
      //  if( !(TCFire[iii-32] || TCFire[63] ) ){
      if (!(TCFire[63])) {
        if (!(TCFire[iii + 32] && TCFire[95])) {
          icn++;
          _icnquadrant[0][quad_sec]++;
        }
      }
    }
  }
  //
  // Third, calculate the tail part
  //
  for (int iii = 64; iii < 96 ; iii++) {
    if (TCFire[iii] != 1) { continue; }

    int quad_sec = (iii / 24);

    if (iii % 32 != 0) {
      if (!(TCFire[iii - 32] || TCFire[iii - 1])) {
        icn++;
        _icnquadrant[0][quad_sec]++;
      }
    } else {
      if (!(TCFire[iii - 32] || TCFire[95])) {
        icn++;
        _icnquadrant[0][quad_sec]++;
      }
    }
  }
  //
  //
  //

  return icn;
}
//
//
//
int
//iwasaki TrgEclCluster::setBackwardICN(TCHit *obj_tchit){
TrgEclCluster::setBackwardICN(TrgEclFAM*)
{

  //  int TCFire[60] = { 0 };
  int icn = 0;
  for (int iii = 0; iii < 4 ; iii++) {
    _icnquadrant[2][iii] = 0;
  }

  return icn;

}
//
//===<END>
//
