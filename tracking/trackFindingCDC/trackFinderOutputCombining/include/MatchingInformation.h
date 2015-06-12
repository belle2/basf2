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

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <vector>
#include <list>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class SegmentInformation;
    class TrackInformation;

    template <class OwnType, class MatchingType>
    class MatchingInformation {
    public:
      typedef std::vector<MatchingType*> ListOfMatchCandidates;

      MatchingInformation(OwnType* input) : m_object(input)
      {
        m_listOfMatches.reserve(5);
      };

      ListOfMatchCandidates& getMatches()
      {
        return m_listOfMatches;
      }

      const ListOfMatchCandidates& getMatches() const
      {
        return m_listOfMatches;
      }

      void clearMatches()
      {
        m_listOfMatches.clear();
      }

      void addMatch(MatchingType* match)
      {
        m_listOfMatches.push_back(match);
      }

      OwnType* getObject() const
      {
        return m_object;
      }

    private:
      OwnType* m_object;
      ListOfMatchCandidates m_listOfMatches;
    };

    class TrackInformation : public MatchingInformation<TrackFindingCDC::CDCTrack, SegmentInformation> {
    public:
      TrackInformation(TrackFindingCDC::CDCTrack* trackCand) :
        MatchingInformation<TrackFindingCDC::CDCTrack, SegmentInformation>(trackCand),
        m_perpSList(), m_goodFittingSegmentTrain(), m_minPerpS(0), m_maxPerpS(0)
      {
      }

      TrackFindingCDC::CDCTrack* getTrackCand() const
      {
        return getObject();
      }

      double getMaxPerpS() const
      {
        return m_maxPerpS;
      }

      double getMinPerpS() const
      {
        return m_minPerpS;
      }

      std::vector<double>& getPerpSList()
      {
        return m_perpSList;
      }

      const std::vector<double>& getPerpSList() const
      {
        return m_perpSList;
      }

      void calcPerpS()
      {
        std::sort(m_perpSList.begin(), m_perpSList.end());
        m_minPerpS = m_perpSList.front();
        m_maxPerpS = m_perpSList.back();
      }

      void setGoodSegmentTrain(const std::vector<SegmentInformation*>& goodFittingSegmentTrain)
      {
        m_goodFittingSegmentTrain = goodFittingSegmentTrain;
      }

      const std::vector<SegmentInformation*>& getGoodSegmentTrain() const
      {
        return m_goodFittingSegmentTrain;
      }

      void clearGoodSegmentTrain()
      {
        m_goodFittingSegmentTrain.clear();
      }

    private:
      std::vector<double> m_perpSList;
      std::vector<SegmentInformation*> m_goodFittingSegmentTrain;
      double m_minPerpS;
      double m_maxPerpS;
    };

    class SegmentInformation : public MatchingInformation<TrackFindingCDC::CDCRecoSegment2D, TrackInformation> {
    public:
      SegmentInformation(TrackFindingCDC::CDCRecoSegment2D* segment) :
        MatchingInformation<TrackFindingCDC::CDCRecoSegment2D, TrackInformation>(segment)
      {
      }

      TrackFindingCDC::CDCRecoSegment2D* getSegment() const
      {
        return getObject();
      }

      bool isAlreadyTaken() const
      {
        return getSegment()->getAutomatonCell().hasTakenFlag();
      }
    };
  }
}
