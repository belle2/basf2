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
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TRGSummary.h>
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
  };
}
