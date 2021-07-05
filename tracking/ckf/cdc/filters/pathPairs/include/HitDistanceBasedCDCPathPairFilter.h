/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/pathPairs/BaseCDCPathPairFilter.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  /// Prefers path with smallest sum dist^2 / length of path
  class HitDistanceBasedCDCPathPairFilter : public BaseCDCPathPairFilter {
  public:
    /// Input: pair of paths, returns 1 if pair.first to be selected, 0 otherwise.
    TrackFindingCDC::Weight operator()(const BaseCDCPathPairFilter::Object& pair) final {
      const auto& lhs = *pair.first;
      const auto& rhs = *pair.second;

      // prefer longer paths
      if (lhs.size() > rhs.size() + 2)
      {
        return true;
      } else if (lhs.size() + 2 < rhs.size())
      {
        return false;
      }

      auto sumOfDistances = [](const CDCCKFPath & path)
      {
        // compute sum of distances for the two parts:
        double sum = 0;
        for (auto const& state : path) {
          double dist = (state.getHitDistance());
          sum += dist * dist;
        }
        return sum;
      };

      const auto lhsDist = sumOfDistances(lhs) / lhs.size();
      const auto rhsDist = sumOfDistances(rhs) / rhs.size();

      // Order by bset sum of dist^2
      return lhsDist < rhsDist;
    }
  };
}
