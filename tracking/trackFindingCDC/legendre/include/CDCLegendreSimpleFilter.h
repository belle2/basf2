/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <list>
#include <vector>
#include <map>


namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackCandidate;
    class TrackHit;

    class SimpleFilter {

    public:

      /** This is a static class only. We do not need a constructor here */
      SimpleFilter() = delete;

      /** Returns the probability of hit assignment to the track. Actually this is not a probability at all but only a number between 0 and 1.
       * 1 is best.
       * TODO: more robust criteria should be implemented */
      static double getAssigmentProbability(const TrackHit* hit, const TrackCandidate* track);

      /** Hits reassignment */
      static void reassignHitsFromOtherTracks(const std::list<TrackCandidate*>& trackList);

      /** Append unused hits to tracks */
      static void appendUnusedHits(const std::list<TrackCandidate*>& trackList, const std::vector<TrackHit*>& AxialHitList,
                                   double minimal_assignment_probability = m_minimal_assignment_probability);

      /** Tries to find and delete all "bad" hits in a track.
       * By doing so we will loose hit efficiency, but gain a low fake rate. */
      static void deleteWrongHitsOfTrack(TrackCandidate* trackCandidate, double minimal_assignment_probability);

      /** Delete all hits markes as bad in a track. Should be called after every hit reassignment */
      static void deleteAllMarkedHits(TrackCandidate* trackCandidate);

    private:

      static constexpr double m_minimal_assignment_probability = 0.8;
    };
  }
}
