//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TrgEclCluster.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------
// Description : A class to represent TRG ECL.
//---------------------------------------------------------
// $Log$
//---------------------------------------------------------

#ifndef TRGECLCLUSTER_FLAG_
#define TRGECLCLUSTER_FLAG_

#include <iostream>
#include <string>
#include <vector>

#include "trg/ecl/TrgEclFAM.h"
#include "trg/ecl/TrgEclMapping.h"
#include "TVector3.h"
//
//
//
namespace Belle2 {
//
//
//
  class TrgEclCluster;
//
// A class to represent ECL.
//
  class TrgEclCluster {

  public:

    // Constructor
    TrgEclCluster();
    //
    //  static TrgEclCluster & Instance() {
    //    _instance = new TrgEclCluster();
    //    return * _instance;
    //  }
    // Destructor
    virtual ~TrgEclCluster() {};

  public:

    // set ICN for each part(Fw,Br,Bw)
    void setICN(int HitTC[][80]);
    // calculate ICN in Barrel
    int setBarrelICN(int HitTC[][80]);
    // calculate ICN in Foward endcap
    int setForwardICN(int HitTC[][80]);
    // calculate ICN in Backward endcap
    int setBackwardICN(int HitTC[][80]);
    //
    //
    // get ICN in Barrel and Forward
    int getICNFwBr(void);
    // get ICN in each region(Fw(0), Br(1), Bw(2))
    int getICNSub(int);
    //
    // get ICN in QuadrantId in Fw or Br or Bw.
    // FwBrBwId = 0(Fw),1(Br),2(Bw)
    // QuadrantId = 0,1,2,3
    int getICNQuadrant(int FwBrBwId, int QuadrantId);
    int getBrICNCluster(int ICNId, int);//0 : center , 1; upper , 2: right , 3: lower , 4: lower right
    int getBwICNCluster(int ICNId, int);//0 : center , 1; upper , 2: right , 3: lower , 4: lower right
    int getFwICNCluster(int ICNId, int);//0 : center , 1; upper , 2: right , 3: lower , 4: lower right

    int getNofCluster() {return _BRICN + _FWDICN + _BWDICN;}
    int getNofTCinCluster(int icluster) {return NofTCinCluster[icluster];}
    double getClusterEnergy(int icluster) {return ClusterEnergy[icluster];}
    double getClusterTiming(int icluster) {return ClusterTiming[icluster];}
    TVector3 getClusterPosition(int icluster) {return TVector3(ClusterPositionX[icluster], ClusterPositionY[icluster], ClusterPositionZ[icluster]);}
    //
    //
    // get Beam bkg veto flag.
    // On = true, Off = false.
    bool getBeamBkgVeto(void);
    //
  private:
    //
    //  static TrgEclCluster * _instance;
    int _BRICN;
    int _FWDICN;
    int _BWDICN;


    int _icnquadrant[3][4];
    int _icnfwbrbw[3];
    int _BrCluster[20][9];
    int _FwCluster[20][9];
    int _BwCluster[20][9];
    int NofTCinCluster[100];


    double Timing[576][80];
    double Energy[576][80];

    double ClusterTiming[100];
    double ClusterEnergy[100];
    double ClusterPositionX[100];
    double ClusterPositionY[100];
    double ClusterPositionZ[100];


    TrgEclMapping* _TCMap;


  };
//
//
//
} // namespace Belle2

#endif /* TRGECLCLUSTER_FLAG_ */
