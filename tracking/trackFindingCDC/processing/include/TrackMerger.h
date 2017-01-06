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

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <list>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCWireHit;
    class CDCTrajectory2D;

    /// Utility structure implementing functionality for the axial track merging used after the legendre tree search.
    class TrackMerger {
    public:
      /// Static class only.
      TrackMerger() = delete;

      /// Static class only.
      TrackMerger(const TrackMerger& copy) = delete;

      /// Static class only.
      TrackMerger& operator=(const TrackMerger&) = delete;

      /**
       *  The track finding often finds two curling tracks, originating from the same particle.
       *  This function merges them.
       */
      static void doTracksMerging(std::list<CDCTrack>& cdcTrackList,
                                  const std::vector<const CDCWireHit*>& allAxialWireHits,
                                  double minimum_probability_to_be_merged = 0.85);

      /// Try to merge given track with tracks in tracklist.
      static void tryToMergeTrackWithOtherTracks(CDCTrack& track,
                                                 std::list<CDCTrack>& cdcTrackList,
                                                 const std::vector<const CDCWireHit*>& allAxialWireHits,
                                                 double minimum_probability_to_be_merged = 0.85);

    private:
      /**
       *  Function to merge two track candidates. The hits of cand2 are deleted and transfered to cand1.
       *  The hit sorting is not maintained.
       */
      static void mergeTracks(CDCTrack& track1,
                              CDCTrack& track2,
                              const std::vector<const CDCWireHit*>& allAxialWireHits,
                              std::list<CDCTrack>& cdcTrackList);

      /**
       *  Marks hits away from the trajectory as bad. This method is used for calculating the chi2 of the tracks to be merged.
       *
       *  @param factor gives a number how far the hit is allowed to be.
       */
      static void removeStrangeHits(double factor, std::vector<const CDCWireHit*>& wireHits, CDCTrajectory2D& trajectory);

      /**
       *  Try to merge the two tracks
       *  For this, build a common hit list and do a fast fit.
       *  Then, throw away hits with a very high distance and fit again. Repeat this process three times.
       *  As a result. the reduced probability for a good fit is given.
       *  The bad hits are marked but none of them is deleted!
       *  This method does not do the actual merging.
       */
      static double doTracksFitTogether(CDCTrack& track1, CDCTrack& track2);

      /**
       *  Searches for the best candidate to merge this track to.
       *  @param track   track for which we try to find merging partner
       *  @param tracks  search range of tracks
       *  @return a pointer to the best fit candidate including a fit probability
       */
      template <class ACDCTracks>
      static WithWeight<CDCTrack*> calculateBestTrackToMerge(CDCTrack& track, ACDCTracks& tracks);
    };
  }
}
