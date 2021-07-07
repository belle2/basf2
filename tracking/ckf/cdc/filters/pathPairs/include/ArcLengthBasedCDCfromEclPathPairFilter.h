/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
