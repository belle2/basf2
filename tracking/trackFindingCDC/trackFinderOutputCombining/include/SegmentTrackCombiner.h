/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentTrack/BaseSegmentTrackFilter.h>

#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/Lookups.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCTrack;

    /** Class which does the segment - track combining. */
    class SegmentTrackCombiner {
    public:
      /**
       * Match the easy segment-track pairs. Easy in this context means we assume at least one common hit.
       * Because one common hit may be to less, we use a filter after having found the match.
       * There are some difficulties in the case that one segment has hits in common with more than one track
       * and the filter gives a positive result in more than one case.
       * In this case we combine it with the track for which the filter gives the highest result.
       */
      void match(BaseSegmentTrackFilter& segmentTrackFilter, std::vector<CDCTrack>& tracks, std::vector<CDCSegment2D>& segments);

    private:
      /**
       * Helper function to add a segment to a track with respecting the taken information of the segment.
       * If useTakenFlagOfHits is set to true, only those hits are added that do not have a taken flag.
       */
      void addSegmentToTrack(const CDCSegment2D& segment,
                             CDCTrack& track);
    };
  }
}
