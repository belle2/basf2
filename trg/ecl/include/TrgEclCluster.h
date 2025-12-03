/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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

    /** Assignment operator, deleted. */
    TrgEclCluster& operator=(TrgEclCluster&) = delete;

  public:

    //! initialization
    void initialize(void);
    /** set ICN for each part(Fw,Br,Bw) */
    void setICN(const std::vector<int>&);
    /** set Belle 2 Clustering */
    void setICN(const std::vector<int>&, const std::vector<double>&, const std::vector<double>&);
    /** Save Cluster information in TRGECLCluster Table */
    void save(int);
    /** calculate  Belle 2 Cluster in Barrel */
    void setBarrelICN(int);
    /** calculate 3x3 TC hit map for ICN in Barrel */
    void setBarrelICNsub(int, std::vector<int>,
                         int&, int&, int&, int&, int&, int&, int&, int&);
    /**  calculate Belle2 Cluster in Forward endcap */
    void setForwardICN(int);
    /** calculate Belle 2 Cluster in Backward endcap */
    void setBackwardICN(int);
    /**  calculate Belle ICN in Barrel */
    int setBarrelICN();
    /**  calculate Belle  ICN in Forward endcap */
    int setForwardICN();
    /** calculate Belle ICN in Backward endcap */
    int setBackwardICN();
    /** Set EventId */
    void setEventId(int eventId) { m_EventId = eventId; }
    /** Set */
    void setClusteringMethod(int method) { m_Method = method; }
    /** Set Limit of Cluster*/
    void setClusterLimit(int limit) { m_LimitNCluster = limit; }

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
    int getNofCluster() {return m_BWDNofCluster + m_FWDNofCluster + m_BRNofCluster;}
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
    int m_BRICN;
    /** ICN in Forward Endcap */
    int m_FWDICN;
    /** ICN in Backward Endcap */
    int m_BWDICN;
    /** Cluster in Barrel */
    int m_BRNofCluster;
    /** Cluster in Forward Endcap */
    int m_FWDNofCluster;
    /** Cluster in Backward Endcap */
    int m_BWDNofCluster;
    /** event number */
    int m_EventId;
    /** Clustering method */
    int m_Method;
    /** the Limit Number of Cluster */
    int m_LimitNCluster;
    /** Position calculation method(0:Most energetic TC Position,  1 : Energy weighted Position) */
    int m_Position;


    /** icn  */
    std::vector<int> m_icnfwbrbw;
    /**  TC Id */
    std::vector<int> m_TCId;
    /**  TC timing */
    std::vector<double> m_Timing;
    /** TC energy  */
    std::vector<double> m_Energy;

    /** Quadrant for Beam Background veto */
    std::vector<std::vector<int>> m_Quadrant;

    /**  cluster in barrel */
    std::vector<std::vector<int>> m_BrCluster;
    /**  cluster in forward endcap */
    std::vector<std::vector<int>> m_FwCluster;
    /**  cluster in backward endcap */
    std::vector<std::vector<int>> m_BwCluster;



    /** Cluster timing */
    std::vector<std::vector<double>> m_ClusterTiming;
    /**Cluster enrgy  */
    std::vector<std::vector<double>> m_ClusterEnergy;
    /**  Cluster position in X-axis */
    std::vector<std::vector<double>> m_ClusterPositionX;
    /** Cluster position in Y-axis */
    std::vector<std::vector<double>> m_ClusterPositionY;
    /**  Cluster position in Z-axis */
    std::vector<std::vector<double>> m_ClusterPositionZ;
    /** N of TC in Cluster  */
    std::vector<std::vector<int>> m_NofTCinCluster;
    /** Maximum contribution TC Id in Cluster */
    std::vector<std::vector<int>>  m_MaxTCId;
    /** Temporal Cluster **/
    std::vector<int>  m_TempCluster;
    /** Object of TC Mapping */
    TrgEclMapping* m_TCMap;


  };
//
//
//
} // namespace Belle2

#endif /* TRGECLCLUSTER_FLAG_ */
