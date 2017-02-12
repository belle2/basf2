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
  //
  /*! A Class of  ECL Trigger clustering  */
  //
  class TrgEclCluster {

  public:
    /** Constructor */
    TrgEclCluster();    /// Constructor

    /** Destructor */
    virtual ~TrgEclCluster() {};/// Destructor

  public:

    /** set ICN for each part(Fw,Br,Bw) */
    void setICN(int HitTC[][64], int);
    /**  calculate ICN in Barrel */
    int setBarrelICN(int HitTC[][64], int);
    /**  calculate ICN in Foward endcap */
    int setForwardICN(int HitTC[][64], int);
    /** calculate ICN in Backward endcap */
    int setBackwardICN(int HitTC[][64], int);
    //
    //
    /** get ICN in Barrel and Forward */
    int getICNFwBr(void);

    /** get ICN in each region(Fw(0), Br(1), Bw(2)) */
    int getICNSub(int);
    //

    /*! get ICN in QuadrantId in Fw or Br or Bw.
     * FwBrBwId = 0(Fw),1(Br),2(Bw)
     * QuadrantId = 0,1,2,3
     */
    /** get ICN in QuadrantId in Fw or Br or Bw. */
    int getICNQuadrant(int FwBrBwId, int QuadrantId);
    /** */
    int getBrICNCluster(int ICNId, int);///0 : center , 1; upper , 2: right , 3: lower , 4: lower right
    /** */
    int getBwICNCluster(int ICNId, int);///0 : center , 1; upper , 2: right , 3: lower , 4: lower right
    /** */
    int getFwICNCluster(int ICNId, int);///0 : center , 1; upper , 2: right , 3: lower , 4: lower right
    /** Output # of cluster */
    int getNofCluster() {return _BRICN + _FWDICN + _BWDICN;}
    /** Ouput # of TC in Cluster */
    int getNofTCinCluster(int icluster) {return NofTCinCluster[icluster];}

    /** Output cluster Energy  */
    double getClusterEnergy(int icluster) {return ClusterEnergy[icluster];}
    /**Output cluster timing  */
    double getClusterTiming(int icluster) {return ClusterTiming[icluster];}
    /** Output cluster position */
    TVector3 getClusterPosition(int icluster) {return TVector3(ClusterPositionX[icluster], ClusterPositionY[icluster], ClusterPositionZ[icluster]);}/// Output cluster position
    /** Output maximum contribution TC Id in Cluster  */
    int getMaxTCId(int icluster) {return MaxTCId[icluster];}

    //
    //
    /*! get Beam bkg veto flag.
     * On = true, Off = false.
     */
    bool getBeamBkgVeto(void);///get Beam bkg veto flag
  private:
    //


    /**ICN in Barrel */
    int _BRICN;
    /** ICN in Forward Endcap */
    int _FWDICN;
    /** ICN in Backward Endcap */
    int _BWDICN;

    /** 4 region along r phi plane  */
    int _icnquadrant[3][4];
    /** icn  */
    int _icnfwbrbw[3];
    /**  cluster in barrel */
    int _BrCluster[20][9];
    /**  cluster in forward endcap */
    int _FwCluster[20][9];
    /**  cluster in backward endcap */
    int _BwCluster[20][9];
    /**  # of TC in cluster */
    int NofTCinCluster[100];

    /**  TC timing */
    double Timing[576][64];
    /** TC energy  */
    double Energy[576][64];

    /** Cluster timing */
    double ClusterTiming[100];
    /**Cluster enrgy  */
    double ClusterEnergy[100];
    /**  Cluster position in X-axis */
    double ClusterPositionX[100];
    /** Cluster position in Y-axis */
    double ClusterPositionY[100];
    /**  Cluster position in Z-axis */
    double ClusterPositionZ[100];
    /** Maximum contribution TC Id in Cluster */
    int MaxTCId[100];
    /** Temporal Cluster **/
    int TempCluster[9];
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;


  };
//
//
//
} // namespace Belle2

#endif /* TRGECLCLUSTER_FLAG_ */
