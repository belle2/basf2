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
    /** Set the limit # of Cluster*/
    void setClusterLimit(int limit) {_ClusterLimit = limit;}
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
    /** make LowMultiTriggerBit **/
    void makeLowMultiTriggerBit(std::vector<int>, std::vector<double>);
    /** make Trigger bit except for Low Multiplicity related bit **/
    void makeTriggerBit(int, int, int, int, int, int, int, std::vector<int>, int, int, int, int, int, int, int, int);
    /** Set Total Energy*/
    double setTotalEnergy(std::vector<double>);
    int getTriggerbit(int i) {return _Triggerbit[i];}
    int getLowmultibit() {return _Lowmultibit;}
    //! set 2D Bhabha Energy Threshold
    void set2DBhabhaThreshold(std::vector<double> i2DBhabhaThresholdFWD, std::vector<double> i2DBhabhaThresholdBWD)
    {
      _2DBhabhaThresholdFWD = i2DBhabhaThresholdFWD;
      _2DBhabhaThresholdBWD = i2DBhabhaThresholdBWD;
    }
    //! set 2D Bhabha Energy Threshold
    void set3DBhabhaThreshold(std::vector<double> i3DBhabhaThreshold) { _3DBhabhaThreshold = i3DBhabhaThreshold; };

    //! set Total Energy Theshold (low, high, lum)
    void setTotalEnergyThreshold(std::vector<double>  iTotalEnergy) {_TotalEnergy = iTotalEnergy; }
    //! set Low Multiplicity Threshold
    void setLowMultiplicityThreshold(std::vector<double> iLowMultiThreshold) { _LowMultiThreshold = iLowMultiThreshold;}



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
    //   std::vector<double> ClusterEnergy;
    /** Hit TC Timing in time window*/
    //    std::vector<double>  ClusterTiming;




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
    /** The limit number of Cluster */
    int _ClusterLimit;
    // ETM bit
    int _Triggerbit[4];
    //  LowMultibit
    int _Lowmultibit;
    // Bhabha Prescale Factor
    int _PrescaleFactor;
    // Bhabha Prescale Countor
    int _PrescaleCounter;

    //! 2D Bhabha Energy Threshold
    std::vector<double> _2DBhabhaThresholdFWD;
    //! 2D Bhabha Energy Threshold
    std::vector<double> _2DBhabhaThresholdBWD;
    //! 3D Bhabha Energy Threshold
    std::vector<double> _3DBhabhaThreshold;
    //! Total Energy Theshold (low, high, lum)
    std::vector<double> _TotalEnergy;
    //! Low Multiplicity Threshold
    std::vector<double> _LowMultiThreshold;


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
    /**  Beam Backgroud veto object */
    TrgEclDataBase* obj_database;


  };
//
//
//
} /// namespace Belle2

#endif /* TRGECL_FLAG_ */
