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
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <boost/range/adaptor/reversed.hpp>

namespace Belle2 {
  /// For the two paths with the same number of hits prefers one with smallest sum dist^2
  class DuplicateCDCPathPairFilter : public BaseCDCPathPairFilter {
  public:
    /// Input: pair of paths, returns 1 if duplicate hits found
    TrackFindingCDC::Weight operator()(const BaseCDCPathPairFilter::Object& pair) final {
      const auto& lhs = *pair.first;
      const auto& rhs = *pair.second;

      // Right now, use simple (yet efficient) algorithm to distinguish between
      // photon conversion and Bremsstrahlung
      // Check if 90% of the hits shared by both tracks
      // Might not be optimal for short tracks? (forward region)

      const CDCCKFPath* shortPath = &lhs;
      const CDCCKFPath* longPath = &rhs;
      if (longPath->size() < shortPath->size())
      {
        const CDCCKFPath* temp = shortPath;
        shortPath = longPath;
        longPath = temp;
      }

      int match = 0;
      int total = shortPath->size() - 1;
      for (const auto& sState : *shortPath)
      {
        for (const auto& lState : *longPath) {
          if (!sState.isSeed() && !lState.isSeed()
          && sState.getWireHit() == lState.getWireHit()) {
            match++;
            break;
          }
        }
      }

      return double(match) / double(total) > 0.9;
    }
  };
}

