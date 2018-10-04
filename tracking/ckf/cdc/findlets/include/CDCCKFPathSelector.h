/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

namespace Belle2 {
  class CDCCKFPathSelector : public TrackFindingCDC::Findlet<CDCCKFPath>  {
  public:
    void apply(std::vector<CDCCKFPath>& newPaths) override
    {
      const auto pathComparison = [](const CDCCKFPath & lhs, const CDCCKFPath & rhs) {
        const auto& lhsLastState = lhs.back();
        const auto& rhsLastState = rhs.back();

        const auto lhsArcLength = lhsLastState.getArcLength();
        const auto rhsArcLength = rhsLastState.getArcLength();

        // TODO: arc length of everything!
        return lhs.size() / lhsArcLength > rhs.size() / rhsArcLength;
      };

      std::sort(newPaths.begin(), newPaths.end(), pathComparison);

      TrackFindingCDC::only_best_N(newPaths, m_maximalCandidatesInFlight);
    }

  private:
    size_t m_maximalCandidatesInFlight = 10;
  };
}
