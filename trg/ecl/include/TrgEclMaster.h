//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TrgEclMaster.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECL_FLAG_
#define TRGECL_FLAG_

#include <iostream>
#include <string>
#include <vector>
#include "trg/ecl/TrgEclCluster.h"
#include "trg/ecl/TrgEclTiming.h"
#include "trg/ecl/TrgEclBhabha.h"
#include "trg/ecl/TrgEclBeamBKG.h"
#include "trg/ecl/TrgEclMapping.h"

//
//
//
//
//
namespace Belle2 {
//
//
  /*! ETM class */
  class TrgEclMaster;
//
//
//
  class TrgEclMaster {

  public:

    /**  get pointer of TrgEclMaster object  */

    static TrgEclMaster* getTrgEclMaster(void);


    /** Constructor */
    TrgEclMaster(void);

    /** Destructor */
    virtual ~TrgEclMaster();

  public:

    /** initialize */
    void initialize(int);
    /**simulates ECL trigger for Global Cosmic data  */
    void simulate01(int);
    /**simulates ECL trigger for Data Analysis */
    void simulate02(int);

  public:

    /** returns name. */
    std::string name(void) const;

    /** returns version. */
    std::string version(void) const;
    /** ECL bit information for GDL */
    //  void simulate(int);
    /** ECL bit information for GDL */
    //int getECLtoGDL(void) { return bitECLtoGDL; }
    /** Set Phi Ring Sum  */
    void setRS(std::vector<int>, std::vector<double>, std::vector<double>&, std::vector<std::vector<double>>&);
    /** Get Event timing */
    //    void getEventTiming(int option);
    /** Set Cluster*/
    void setClusterMethod(int cluster) {_Clustering = cluster;}
    /** Set Bhabha*/
    void setBhabhaMethod(int bhabha) {_Bhabha = bhabha;}
    /** Set Cluster*/
    void setEventTimingMethod(int EventTiming) {_EventTiming = EventTiming;}
    /** Set Trigger Decision window size*/
    void setTimeWindow(int timewindow) {TimeWindow = timewindow;}
    /** Set Trigger Decision overlap window size*/
    void setOverlapWindow(int overlapwindow) {OverlapWindow = overlapwindow;}
    /** set # of considered TC in energy weighted Timing method */
    void setNofTopTC(int noftoptc) {_NofTopTC = noftoptc;}



  private:

    /** Hit TC Energy */
    std::vector<std::vector<double>>  TCEnergy;
    /** Hit TC Timing */
    std::vector<std::vector<double>>  TCTiming;
    /** Hit TC Beam Background tag */
    std::vector<std::vector<int>>  TCBeamBkgTag;

    /** Hit TC Energy in time window */
    std::vector<int>  HitTCId;
    /** Hit TC Energy in time window */
    std::vector<double>  TCHitEnergy;
    /** Hit TC Timing in time window*/
    std::vector<double>  TCHitTiming;
    /** Hit TC Beam Background tag in time window */
    std::vector<int>  TCHitBeamBkgTag;

    /**  Phi ring sum */
    std::vector< std::vector<double>>  PhiRingSum;
    /**  Theta ring sum */
    std::vector<std::vector<std::vector<double>>> ThetaRingSum;

    /** Hit TC Energy in time window */
    std::vector<double> ClusterEnergy;
    /** Hit TC Timing in time window*/
    std::vector<double>  ClusterTiming;




    /** TRG Decision Time window */
    double TimeWindow;
    /** TRG Decision overlap window */
    double OverlapWindow;


    /** clutering option*/
    int _Clustering;

    /** Bhabha option*/
    int _Bhabha;

    /** EventTiming option*/
    int _EventTiming;
    /** # of considered TC in energy weighted Timing method */
    int _NofTopTC;


    /** ecl object */
    static TrgEclMaster* _ecl;
    /** Mapping object */
    TrgEclMapping* obj_map ;
    /**  Cluster object */
    TrgEclCluster* obj_cluster;
    /**  EventTiming object */
    TrgEclTiming* obj_timing;
    /**  Bhabha object */
    TrgEclBhabha* obj_bhabha;
    /**  Beam Backgroud veto object */
    TrgEclBeamBKG* obj_beambkg;


  };
//
//
//
} /// namespace Belle2

#endif /* TRGECL_FLAG_ */
