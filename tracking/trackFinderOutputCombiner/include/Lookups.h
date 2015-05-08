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

#include <tracking/trackFinderOutputCombiner/MatchingInformation.h>

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
  }

  namespace TrackFinderOutputCombining {

    template <class ItemType, class ListType>
    class LookUpBase {
    public:
      virtual void fillWith(std::vector<ItemType>& items) = 0;
      virtual ~LookUpBase() { }

      typename std::vector<ListType>::const_iterator begin() const
      {
        return m_lookup.begin();
      }
      typename std::vector<ListType>::iterator begin()
      {
        return m_lookup.begin();
      }
      typename std::vector<ListType>::const_iterator end() const
      {
        return m_lookup.end();
      }
      typename std::vector<ListType>::iterator end()
      {
        return m_lookup.end();
      }

    protected:
      std::vector<ListType> m_lookup;
    };

    class SegmentLookUp : public LookUpBase<TrackFindingCDC::CDCRecoSegment2D, std::vector<SegmentInformation*>> {
    public:
      void fillWith(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments) override;
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
      void fillWith(std::vector<TrackFindingCDC::CDCTrack>& tracks) override;
      void clear()
      {
        for (TrackInformation* trackInformation : m_lookup) {
          delete trackInformation;
        }
      }
    };

    class SegmentTrackCombiner {
    public:

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
      void combine();

      /**
       * Clear all the pointer vectors.
       */
      void clear()
      {
        m_trackLookUp.clear();
        m_segmentLookUp.clear();
      }

    private:
      const float m_param_percentageForPerpSMeasurements = 0.05;
      const float m_param_minimalFitProbability = 0.5;

      bool segmentMatchesToTrack(const SegmentInformation* segmentInformation, const TrackInformation* trackInformation);
      bool couldBeASegmentTrain(const TrainOfSegments& list, const TrackInformation* trackInformation);

      const TrainOfSegments& findBestFittingSegmentTrain(std::list<TrainOfSegments>& trainsOfSegments,
                                                         TrackInformation* trackInformation);
      void tryToCombineSegmentTrainAndMatchedTracks(const TrainOfSegments& trainOfSegments);
      void matchTracksToSegment(SegmentInformation* segmentInformation);
      void makeAllCombinations(std::list<TrainOfSegments>& trainsOfSegments, const TrackInformation* trackInformation);
      double testFitSegmentToTrack(SegmentInformation* segmentInformation, const TrackInformation* trackInformation);
      double testFitSegmentTrainToTrack(const TrainOfSegments& train, const TrackInformation* trackInformation);
      void createTrainsOfMatchedSegments(std::list<TrainOfSegments>& trainsOfSegments, const TrackInformation* trackInformation);
      void createTracksForOutput(std::vector<TrackFindingCDC::CDCTrack>& tracks);
      void addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTracks);

    private:
      TrackLookUp m_trackLookUp;
      SegmentLookUp m_segmentLookUp;
    };
  }
}
