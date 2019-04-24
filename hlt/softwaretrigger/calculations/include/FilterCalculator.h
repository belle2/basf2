/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 201 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Chris Hearty                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerVariableManager.h>
#include <hlt/softwaretrigger/calculations/SoftwareTriggerCalculation.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

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
    FilterCalculator() = default;

    /// Require the particle list. We do not need more here.
    void requireStoreArrays() override;

    /// Actually write out the variables into the map.
    void doCalculation(SoftwareTriggerObject& calculationResult) override;

  private:
    /// Store Array of the tracks to be used
    StoreArray<Track> m_tracks;
    /// Store Array of the ecl clusters to be used
    StoreArray<ECLCluster> m_eclClusters;

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
    /// which CMS energy defines nEsingleClust
    double m_EsinglePhoton = 1;
  };
}