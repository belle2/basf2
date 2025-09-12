/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

/* HLT headers */
#include <hlt/dbobjects/HLTPrefilterParameters.h>
#include <hlt/softwaretrigger/core/utilities.h>

/* BASF2 headers */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dbobjects/BunchStructure.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
#include <cdc/dataobjects/CDCHit.h>
#include <ecl/dataobjects/ECLDigit.h>

/* C++ headers */
#include <cstdint>
#include <map>

namespace Belle2 {

  class TRGSummary;

  /**
   * Helper for TimingCut mode
   */
  class TimingCutMode {
  private:

    /// Store array object for injection time info.
    StoreObjPtr<EventLevelTriggerTimeInfo> m_TTDInfo;

    /// Define object for BunchStructure class
    DBObjPtr<BunchStructure> m_bunchStructure; /**< bunch structure (fill pattern) */

    /// Define object for HardwareClockSettings class
    DBObjPtr<HardwareClockSettings> m_clockSettings; /**< hardware clock settings */

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

    bool computeDecision()
    {
      if (m_TTDInfo.isValid()) {
        /// Calculate revolution time of beam
        double c_revolutionTime = m_bunchStructure->getRFBucketsPerRevolution() * 1e-3 /
                                  m_clockSettings->getAcceleratorRF(); // [microsecond]
        /// Fetch global clock
        double c_globalClock = m_clockSettings->getGlobalClockFrequency() * 1e3; // [microsecond]
        // Calculate time since last injection
        double timeSinceLastInj = m_TTDInfo->getTimeSinceLastInjection() / c_globalClock; // [microsecond]
        // Calculate time in beam cycle
        double timeInBeamCycle = timeSinceLastInj - (int)(timeSinceLastInj / c_revolutionTime) * c_revolutionTime; // [microsecond]


        // Check if events are in injection strip of LER
        bool LER_strip = (LERtimeSinceLastInjectionMin < timeSinceLastInj &&
                          timeSinceLastInj < LERtimeSinceLastInjectionMax &&
                          LERtimeInBeamCycleMin < timeInBeamCycle &&
                          timeInBeamCycle < LERtimeInBeamCycleMax);

        // Check if events are in injection strip of HER
        bool HER_strip = (HERtimeSinceLastInjectionMin < timeSinceLastInj &&
                          timeSinceLastInj < HERtimeSinceLastInjectionMax &&
                          HERtimeInBeamCycleMin < timeInBeamCycle &&
                          timeInBeamCycle < HERtimeInBeamCycleMax);

        // Tag events inside injection strip with a prescale
        return (LER_strip || HER_strip) &&
               !Belle2::SoftwareTrigger::makePreScale(prescale);
      } else
        return false;
    }
  };

  /**
   * Helper for CdcEclCut mode
   */
  class CdcEclCutMode {
  private:

    /// CDChits StoreArray
    StoreArray<CDCHit> m_cdcHits;

    /// ECLDigits StoreArray
    StoreArray<ECLDigit> m_eclDigits;

  public:
    /// Define thresholds for variables. By default, no events are skipped based upon these requirements.
    /// Maximum threshold for CDC Hits
    uint32_t nCDCHitsMax   = 0.0;
    /// Maximum threshold for ECL Digits
    uint32_t nECLDigitsMax = 0.0;
    /// Prescale for accepting HLTPrefilter lines, by default we randomly accept 1 out of every 1000 events
    unsigned int prescale = 1000;

    bool computeDecision()
    {
      /// Get NCDCHits for the event
      const uint32_t c_NcdcHits = m_cdcHits.isOptional() ? m_cdcHits.getEntries() : 0;
      /// Get NECLDigits for the event
      const uint32_t c_NeclDigits = m_eclDigits.isOptional() ? m_eclDigits.getEntries() : 0;

      return (c_NcdcHits > nCDCHitsMax && c_NeclDigits > nECLDigitsMax) &&
             !Belle2::SoftwareTrigger::makePreScale(prescale);
    }

  };

  /**
   * Prefilter module to suppress the injection background
   */
  class HLTPrefilterModule final : public Module {

  public:
    /// Module constructor.
    HLTPrefilterModule();

    /// Default Destructor.
    virtual ~HLTPrefilterModule() final;

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

    // Helper instance for timing based prefilter
    TimingCutMode m_timingPrefilter;

    // Helper instance for CDC-ECL occupancy based prefilter
    CdcEclCutMode m_cdceclPrefilter;

    // Decision results
    std::map<HLTPrefilterMode, bool> m_decisions;

    // Instance for prefilter mode
    HLTPrefilterMode m_HLTPrefilterMode;

    // BASF2 objects
    /// Event Meta Data Store ObjPtr
    StoreObjPtr<EventMetaData> m_eventInfo;

    /// Trigger summary
    StoreObjPtr<TRGSummary> m_trgSummary;

    /// HLTprefilterParameters Database OjbPtr
    DBObjPtr<HLTPrefilterParameters> m_hltPrefilterParameters;

  };
}

