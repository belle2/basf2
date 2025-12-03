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
#include <framework/database/DBObjPtr.h>

#include "trg/ecl/dataobjects/TRGECLHit.h"
#include "trg/ecl/dataobjects/TRGECLTrg.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include "trg/ecl/dbobjects/TRGECLETMPara.h"

#include <map>

namespace Belle2 {

  /** A module of ETM */
  class TRGECLModule : public Module {


  public:

    /** Constructor */
    TRGECLModule();

    /** Destructor  */
    virtual ~TRGECLModule();

    /** Initializes TRGECLModule.*/
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
    int m_debugLevel = 0;

    /** Config. file name.*/
    std::string m_configFilename;

    //! get payload from conditionDB
    double getDBparmap(const std::map<std::string, double>, std::string, double);

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
    //! Clustering option
    int m_Clustering = 1;
    //! Cluster Limit
    int m_ClusterLimit = 6;
    //! Eventtiming option
    int m_EventTiming = 1;
    //! Trigger decision time window
    double m_TimeWindow = 250.0;
    //! Trigger decision overlap window in order to avoid boundary effect
    double m_OverlapWindow = 125.0;
    //! # of considered TC in energy weighted Timing method
    int m_NofTopTC = 3;
    //! Event selection
    int m_SelectEvent = 1;
    //! Flag to use Condition DB
    bool m_ConditionDB = true;
    //! conversion factor of ADC to Energy in GeV
    double m_ADCtoEnergy = 0.0;
    //! Total Energy Threshold (low, high, lum) in Lab in GeV
    std::vector<double> m_TotalEnergy;
    //! 2D Bhabha Energy Threshold
    std::vector<double> m_2DBhabhaThresholdFWD;
    //! 2D Bhabha Energy Threshold
    std::vector<double> m_2DBhabhaThresholdBWD;
    //! 3D Veto Bhabha Energy Threshold
    std::vector<double> m_3DBhabhaVetoThreshold;
    //! 3D Veto Bhabha Energy Angle
    std::vector<int> m_3DBhabhaVetoAngle;
    //! 3D Selection Bhabha Energy Threshold
    std::vector<double> m_3DBhabhaSelectionThreshold;
    //! 3D Selection Bhabha Energy Angle
    std::vector<int> m_3DBhabhaSelectionAngle;
    //! 3D Selection Bhabha preselection
    std::vector<int> m_3DBhabhaSelectionPreScale;
    //! mumu bit Energy Threshold
    double m_mumuThreshold = 0.0;
    //! mumu bit  Angle
    std::vector<int> m_mumuAngle;
    //! Low Multiplicity cluster E cut in Lab in GeV
    std::vector<double> m_lmlCLELabCut;
    //! Low Multiplicity cluster E cut in CMS in GeV
    std::vector<double> m_lmlCLECMSCut;
    //! lml0 the number of cluster for minimum energy
    int m_lml00NCLforMinE = 0;
    //! lml12 the number of cluster for minimum energy
    int m_lml12NCLforMinE = 0;
    //! lml13 cluster ThetaID selection
    int m_lml13ThetaIdSelection = 0;
    //!ECL Burst Bit Threshold
    double m_ECLBurstThreshold = 0.0;
    //! Energy threshold(low, high) of event timing quality flag (GeV)
    std::vector<double> m_EventTimingQualityThreshold;
    //! Theta region (low, high) of 3D Bhabha Veto InTrack
    std::vector<int> m_3DBhabhaVetoInTrackThetaRegion;
    //! taub2b 2 cluster angle cut (degree)
    //! (dphi low, dphi high, theta_sum low, theta_sum high)
    std::vector<int> m_taub2bAngleCut;
    //! taub2b total energy (TC theta ID =1-17) (GeV)
    double m_taub2bEtotCut = 0.0;
    //! taub2b Cluster energy selection in Lab (GeV)
    double m_taub2bCLELabCut = 0.0;
    //! (hie1 and hie2) 2 cluster angle cut in CMS in degree for Bhabha veto in hie1 and hie2
    std::vector<int> m_hie12BhabhaVetoAngle;
    //! taub2b2 total energy cut in Lab (GeV)
    double m_taub2b2EtotCut = 0.0;
    //! taub2b2 two Cluster angle cut (degree)
    std::vector<int> m_taub2b2AngleCut;
    //! taub2b2 cluster energy cut(low, high) (GeV) in lab
    std::vector<double> m_taub2b2CLELabCut;
    //! taub2b3 total energy cut in lab (GeV)
    double m_taub2b3EtotCut = 0.0;
    //! taub2b3 two Cluster angle cut in cms (degree)
    std::vector<int> m_taub2b3AngleCut;
    //! taub2b3 cluster energy cut in lab for one of b2b clusters (GeV)
    double m_taub2b3CLEb2bLabCut = 0.0;
    //! taub2b3 cluster energy cut(low and high) in lab for all clusters (GeV)
    std::vector<double> m_taub2b3CLELabCut;
    //! (hie4) CL E cut for minimum energy cluster
    double m_hie4LowCLELabCut = 0.0;

    //! ETM (ecl trigger master) to generate all trigger bits of ecl trigger
    TrgEclMaster* etm;

    /** output for TRGECLHit */
    StoreArray<TRGECLHit> m_TRGECLHit;
    /** output for TRGECLTrg */
    StoreArray<TRGECLTrg> m_TRGECLTrg;
    /** output for TRGECLCluster */
    StoreArray<TRGECLCluster> m_TRGECLCluster;

  };

} // namespace Belle2
