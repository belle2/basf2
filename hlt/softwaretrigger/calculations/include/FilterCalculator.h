/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerObject.h>
#include <hlt/softwaretrigger/calculations/SoftwareTriggerCalculation.h>
//#include <hlt/dbobjects/HLTprefilterParameters.h>
#include <framework/dbobjects/BunchStructure.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
#include <cdc/dataobjects/CDCHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <trg/cdc/Unpacker.h>
#include <framework/gearbox/Unit.h>

namespace Belle2::SoftwareTrigger {
  /**
   * Implementation of a calculator used in the SoftwareTriggerModule
   * to fill a SoftwareTriggerObject for doing HLT cuts.
   *
   * This calculator exports variables needed for the trigger HLT part
   * of the path ( = filtering out events)
   *
   * This class implements the two main functions requireStoreArrays and doCalculation of the
   * SoftwareTriggerCalculation class.
   */
  class FilterCalculator : public SoftwareTriggerCalculation {
  public:
    /// Set the default names for the store object particle lists.
    FilterCalculator();

    /// Require the particle list. We do not need more here.
    void requireStoreArrays() override;

    /// Actually write out the variables into the map.
    void doCalculation(SoftwareTriggerObject& calculationResult) override;

  private:
    /// Store Array of the tracks to be used
    StoreArray<Track> m_tracks;
    /// Store Array of the ecl clusters to be used
    StoreArray<ECLCluster> m_eclClusters;
    /// Store Object with the trigger result
    StoreObjPtr<TRGSummary> m_l1Trigger;
    /// Store Object with the trigger NN bits
    StoreArray<CDCTriggerUnpacker::NNBitStream> m_bitsNN;

    /// which Z0 defines a loose track
    double m_looseTrkZ0 = 10 * Unit::cm;
    /// which Z0 defines a tight track
    double m_tightTrkZ0 = 2 * Unit::cm;
    /// which CMS energy defines nElow
    double m_E2min = 0.2;
    /// which CMS energy defines nEmedium
    double m_E0min = 0.3;
    /// which CMS energy defines nEhigh
    double m_Ehigh = 2;
    /// which lab energy defines nE180Lab
    double m_EminLab = 0.18;
    /// which lab energy defines nE300Lab
    double m_EminLab4Cluster = 0.3;
    /// which lab energy defines nE500Lab
    double m_EminLab3Cluster = 0.5;
    /// which CMS energy defines nEsingleClust
    double m_EsinglePhoton = 1;
    /// which CMS energy defines nReducedEsingle clusters
    double m_reducedEsinglePhoton = 0.5;
    /// which LAB pt defines a cosmic
    double m_cosmicMinPt = 0.5 * Unit::GeV;
    /// which LAB cluster energy vetoes a cosmic candidate
    double m_cosmicMaxClusterEnergy = 1.0 * Unit::GeV;
    /// maximum z0 for well understood magnetic field (cm)
    double m_goodMagneticRegionZ0 = 57.;
    /// minimum d0 for well understood magnetic field, if z0 is large (cm)
    double m_goodMagneticRegionD0 = 26.5;

    /// Objects relevant to HLTprefilter monitoring
    /// Store array for injection time info.
    StoreObjPtr<EventLevelTriggerTimeInfo> m_TTDInfo;
    /// Define object for BunchStructure class
    DBObjPtr<BunchStructure> m_bunchStructure; /**< bunch structure (fill pattern) */
    /// Define object for HardwareClockSettings class
    DBObjPtr<HardwareClockSettings> m_clockSettings; /**< hardware clock settings */
    /// Trigger summary
    StoreObjPtr<TRGSummary> m_trgSummary;

    /// CDChits StoreArray
    StoreArray<CDCHit> m_cdcHits;

    /// ECLDigits StoreArray
    StoreArray<ECLDigit> m_eclDigits;

    /// HLTprefilterParameters Database OjbPtr
    /*DBObjPtr<HLTprefilterParameters> m_hltPrefilterParameters;*/ /**< HLT prefilter parameters */

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

    /// Define thresholds for variables. By default, no events are skipped based upon these requirements.
    /// Maximum threshold for CDC Hits
    double m_cdcHitsMax = 1e9;
    /// Maximum threshold for ECL digits
    double m_eclDigitsMax = 1e9;



  };
}
