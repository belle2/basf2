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

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  class CDCCKFPathSelector : public TrackFindingCDC::Findlet<CDCCKFPath> {
  public:
    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalCandidatesInFlight"),
                                    m_maximalCandidatesInFlight,
                                    "Maximal candidates in flight", m_maximalCandidatesInFlight);
    }

    void apply(std::vector<CDCCKFPath>& newPaths) override
    {

      const auto pathComparison = [](const CDCCKFPath & lhs, const CDCCKFPath & rhs) {


        auto sumOfDistances = [](const CDCCKFPath & path) {
          // compute sum of distances for the two pats:
          double sum = 0;
          for (auto const& state : path) {
            double dist = (state.getHitDistance());
            sum += dist * dist;
          }
          return sum;
        };


        const auto& lhsLastState = lhs.back();
        const auto& rhsLastState = rhs.back();


        const auto lhsDistSum = sumOfDistances(lhs);
        const auto rhsDistSum = sumOfDistances(rhs);

        const auto lhsArcLength = lhsLastState.getArcLength();
        const auto rhsArcLength = rhsLastState.getArcLength();

        // priority is most hits
        if (lhs.size() != rhs.size()) {
          return lhs.size() > rhs.size();
        }

        // Otherwise order by bset sum of dist^2
        return lhsDistSum < rhsDistSum;

        return lhsArcLength < rhsArcLength;
      };

      std::sort(newPaths.begin(), newPaths.end(), pathComparison);

      TrackFindingCDC::only_best_N(newPaths, m_maximalCandidatesInFlight);
    }

  private:
    size_t m_maximalCandidatesInFlight = 3;
  };
}
