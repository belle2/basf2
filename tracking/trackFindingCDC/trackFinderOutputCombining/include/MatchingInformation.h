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

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

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
     * It has capabilities of storing a matchable type and their matches together with the probability of a match.
     */
    template<class OwnType, class MatchingType>
    class MatchingInformation {
    public:
      /** Vector of pairs: object, probability of match */
      using ListOfMatchCandidates = std::vector<std::pair<MatchingType*, double>>;

      /**
       * Initialize the object with the underlying instance that should be matched.
       * @param input object to be used as matchable base object.
       */
      explicit MatchingInformation(OwnType* input) : m_object(input), m_isSorted(true)
      {
        m_listOfMatches.reserve(5);
      };

      /**
       * Return the matches with the probabilities as a vector of pairs.
       * @return the list of matches with their probabilities as set before.
       */
      ListOfMatchCandidates& getMatches()
      {
        m_isSorted = false;
        return m_listOfMatches;
      }

      /**
       * Return the matches with the probabilities as a vector of pairs.
       * Const version.
       * @return the list of matches with their probabilities as set before.
       */
      const ListOfMatchCandidates& getMatches() const
      {
        return m_listOfMatches;
      }

      /**
       * If not already sorted sort the matches according to their match probability
       * and return the one with the highest probability.
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
       * Add the object with the given probability.
       * @param match the object to add as match.
       * @param matchingProbability the probability of that match.
       */
      void addMatch(MatchingType* match, double matchingProbability = 0)
      {
        m_listOfMatches.push_back(std::make_pair(match, matchingProbability));
        m_isSorted = false;
      }

    protected:
      /**
       * Return the underlying matchable object.
       * @return The object.
       */
      OwnType* getObject() const
      {
        return m_object;
      }

    private:
      OwnType* m_object; /**< The underlying object. */
      ListOfMatchCandidates m_listOfMatches; /**< The list of matches with their probabilities. */
      bool m_isSorted; /**< A flag to cache the sorted state. */
    };

    /**
     * This is a MatchingInformation for the relation CDCTrack -> CDCSegment
     * It has additional information on the track like a storage for the best fitting segment train
     * and the list of arc length 2D of the track.
     */
    class TrackInformation : public MatchingInformation<TrackFindingCDC::CDCTrack, SegmentInformation> {
    public:
      /** Initialize with a track cand pointer. We do not have the ownership. */
      explicit TrackInformation(TrackFindingCDC::CDCTrack* trackCand) :
        MatchingInformation<TrackFindingCDC::CDCTrack, SegmentInformation>(trackCand),
        m_arcLength2DList(), m_goodFittingSegmentTrain(), m_minArcLength2D(0), m_maxArcLength2D(0)
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
       * Return the maximum arc length 2D. calcArcLength2D must be called before!
       */
      double getMaxArcLength2D() const
      {
        return m_maxArcLength2D;
      }

      /**
       * Return the minimum arc length 2D. calcArcLength2D must be called before!
       */
      double getMinArcLength2D() const
      {
        return m_minArcLength2D;
      }

      /**
       * Return the arc length 2D list.
       */
      std::vector<double>& getArcLength2DList()
      {
        return m_arcLength2DList;
      }

      /**
       * Return the arc length 2D list.
       * Const version.
       */
      const std::vector<double>& getArcLength2DList() const
      {
        return m_arcLength2DList;
      }

      /**
       * Calculate the maximum and minimum arc length 2D.
       */
      void calcArcLength2D()
      {
        std::sort(m_arcLength2DList.begin(), m_arcLength2DList.end());
        m_minArcLength2D = m_arcLength2DList.front();
        m_maxArcLength2D = m_arcLength2DList.back();
      }

      /**
       * Fill the storage for the best matching segment train for caching.
       */
      void setGoodSegmentTrain(const std::vector<SegmentInformation*>& goodFittingSegmentTrain)
      {
        m_goodFittingSegmentTrain = goodFittingSegmentTrain;
      }

      /**
       * Receive the storage for the best matching segment train.
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
      std::vector<double> m_arcLength2DList; /**< The list of arc length 2D. Must be provided by the user. */
      std::vector<SegmentInformation*> m_goodFittingSegmentTrain; /**< The cache for the best fitting segment train. */
      double m_minArcLength2D; /**< The minimum arc length 2D. Is calculated with calcArcLength2D */
      double m_maxArcLength2D; /**< The maximum arc length 2D. Is calculated with calcArcLength2D */
    };

    /**
     * This is a MatchingInformation for the relation CDCSegment -> CDCTrack.
     * It has additional information on the segment like the infromation if this segment is already taken by a track finder.
     */
    class SegmentInformation : public MatchingInformation<TrackFindingCDC::CDCSegment2D, TrackInformation> {
    public:
      /** Initialize with a segmentpointer. We do not have the ownership. */
      explicit SegmentInformation(TrackFindingCDC::CDCSegment2D* segment) :
        MatchingInformation<TrackFindingCDC::CDCSegment2D, TrackInformation>(segment)
      {
      }

      /**
       * Convenience wrapper for better naming. Same as getObject.
       */
      TrackFindingCDC::CDCSegment2D* getSegment() const
      {
        return getObject();
      }

      /**
       * Calls the hasTakenFlag function of the underlying segment.
       */
      bool isAlreadyTaken() const
      {
        return getSegment()->getAutomatonCell().hasTakenFlag();
      }
    };
  }
}
