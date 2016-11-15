/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCSegment2D;

    /** Class to add the matched hits to the track and set the taken flag correctly. */
    class StereoSegmentTrackAdder {
    public:
      /** Add the matched segment to the track and set the taken flag correctly. We ignore the weight completely here. */
      static void add(CDCTrack& track,
                      const std::vector<WithWeight<const CDCSegment2D*>>& matchedSegments);
    };
  }
}
