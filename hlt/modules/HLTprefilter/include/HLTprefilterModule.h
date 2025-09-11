/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

/* HLT headers */
#include <hlt/dbobjects/HLTprefilterParameters.h>
#include <hlt/softwaretrigger/core/utilities.h>

/* BASF2 headers */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

/* C++ headers */
#include <cstdint>
#include <map>

namespace Belle2 {

  // Forward declarations
  class BunchStructure;
  class HardwareClockSettings;
  class TRGSummary;
  class EventLevelTriggerTimeInfo;
  class CDCHit;
  class ECLDigit;

  /**
   * Helper for TimingCut mode
   */
  class TimingCutMode {
  public:
    /// Define thresholds for variables. By default, no events are skipped based upon these requirements.
    /// Minimum threshold of timeSinceLastInjection for LER injection
    double LERtimeSinceLastInjectionMin = 0.0;
    /// Maximum threshold of timeSinceLastInjection for LER injection
    double LERtimeSinceLastInjectionMax = 0.0;
    /// Minimum threshold of timeSinceLastInjection for HER injection
    double HERtimeSinceLastInjectionMin = 0.0;
    /// Maximum threshold of timeSinceLastInjection for HER injection
    double HERtimeSinceLastInjectionMax = 0.0;
    /// Minimum threshold of timeInBeamCycle for LER injection
    double LERtimeInBeamCycleMin        = 0.0;
    /// Maximum threshold of timeInBeamCycle for LER injection
    double LERtimeInBeamCycleMax        = 0.0;
    /// Minimum threshold of timeInBeamCycle for HER injection
    double HERtimeInBeamCycleMin        = 0.0;
    /// Maximum threshold of timeInBeamCycle for LER injection
    double HERtimeInBeamCycleMax        = 0.0;
    /// Prescale for accepting HLTPrefilter lines, by default we randomly accept 1 out of every 1000 events
    unsigned int prescale = 1000;

    bool computeDecision(StoreObjPtr<EventLevelTriggerTimeInfo>* ttd,
                         DBObjPtr<BunchStructure>* bunch,
                         DBObjPtr<HardwareClockSettings>* clock) const;
  };

  /**
   * Helper for CdcEclCut mode
   */
  class CdcEclCutMode {
  public:
    /// Define thresholds for variables. By default, no events are skipped based upon these requirements.
    /// Maximum threshold for CDC Hits
    double nCDCHitsMax   = 0.0;
    /// Maximum threshold for ECL Digits
    double nECLDigitsMax = 0.0;
    /// Prescale for accepting HLTPrefilter lines, by default we randomly accept 1 out of every 1000 events
    unsigned int prescale = 1000;

    bool computeDecision() const;
  };

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
    // Mode selection
    enum HLTPrefilterMode { TimingCut = 0, CdcEclCut = 1 };

    // Helper instances
    TimingCutMode m_timingPrefilter;
    CdcEclCutMode m_cdceclPrefilter;

    // Decision results
    std::map<HLTPrefilterMode, bool> m_decisions;
    HLTPrefilterMode m_HLTprefilterMode;

    // BASF2 objects
    /// Event Meta Data Store ObjPtr
    StoreObjPtr<EventMetaData> m_eventInfo;

    /// Trigger summary
    StoreObjPtr<TRGSummary> m_trgSummary;

    /// Store array object for injection time info.
    StoreObjPtr<EventLevelTriggerTimeInfo> m_TTDInfo;

    /// Define object for BunchStructure class
    DBObjPtr<BunchStructure> m_bunchStructure; /**< bunch structure (fill pattern) */

    /// Define object for HardwareClockSettings class
    DBObjPtr<HardwareClockSettings> m_clockSettings; /**< hardware clock settings */

    /// CDChits StoreArray
    StoreArray<CDCHit> m_cdcHits;

    /// ECLDigits StoreArray
    StoreArray<ECLDigit> m_eclDigits;

    /// HLTprefilterParameters Database OjbPtr
    DBObjPtr<HLTprefilterParameters> m_hltPrefilterParameters; /**< HLT prefilter parameters */

  };
}

