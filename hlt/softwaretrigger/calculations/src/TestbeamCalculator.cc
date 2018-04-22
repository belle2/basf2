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

    void TestbeamCalculator::doCalculation(SoftwareTriggerObject& calculationResult)
    {
      // output the number of SVD Clusters
      calculationResult["nSVDClusters"] = m_svdClusters.getEntries();

      // find track with the largest pt
      double ptMax = 0.0f;
      for (auto const& t : m_svdTracks) {
        // check for correct fit
        if (t.getNumberOfFittedHypotheses() == 0)
          continue;

        auto fitRes = t.getTrackFitResultWithClosestMass(Const::electron);
        if (not fitRes) {
          continue;
        }
        ptMax = std::max(ptMax, fitRes->getTransverseMomentum());
      }

      // store the track with the largest pt of all found tracks
      calculationResult["SVDTrackMaxPt"] = ptMax;

      // store the number of tracks
      calculationResult["nSVDTracks"] = m_svdTracks.getEntries();
    }
  }
}
