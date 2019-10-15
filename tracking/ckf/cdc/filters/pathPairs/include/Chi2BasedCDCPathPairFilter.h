/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Brau, Simon Kurz                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/pathPairs/BaseCDCPathPairFilter.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  /// Prefers path with smallest sum dist^2 / length of path
  class Chi2BasedCDCPathPairFilter : public BaseCDCPathPairFilter {
  public:
    /// Input: pair of paths, returns 1 if pair.first to be selected, 0 otherwise.
    TrackFindingCDC::Weight operator()(const BaseCDCPathPairFilter::Object& pair) final {
      const auto& lhs = *pair.first;
      const auto& rhs = *pair.second;

      if (lhs.size() > rhs.size() + 2)
      {
        return true;
      } else if (lhs.size() + 2 < rhs.size())
      {
        return false;
      }

      // Order by chi2 of fit of last state
      return lhs.back().getChi2() < rhs.back().getChi2();
    }
  };
}
