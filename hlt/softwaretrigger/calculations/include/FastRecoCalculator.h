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

#include <skim/softwaretrigger/core/SoftwareTriggerVariableManager.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Implementation of a calculator used in the SoftwareTriggerModule
     * to fill a SoftwareTriggerObject for doing HLT cuts.
     *
     * This calculator exports variables needed for the FastReco part
     * of the path, e.g.
     * * visible_energy
     * * highest_1_ecl
     * * max_pt
     * * max_pz
     * * first_highest_cdc_energies
     *
     * You do not have to create an instance of this class by yourself,
     * but rather use the SoftwareTriggerCalculation for it.
     */
    class FastRecoCalculator {
    public:
      /// Require the CDCRecoHits and the ECLClusters. We do not need more here.
      void requireStoreArrays();

      /// Actually write out the variables into the map.
      void doCalculation(SoftwareTriggerObject& calculationResult) const;

    private:
      /// Internal storage of the cdc reco tracks.
      StoreArray <RecoTrack> m_cdcRecoTracks;
      /// Internal storage of the ecl clusters.
      StoreArray <ECLCluster> m_eclClusters;
      /// Cached transformation object to be used in each calculation (we cache it because of the slow database fetch).
      PCmsLabTransform m_transformer;
    };
  }
}