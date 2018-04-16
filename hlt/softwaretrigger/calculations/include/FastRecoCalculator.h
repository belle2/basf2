/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerVariableManager.h>
#include <hlt/softwaretrigger/calculations/SoftwareTriggerCalculation.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/datastore/StoreArray.h>

#include <vector>
#include <functional>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Implementation of a calculator used in the SoftwareTriggerModule
     * to fill a SoftwareTriggerObject for doing FastReco cuts.
     *
     * This calculator exports variables needed for the FastReco part
     * of the path, e.g.
     * * visible_energy
     * * highest_1_ecl
     * * max_pt
     * * max_pz
     * * first_highest_cdc_energies
     *
     * This class implements the two main functions requireStoreArrays and doCalculation of the
     * SoftwareTriggerCalculation class.
     */
    class FastRecoCalculator : public SoftwareTriggerCalculation {
    public:
      /// Set the default names for the store arrays.
      FastRecoCalculator() : m_cdcRecoTracks("CDCRecoTracks"), m_eclClusters() {}
      /// Require the CDCRecoHits and the ECLClusters. We do not need more here.
      void requireStoreArrays() override;
      /// Actually write out the variables into the map.
      void doCalculation(SoftwareTriggerObject& calculationResult) override;

    private:
      /// Internal storage of the cdc reco tracks.
      StoreArray <RecoTrack> m_cdcRecoTracks;
      /// Internal storage of the all reco tracks, in case we ran without SVD,
      /// where the cdc reco tracks are stored into the "normal" reco tracks store array
      StoreArray <RecoTrack> m_recoTracks;
      /// Internal storage of the ecl clusters.
      StoreArray <ECLCluster> m_eclClusters;
      /// Cached transformation object to be used in each calculation (we cache it because of the slow database fetch).
      PCmsLabTransform m_transformer;
      /// Storage for the ECL clusters with photon hypothesis
      std::vector<std::reference_wrapper<const ECLCluster>> m_eclClustersWithPhotonHypothesis;
    };
  }
}