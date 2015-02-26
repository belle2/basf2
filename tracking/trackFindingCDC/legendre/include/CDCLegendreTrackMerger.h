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

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackHit;
    class TrackCandidate;

    class TrackMerger {
    public:

      /**
       * This class is to be used static only.
       */
      TrackMerger() = delete;

      /** The track finding often finds two curling tracks, originating from the same particle. This function merges them. */
      static void doTracksMerging(std::list<TrackCandidate*>& trackList);

      /** Try to merge given track with tracks in tracklist. */
      static void tryToMergeTrackWithOtherTracks(TrackCandidate* cand1, std::list<TrackCandidate*>& trackList);

      /** Tries to split back-to-back tracks into two different tracks */
      static TrackCandidate* splitBack2BackTrack(TrackCandidate* trackCandidate);

    private:


      /** Some typedefs for the results of the merging process */
      typedef unsigned int TrackCandidateIndex;
      typedef std::pair<TrackCandidateIndex, TrackCandidate*> TrackCandidateWithIndex;
      typedef double Probability;
      typedef std::pair<TrackCandidate*, Probability> BestMergePartner;

      /**
       * Function to merge two track candidates. The hits of cand2 are deleted and transfered to cand1.
       * The hit sorting is not maintained.
       */
      static void mergeTracks(TrackCandidate* cand1, TrackCandidate* cand2);

      /**
       * Marks hits away from the trajectory as bad. This method is used for calculating the chi2 of the tracks to be merged.
       * @param factor gives a number how far the hit is allowed to be.
       */
      static void removeStrangeHits(double factor, std::vector<TrackHit*>& trackHits, std::pair<double, double>& track_par, std::pair<double , double>& ref_point);

      /** Try to merge the two tracks
       * For this, build a common hit list and do a fast fit.
       * Then, throw away hits with a very high distance and fit again. Repeat this process three times.
       * As a result. the reduced probability for a good fit is given.
       * The bad hits are marked but none of them is deleted!
       * This method does not do the actual merging. */
      static double doTracksFitTogether(TrackCandidate* cand1, TrackCandidate* cand2);

      /**
       * Searches for the best candidate to merge this track to.
       * @param trackCandidateToBeMerged
       * @param start_iterator the iterator where to start searching (this element included)
       * @return a pointer to the best fit candidate.
       */
      static BestMergePartner calculateBestTrackToMerge(TrackCandidate* trackCandidateToBeMerged, std::list<TrackCandidate*>::iterator start_iterator, std::list<TrackCandidate*>::iterator end_iterator);

      /**
       * After the candidate-to-merge finding, some hits are marked as bad. This method resets them.
       * @param otherTrackCandidate to reset
       */
      static void resetHits(TrackCandidate* otherTrackCandidate);

      /**
       * This parameter is the minimum probability a fit must have to lead to the result: merge
       */
      static constexpr double m_minimum_probability_to_be_merged = 0.3;
    };
  }
}
