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

#include <tracking/trackFindingCDC/filters/segmentTrain/BaseSegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTrack/BaseSegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segmentInformationListTrack/BaseSegmentInformationListTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segment/BaseSegmentFilter.h>
#include <tracking/trackFindingCDC/filters/newSegment/BaseNewSegmentFilter.h>
#include <tracking/trackFindingCDC/filters/track/BaseTrackFilter.h>

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
       * Fill the given elements into the internal lookup tables.
       */
      void fillWith(std::vector<CDCTrack>& tracks, std::vector<CDCSegment2D>& segments)
      {
        m_trackLookUp.fillWith(tracks);
        m_segmentLookUp.fillWith(segments);
      }

      /**
       * Match the easy segment-track pairs. Easy in this context means we assume at least one common hit.
       * Because one common hit may be to less, we use a filter after having found the match.
       * There are some difficulties in the case that one segment has hits in common with more than one track
       * and the filter gives a positive result in more than one case.
       * In this case we combine it with the track for which the filter gives the highest result.
       */
      void match(BaseSegmentTrackFilter& segmentTrackChooserFirstStep);

      /**
       * Clear all the pointer vectors and reset the "new segments" to be not taken.
       */
      void clearAndRecover();

      /**
       * Helper function to add a segment to a track with respecting the taken information of the segment.
       * If useTakenFlagOfHits is set to true, only those hits are added that do not have a taken flag.
       */
      static void addSegmentToTrack(const CDCSegment2D& segment,
                                    CDCTrack& track,
                                    bool useTakenFlagOfHits = true);

    private:
      const float m_param_minimalFitProbability = 0.5; /**< The probability of the chi2 of a fit should be better than this */

      /** Do the Segment <-> Track matching. */
      void matchTracksToSegment(SegmentInformation* segmentInformation, BaseSegmentTrackFilter& segmentTrackChooser);

      /** Combine a segment and a track. */
      void addSegmentToTrack(SegmentInformation* segmentInformation,
                             TrackInformation* matchingTrack);

    private:
      TrackLookUp m_trackLookUp; /**< The used track list. */
      SegmentLookUp m_segmentLookUp; /**< The used segment list. */
    };
  }
}
