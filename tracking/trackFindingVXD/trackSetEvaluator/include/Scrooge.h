/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>

#include <vector>
#include <algorithm>

namespace Belle2 {
  /** Executes greedy algorithm for vector of QITrackOverlap structs.
   *
   *  In contrast to Jakob's original greedy implementation, this one doesn't use a recursive
   *  algorithm, but sorts according to QI and simply goes from the top and kills all overlapping
   *  SpacePointTrackCands.
   */
  class Scrooge {
  public:
    /** Sets the isActive flag in m_qiTrackOverlap to false, for killed tracks.
     *
     *  Sorts the "tracks" according to quality, loops from the top and kills
     *  overlapping tracks of all tracks, that are still active.
     */
    void performSelection(std::vector <OverlapResolverNodeInfo>& overlapResolverNodeInfo)
    {
      //sort the vector according to the QI supplied.
      std::sort(overlapResolverNodeInfo.begin(), overlapResolverNodeInfo.end(),
      [](OverlapResolverNodeInfo const & lhs, OverlapResolverNodeInfo const & rhs) -> bool {
        return lhs.qualityIndicator > rhs.qualityIndicator;
      });

      //kill all tracks, that have overlaps and lower QI:
      auto endOfOverlapResolverNodeInfo   = overlapResolverNodeInfo.cend();
      for (auto trackIter = overlapResolverNodeInfo.begin(); trackIter != endOfOverlapResolverNodeInfo; trackIter++) {
        if (!trackIter->activityState) continue;
        for (auto testTrackIter = trackIter; testTrackIter != endOfOverlapResolverNodeInfo; testTrackIter++) {
          if (std::find(trackIter->overlaps.begin(), trackIter->overlaps.end(), testTrackIter->trackIndex) !=
              trackIter->overlaps.end()) {
            testTrackIter->activityState = 0.;
          }
        }
      }
    }
  };
}
