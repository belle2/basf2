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

#include <tracking/trackFindingCDC/filters/segment_train/SegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segment_information_list_track/SegmentInformationListTrackFilter.h>
#include <tracking/trackFindingCDC/filters/background_segment/BackgroundSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/new_segment/NewSegmentsFilter.h>

#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/Lookups.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;

    /** Class which does the segment - track combining */
    class SegmentTrackCombiner {
    public:

      /** We use this to describe more than one segment that could belong together in one single superlayer.
       * Actually the segment finder should not preduce such things... */
      typedef std::vector<SegmentInformation*> TrainOfSegments;

      /**
       * Fill the given elements into the internal lookup tables.
       * @param tracks
       * @param segments
       */
      void fillWith(std::vector<CDCTrack>& tracks, std::vector<CDCRecoSegment2D>& segments)
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
       * @param segmentTrackChooserFirstStep
       */
      void match(BaseSegmentTrackFilter& segmentTrackChooserFirstStep);

      /**
       * Filter out the segments that are fake or background. Mark them as taken.
       * For deciding which is background or not we use the given filter.
       * @param backgroundSegmentFilter
       */
      void filter(BaseBackgroundSegmentsFilter& backgroundSegmentFilter);

      /**
       * Filter out the segments that are likely to be new tracks. Mark them as taken and assigned.
       * For deciding which is new or not we use the given filter.
       * @param newSegmentFilter
       */
      void filterOutNewSegments(BaseNewSegmentsFilter& newSegmentFilter);

      /**
       * Do the heavy combining works:
       * - Go through all superlayers
       * - In each superlayer: Go through all segments in this superlayer and match them to the tracks.
       *   If a match is made is calculated using the method segmentMatchesToTrack.
       *   After that we have a segment <-> track network.
       *   If there is a unique relation, add the segment to the track (only the new hits).
       * - With the remaining ones: Go through all tracks and try to concatenate the segments in this superlayer to a larger segment train that fits together.
       *   For deciding if a list of segments could be a train we use the method couldBeASegmentTrain.
       * - If there is more than one segment (or train of segments) in this superlayer, that match to this track, we use a fit to decide
       *   which train of segments should be kept.
       * - If there is (now) only one possible train/segment left, we mark this as the goodSegmentTrain for this track.
       * - We now still have the problem that there could be two ore more tracks matched to the same segment (or segment in a train)
       *   So we go through all the segments in the good-markes trains and check if they have more than one match. We try to find the best matching candidate.
       * - Now we really have a one-on-one relation between tracks and segments. We can put them all together.
       */

      void combine(BaseSegmentTrackFilter& segmentTrackChooserSecondStep,
                   BaseSegmentTrainFilter& segmentTrainFilter,
                   BaseSegmentInformationListTrackFilter& segmentTrackFilter);


      /**
       * Clear all the pointer vectors.
       */
      void clear()
      {
        m_trackLookUp.clear();
        m_segmentLookUp.clear();
      }

      static void addSegmentToTrack(const CDCRecoSegment2D& segment, CDCTrack& track);

    private:
      const float m_param_minimalFitProbability = 0.5; /**< The probability of the chi2 of a fit should be better than this */

      /** Find the best fitting train of segments to a given track from the list */
      const TrainOfSegments* findBestFittingSegmentTrain(std::list<TrainOfSegments>& trainsOfSegments,
                                                         TrackInformation* trackInformation, BaseSegmentInformationListTrackFilter& segmentTrackFilter);

      /** Go through all segments a combine them to their best matches */
      void tryToCombineSegmentTrainAndMatchedTracks(const TrainOfSegments& trainOfSegments,
                                                    BaseSegmentInformationListTrackFilter& segmentTrackFilter);

      /** Do the Segment <-> Track matching */
      void matchTracksToSegment(SegmentInformation* segmentInformation, BaseSegmentTrackFilter& segmentTrackChooser);

      /** Make all possible subsets of a given list */
      void makeAllCombinations(std::list<TrainOfSegments>& trainsOfSegments, const TrackInformation* trackInformation,
                               BaseSegmentTrainFilter& segmentTrainFilter);

      /** Create all possible trains with a given segments list */
      void createTrainsOfMatchedSegments(std::list<TrainOfSegments>& trainsOfSegments, const TrackInformation* trackInformation,
                                         BaseSegmentTrainFilter& segmentTrainFilter);

      /** Delete all trains which are found as a bigger one also */
      void clearSmallerCombinations(std::list<TrainOfSegments>& trainsOfSegments);

      /** Combine a segment and a track */
      void addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTracks);

    private:
      TrackLookUp m_trackLookUp; /**< The used track list */
      SegmentLookUp m_segmentLookUp; /**< The used segment list */
    };
  }
}
