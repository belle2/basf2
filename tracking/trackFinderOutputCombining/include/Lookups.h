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

#include <tracking/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrainFilter.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
  }

  namespace TrackFinderOutputCombining {

    /** Abstract base class for a list of objects which could be filled (more or less a vector) */
    template <class ItemType, class ListType>
    class LookUpBase {
    public:
      /** Should be overloaded. Fill the vector with the given elements */
      virtual void fillWith(std::vector<ItemType>& items) = 0;
      virtual ~LookUpBase() { }

      /** STL: begin */
      typename std::vector<ListType>::const_iterator begin() const
      {
        return m_lookup.begin();
      }

      /** STL: begin */
      typename std::vector<ListType>::iterator begin()
      {
        return m_lookup.begin();
      }

      /** STL: end */
      typename std::vector<ListType>::const_iterator end() const
      {
        return m_lookup.end();
      }

      /** STL: end */
      typename std::vector<ListType>::iterator end()
      {
        return m_lookup.end();
      }

    protected:
      std::vector<ListType> m_lookup; /**< The internal store for the elements list */
    };

    /** We use this class for storing our segment lists - one for each superlayer */
    class SegmentLookUp : public LookUpBase<TrackFindingCDC::CDCRecoSegment2D, std::vector<SegmentInformation*>> {
    public:
      /** Create the lists. Do not forget to call clear before the next event. */
      void fillWith(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments) override;

      /** Clear all pointer vectors */
      void clear()
      {
        for (std::vector<SegmentInformation*>& segmentList : m_lookup) {
          for (SegmentInformation* segmentInformation : segmentList) {
            delete segmentInformation;
          }
        }
      }
    };

    class TrackLookUp : public LookUpBase<TrackFindingCDC::CDCTrack, TrackInformation*> {
    public:
      /** Create the lists. Do not forget to call clear before the next event. */
      void fillWith(std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

      /** Clear all pointer vectors */
      void clear()
      {
        for (TrackInformation* trackInformation : m_lookup) {
          delete trackInformation;
        }
      }
    };

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
      void fillWith(std::vector<TrackFindingCDC::CDCTrack>& tracks, std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments)
      {
        m_trackLookUp.fillWith(tracks);
        m_segmentLookUp.fillWith(segments);
      }

      /**
       * Do the heavy combining works:
       * - Go through all superlayers
       * - In each superlayer: Go through all segments in this superlayer and match them to the tracks.
       *   If a match is made is calculated using the method segmentMatchesToTrack.
       *   After that we have a segment <-> track network.
       * - Go through all tracks and try to concatenate the segments in this superlayer to a larger segment train that fits together.
       *   For deciding if a list of segments could be a train we use the method couldBeASegmentTrain.
       * - If there is more than one segment (or train of segments) in this superlayer, that match to this track, we use a fit to decide
       *   which train of segments should be kept.
       * - If there is (now) only one possible train/segment left, we mark this as the goodSegmentTrain for this track.
       * - We now still have the problem that there could be two ore more tracks matched to the same segment (or segment in a train)
       *   So we go through all the segments in the good-markes trains and check if they have more than one match. We try to find the best matching candidate.
       * - Now we really have a one-on-one relation between tracks and segments. We can put them all together.
       */

      void combine(TrackFindingCDC::BaseSegmentTrackChooser& segmentTrackChooser,
                   TrackFindingCDC::BaseSegmentTrainFilter& segmentTrainFilter,
                   TrackFindingCDC::BaseSegmentTrackFilter& segmentTrackFilter);

      /**
       * Clear all the pointer vectors.
       */
      void clear()
      {
        m_trackLookUp.clear();
        m_segmentLookUp.clear();
      }

    private:
      const float m_param_percentageForPerpSMeasurements = 0.05; /**< We use this amount of overlap when defining a segment train */
      const float m_param_minimalFitProbability = 0.5; /**< The probability of the chi2 of a fit should be better than this */

      /** Decide if a list of segments could be a segment train (of the do not overlap each other) */
      bool couldBeASegmentTrain(const TrainOfSegments& list, const TrackFindingCDC::CDCTrack* track);

      /** Find the best fitting train of segments to a given track from the list */
      const TrainOfSegments& findBestFittingSegmentTrain(std::list<TrainOfSegments>& trainsOfSegments,
                                                         TrackInformation* trackInformation, TrackFindingCDC::BaseSegmentTrackFilter& segmentTrackFilter);

      /** Go through all segments a combine them to their best matches */
      void tryToCombineSegmentTrainAndMatchedTracks(const TrainOfSegments& trainOfSegments);

      /** Do the Segment <-> Track matching */
      void matchTracksToSegment(SegmentInformation* segmentInformation, TrackFindingCDC::BaseSegmentTrackChooser& segmentTrackChooser);

      /** Make all possible subsets of a given list */
      void makeAllCombinations(std::list<TrainOfSegments>& trainsOfSegments, const TrackInformation* trackInformation,
                               TrackFindingCDC::BaseSegmentTrainFilter& segmentTrainFilter);

      /** Do a fit and calculate the chi2 */
      double testFitSegmentToTrack(SegmentInformation* segmentInformation, const TrackInformation* trackInformation);

      /** Do a fit and calculate the chi2 */
      double testFitSegmentTrainToTrack(const TrainOfSegments& train, const TrackInformation* trackInformation);

      /** Create all possible trains with a given segments list */
      void createTrainsOfMatchedSegments(std::list<TrainOfSegments>& trainsOfSegments, const TrackInformation* trackInformation,
                                         TrackFindingCDC::BaseSegmentTrainFilter& segmentTrainFilter);

      /** Combine a segment and a track */
      void addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTracks);

    private:
      TrackLookUp m_trackLookUp; /**< The used track list */
      SegmentLookUp m_segmentLookUp; /**< The used segment list */
    };
  }
}
