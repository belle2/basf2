/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Thomas Hauth                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerVariableManager.h>
#include <hlt/softwaretrigger/calculations/SoftwareTriggerCalculation.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Implementation of a calculator used in the SoftwareTriggerModule
     * during VXD beamtest trials. Here, the HLT is not used to filter events
     * but only to generate the ROI.
     * This calculator implements some simple variables for SoftwareTrigger
     * in this scenario and with the input values that are available.
     */
    class TestbeamCalculator : public SoftwareTriggerCalculation {
    public:
      /// Require the particle list. We do not need more here.
      void requireStoreArrays() override;

      /// Actually write out the variables into the map.
      void doCalculation(SoftwareTriggerObject& calculationResult) override;

    private:
      /// Internal storage of the SVD Clusters
      StoreArray <SVDCluster> m_svdClusters;
      /// Internal storage for reconstructed SVD-onyl tracks
      StoreArray<Track> m_svdTracks;
    };
  }
}
