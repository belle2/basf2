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
      ~SegmentLookUp()
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
      ~TrackLookUp()
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
       * TODO: DESCRIBE!
       */
      void combine();

    private:
      const float m_param_percentageForPerpSMeasurements = 0.05;
      const float m_param_minimalFitProbability = 0.5;

      void createTracksForOutput(std::vector<TrackFindingCDC::CDCTrack>& tracks);
      void combineSegmentTrainAndMatchedTracks(const TrainOfSegments& trainOfSegments);
      void matchTracksToSegment(SegmentInformation* segmentInformation);
      bool segmentMatchesToTrack(const SegmentInformation* segmentInformation, const TrackInformation* trackInformation);
      bool doesFitTogether(const TrainOfSegments& list, const TrackInformation* trackInformation);
      void makeAllCombinations(std::list<TrainOfSegments>& trainsOfSegments, const TrackInformation* trackInformation);
      void addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTracks);
      double testFitSegmentToTrack(const SegmentInformation* segmentInformation, const TrackInformation* trackInformation);
      void createTrainsOfSegments(std::list<TrainOfSegments>& trainsOfSegments, const TrackInformation* trackInformation);

    private:
      TrackLookUp m_trackLookUp;
      SegmentLookUp m_segmentLookUp;
    };
  }
}
