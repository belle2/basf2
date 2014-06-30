//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TrgEclCluster.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
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
    void setICN(TrgEclFAM*, int HitTC[][20]);
    // calculate ICN in Barrel
    int setBarrelICN(TrgEclFAM*, int HitTC[][20]);
    // calculate ICN in Foward endcap
    int setForwardICN(TrgEclFAM*, int HitTC[][20]);
    // calculate ICN in Backward endcap
    int setBackwardICN(TrgEclFAM*);
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
    //
    //
    // get Beam bkg veto flag.
    // On = true, Off = false.
    bool getBeamBkgVeto(void);
    //
  private:
    //
    //  static TrgEclCluster * _instance;
    int _icn;
    int _icnquadrant[3][4];
    int _icnfwbrbw[3];

  };
//
//
//
} // namespace Belle2

#endif /* TRGECLCLUSTER_FLAG_ */
