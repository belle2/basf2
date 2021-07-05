/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

#include <vector>

//#include "trg/ecl/TrgEclFAM.h"
#include "trg/ecl/TrgEclMapping.h"
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
    virtual ~TrgEclCluster();/// Destructor

    /** Copy constructor, deleted. */
    TrgEclCluster(TrgEclCluster&) = delete;

    /** Assignement operator, deleted. */
    TrgEclCluster& operator=(TrgEclCluster&) = delete;

  public:

    /** set ICN for each part(Fw,Br,Bw) */
    void setICN(const std::vector<int>&);
    /** set Belle 2 Clustering */
    void setICN(const std::vector<int>&, const std::vector<double>&, const std::vector<double>&);
    /** Save Cluster information in TRGECLCluster Table */
    void save(int);
    /**  calculate  Belle 2 Cluster in Barrel */
    void setBarrelICN(int);
    /**  calculate Belle2 Cluster in Foward endcap */
    void setForwardICN(int);
    /** calculate Belle 2 Cluster in Backward endcap */
    void setBackwardICN(int);
    /**  calculate Belle ICN in Barrel */
    int setBarrelICN();
    /**  calculate Belle  ICN in Foward endcap */
    int setForwardICN();
    /** calculate Belle ICN in Backward endcap */
    int setBackwardICN();
    /** Set EventId */
    void setEventId(int eventId) {_EventId = eventId; }
    /** Set */
    void setClusteringMethod(int method) {_Method = method; }
    /** Set Limit of Cluster*/
    void setClusterLimit(int limit) {_LimitNCluster = limit; }

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
    int getNofCluster() {return _BWDNofCluster + _FWDNofCluster + _BRNofCluster;}
    /** get # Cluster in case of exceeding limit*/
    int getNofExceedCluster();


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
    /** Cluster in Barrel */
    int _BRNofCluster;
    /** Cluster in Forward Endcap */
    int _FWDNofCluster;
    /** Cluster in Backward Endcap */
    int _BWDNofCluster;
    /** event number */
    int _EventId;
    /** Clustering method */
    int _Method;
    /** the Limit Number of Cluster */
    int _LimitNCluster;
    /** Position calculation method(0:Most energetic TC Postion,  1 : Energy weighted Postion) */
    int _Position;


    /** icn  */
    std::vector<int> _icnfwbrbw;
    /**  TC Id */
    std::vector<int> TCId;
    /**  TC timing */
    std::vector<double> Timing;
    /** TC energy  */
    std::vector<double> Energy;

    /** Quadrant for Beam Backgournd veto */
    std::vector<std::vector<int>>  _Quadrant;

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
