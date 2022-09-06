/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBArray.h>

#include "trg/ecl/dataobjects/TRGECLHit.h"
#include "trg/ecl/dataobjects/TRGECLTrg.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include "trg/ecl/dbobjects/TRGECLETMPara.h"

namespace Belle2 {

  /** A module of ETM */
  class TRGECLModule : public Module {


  public:

    /** Constructor */
    TRGECLModule();

    /** Destructor  */
    virtual ~TRGECLModule();

    /** Initilizes TRGECLModule.*/
    virtual void initialize() override;

    /** Called when new run started.*/
    virtual void beginRun() override;

    /** Called event by event.*/
    virtual void event() override;

    /** Called when run ended.*/
    virtual void endRun() override;

    /** Called when processing ended.*/
    virtual void terminate() override;

  public:

    /** returns version of TRGECLModule.*/
    std::string version(void) const;

  private: //! Parameters

    /** Debug level.*/
    int _debugLevel;

    /** Config. file name.*/
    std::string _configFilename;

    //! A pointer to a TRGECL
    //!    TrgEcl* _ecl; */

  protected:
    //! Input array name.
    std::string m_inColName;
    //! Output array name for Xtal
    std::string m_eclHitOutColName;
    //! Output array name for TC
    std::string m_eclTCHitOutColName;

    //! The current number of created hits in an event.
    //! Used to fill the DataStore ECL array.
    int m_hitNum = 0;
    //! TC Hit number
    int m_hitTCNum = 0;

  private:

    // double m_timeCPU;        //  CPU time
    //!  Run number
    int m_nRun = 0;
    //!  Event number
    int m_nEvent = 0;
    //! Bhabha option
    int _Bhabha;
    //! Clustering option
    int _Clustering;
    //! Cluster Limit
    int _ClusterLimit;
    //! Eventtiming option
    int _EventTiming;
    //! Trigger decision time window
    double _TimeWindow;
    //! Trigger decision overlap window in oder to avoid boundary effect
    double _OverlapWindow;
    //! # of considered TC in energy weighted Timing method
    int _NofTopTC;
    //! Event selection
    int _SelectEvent;
    //! Flag to use Condition DB
    int _ConditionDB;
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
    //! Angle selection of additional Bhabha veto in CM frame
    std::vector<double> m_3DBhabhaAddAngleCut;
    //! The number of Cluster exceeding 300 MeV
    int _n300MeVCluster;
    //!ECL Burst Bit Threshold
    double _ECLBurstThreshold;
    //! Total Energy Theshold (low, high, lum)
    std::vector<double> _TotalEnergy;
    //! Low Multiplicity Threshold
    std::vector<double> _LowMultiThreshold;
    //! Theta region (low, high) of 3D Bhabha Veto InTrack
    std::vector<int> m_3DBhabhaVetoInTrackThetaRegion;
    //! Energy threshold(low, high) of event timing quality flag (GeV)
    std::vector<double> m_EventTimingQualityThresholds;
    //! taub2b 2 cluster angle cut (degree)
    //! (dphi low, dphi high, theta_sum low, theta_sum high)
    std::vector<int> m_taub2bAngleCut;
    //! taub2b total energy (TC theta ID =1-17) (GeV)
    double m_taub2bEtotCut;
    //! taub2b Cluster energy selection (GeV)
    double m_taub2bClusterECut1;
    //! taub2b Cluster energy selection (GeV)
    double m_taub2bClusterECut2;
    //! taub2b2 total energy cut (GeV)
    double m_taub2b2EtotCut;
    //! taub2b2 two Cluster angle cut (degree)
    std::vector<int> m_taub2b2AngleCut;
    //! taub2b2 cluster energy cut for endcap cluster (GeV)
    double m_taub2b2CLEEndcapCut;
    //! taub2b2 cluster energy cut (GeV)
    double m_taub2b2CLECut;
    //! taub2b3 total energy cut in lab (GeV)
    double m_taub2b3EtotCut;
    //! taub2b3 two Cluster angle cut in cms (degree)
    std::vector<int> m_taub2b3AngleCut;
    //! taub2b3 cluster energy cut in lab for one of b2b clusters (GeV)
    double m_taub2b3CLEb2bCut;
    //! taub2b3 cluster energy low cut in lab for all clusters (GeV)
    double m_taub2b3CLELowCut;
    //! taub2b3 cluster energy high cut in lab for all clusters (GeV)
    double m_taub2b3CLEHighCut;
    /** output for TRGECLHit */
    StoreArray<TRGECLHit> m_TRGECLHit;
    /** output for TRGECLTrg */
    StoreArray<TRGECLTrg> m_TRGECLTrg;
    /** output for TRGECLCluster */
    StoreArray<TRGECLCluster> m_TRGECLCluster;
    /** ETN Parameters */
    DBArray<TRGECLETMPara> m_ETMPara;

  };

} // namespace Belle2
