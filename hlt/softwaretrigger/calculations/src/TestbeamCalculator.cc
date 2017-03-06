/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Thomas Hauth                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/softwaretrigger/calculations/TestbeamCalculator.h>
#include <framework/gearbox/Const.h>

#include <algorithm>

namespace Belle2 {
  namespace SoftwareTrigger {
    void TestbeamCalculator::requireStoreArrays()
    {
      m_svdClusters.isRequired();
      m_svdTracks.isRequired();
    };

    void TestbeamCalculator::doCalculation(SoftwareTriggerObject& calculationResult) const
    {
      // output the number of SVD Clusters
      calculationResult["nSVDCluster"] = m_svdClusters.getEntries();

      // find track with the largest pt
      double ptMax = 0.00;
      for (auto const& t : m_svdTracks) {
        // check for correct fit
        if (t.getNumberOfFittedHypotheses() == 0)
          continue;

        auto fitRes = t.getTrackFitResult(Const::electron);
        ptMax = std::max(ptMax, fitRes->getTransverseMomentum());
      }

      // store the track with the largest pt of all found tracks
      calculationResult["nSVDTrackMaxPt"] = ptMax;
    }
  }
}
