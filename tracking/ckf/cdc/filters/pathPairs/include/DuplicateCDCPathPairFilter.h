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

#include <boost/range/adaptor/reversed.hpp>

namespace Belle2 {
  /// For the two paths with the same number of hits prefers one with smallest sum dist^2
  class DuplicateCDCPathPairFilter : public BaseCDCPathPairFilter {
  public:
    /// Input: pair of paths, returns 1 if duplicate hits found
    TrackFindingCDC::Weight operator()(const BaseCDCPathPairFilter::Object& pair) final {
      const auto& lhs = *pair.first;
      const auto& rhs = *pair.second;

      // Idea: The hits in the center of the CDC cannot be used for a duplicate check
      // Typically, pair production electrons are close to each other there, so that only single hits
      // are produced for both particles
      // However, close to the ECL, the tracks should diverge
      // In contrast to that, if a second track is created by Bremsstrahlung, one of the
      // tracks should be a subset of the other one (more or less)

      // Unfortunately, charge conservation cannot be used to distinguish both effects

      // Right now, use simpler algorithm
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

      B2INFO("~~~~~~~~~~>>> Matched hits: " << double(match) / double(total));

      return double(match) / double(total) > 0.9;
    }
  };
}

