/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <hlt/dbobjects/HLTprefilterParameters.h>
#include <framework/dbobjects/BunchStructure.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
/* C++ headers. */
#include <cstdint>

namespace Belle2 {
  /**
   * Prefilter module to suppress the injection background
   */
  class HLTprefilterModule : public Module {

  public:

    /// Module constructor.
    HLTprefilterModule();

    /// Default Destructor.
    virtual ~HLTprefilterModule() final;

    /// Module initializer.
    void initialize() final;

    /// Called when entering a new run.
    void beginRun() final;

    /**
     * Flag each event.
     * True if event exceeds `nCDCHitsMax or nECLDigitsMax`.
     * In that case, the event should be skipped for reco.
     */
    void event() final;

  private:
    /// Event Meta Data Store ObjPtr
    StoreObjPtr<EventMetaData> m_eventInfo;

    /// Store array for injection time info.
    StoreObjPtr<EventLevelTriggerTimeInfo> m_TTDInfo;

    /// Define global constants for timing variables
    /// Define object for BunchStructure class
    Belle2::BunchStructure bunchStructure;
    /// Define object for HardwareClockSettings class
    Belle2::HardwareClockSettings clockSettings;
    /// Define the c_revolutionTime
    double m_revolutionTime = bunchStructure.getRFBucketsPerRevolution() / (clockSettings.getAcceleratorRF() * 1e3);
    /// Define the c_globalClock
    double m_globalClock = clockSettings.getGlobalClockFrequency() * 1e3;

    /// Define thresholds for variables. By default, no events are skipped based upon these requirements. (Set everything to zero by default)
    /// Minimum threshold of timeSinceLastInjection for LER injection
    double m_LERtimeSinceLastInjectionMin = 5000;
    /// Maximum threshold of timeSinceLastInjection for LER injection
    double m_LERtimeSinceLastInjectionMax = 20000;
    /// Minimum threshold of timeSinceLastInjection for HER injection
    double m_HERtimeSinceLastInjectionMin = 600;
    /// Maximum threshold of timeSinceLastInjection for HER injection
    double m_HERtimeSinceLastInjectionMax = 20000;
    /// Minimum threshold of timeInBeamCycle for LER injection
    double m_LERtimeInBeamCycleMin = 1.25;
    /// Maximum threshold of timeInBeamCycle for LER injection
    double m_LERtimeInBeamCycleMax = 1.55;
    /// Minimum threshold of timeInBeamCycle for HER injection
    double m_HERtimeInBeamCycleMin = 2.2;
    /// Maximum threshold of timeInBeamCycle for HER injection
    double m_HERtimeInBeamCycleMax = 2.33;

    /// Trigger summary
    StoreObjPtr<TRGSummary> m_trgSummary;

    /// HLTprefilter result with timing cuts
    bool injection_strip;

  };
}

