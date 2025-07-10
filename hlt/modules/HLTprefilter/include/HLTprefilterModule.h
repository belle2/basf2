/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/dbobjects/BunchStructure.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <rawdata/dataobjects/RawFTSW.h>
/* C++ headers. */
#include <cstdint>

namespace Belle2 {
  /**
   * Prefilter module to suppress the injection background
   */
  class HLTprefilterModule final : public Module {

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
     * True if event is inside injection background.
     * In that case, the event should be skipped from HLT processing.
     */
    void event() final;

  private:
    /// Event Meta Data Store ObjPtr
    StoreObjPtr<EventMetaData> m_eventInfo;

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

    /// Store array object for injection time info.
    StoreArray<RawFTSW> m_rawTTD;

    /// Define object for BunchStructure class
    DBObjPtr<BunchStructure> m_bunchStructure; /**< bunch structure (fill pattern) */
    /// Define object for HardwareClockSettings class
    DBObjPtr<HardwareClockSettings> m_clockSettings; /**< hardware clock settings */


    /// HLTprefilter result with timing cuts
    bool injection_strip;

  };
}

