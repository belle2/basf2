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

    /**
     * This class represents a matchable type.
     * It has capabilities of storing the matches together
     * with the probability of a match.
     */
    template <class OwnType, class MatchingType>
    class MatchingInformation {
    public:
      /** Vector of pairs: object, probability of match */
      typedef std::vector<std::pair<MatchingType*, double>> ListOfMatchCandidates;

      /**
       * Initialize the object with the underlying instance that should be matched.
       * @param input
       */
      MatchingInformation(OwnType* input) : m_object(input), m_isSorted(true)
      {
        m_listOfMatches.reserve(5);
      };

      /**
       * Return the matches with the probabilities as a vector of pairs.
       * @return
       */
      ListOfMatchCandidates& getMatches()
      {
        m_isSorted = false;
        return m_listOfMatches;
      }

      /**
       * Return the matches with the probabilities as a vector of pairs.
       * Const version.
       * @return
       */
      const ListOfMatchCandidates& getMatches() const
      {
        return m_listOfMatches;
      }

      /**
       * If not already sorted sort the matches according to their match probability
       * and return the one with the highest.
       * @return A object with the highest match probability. If there are more than one
       * with the same probability a random one is returned.
       */
      MatchingType* getBestMatch()
      {
        if (m_listOfMatches.size() > 0) {
          if (not m_isSorted) {
            std::sort(m_listOfMatches.begin(), m_listOfMatches.end(), [](const std::pair<MatchingType*, double>& a,
            const std::pair<MatchingType*, double>& b) -> bool {
              return a.second < b.second;
            });
            m_isSorted = true;
          }

          return m_listOfMatches[0].first;
        } else {
          return nullptr;
        }
      }

      /**
       * Reset all matches.
       */
      void clearMatches()
      {
        m_listOfMatches.clear();
        m_isSorted = true;
      }

      /**
       * Add the object with the probability.
       * @param match
       * @param matchingProbability
       */
      void addMatch(MatchingType* match, double matchingProbability = 0)
      {
        m_listOfMatches.push_back(std::make_pair(match, matchingProbability));
        m_isSorted = false;
      }

      /**
       * Return the underlying object.
       * @return
       */
      OwnType* getObject() const
      {
        return m_object;
      }

    private:
      OwnType* m_object; /**< The underlying object */
      ListOfMatchCandidates m_listOfMatches; /**< The list of matches with their probabilities. */
      bool m_isSorted; /**< A flag to cache the sorted state. */
    };

    /**
     * This is a MatchingInformation for the relation CDCTrack -> CDCSegment
     * It has additional information on the track like a storage for the best fitting segment train
     * and the list of perpS of the track.
     */
    class TrackInformation : public MatchingInformation<TrackFindingCDC::CDCTrack, SegmentInformation> {
    public:
      /** Initialize with a track cand pointer. We do not have the ownership */
      TrackInformation(TrackFindingCDC::CDCTrack* trackCand) :
        MatchingInformation<TrackFindingCDC::CDCTrack, SegmentInformation>(trackCand),
        m_perpSList(), m_goodFittingSegmentTrain(), m_minPerpS(0), m_maxPerpS(0)
      {
      }

      /**
       * Convenience wrapper for better naming. Same as getObject.
       */
      TrackFindingCDC::CDCTrack* getTrackCand() const
      {
        return getObject();
      }

      /**
       * Return the maximum perpS. calcPerpS must be called before!
       * @return
       */
      double getMaxPerpS() const
      {
        return m_maxPerpS;
      }

      /**
       * Return the minimum perpS. calcPerpS must be called before!
       * @return
       */
      double getMinPerpS() const
      {
        return m_minPerpS;
      }

      /**
       * Return the perpS list.
       * @return
       */
      std::vector<double>& getPerpSList()
      {
        return m_perpSList;
      }

      /**
       * Return the perpS list.
       * Const version.
       * @return
       */
      const std::vector<double>& getPerpSList() const
      {
        return m_perpSList;
      }

      /**
       * Calculate the maximum and minimum perpS.
       */
      void calcPerpS()
      {
        std::sort(m_perpSList.begin(), m_perpSList.end());
        m_minPerpS = m_perpSList.front();
        m_maxPerpS = m_perpSList.back();
      }

      /**
       * Fill the storage for the best matching segment train for caching.
       * @param goodFittingSegmentTrain
       */
      void setGoodSegmentTrain(const std::vector<SegmentInformation*>& goodFittingSegmentTrain)
      {
        m_goodFittingSegmentTrain = goodFittingSegmentTrain;
      }

      /**
       * Receive the storage for the best matching segment train.
       * @return
       */
      const std::vector<SegmentInformation*>& getGoodSegmentTrain() const
      {
        return m_goodFittingSegmentTrain;
      }

      /**
       * Clear the storage for the best matching segment train.
       */
      void clearGoodSegmentTrain()
      {
        m_goodFittingSegmentTrain.clear();
      }

    private:
      std::vector<double> m_perpSList; /**< The list of perpS. Must be provided by the user */
      std::vector<SegmentInformation*> m_goodFittingSegmentTrain; /**< The cache for the best fitting segment train. */
      double m_minPerpS; /**< The minimum perpS. Is calculated with calcPerpS */
      double m_maxPerpS; /**< The maximum perpS. Is calculated with calcPerpS */
    };

    /**
     * This is a MatchingInformation for the relation CDCSegment -> CDCTrack
     * It has additional information on the segment like the infromation if this segment is already taken by a track finder.
     */
    class SegmentInformation : public MatchingInformation<TrackFindingCDC::CDCRecoSegment2D, TrackInformation> {
    public:
      /** Initialize with a segmentpointer. We do not have the ownership */
      SegmentInformation(TrackFindingCDC::CDCRecoSegment2D* segment) :
        MatchingInformation<TrackFindingCDC::CDCRecoSegment2D, TrackInformation>(segment)
      {
      }

      /**
       * Convenience wrapper for better naming. Same as getObject.
       */
      TrackFindingCDC::CDCRecoSegment2D* getSegment() const
      {
        return getObject();
      }

      /**
       * Calls the hasTakenFlag function of the underlying segment.
       * @return
       */
      bool isAlreadyTaken() const
      {
        return getSegment()->getAutomatonCell().hasTakenFlag();
      }
    };
  }
}
