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
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    class FastRecoCalculator {
    public:
      void requireStoreArrays();

      void doCalculation(SoftwareTriggerObject& calculationResult) const;

    public:
      StoreArray <RecoTrack> m_cdcRecoTracks;
      StoreArray <ECLCluster> m_eclClusters;

    };
  }
}