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

#include <vector>

namespace Belle2 {
  /** Struct for holding information needed by overlap resolving algorithms for one node.
   *
   *  The node can be e.g. a SpacePointTrackCand(SPTC), that overlaps with other SPTCs due to e.g.
   *  sharing hits, but other uses seem likely in the future.
   */
  struct OverlapResolverNodeInfo {
    /** Constructor with initializer list for all relevant info for e.g. greedy algorithm (Scrooge) or Hopfield.
     */
    OverlapResolverNodeInfo(float qI, unsigned short tI, std::vector<unsigned short> const& o, float aS):
      qualityIndicator(qI), trackIndex(tI), overlaps(o), activityState(aS) {}
    float                       qualityIndicator;   ///<Estimate of quality, e.g. from Circle Fitter or other fast fitter.
    unsigned short              trackIndex;     ///<Index of the SpacePointTrackCand in the StoreArray.
    std::vector<unsigned short> overlaps;       ///<Vector of indices of tracks, that overlap with this candidate.
    float                       activityState;  ///<An active track has a state close to 1.
  };
}
