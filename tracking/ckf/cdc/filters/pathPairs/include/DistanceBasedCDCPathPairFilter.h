/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/pathPairs/BaseCDCPathPairFilter.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  /// For the two paths with the same number of hits prefers one with smallest sum dist^2
  class DistanceBasedCDCPathPairFilter : public BaseCDCPathPairFilter {
  public:
    /// Input: pair of paths, returns 1 if pair.first to be selected, 0 otherwise.
    TrackFindingCDC::Weight operator()(const BaseCDCPathPairFilter::Object& pair) final {
      const auto& lhs = *pair.first;
      const auto& rhs = *pair.second;

      auto sumOfDistances = [](const CDCCKFPath & path)
      {
        // compute sum of distances for the two pats:
        double sum = 0;
        for (auto const& state : path) {
          double dist = (state.getHitDistance());
          sum += dist * dist;
        }
        return sum;
      };

      const auto lhsDistSum = sumOfDistances(lhs);
      const auto rhsDistSum = sumOfDistances(rhs);

      // priority is most hits
      if (lhs.size() != rhs.size())
      {
        return lhs.size() > rhs.size();
      }

      // Otherwise order by bset sum of dist^2
      return lhsDistSum < rhsDistSum;
    }
  };
}
