//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TRGECLCluster.h
// Section  : TRG ECL
// Owner    : Yuuji Unno
// Email    : yunno@post.kek.jp
//---------------------------------------------------------
// Description : A class to represent ECL.
//---------------------------------------------------------
// $Log$
//---------------------------------------------------------

#ifndef TRGECLCLUSTER_FLAG_
#define TRGECLCLUSTER_FLAG_

#include <string>
#include <vector>

#include "trg/ecl/TCHit.h"
//
//
//
namespace Belle2 {
//
//
//
class TRGECLCluster;
//
// A class to represent ECL.
//
class TRGECLCluster {

 public:

  // Constructor
  TRGECLCluster();
  //
  //  static TRGECLCluster & Instance() {
  //    _instance = new TRGECLCluster();
  //    return * _instance;
  //  }
  // Destructor
  virtual ~TRGECLCluster(){};

 public:

  // set ICN for each part(Fw,Br,Bw)
  void setICN(TCHit *);
  // calculate ICN in Barrel
  int setBarrelICN(TCHit *);
  // calculate ICN in Foward endcap
  int setForwardICN(TCHit *);
  // calculate ICN in Backward endcap
  int setBackwardICN(TCHit *);
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
  //  static TRGECLCluster * _instance;
  int _icn;
  int _icnquadrant[3][4];
  int _icnfwbrbw[3];

};
//
//
//
} // namespace Belle2

#endif /* TRGECLCLUSTER_FLAG_ */
