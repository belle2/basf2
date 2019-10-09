/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Simon Kurz                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/pathPairs/BaseCDCPathPairFilter.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  /// For the two paths with the same number of hits prefers one with shortest arcLength ("densest path")
  class ArcLengthBasedCDCfromEclPathPairFilter : public BaseCDCPathPairFilter {
  public:
    /// Input: pair of paths, returns 1 if pair.first to be selected, 0 otherwise.
    TrackFindingCDC::Weight operator()(const BaseCDCPathPairFilter::Object& pair) final {
      const auto& lhs = *pair.first;
      const auto& rhs = *pair.second;
      const auto& lhsLastState = lhs.back();
      const auto& rhsLastState = rhs.back();

      const auto lhsArcLength = - lhsLastState.getArcLength();
      const auto rhsArcLength = - rhsLastState.getArcLength();

      // priority is most hits
      if (lhs.size() != rhs.size())
      {
        return lhs.size() > rhs.size();
      }

      return lhsArcLength < rhsArcLength;
    }
  };
}
