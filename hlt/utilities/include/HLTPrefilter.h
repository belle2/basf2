/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

/* HLT headers */
#include <hlt/softwaretrigger/core/utilities.h>
#include <hlt/dbobjects/HLTPrefilterParameters.h>

/* basf2 headers */
#include <svd/dataobjects/SVDShaperDigit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dbobjects/BunchStructure.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <reconstruction/dbobjects/EventsOfDoomParameters.h>

/* C++ headers */
#include <cstdint>

namespace Belle2::HLTPrefilter {

  /**
   * Helper for TimingCut state
   */
  class TimingCutState {
  private:

    /// Store Object with the trigger result
    StoreObjPtr<TRGSummary> m_l1Trigger;

    /// HLTprefilterParameters Database OjbPtr
    DBObjPtr<HLTPrefilterParameters> m_hltPrefilterParameters; /**< HLT prefilter parameters */

    /// Store array object for injection time info.
    StoreObjPtr<EventLevelTriggerTimeInfo> m_TTDInfo;

    /// Define object for BunchStructure class
    DBObjPtr<BunchStructure> m_bunchStructure; /**< bunch structure (fill pattern) */

    /// Define object for HardwareClockSettings class
    DBObjPtr<HardwareClockSettings> m_clockSettings; /**< hardware clock settings */

  public:

    bool computeDecision()
    {
      if (m_TTDInfo.isValid() && m_hltPrefilterParameters.isValid() && m_l1Trigger.isValid()) {

        // Injection Timing mode thresholds
        /// Minimum threshold of timeSinceLastInjection for LER injection
        const double LERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMin();
        /// Maximum threshold of timeSinceLastInjection for LER injection
        const double LERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMax();
        /// Minimum threshold of timeSinceLastInjection for HER injection
        const double HERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMin();
        /// Maximum threshold of timeSinceLastInjection for HER injection
        const double HERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMax();
        /// Minimum threshold of timeInBeamCycle for LER injection
        const double LERtimeInBeamCycleMin        = m_hltPrefilterParameters->getLERtimeInBeamCycleMin();
        /// Maximum threshold of timeInBeamCycle for LER injection
        const double LERtimeInBeamCycleMax        = m_hltPrefilterParameters->getLERtimeInBeamCycleMax();
        /// Minimum threshold of timeInBeamCycle for HER injection
        const double HERtimeInBeamCycleMin        = m_hltPrefilterParameters->getHERtimeInBeamCycleMin();
        /// Maximum threshold of timeInBeamCycle for LER injection
        const double HERtimeInBeamCycleMax        = m_hltPrefilterParameters->getHERtimeInBeamCycleMax();

        /// Calculate revolution time of beam
        const double revolutionTime = m_bunchStructure->getRFBucketsPerRevolution() * 1e-3 /
                                      m_clockSettings->getAcceleratorRF(); // [microsecond]
        /// Fetch global clock
        const double globalClock = m_clockSettings->getGlobalClockFrequency() * 1e3; // [microsecond]
        // Calculate time since last injection
        const double timeSinceLastInj = m_TTDInfo->getTimeSinceLastInjection() / globalClock; // [microsecond]
        // Calculate time in beam cycle
        const double timeInBeamCycle = timeSinceLastInj - (int)(timeSinceLastInj / revolutionTime) * revolutionTime; // [microsecond]

        // Check if events are in injection strip of LER
        const bool LER_strip = (LERtimeSinceLastInjectionMin < timeSinceLastInj &&
                                timeSinceLastInj < LERtimeSinceLastInjectionMax &&
                                LERtimeInBeamCycleMin < timeInBeamCycle &&
                                timeInBeamCycle < LERtimeInBeamCycleMax);

        // Check if events are in injection strip of HER
        const bool HER_strip = (HERtimeSinceLastInjectionMin < timeSinceLastInj &&
                                timeSinceLastInj < HERtimeSinceLastInjectionMax &&
                                HERtimeInBeamCycleMin < timeInBeamCycle &&
                                timeInBeamCycle < HERtimeInBeamCycleMax);

        //find out if we are in the passive veto or in the active veto window
        bool inActiveInjectionVeto = false; //events accepted in the passive veto window but not in the active
        try {
          if (m_l1Trigger->testInput("passive_veto") == 1 &&  m_l1Trigger->testInput("cdcecl_veto") == 0)
            inActiveInjectionVeto = true; //events in active veto
        } catch (const std::exception&) {
        }

        if (inActiveInjectionVeto && (LER_strip || HER_strip))
          B2WARNING("Skip event if HLTPrefilter On --> Event tagged by HLTPrefilter as injection background");

        // Tag events from active veto inside injection strip with a prescale
        return inActiveInjectionVeto && (LER_strip || HER_strip);
      } else
        return false;
    }
  };

  /**
   * Helper for CDCECLCut state
   */
  class CDCECLCutState {
  private:

    /// Store Object with the trigger result
    StoreObjPtr<TRGSummary> m_l1Trigger;

    /// HLTprefilterParameters Database OjbPtr
    DBObjPtr<HLTPrefilterParameters> m_hltPrefilterParameters; /**< HLT prefilter parameters */

    /// CDChits StoreArray
    StoreArray<CDCHit> m_cdcHits;

    /// ECLDigits StoreArray
    StoreArray<ECLDigit> m_eclDigits;

  public:

    bool computeDecision()
    {
      if (m_hltPrefilterParameters.isValid() && m_l1Trigger.isValid()) {

        // CDC-ECL mode thresholds
        /// Maximum threshold for CDC Hits
        const uint32_t nCDCHitsMax = m_hltPrefilterParameters->getCDCHitsMax();
        /// Maximum threshold for ECL Digits
        const uint32_t nECLDigitsMax = m_hltPrefilterParameters->getECLDigitsMax();

        /// Get NCDCHits for the event
        const uint32_t nCDCHits = m_cdcHits.isOptional() ? m_cdcHits.getEntries() : 0;
        /// Get NECLDigits for the event
        const uint32_t nECLDigits = m_eclDigits.isOptional() ? m_eclDigits.getEntries() : 0;

        //find out if we are in the passive veto or in the active veto window
        bool inActiveInjectionVeto = false; //events accepted in the passive veto window but not in the active
        try {
          if (m_l1Trigger->testInput("passive_veto") == 1 &&  m_l1Trigger->testInput("cdcecl_veto") == 0)
            inActiveInjectionVeto = true; //events in active veto
        } catch (const std::exception&) {
        }

        if (inActiveInjectionVeto && (nCDCHits > nCDCHitsMax && nECLDigits > nECLDigitsMax))
          B2WARNING("Skip event if HLTPrefilter On --> Event tagged by HLTPrefilter as high CDC-ECL occupancy");

        // Tag events having a large CDC and ECL occupancy with a prescale
        return inActiveInjectionVeto && (nCDCHits > nCDCHitsMax && nECLDigits > nECLDigitsMax);
      } else
        return false;
    }

  };

  /**
  * Helper for EventsOfDoomBuster state
  */
  class EventsOfDoomBusterState {
  private:
    /// CDCHits StoreArray
    StoreArray<CDCHit> m_cdcHits;
    /// SVDShaperDigits StoreArray
    StoreArray<SVDShaperDigit> m_svdShaperDigits;
    /// EventsOfDoomParameters Database OjbPtr
    DBObjPtr<EventsOfDoomParameters> m_eventsOfDoomParameters;

  public:

    bool computeDecision()
    {
      if (m_eventsOfDoomParameters.isValid()) {

        // EventsOfDoomBuster mode thresholds
        /// The max number of CDC hits for an event to be kept for reconstruction
        const uint32_t nCDCHitsMax = m_eventsOfDoomParameters->getNCDCHitsMax();
        /// The max number of SVD shaper digits for an event to be kept for reconstruction
        const uint32_t nSVDShaperDigitsMax = m_eventsOfDoomParameters->getNSVDShaperDigitsMax();

        /// Get NCDCHits for the event
        const uint32_t nCDCHits = m_cdcHits.isOptional() ? m_cdcHits.getEntries() : 0;
        /// Get NSVDShaperDigits for the event
        const uint32_t nSVDShaperDigits = m_svdShaperDigits.isOptional() ? m_svdShaperDigits.getEntries() : 0;

        const bool doomCDC = nCDCHits > nCDCHitsMax;
        const bool doomSVD = nSVDShaperDigits > nSVDShaperDigitsMax;

        if (doomCDC) {
          B2ERROR("Skip event --> Too much occupancy from CDC for reconstruction!" <<
                  LogVar("nCDCHits", nCDCHits) <<
                  LogVar("nCDCHitsMax", nCDCHitsMax));
        }

        if (doomSVD) {
          B2ERROR("Skip event --> Too much occupancy from SVD for reconstruction!" <<
                  LogVar("nSVDShaperDigits", nSVDShaperDigits) <<
                  LogVar("nSVDShaperDigitsMax", nSVDShaperDigitsMax));
        }

        // Tag events having a large SVD or CDC occupancy
        return (doomCDC || doomSVD);
      } else
        return false;
    }

  };

}
