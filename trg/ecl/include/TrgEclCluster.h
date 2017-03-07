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

//#include "trg/ecl/TrgEclFAM.h"
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
    void setICN(std::vector<int> , std::vector<double>, std::vector<double>);
    /** Save Cluster information in TRGECLCluster Table */
    void save(int);
    /**  calculate ICN in Barrel */
    int setBarrelICN(int);
    /**  calculate ICN in Foward endcap */
    int setForwardICN(int);
    /** calculate ICN in Backward endcap */
    int setBackwardICN(int);
    /** Set EventId */
    void setEventId(int eventId) {_EventId = eventId; }
    /** Set */
    void setClusteringMethod(int method) {_Method = method; }

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
    /** */
    int getBrICNCluster(int ICNId, int);///0 : center , 1; upper , 2: right , 3: lower , 4: lower right
    /** */
    int getBwICNCluster(int ICNId, int);///0 : center , 1; upper , 2: right , 3: lower , 4: lower right
    /** */
    int getFwICNCluster(int ICNId, int);///0 : center , 1; upper , 2: right , 3: lower , 4: lower right
    /** Output # of cluster */
    int getNofCluster() {return _BRICN + _FWDICN + _BWDICN;}


    //
    //
    /*! get Beam bkg veto flag.
     * On = true, Off = false.
     */
  private:
    //


    /**ICN in Barrel */
    int _BRICN;
    /** ICN in Forward Endcap */
    int _FWDICN;
    /** ICN in Backward Endcap */
    int _BWDICN;
    /** event number */
    int _EventId;
    /** Clustering method */
    int _Method;

    /** icn  */
    std::vector<int> _icnfwbrbw;

    /**  TC Id */
    std::vector<int> TCId;
    /**  TC timing */
    std::vector<double> Timing;
    /** TC energy  */
    std::vector<double> Energy;


    /**  cluster in barrel */
    std::vector<std::vector<int>> _BrCluster;
    /**  cluster in forward endcap */
    std::vector<std::vector<int>> _FwCluster;
    /**  cluster in backward endcap */
    std::vector<std::vector<int>> _BwCluster;



    /** Cluster timing */
    std::vector<std::vector<double>> ClusterTiming;
    /**Cluster enrgy  */
    std::vector<std::vector<double>> ClusterEnergy;
    /**  Cluster position in X-axis */
    std::vector<std::vector<double>> ClusterPositionX;
    /** Cluster position in Y-axis */
    std::vector<std::vector<double>> ClusterPositionY;
    /**  Cluster position in Z-axis */
    std::vector<std::vector<double>> ClusterPositionZ;
    /** N of TC in Cluster  */
    std::vector<std::vector<int>> NofTCinCluster;
    /** Maximum contribution TC Id in Cluster */
    std::vector<std::vector<int>>  MaxTCId;
    /** Temporal Cluster **/
    std::vector<int>  TempCluster;
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;


  };
//
//
//
} // namespace Belle2

#endif /* TRGECLCLUSTER_FLAG_ */
