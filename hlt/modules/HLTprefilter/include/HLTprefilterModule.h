/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
/* HLT headers. */
//#include <hlt/dbobjects/HLTprefilterParameters.h>

/* Basf2 headers. */
#include <framework/dbobjects/BunchStructure.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
#include <cdc/dataobjects/CDCHit.h>
#include <ecl/dataobjects/ECLDigit.h>
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

    /// Trigger summary
    StoreObjPtr<TRGSummary> m_trgSummary;

    /// Store array object for injection time info.
    StoreObjPtr<EventLevelTriggerTimeInfo> m_TTDInfo;

    /// Define object for BunchStructure class
    DBObjPtr<BunchStructure> m_bunchStructure; /**< bunch structure (fill pattern) */
    /// Define object for HardwareClockSettings class
    DBObjPtr<HardwareClockSettings> m_clockSettings; /**< hardware clock settings */

    /// Define thresholds for variables. By default, no events are skipped based upon these requirements. (Set everything to zero by default)
    /// Minimum threshold of timeSinceLastInjection for LER injection
    double m_LERtimeSinceLastInjectionMin = 0;
    /// Maximum threshold of timeSinceLastInjection for LER injection
    double m_LERtimeSinceLastInjectionMax = 0;
    /// Minimum threshold of timeSinceLastInjection for HER injection
    double m_HERtimeSinceLastInjectionMin = 0;
    /// Maximum threshold of timeSinceLastInjection for HER injection
    double m_HERtimeSinceLastInjectionMax = 0;
    /// Minimum threshold of timeInBeamCycle for LER injection
    double m_LERtimeInBeamCycleMin = 0;
    /// Maximum threshold of timeInBeamCycle for LER injection
    double m_LERtimeInBeamCycleMax = 0;
    /// Minimum threshold of timeInBeamCycle for HER injection
    double m_HERtimeInBeamCycleMin = 0;
    /// Maximum threshold of timeInBeamCycle for HER injection
    double m_HERtimeInBeamCycleMax = 0;

    /// HLTprefilter result with timing cuts
    bool injection_strip; /** < HLT prefilter decision for injection strip */

    /// CDChits StoreArray
    StoreArray<CDCHit> m_cdcHits;

    /// ECLDigits StoreArray
    StoreArray<ECLDigit> m_eclDigits;

    /// Define thresholds for variables. By default, no events are skipped based upon these requirements.
    /// Maximum threshold for CDC Hits
    double m_cdcHitsMax = 1e9;
    /// Maximum threshold for ECL digits
    double m_eclDigitsMax = 1e9;

    /// HLTprefilter result with CDC-ECL cuts
    bool cdcecl_threshold; /** < HLT prefilter decision using CDC-ECL cuts */

    /// HLTprefilter mode
    uint32_t m_HLTprefilterMode = 0; /** == 0 for timing cuts, == 1 for CDC-ECL cut */

    /// HLTprefilterParameters Database OjbPtr
    /*DBObjPtr<HLTprefilterParameters> m_hltPrefilterParameters;*/ /**< HLT prefilter parameters */


  };
}

