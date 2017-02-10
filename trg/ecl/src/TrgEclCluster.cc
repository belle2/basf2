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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>


#include <trg/ecl/TrgEclCluster.h>
#include <framework/logging/Logger.h>
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include "trg/ecl/dataobjects/TRGECLHit.h"

using namespace std;
using namespace Belle2;
//
//
//
TrgEclCluster::TrgEclCluster(): _BRICN(0), _FWDICN(0), _BWDICN(0)
{

  for (int iii = 0; iii < 3; iii++) {
    _icnfwbrbw[iii] = 0;
    for (int jjj = 0; jjj < 4; jjj++) {
      _icnquadrant[iii][jjj] = 0;
    }
  }

  for (int iii = 0; iii < 20; iii++) {
    for (int jjj = 0; jjj < 9; jjj++) {
      TempCluster[jjj] = 0;
      _BrCluster[iii][jjj] = 0;
    }
  }

  for (int iii = 0; iii < 20; iii++) {
    for (int jjj = 0; jjj < 9; jjj++) {
      _BwCluster[iii][jjj] = 0;
    }
  }

  for (int iii = 0; iii < 20; iii++) {
    for (int jjj = 0; jjj < 9; jjj++) {
      _FwCluster[iii][jjj] = 0;
    }
  }




  for (int iTCId = 0; iTCId < 576; iTCId++) {
    for (int iTimeindex = 0; iTimeindex < 64; iTimeindex++) {
      Timing[iTCId][iTimeindex] = 0;
      Energy[iTCId][iTimeindex] = 0;
    }
  }
  StoreArray<TRGECLHit> trgeclHitArray;
  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {

    TRGECLHit* aTRGECLHit = trgeclHitArray[ii];
    int TCID = (aTRGECLHit->getCellId() - 1);
    double TCHitTiming    = aTRGECLHit ->getTimeAve();
    double TCHitEnergy =  aTRGECLHit -> getEnergyDep();
    int itimeindex = (int)(TCHitTiming / 100 + 40);

    Timing[TCID][itimeindex] = TCHitTiming;
    Energy[TCID][itimeindex] = TCHitEnergy;

  }

  for (int iNCluster = 0; iNCluster < 100 ; iNCluster++) {
    ClusterTiming[iNCluster] = 0;
    ClusterEnergy[iNCluster] = 0;
    ClusterPositionX[iNCluster] = 0;
    ClusterPositionY[iNCluster] = 0;
    ClusterPositionZ[iNCluster] = 0;
    NofTCinCluster[iNCluster] = 0;
    MaxTCId[iNCluster] = 0;
  }


  _TCMap = new TrgEclMapping();



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
int
TrgEclCluster::getBrICNCluster(int ICNId, int location)
{

  return _BrCluster[ICNId][location];

}
//
//
//
int
TrgEclCluster::getBwICNCluster(int ICNId, int location)
{

  return _BwCluster[ICNId][location];

}

//
//
//
int
TrgEclCluster::getFwICNCluster(int ICNId, int location)
{

  return _FwCluster[ICNId][location];

}

//
//
//
void
TrgEclCluster::setICN(int HitTC[][64], int Method)
{
  _icnfwbrbw[1] = setBarrelICN(HitTC, Method);
  _icnfwbrbw[0] = setForwardICN(HitTC, Method);
  _icnfwbrbw[2] = setBackwardICN(HitTC, Method);



  return;
}
//
//
//
bool
TrgEclCluster::getBeamBkgVeto(void)
{

  // bool boolBeamBkgVeto = false;
  bool boolForward =
    ((_icnquadrant[0][0] && _icnquadrant[0][2]) ||
     (_icnquadrant[0][1] && _icnquadrant[0][3]));
  bool boolBarrel =
    ((_icnquadrant[1][0] && _icnquadrant[1][2]) ||
     (_icnquadrant[1][1] && _icnquadrant[1][3]));
  bool boolBeamBkgVeto = (boolForward || boolBarrel);

  return boolBeamBkgVeto;
}
//
//
//
int
TrgEclCluster::setBarrelICN(int HitTC[][64], int Method)
{

  int TCFire[432] = { 0 };
  double TCFireEnergy[432] = {0};
  double TCFireTiming[432] = {0};
  double TCFirePosition[432][3] = {{0}};


  for (int iTC = 0; iTC < 432; iTC ++) {
    if (iTC < 9) {TempCluster[iTC] = 0;}

    TCFire[iTC] =  0 ;
    TCFireEnergy[iTC] = 0;
    TCFireTiming[iTC] = 0 ;
    TCFirePosition[iTC][0] = 0;
    TCFirePosition[iTC][1] = 0;
    TCFirePosition[iTC][2] = 0;


  }



  for (int iTCId0 = 0; iTCId0 < 576 ; iTCId0++) {
    int TCId = iTCId0 + 1;

    for (int itime = 0 ; itime < 64 ; itime++) {
      if (HitTC[iTCId0][itime] > 0) {
        if (TCId >= 81 && TCId <= 512) {
          TCFire[TCId - 81] = TCId;
          TCFireEnergy[TCId - 81] += Energy[TCId - 1][itime];
          TCFireTiming[TCId - 81] = Timing[TCId - 1][itime];
          TCFirePosition[TCId - 81][0] = (_TCMap->getTCPosition(TCId)).X();
          TCFirePosition[TCId - 81][1] = (_TCMap->getTCPosition(TCId)).Y();
          TCFirePosition[TCId - 81][2] = (_TCMap->getTCPosition(TCId)).Z();


        }
      }
    }
  }

  //
  //
  //
  int tc_upper = 0; // check upper TC
  int tc_upper_right = 0; // check right TC
  int tc_right = 0; // check right TC
  int tc_lower_right = 0;
  int tc_lower = 0; // check lower TC
  int tc_lower_left = 0; // check lower TC
  int tc_left = 0;
  int tc_upper_left = 0;



  for (int iii = 0 ; iii < 432 ; iii++) {
    int quad_sec = (iii / 108);
    if (TCFire[iii] == 0) { continue; }

    if (iii < 12) {
      tc_upper      = TCFire[iii + 420] ;
      tc_upper_right = TCFire[iii + 419] ;
      tc_right      = TCFire[iii - 1] ;
      tc_lower_right = TCFire[iii + 11] ;
      tc_lower      = TCFire[iii + 12] ;
      tc_lower_left = TCFire[iii + 13] ;
      tc_left = TCFire[iii + 1] ;
      tc_upper_left = TCFire[iii + 421] ;
      if (iii % 12 == 0) {
        tc_upper_right = 0;
        tc_right      = 0;
        tc_lower_right = 0;

      }
      if (iii % 12 == 11) {
        tc_lower_left = 0;
        tc_left = 0;
        tc_upper_left = 0;

      }
    }
    if (iii > 11 && iii < 420) {
      tc_upper      = TCFire[iii - 12] ;
      tc_upper_right = TCFire[iii - 13] ;
      tc_right      = TCFire[iii - 1] ;
      tc_lower_right = TCFire[iii + 11] ;
      tc_lower      = TCFire[iii + 12] ;
      tc_left = TCFire[iii + 1] ;
      tc_upper_left = TCFire[iii + -11] ;
      if (iii % 12 == 0) {
        tc_upper_right = 0;
        tc_right      = 0;
        tc_lower_right = 0;

      }
      if (iii % 12 == 11) {
        tc_lower_left = 0;
        tc_left = 0;
        tc_upper_left = 0;

      }

    }
    if (iii > 419) {
      tc_upper      = TCFire[iii - 12] ;
      tc_upper_right = TCFire[iii - 13] ;
      tc_right      = TCFire[iii - 1] ;
      tc_lower_right = TCFire[iii - 421] ;
      tc_lower      = TCFire[iii - 420] ;
      tc_left = TCFire[iii + 1] ;
      tc_upper_left = TCFire[iii + -11] ;
      if (iii % 12 == 0) {
        tc_upper_right = 0;
        tc_right      = 0;
        tc_lower_right = 0;
      }
      if (iii % 12 == 11) {
        tc_lower_left = 0;
        tc_left = 0;
        tc_upper_left = 0;

      }
    }


    TempCluster[0] = iii + 80 + 1; //middle of ICN
    TempCluster[1] = tc_upper; // upper
    TempCluster[2] = tc_upper_right; //right
    TempCluster[3] = tc_right; //right
    TempCluster[4] = tc_lower_right; //lower
    TempCluster[5] = tc_lower; //lower
    TempCluster[6] = tc_lower_left; //lower
    TempCluster[7] = tc_left; //lower
    TempCluster[8] = tc_upper_left; //lower right;


    if (!(tc_upper != 0 || tc_left != 0)) {
      if (!(tc_lower != 0 && tc_lower_left != 0)) {


        if (Method == 1) { //for cluster method2(Consider TC energy in oderto find the center of cluster)

          int maxTCid = 0;
          double maxTCEnergy = 0;
          for (int iTC = 0; iTC < 9; iTC++) { //find center of Cluster
            if (TempCluster[iTC] == 0) {continue;}
            if (maxTCEnergy <       TCFireEnergy[TempCluster[iTC] - 81]) {
              maxTCEnergy = TCFireEnergy[TempCluster[iTC] - 81];
              maxTCid = TempCluster[iTC];
            }
          }
          TempCluster[0] = maxTCid;

          if (maxTCid < 93) {
            // tc_upper       = TCFire[maxTCid + 420] ;
            // tc_upper_right = TCFire[maxTCid + 419] ;
            // tc_right       = TCFire[maxTCid - 1] ;
            // tc_lower_right = TCFire[maxTCid + 11] ;
            // tc_lower       = TCFire[maxTCid + 12] ;
            // tc_lower_left  = TCFire[maxTCid + 13] ;
            // tc_left        = TCFire[maxTCid + 1] ;
            // tc_upper_left  = TCFire[maxTCid + 421] ;

            TempCluster[1] = TCFire[maxTCid + 420 - 81 ] ;
            TempCluster[2] = TCFire[maxTCid + 419 - 81 ] ;
            TempCluster[3] = TCFire[maxTCid - 1 - 81 ] ;
            TempCluster[4] = TCFire[maxTCid + 11 - 81 ] ;
            TempCluster[5] = TCFire[maxTCid + 12 - 81 ] ;
            TempCluster[6] = TCFire[maxTCid + 13 - 81 ] ;
            TempCluster[7] = TCFire[maxTCid + 1 - 81 ] ;
            TempCluster[8] = TCFire[maxTCid + 421 - 81 ] ;


            if ((maxTCid - 81) % 12 == 0) {

              TempCluster[2] = 0 ;
              TempCluster[3] = 0 ;
              TempCluster[4] = 0 ;

            }
            if ((maxTCid - 81) % 12 == 11) {

              TempCluster[6] = 0 ;
              TempCluster[7] = 0 ;
              TempCluster[8] = 0 ;

            }
          }
          if (maxTCid > 93 && maxTCid < 501) {

            TempCluster[1] = TCFire[maxTCid - 12 - 81] ;
            TempCluster[2] = TCFire[maxTCid - 13 - 81] ;
            TempCluster[3] = TCFire[maxTCid - 1 - 81] ;
            TempCluster[4] = TCFire[maxTCid + 11 - 81] ;
            TempCluster[5] = TCFire[maxTCid + 12 - 81] ;
            TempCluster[6] = TCFire[maxTCid + 13 - 81] ;
            TempCluster[7] = TCFire[maxTCid + 1 - 81] ;
            TempCluster[8] = TCFire[maxTCid - 11 - 81] ;

            if ((maxTCid - 81) % 12 == 0) {
              TempCluster[2] = 0 ;
              TempCluster[3] = 0 ;
              TempCluster[4] = 0 ;

            }
            if ((maxTCid - 81) % 12 == 11) {

              TempCluster[6] = 0 ;
              TempCluster[7] = 0 ;
              TempCluster[8] = 0 ;

            }

          }
          if (maxTCid > 500) {

            TempCluster[1] = TCFire[maxTCid - 12 - 81] ;
            TempCluster[2] = TCFire[maxTCid - 13 - 81] ;
            TempCluster[3] = TCFire[maxTCid - 1 - 81] ;
            TempCluster[4] = TCFire[maxTCid - 421 - 81] ;
            TempCluster[5] = TCFire[maxTCid - 420 - 81] ;
            TempCluster[6] = TCFire[maxTCid - 419 - 81] ;
            TempCluster[7] = TCFire[maxTCid + 1 - 81] ;
            TempCluster[8] = TCFire[maxTCid - 11 - 81] ;


            if ((maxTCid - 81) % 12 == 0) {
              TempCluster[2] = 0 ;
              TempCluster[3] = 0 ;
              TempCluster[4] = 0 ;

            }
            if ((maxTCid - 81) % 12 == 11) {

              TempCluster[6] = 0 ;
              TempCluster[7] = 0 ;
              TempCluster[8] = 0 ;

            }
          }
        }
        double maxTC = 0;
        int maxTCId = 999;
        for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
          _BrCluster[_BRICN][iNearTC] = TempCluster[iNearTC];
          if (iii % 12 == 0) {
            if (iNearTC == 2 || iNearTC == 3 || iNearTC == 4) {continue;}
          }
          if (iii % 12 == 11) {
            if (iNearTC == 6 || iNearTC == 7 || iNearTC == 8) {continue;}

          }
          if (_BrCluster[_BRICN][iNearTC] != 0) {NofTCinCluster[_BRICN]++;}
          else {continue;}
          ClusterEnergy[_BRICN] +=   TCFireEnergy[_BrCluster[_BRICN][iNearTC] - 81];
          ClusterTiming[_BRICN] +=   TCFireEnergy[_BrCluster[_BRICN][iNearTC] - 81] * TCFireTiming[_BrCluster[_BRICN][iNearTC] - 81];
          ClusterPositionX[_BRICN] += TCFireEnergy[_BrCluster[_BRICN][iNearTC] - 81] * TCFirePosition[_BrCluster[_BRICN][iNearTC] - 81][0];
          ClusterPositionY[_BRICN] += TCFireEnergy[_BrCluster[_BRICN][iNearTC] - 81] * TCFirePosition[_BrCluster[_BRICN][iNearTC] - 81][1];
          ClusterPositionZ[_BRICN] += TCFireEnergy[_BrCluster[_BRICN][iNearTC] - 81] * TCFirePosition[_BrCluster[_BRICN][iNearTC] - 81][2];

          if (maxTC < TCFireEnergy[_BrCluster[_BRICN][iNearTC] - 81]) {
            maxTC = TCFireEnergy[_BrCluster[_BRICN][iNearTC] - 81];
            maxTCId = _BrCluster[_BRICN][iNearTC] ;

          }


        }
        ClusterTiming[_BRICN] /= ClusterEnergy[_BRICN];
        ClusterPositionX[_BRICN] /= ClusterEnergy[_BRICN];
        ClusterPositionY[_BRICN] /= ClusterEnergy[_BRICN];
        ClusterPositionZ[_BRICN] /= ClusterEnergy[_BRICN];
        MaxTCId[_BRICN] = maxTCId;
        if (ClusterTiming[_BRICN] == 0 && ClusterEnergy[_BRICN]) {continue;}
        _BRICN++;
        _icnquadrant[1][quad_sec]++;


      }
    }
  } // iii loop
  return _BRICN++;
}
//
//
//
int
TrgEclCluster::setForwardICN(int HitTC[][64], int Method)
{

  int TCFire[96] = { 0 };
  double TCFireEnergy[96] = {0};
  double TCFireTiming[96] = {0};
  double TCFirePosition[96][3] = {{0}};
  for (int iTC = 0; iTC < 96; iTC ++) {
    if (iTC < 9) {TempCluster[iTC] = 0;}
    TCFire[iTC] =  0 ;
    TCFireEnergy[iTC] = 0;
    TCFireTiming[iTC] = 0 ;
    TCFirePosition[iTC][0] = 0;
    TCFirePosition[iTC][1] = 0;
    TCFirePosition[iTC][2] = 0;


  }

  for (int iTCId0 = 0; iTCId0 < 80 ; iTCId0++) {
    int TCId = iTCId0 + 1;

    for (int itime = 0 ; itime < 64 ; itime++) {
      if (HitTC[TCId - 1][itime] == 0) {continue;}
      if (TCId > 80) { continue; }
      // TCFire[TCId-1] = TCId;
      TCFireEnergy[TCId - 1] += Energy[TCId - 1][itime];
      TCFireTiming[TCId - 1] = Timing[TCId - 1][itime];
      TCFirePosition[TCId - 1][0] = (_TCMap->getTCPosition(TCId)).X();
      TCFirePosition[TCId - 1][1] = (_TCMap->getTCPosition(TCId)).Y();
      TCFirePosition[TCId - 1][2] = (_TCMap->getTCPosition(TCId)).Z();

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
        TCFire[kkk]   = TCId;
        TCFire[kkk + 1] = TCId;
      } else {
        kkk = iTCId0 / 5;
        switch (iTCId0 % 5) {
          case 1 :
            TCFire[32 + 2 * kkk]   = TCId; break;
          case 2 :
            TCFire[64 + 2 * kkk]   = TCId; break;
          case 3 :
            TCFire[64 + 2 * kkk + 1] = TCId; break;
          case 4 :
            TCFire[32 + 2 * kkk + 1] = TCId; break;
          default:
            break;
        }
      }
    }

  }


  for (int iii = 0 ; iii < 96 ; iii++) {
    int quad_sec = (iii / 24);
    for (int iinit = 0; iinit < 9; iinit ++) {TempCluster[iinit] = 0;}
    if (TCFire[iii] == 0) { continue; }
    if (iii < 32) { // most inner

      if (iii < 2) {

        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[31]; // top
        TempCluster[2] = 0;// right top
        TempCluster[3] = 0;
        TempCluster[4] = 0;
        TempCluster[5] = TCFire[iii + 2]; //bottom
        if ((iii % 2) == 0) {
          TempCluster[6] = TCFire[iii + 34]; //bottom left
          TempCluster[8] = TCFire[63]; //top left
        }
        if ((iii % 2) == 1) {
          TempCluster[6] = TCFire[iii + 33]; //bottom left
          TempCluster[8] = TCFire[63]; //top left
        }

        TempCluster[7] = TCFire[iii + 32]; // left
      } else if (iii > 1 && iii < 30) {

        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 2]; // top
        TempCluster[2] = 0;// right top
        TempCluster[3] = 0;
        TempCluster[4] = 0;
        TempCluster[5] = TCFire[iii + 2]; //bottom
        if ((iii % 2) == 0) {
          TempCluster[6] = TCFire[iii + 34]; //bottom left
          TempCluster[8] = TCFire[iii + 31]; //top left
        }
        if ((iii % 2) == 1) {
          TempCluster[6] = TCFire[iii + 33]; //bottom left
          TempCluster[8] = TCFire[iii + 30]; //top left
        }

        TempCluster[7] = TCFire[iii + 32]; // left

      }

      else if (iii > 29) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 2]; // top
        TempCluster[2] = 0;// right top
        TempCluster[3] = 0;
        TempCluster[4] = 0;
        TempCluster[5] = TCFire[0]; //bottom
        if ((iii % 2) == 0) {
          TempCluster[6] = TCFire[32]; //bottom left
          TempCluster[8] = TCFire[iii + 31]; //top left
        }
        if ((iii % 2) == 1) {
          TempCluster[6] = TCFire[32]; //bottom left
          TempCluster[8] = TCFire[iii + 30]; //top left
        }
        TempCluster[7] = TCFire[iii + 32]; // left
      }
    } else if (iii > 31 && iii < 64) {
      if (iii == 32 || iii == 33) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[64]; // top
        TempCluster[2] = TCFire[31];// right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[iii - 31]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = TCFire[iii + 33]; //bottom left
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[95]; //top left
      } else if (iii == 63 || iii == 62) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = TCFire[iii - 33]; // right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[0]; //right bottom
        TempCluster[5] = TCFire[32]; //bottom
        TempCluster[6] = TCFire[64]; //bottom left
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[iii + 31]; //top left
      } else {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = TCFire[iii - 33]; // right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[iii - 31]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = TCFire[iii + 33]; //bottom left
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[iii + 31]; //top left
      }
    } else {
      if (iii == 64) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[95]; // top
        TempCluster[2] = TCFire[63];// right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[iii - 31]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = 0; //bottom left
        TempCluster[7] = 0; // left
        TempCluster[8] = 0; //top left
      } else if (iii == 95) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = TCFire[iii - 33]; // right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[32]; //right bottom
        TempCluster[5] = TCFire[64]; //bottom
        TempCluster[6] = 0; //bottom left
        TempCluster[7] = 0; // left
        TempCluster[8] = 0; //top left

      } else {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = TCFire[iii - 33]; // right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[iii - 31]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = 0; //bottom left
        TempCluster[7] = 0; // left
        TempCluster[8] = 0; //top left
      }

    }
    if (!(TempCluster[1] != 0 || TempCluster[7] != 0)) {
      if ((TempCluster[5] != 0 && TempCluster[6] != 0)) {
        if (Method == 1) { //for cluster method2
          int maxTCid = 0;
          double maxTCEnergy = 0;
          for (int iTC = 0; iTC < 9; iTC++) { //find center of Cluster
            if (TempCluster[iTC] == 0) {continue;}
            if (maxTCEnergy <       TCFireEnergy[TempCluster[iTC] - 81]) {
              maxTCEnergy = TCFireEnergy[TempCluster[iTC] - 81];
              maxTCid = TempCluster[iTC];
            }
          }
          int kkk = 0;
          TempCluster[0] = maxTCid;
          if ((TempCluster[0] - 1) % 5 == 0) {
            kkk = ((TempCluster[0] - 1) / 5) * 2;
          } else {
            kkk = (TempCluster[0] - 1) / 5;
          }

          if (kkk < 32) { // most inner

            if (kkk < 2) {

              TempCluster[0] = TCFire[kkk];
              TempCluster[1] = TCFire[31]; // top
              TempCluster[2] = 0;// right top
              TempCluster[3] = 0;
              TempCluster[4] = 0;
              TempCluster[5] = TCFire[kkk + 2]; //bottom
              if ((kkk % 2) == 0) {
                TempCluster[6] = TCFire[kkk + 34]; //bottom left
                TempCluster[8] = TCFire[63]; //top left
              }
              if ((kkk % 2) == 1) {
                TempCluster[6] = TCFire[kkk + 33]; //bottom left
                TempCluster[8] = TCFire[63]; //top left
              }

              TempCluster[7] = TCFire[kkk + 32]; // left
            } else if (kkk > 1 && kkk < 30) {

              TempCluster[0] = TCFire[kkk];
              TempCluster[1] = TCFire[kkk - 2]; // top
              TempCluster[2] = 0;// right top
              TempCluster[3] = 0;
              TempCluster[4] = 0;
              TempCluster[5] = TCFire[kkk + 2]; //bottom
              if ((kkk % 2) == 0) {
                TempCluster[6] = TCFire[kkk + 34]; //bottom left
                TempCluster[8] = TCFire[kkk + 31]; //top left
              }
              if ((kkk % 2) == 1) {
                TempCluster[6] = TCFire[kkk + 33]; //bottom left
                TempCluster[8] = TCFire[kkk + 30]; //top left
              }

              TempCluster[7] = TCFire[kkk + 32]; // left

            }

            else if (kkk > 29) {
              TempCluster[0] = TCFire[kkk];
              TempCluster[1] = TCFire[kkk - 2]; // top
              TempCluster[2] = 0;// right top
              TempCluster[3] = 0;
              TempCluster[4] = 0;
              TempCluster[5] = TCFire[0]; //bottom
              if ((kkk % 2) == 0) {
                TempCluster[6] = TCFire[32]; //bottom left
                TempCluster[8] = TCFire[kkk + 31]; //top left
              }
              if ((kkk % 2) == 1) {
                TempCluster[6] = TCFire[32]; //bottom left
                TempCluster[8] = TCFire[kkk + 30]; //top left
              }
              TempCluster[7] = TCFire[kkk + 32]; // left
            }
          } else if (kkk > 31 && kkk < 64) {
            if (kkk == 32 || kkk == 33) {
              TempCluster[0] = TCFire[kkk];
              TempCluster[1] = TCFire[64]; // top
              TempCluster[2] = TCFire[31];// right top
              TempCluster[3] = TCFire[kkk - 32]; //right
              TempCluster[4] = TCFire[kkk - 31]; //right bottom
              TempCluster[5] = TCFire[kkk + 1]; //bottom
              TempCluster[6] = TCFire[kkk + 33]; //bottom left
              TempCluster[7] = TCFire[kkk + 32]; // left
              TempCluster[8] = TCFire[95]; //top left
            } else if (kkk == 63 || kkk == 62) {
              TempCluster[0] = TCFire[kkk];
              TempCluster[1] = TCFire[kkk - 1]; // top
              TempCluster[2] = TCFire[kkk - 33]; // right top
              TempCluster[3] = TCFire[kkk - 32]; //right
              TempCluster[4] = TCFire[0]; //right bottom
              TempCluster[5] = TCFire[32]; //bottom
              TempCluster[6] = TCFire[64]; //bottom left
              TempCluster[7] = TCFire[kkk + 32]; // left
              TempCluster[8] = TCFire[kkk + 31]; //top left
            } else {
              TempCluster[0] = TCFire[kkk];
              TempCluster[1] = TCFire[kkk - 1]; // top
              TempCluster[2] = TCFire[kkk - 33]; // right top
              TempCluster[3] = TCFire[kkk - 32]; //right
              TempCluster[4] = TCFire[kkk - 31]; //right bottom
              TempCluster[5] = TCFire[kkk + 1]; //bottom
              TempCluster[6] = TCFire[kkk + 33]; //bottom left
              TempCluster[7] = TCFire[kkk + 32]; // left
              TempCluster[8] = TCFire[kkk + 31]; //top left
            }
          } else {
            if (kkk == 64) {
              TempCluster[0] = TCFire[kkk];
              TempCluster[1] = TCFire[95]; // top
              TempCluster[2] = TCFire[63];// right top
              TempCluster[3] = TCFire[kkk - 32]; //right
              TempCluster[4] = TCFire[kkk - 31]; //right bottom
              TempCluster[5] = TCFire[kkk + 1]; //bottom
              TempCluster[6] = 0; //bottom left
              TempCluster[7] = 0; // left
              TempCluster[8] = 0; //top left
            } else if (kkk == 95) {
              TempCluster[0] = TCFire[kkk];
              TempCluster[1] = TCFire[kkk - 1]; // top
              TempCluster[2] = TCFire[kkk - 33]; // right top
              TempCluster[3] = TCFire[kkk - 32]; //right
              TempCluster[4] = TCFire[32]; //right bottom
              TempCluster[5] = TCFire[64]; //bottom
              TempCluster[6] = 0; //bottom left
              TempCluster[7] = 0; // left
              TempCluster[8] = 0; //top left

            } else {
              TempCluster[0] = TCFire[kkk];
              TempCluster[1] = TCFire[kkk - 1]; // top
              TempCluster[2] = TCFire[kkk - 33]; // right top
              TempCluster[3] = TCFire[kkk - 32]; //right
              TempCluster[4] = TCFire[kkk - 31]; //right bottom
              TempCluster[5] = TCFire[kkk + 1]; //bottom
              TempCluster[6] = 0; //bottom left
              TempCluster[7] = 0; // left
              TempCluster[8] = 0; //top left
            }

          }

        }
        double maxTC = 0;
        int maxTCId = 999;
        for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
          if (TempCluster[iNearTC] == 0) {continue;}
          _FwCluster[_FWDICN][iNearTC] = TempCluster[iNearTC];
          if (_FwCluster[_FWDICN][iNearTC] != 0) {NofTCinCluster[_BRICN + _FWDICN]++;}
          else {continue;}
          ClusterEnergy[_BRICN + _FWDICN] +=   TCFireEnergy[_FwCluster[_FWDICN][iNearTC] - 1];

          ClusterTiming[_BRICN + _FWDICN] +=   TCFireEnergy[_FwCluster[_FWDICN][iNearTC] - 1] * TCFireTiming[_FwCluster[_FWDICN][iNearTC] -
                                               1];

          ClusterPositionX[_BRICN + _FWDICN] += TCFireEnergy[_FwCluster[_FWDICN][iNearTC] - 1] * TCFirePosition[_FwCluster[_FWDICN][iNearTC] -
                                                1][0];
          ClusterPositionY[_BRICN + _FWDICN] += TCFireEnergy[_FwCluster[_FWDICN][iNearTC] - 1] * TCFirePosition[_FwCluster[_FWDICN][iNearTC] -
                                                1][1];

          ClusterPositionZ[_BRICN + _FWDICN] += TCFireEnergy[_FwCluster[_FWDICN][iNearTC] - 1] * TCFirePosition[_FwCluster[_FWDICN][iNearTC] -
                                                1][2];
          if (maxTC < TCFireEnergy[_FwCluster[_FWDICN][iNearTC] - 1]) {
            maxTC = TCFireEnergy[_FwCluster[_FWDICN][iNearTC] - 1];
            maxTCId = _FwCluster[_FWDICN][iNearTC] ;

          }
        }

        ClusterTiming[_BRICN + _FWDICN] /= ClusterEnergy[_BRICN + _FWDICN];
        ClusterPositionX[_BRICN + _FWDICN] /= ClusterEnergy[_BRICN + _FWDICN];
        ClusterPositionY[_BRICN + _FWDICN] /= ClusterEnergy[_BRICN + _FWDICN];
        ClusterPositionZ[_BRICN + _FWDICN] /= ClusterEnergy[_BRICN + _FWDICN];
        MaxTCId[_BRICN + _FWDICN] = maxTCId;

        if (ClusterTiming[_BRICN + _FWDICN] == 0 && ClusterEnergy[_BRICN + _FWDICN] == 0) {continue;}
        _FWDICN++;


        _icnquadrant[0][quad_sec]++;
      }
    }
  }

  return _FWDICN;
}
//
//
//
int TrgEclCluster::setBackwardICN(int HitTC[][64], int Method)
{

  int TCFire[64] = { 0 };
  double TCFireEnergy[64] = {0};
  double TCFireTiming[64] = {0};
  double TCFirePosition[64][3] = {{0}};
  for (int iinit = 0; iinit < 9; iinit ++) {TempCluster[iinit] = 0;}

  for (int iTC = 0; iTC < 64; iTC ++) {

    TCFire[iTC] =  0 ;
    TCFireEnergy[iTC] = 0;
    TCFireTiming[iTC] = 0 ;
    TCFirePosition[iTC][0] = 0;
    TCFirePosition[iTC][1] = 0;
    TCFirePosition[iTC][2] = 0;

  }



  for (int iii = 0; iii < 4 ; iii++) {
    _icnquadrant[2][iii] = 0;
  }
  for (int iTCId0 = 0; iTCId0 < 64 ; iTCId0++) {
    int TCId = iTCId0 + 513;
    for (int itime = 0 ; itime < 64 ; itime++) {
      if (HitTC[TCId - 1][itime] == 0) {continue;}
      if (TCId < 513) { continue; }
      //  TCFire[TCId-1] = TCId;
      TCFireEnergy[TCId - 513] += Energy[TCId - 1][itime];
      TCFireTiming[TCId - 513] = Timing[TCId - 1][itime];
      TCFirePosition[TCId - 513][0] = (_TCMap->getTCPosition(TCId)).X();
      TCFirePosition[TCId - 513][1] = (_TCMap->getTCPosition(TCId)).Y();
      TCFirePosition[TCId - 513][2] = (_TCMap->getTCPosition(TCId)).Z();

      //------------------------------------
      // To rearrange the hitted map
      //
      //   orignal       converted
      //  (<- Theta)    (<- Theta)
      //
      //     516  515       32  0
      //     513  514       33  1
      //     520  519  =>   34  2
      //     517  518       35  3
      //     ...            .
      //     576  575       62 30
      //     573  574       63 31
      //
      // Here, TCId-1 becomes TCId=0 and 1.
      //------------------------------------
      int kkk = 0;
      if ((TCId - 513) % 4 == 2) {
        kkk = (TCId - 513) / 2 - 1;
      }
      if ((TCId - 513) % 4 == 1) {
        kkk = ((TCId - 513) + 1) / 2;
      }
      if ((TCId - 513) % 4 == 3) {
        kkk =  32 + ((TCId - 513) - 3) / 2;
      }
      if ((TCId - 513) % 4 == 0) {
        kkk =  33 + ((TCId - 513)) / 2;
      }

      TCFire[kkk] = TCId;

    }
  }


  for (int iii = 0 ; iii < 64 ; iii ++) {
    int quad_sec = (iii / 20);
    if (TCFire[iii] == 0) { continue; }

    for (int iinit = 0; iinit < 9; iinit ++) {TempCluster[iinit] = 0;}
    if (iii < 32) {
      TempCluster[0] = TCFire[iii];
      TempCluster[2] = 0;// right top
      TempCluster[3] = 0; //right
      TempCluster[4] = 0; //right bottom
      TempCluster[5] = TCFire[iii + 1]; //bottom
      TempCluster[7] = TCFire[iii + 32]; // left
      TempCluster[8] = TCFire[iii + 31]; //top left


      if (iii == 0) {
        TempCluster[1] = TCFire[31]; // top
        TempCluster[8] = TCFire[63]; //top left
      } else {
        TempCluster[1] = TCFire[iii - 1]; // top
      }
      if (iii == 31) {
        TempCluster[5] = TCFire[0]; //bottom
        TempCluster[6] = TCFire[31]; //bottom left
      } else {
        TempCluster[6] = TCFire[iii + 33]; //bottom left
      }

    } else if (iii > 31) {
      TempCluster[0] = TCFire[iii];
      TempCluster[1] = TCFire[iii - 1]; // top
      TempCluster[3] = TCFire[iii - 32]; //right
      TempCluster[4] = TCFire[iii - 31]; //right bottom
      TempCluster[6] = 0; //bottom left
      TempCluster[7] = 0; // left
      TempCluster[8] = 0; //top left
      if (iii == 32) {
        TempCluster[1] = TCFire[63]; // top
        TempCluster[2] = TCFire[31];// right top

      } else {
        TempCluster[2] = TCFire[iii - 33]; // right top
      }
      if (iii == 63) {
        TempCluster[5] = TCFire[32]; //bottom
        TempCluster[4] = TCFire[0]; //right bottom

      } else {
        TempCluster[5] = TCFire[iii + 1]; //bottom
      }
    }

    if (!(TempCluster[1] != 0 || TempCluster[7] != 0)) {
      if (!(TempCluster[5] != 0 && TempCluster[6] != 0)) {
        if (Method == 1) {
          int maxTCid = 0;
          double maxTCEnergy = 0;
          for (int iTC = 0; iTC < 9; iTC++) { //find center of Cluster
            if (TempCluster[iTC] == 0) {continue;}
            if (maxTCEnergy <       TCFireEnergy[TempCluster[iTC] - 81]) {
              maxTCEnergy = TCFireEnergy[TempCluster[iTC] - 81];
              maxTCid = TempCluster[iTC];
            }
          }
          int kkk = 0;
          TempCluster[0] = maxTCid;
          if ((TempCluster[0] - 513) % 4 == 2) {
            kkk = (TempCluster[0] - 513) / 2 - 1;
          }
          if ((TempCluster[0] - 513) % 4 == 1) {
            kkk = ((TempCluster[0] - 513) + 1) / 2;
          }
          if ((TempCluster[0] - 513) % 4 == 3) {
            kkk =  32 + ((TempCluster[0] - 513) - 3) / 2;
          }
          if ((TempCluster[0] - 513) % 4 == 0) {
            kkk =  33 + ((TempCluster[0] - 513)) / 2;
          }

          if (kkk < 32) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[2] = 0;// right top
            TempCluster[3] = 0; //right
            TempCluster[4] = 0; //right bottom
            TempCluster[5] = TCFire[kkk + 1]; //bottom
            TempCluster[7] = TCFire[kkk + 32]; // left
            TempCluster[8] = TCFire[kkk + 31]; //top left


            if (kkk == 0) {
              TempCluster[1] = TCFire[31]; // top
              TempCluster[8] = TCFire[63]; //top left
            } else {
              TempCluster[1] = TCFire[kkk - 1]; // top
            }
            if (kkk == 31) {
              TempCluster[5] = TCFire[0]; //bottom
              TempCluster[6] = TCFire[31]; //bottom left
            } else {
              TempCluster[6] = TCFire[kkk + 33]; //bottom left
            }

          } else if (kkk > 31) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 1]; // top
            TempCluster[3] = TCFire[kkk - 32]; //right
            TempCluster[4] = TCFire[kkk - 31]; //right bottom
            TempCluster[6] = 0; //bottom left
            TempCluster[7] = 0; // left
            TempCluster[8] = 0; //top left
            if (kkk == 32) {
              TempCluster[1] = TCFire[63]; // top
              TempCluster[2] = TCFire[31];// right top

            } else {
              TempCluster[2] = TCFire[kkk - 33]; // right top
            }
            if (kkk == 63) {
              TempCluster[5] = TCFire[32]; //bottom
              TempCluster[4] = TCFire[0]; //right bottom

            } else {
              TempCluster[5] = TCFire[kkk + 1]; //bottom
            }
          }

        }
        double maxTC = 0;
        int maxTCId = 999;

        for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
          if (TempCluster[iNearTC] == 0) {continue;}
          _BwCluster[_FWDICN][iNearTC] = TempCluster[iNearTC];
          if (iNearTC == 2 || iNearTC == 3 || iNearTC == 4) {continue;}
          if (_BwCluster[_BWDICN][iNearTC] != 0) {NofTCinCluster[_BRICN + _FWDICN + _BWDICN]++;}
          else {continue;}


          ClusterEnergy[_BRICN + _FWDICN + _BWDICN] +=   TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513];

          ClusterTiming[_BRICN + _FWDICN + _BWDICN] +=   TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
                                                         TCFireTiming[_BwCluster[_BWDICN][iNearTC] - 513];
          ClusterPositionX[_BRICN + _FWDICN + _BWDICN] += TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
                                                          TCFirePosition[_BwCluster[_BWDICN][iNearTC] - 513][0];
          ClusterPositionY[_BRICN + _FWDICN + _BWDICN] += TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
                                                          TCFirePosition[_BwCluster[_BWDICN][iNearTC] - 513][1];
          ClusterPositionZ[_BRICN + _FWDICN + _BWDICN] += TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
                                                          TCFirePosition[_BwCluster[_BWDICN][iNearTC] - 513][2];
          if (maxTC < TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513]) {
            maxTC = TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513];
            maxTCId = _BwCluster[_BWDICN][iNearTC]  ;
          }
        }
        ClusterTiming[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
        ClusterPositionX[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
        ClusterPositionY[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
        ClusterPositionZ[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
        MaxTCId[_BRICN +  _FWDICN + _BWDICN] = maxTCId;

        if (ClusterTiming[_BRICN + _FWDICN + _BWDICN] == 0 && ClusterEnergy[_BRICN + _FWDICN + _BWDICN] == 0) {continue;}

        _BWDICN++;
        _icnquadrant[2][quad_sec]++;


      }

    }



  }

  // for (int iii = 0 ; iii < 32 ; iii ++) {
  //   int quad_sec = (iii / 20);
  //   if (TCFire[iii] == 0) { continue; }

  //   for(int iinit = 0; iinit <9; iinit ++){TempCluster[iinit]= 0;}

  //   TempCluster[0] = TCFire[iii];
  //   TempCluster[2] = 0;// right top
  //   TempCluster[3] = 0; //right
  //   TempCluster[4] = 0; //right bottom
  //   TempCluster[5] = TCFire[iii + 1]; //bottom
  //   TempCluster[7] = TCFire[iii + 32]; // left
  //   TempCluster[8] = TCFire[iii + 31]; //top left


  //   if (iii == 0) {
  //     TempCluster[1] = TCFire[31]; // top
  //     TempCluster[8] = TCFire[63]; //top left
  //   } else {
  //     TempCluster[1] = TCFire[iii - 1]; // top
  //   }
  //   if (iii == 31) {
  //     TempCluster[5] = TCFire[0]; //bottom
  //     TempCluster[6] = TCFire[31]; //bottom left
  //   } else {
  //     TempCluster[6] = TCFire[iii + 33]; //bottom left
  //   }
  //   if (!(TempCluster[1] != 0 || TempCluster[7] != 0)) {
  //     if (!(TempCluster[5] != 0 && TempCluster[6] != 0)) {

  //  if(Method==1){
  //    int maxTCid = 0;
  //    double maxTCEnergy = 0;
  //    for(int iTC = 0; iTC< 9; iTC++){ //find center of Cluster
  //      if(TempCluster[iTC]==0){continue;}
  //      if(maxTCEnergy<       TCFireEnergy[TempCluster[iTC]- 81] ){
  //        maxTCEnergy = TCFireEnergy[TempCluster[iTC]- 81];
  //        maxTCid = TempCluster[iTC];
  //      }
  //    }
  //    int kkk =0;
  //    TempCluster[0] = maxTCid;

  //    if ((TempCluster[0] - 513) % 4 == 2) {
  //      kkk = (TempCluster[0] - 513) / 2 - 1;
  //    }
  //    if ((TempCluster[0] - 513) % 4 == 1) {
  //      kkk = ((TempCluster[0] - 513) + 1) / 2;
  //    }
  //    if ((TempCluster[0] - 513) % 4 == 3) {
  //      kkk =  32 + ((TempCluster[0] - 513) - 3) / 2;
  //    }
  //    if ((TempCluster[0] - 513) % 4 == 0) {
  //      kkk =  33 + ((TempCluster[0] - 513)) / 2;
  //    }

  //    if (kkk == 0) {
  //      TempCluster[1] = TCFire[31]; // top
  //      TempCluster[8] = TCFire[63]; //top left
  //    } else {
  //      TempCluster[1] = TCFire[kkk - 1]; // top
  //    }
  //    if (kkk == 31) {
  //      TempCluster[5] = TCFire[0]; //bottom
  //      TempCluster[6] = TCFire[31]; //bottom left
  //    } else {
  //      TempCluster[6] = TCFire[kkk + 33]; //bottom left
  //    }
  //  }

  //       double maxTC = 0;
  //       int maxTCId = 999;
  //       for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
  //    _BwCluster[_FWDICN][iNearTC] = TempCluster[iNearTC];

  //         if (iNearTC == 2 || iNearTC == 3 || iNearTC == 4) {continue;}
  //         if (_BwCluster[_BWDICN][iNearTC] != 0) {NofTCinCluster[_BRICN + _FWDICN + _BWDICN]++;}
  //         else {continue;}

  //         ClusterEnergy[_BRICN + _FWDICN + _BWDICN] +=   TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513];
  //         ClusterTiming[_BRICN + _FWDICN + _BWDICN] +=   TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
  //                                                        TCFireTiming[_BwCluster[_BWDICN][iNearTC] - 513];
  //         ClusterPositionX[_BRICN + _FWDICN + _BWDICN] += TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
  //                                                         TCFirePosition[_BwCluster[_BWDICN][iNearTC] - 513][0];
  //         ClusterPositionY[_BRICN + _FWDICN + _BWDICN] += TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
  //                                                         TCFirePosition[_BwCluster[_BWDICN][iNearTC] - 513][1];
  //         ClusterPositionZ[_BRICN + _FWDICN + _BWDICN] += TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
  //                                                         TCFirePosition[_BwCluster[_BWDICN][iNearTC] - 513][2];
  //         if (maxTC < TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513]) {
  //           maxTC = TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513];
  //           maxTCId = _BwCluster[_BWDICN][iNearTC]  ;
  //         }
  //       }
  //       ClusterTiming[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
  //       ClusterPositionX[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
  //       ClusterPositionY[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
  //       ClusterPositionZ[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
  //       MaxTCId[_BRICN +  _FWDICN + _BWDICN] = maxTCId;

  //       if (ClusterTiming[_BRICN + _FWDICN + _BWDICN] == 0 && ClusterEnergy[_BRICN + _FWDICN + _BWDICN] == 0) {continue;}
  //       _BWDICN++;
  //       _icnquadrant[2][quad_sec]++;


  //     }
  //   }




  // }
  // for (int iii = 32 ; iii < 64 ; iii ++) {
  //   int quad_sec = (iii / 20);
  //   if (TCFire[iii] == 0) { continue; }
  //   for(int iinit = 0; iinit <9; iinit ++){TempCluster[iinit]= 0;}

  //   TempCluster[0] = TCFire[iii];
  //   TempCluster[1] = TCFire[iii - 1]; // top
  //   TempCluster[3] = TCFire[iii - 32]; //right
  //   TempCluster[4] = TCFire[iii - 31]; //right bottom
  //   TempCluster[6] = 0; //bottom left
  //   TempCluster[7] = 0; // left
  //   TempCluster[8] = 0; //top left
  //   if (iii == 32) {
  //     TempCluster[1] = TCFire[63]; // top
  //     TempCluster[2] = TCFire[31];// right top

  //   } else {
  //     TempCluster[2] = TCFire[iii - 33]; // right top
  //   }
  //   if (iii == 63) {
  //     TempCluster[5] = TCFire[32]; //bottom
  //     TempCluster[4] = TCFire[0]; //right bottom

  //   } else {
  //     TempCluster[5] = TCFire[iii + 1]; //bottom
  //   }



  //   if (!(TempCluster[1] != 0 || TempCluster[7] != 0)) {
  //     if (!(TempCluster[5] != 0 && TempCluster[6] != 0)) {
  //  if(Method==1){
  //    int maxTCid = 0;
  //    double maxTCEnergy = 0;
  //    for(int iTC = 0; iTC< 9; iTC++){ //find center of Cluster
  //      if(TempCluster[iTC]==0){continue;}
  //      if(maxTCEnergy<       TCFireEnergy[TempCluster[iTC]- 81] ){
  //        maxTCEnergy = TCFireEnergy[TempCluster[iTC]- 81];
  //        maxTCid = TempCluster[iTC];
  //      }
  //    }
  //    int kkk =0;
  //    TempCluster[0] = maxTCid;
  //    if ((TempCluster[0] - 513) % 4 == 2) {
  //      kkk = (TempCluster[0] - 513) / 2 - 1;
  //    }
  //    if ((TempCluster[0] - 513) % 4 == 1) {
  //      kkk = ((TempCluster[0] - 513) + 1) / 2;
  //    }
  //    if ((TempCluster[0] - 513) % 4 == 3) {
  //      kkk =  32 + ((TempCluster[0] - 513) - 3) / 2;
  //    }
  //    if ((TempCluster[0] - 513) % 4 == 0) {
  //      kkk =  33 + ((TempCluster[0] - 513)) / 2;
  //    }

  //    TempCluster[0] = TCFire[kkk];
  //    TempCluster[1] = TCFire[kkk - 1]; // top
  //    TempCluster[3] = TCFire[kkk - 32]; //right
  //    TempCluster[4] = TCFire[kkk - 31]; //right bottom
  //    TempCluster[6] = 0; //bottom left
  //    TempCluster[7] = 0; // left
  //    TempCluster[8] = 0; //top left
  //    if (kkk == 32) {
  //      TempCluster[1] = TCFire[63]; // top
  //      TempCluster[2] = TCFire[31];// right top

  //    } else {
  //      TempCluster[2] = TCFire[kkk - 33]; // right top
  //    }
  //    if (kkk == 63) {
  //      TempCluster[5] = TCFire[32]; //bottom
  //      TempCluster[4] = TCFire[0]; //right bottom

  //    } else {
  //      TempCluster[5] = TCFire[kkk + 1]; //bottom
  //    }



  //  }
  //       double maxTC = 0;
  //       int maxTCId = 999;
  //       for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
  //    _BwCluster[_FWDICN][iNearTC] = TempCluster[iNearTC];
  //         if (iNearTC == 2 || iNearTC == 3 || iNearTC == 4) {continue;}
  //         if (_BwCluster[_BWDICN][iNearTC] != 0) {NofTCinCluster[_BRICN + _FWDICN + _BWDICN]++;}
  //         else {continue;}


  //         ClusterEnergy[_BRICN + _FWDICN + _BWDICN] +=   TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513];

  //         ClusterTiming[_BRICN + _FWDICN + _BWDICN] +=   TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
  //                                                        TCFireTiming[_BwCluster[_BWDICN][iNearTC] - 513];
  //         ClusterPositionX[_BRICN + _FWDICN + _BWDICN] += TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
  //                                                         TCFirePosition[_BwCluster[_BWDICN][iNearTC] - 513][0];
  //         ClusterPositionY[_BRICN + _FWDICN + _BWDICN] += TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
  //                                                         TCFirePosition[_BwCluster[_BWDICN][iNearTC] - 513][1];
  //         ClusterPositionZ[_BRICN + _FWDICN + _BWDICN] += TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513] *
  //                                                         TCFirePosition[_BwCluster[_BWDICN][iNearTC] - 513][2];
  //         if (maxTC < TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513]) {
  //           maxTC = TCFireEnergy[_BwCluster[_BWDICN][iNearTC] - 513];
  //           maxTCId = _BwCluster[_BWDICN][iNearTC]  ;
  //         }
  //       }
  //       ClusterTiming[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
  //       ClusterPositionX[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
  //       ClusterPositionY[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
  //       ClusterPositionZ[_BRICN + _FWDICN + _BWDICN] /= ClusterEnergy[_BRICN + _FWDICN + _BWDICN];
  //       MaxTCId[_BRICN +  _FWDICN + _BWDICN] = maxTCId;

  //       if (ClusterTiming[_BRICN + _FWDICN + _BWDICN] == 0 && ClusterEnergy[_BRICN + _FWDICN + _BWDICN] == 0) {continue;}



  //       _BWDICN++;
  //       _icnquadrant[2][quad_sec]++;


  //     }
  //   }



  // }



  return _BWDICN;

}
//
//===<END>
//
