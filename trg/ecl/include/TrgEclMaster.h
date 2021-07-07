/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>

#include <framework/core/Module.h>

#include "trg/ecl/TrgEclCluster.h"
#include "trg/ecl/TrgEclTiming.h"
#include "trg/ecl/TrgEclBhabha.h"
#include "trg/ecl/TrgEclBeamBKG.h"
#include "trg/ecl/TrgEclMapping.h"
//
//
//
namespace Belle2 {
//
//
//
  /** ETM class */
  class TrgEclMaster {

  public:

    //!  get pointer of TrgEclMaster object
    static TrgEclMaster* getTrgEclMaster(void);

    //! TrgEclMaster Constructor
    TrgEclMaster(void);

    //! TrgEclMaster Destructor
    virtual ~TrgEclMaster();

    //! Copy constructor, deleted
    TrgEclMaster(TrgEclMaster&) = delete;

    //! Assignment operator, deleted
    TrgEclMaster& operator=(TrgEclMaster&) = delete;

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
    void setRS(std::vector<int>, std::vector<double>,
               std::vector<double>&, std::vector<std::vector<double>>&);
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
    void makeTriggerBit(int, int, int, int, double, int, int,
                        std::vector<int>, int, int, int, int,
                        int, int, int, int, int, int, int, int, int);
    /** Set Total Energy*/
    double setTotalEnergy(std::vector<double>);
    //! Get ECL Trigger bit
    int getTriggerbit(int i) {return _Triggerbit[i];}
    //! Get Low Multiplicity Trigger Bit
    int getLowmultibit() {return _Lowmultibit;}
    //! set 2D Bhabha Energy Threshold
    void set2DBhabhaThreshold(const std::vector<double>& i2DBhabhaThresholdFWD,
                              const std::vector<double>& i2DBhabhaThresholdBWD)
    {
      _2DBhabhaThresholdFWD = i2DBhabhaThresholdFWD;
      _2DBhabhaThresholdBWD = i2DBhabhaThresholdBWD;
    }
    //! set 3D selection Bhabha Energy Threshold
    void set3DBhabhaSelectionThreshold(const std::vector<double>& i3DBhabhaSelectionThreshold)
    {
      _3DBhabhaSelectionThreshold = i3DBhabhaSelectionThreshold;
    };
    //! set 3D veto Bhabha Energy Threshold
    void set3DBhabhaVetoThreshold(const std::vector<double>& i3DBhabhaVetoThreshold)
    {
      _3DBhabhaVetoThreshold = i3DBhabhaVetoThreshold;
    };

    //! set 3D selection Bhabha Energy Angle
    void set3DBhabhaSelectionAngle(const std::vector<double>& i3DBhabhaSelectionAngle)
    {
      _3DBhabhaSelectionAngle = i3DBhabhaSelectionAngle;
    };
    //! set 3D veto Bhabha Energy Angle
    void set3DBhabhaVetoAngle(const std::vector<double>& i3DBhabhaVetoAngle)
    {
      _3DBhabhaVetoAngle = i3DBhabhaVetoAngle;
    };
    //! set mumu bit Threshold
    void setmumuThreshold(int mumuThreshold) {_mumuThreshold = mumuThreshold; }
    //! set mumu bit Angle selection
    void setmumuAngle(const std::vector<double>& imumuAngle)
    {
      _mumuAngle = imumuAngle;
    }
    //! set tau b2b 2 cluster angle cut
    void setTaub2bAngleCut(const std::vector<int>& itaub2bAngleCut)
    {
      m_taub2bAngleCut = itaub2bAngleCut;
    }
    //! set tau b2b total energy cut
    void setTaub2bEtotCut(double itaub2bEtotCut)
    {
      m_taub2bEtotCut = itaub2bEtotCut;
    }
    //! set tau b2b  1Cluster energy cut
    void setTaub2bClusterECut(double itaub2bClusterECut1,
                              double itaub2bClusterECut2)
    {
      m_taub2bClusterECut1 = itaub2bClusterECut1;
      m_taub2bClusterECut2 = itaub2bClusterECut2;
    }
    //! set the number of cluster exceeding 300 MeV
    void setn300MeVClusterThreshold(int n300MeVCluster)
    {
      _n300MeVCluster = n300MeVCluster;
    }
    //! set mumu bit Threshold
    void setECLBurstThreshold(int ECLBurstThreshold)
    {
      _ECLBurstThreshold = ECLBurstThreshold;
    }
    //! set Total Energy Theshold (low, high, lum)
    void setTotalEnergyThreshold(const std::vector<double>& iTotalEnergy)
    {
      _TotalEnergy = iTotalEnergy;
    }
    //! set Low Multiplicity Threshold
    void setLowMultiplicityThreshold(const std::vector<double>& iLowMultiThreshold)
    {
      _LowMultiThreshold = iLowMultiThreshold;
    }
    // set theta ID region (low and high) of 3DBhabhaVeto InTrack for gg selection
    void set3DBhabhaVetoInTrackThetaRegion(const std::vector<int>& i3DBhabhaVetoInTrackThetaRegion)
    {
      m_3DBhabhaVetoInTrackThetaRegion = i3DBhabhaVetoInTrackThetaRegion;
    }
    // set energy threshold(low and high) of event timing quality flag (GeV)
    void setEventTimingQualityThresholds(const std::vector<double>& iEventTimingQualityThresholds)
    {
      m_EventTimingQualityThresholds = iEventTimingQualityThresholds;
    }

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
    //! ECL Trigger  bit
    int _Triggerbit[4];
    //!  Low Multiplicity bit
    int _Lowmultibit;
    //!  Bhabha Prescale Factor
    int _PrescaleFactor;
    //! Bhabha Prescale Countor
    int _PrescaleCounter;

    //! 2D Bhabha Energy Threshold
    std::vector<double> _2DBhabhaThresholdFWD;
    //! 2D Bhabha Energy Threshold
    std::vector<double> _2DBhabhaThresholdBWD;
    //! 3D Selection Bhabha Energy Threshold
    std::vector<double> _3DBhabhaSelectionThreshold;
    //! 3D Veto Bhabha Energy Threshold
    std::vector<double> _3DBhabhaVetoThreshold;
    //! 3D Selection Bhabha Energy Angle
    std::vector<double> _3DBhabhaSelectionAngle;
    //! 3D Veto Bhabha Energy Angle
    std::vector<double> _3DBhabhaVetoAngle;
    //! mumu bit Energy Threshold
    double _mumuThreshold;
    //! mumu bit  Angle
    std::vector<double> _mumuAngle;
    //! tau b2b 2 cluster angle cut (degree)
    // (dphi low, dphi high, theta_sum low, theta_sum high)
    std::vector<int> m_taub2bAngleCut;
    //! tau b2b total energy (TC theta ID =1-17) (GeV)
    double m_taub2bEtotCut;
    //! tau b2b 1 Cluster energy selection (GeV)
    double m_taub2bClusterECut1;
    double m_taub2bClusterECut2;
    //! The number of Cluster exceeding 300 MeV
    int _n300MeVCluster;
    //!ECL Burst Bit Threshold
    double _ECLBurstThreshold;
    //! Total Energy Theshold (low, high, lum)
    std::vector<double> _TotalEnergy;
    //! Low Multiplicity Threshold
    std::vector<double> _LowMultiThreshold;
    // Theta region (low, high) of 3D Bhabha Veto InTrack
    std::vector<int> m_3DBhabhaVetoInTrackThetaRegion;
    // energy threshold(low, high) for quality flag (GeV)
    std::vector<double> m_EventTimingQualityThresholds;

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
