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

#include <vector>
#include <list>


namespace Belle2 {

  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCWireHit;
    class CDCTrajectory2D;

    /// Class with high-level track (post)processing.
    class TrackProcessor {
    public:
      /// Static use only.
      TrackProcessor() = delete;

      /// Do not copy this class.
      TrackProcessor(const TrackProcessor& copy) = delete;

      /// Do not copy this class.
      TrackProcessor& operator=(const TrackProcessor&) = delete;

      /// Create CDCTrack using CDCWireHit hits and store it in the list. Then call the postprocessing on it.
      static void addCandidateFromHitsWithPostprocessing(const std::vector<const CDCWireHit*>& foundAxialWireHits,
                                                         const std::vector<const CDCWireHit*>& allAxialWireHits,
                                                         std::list<CDCTrack>& cdcTrackList);

      /// Assign new hits to all tracks (using the assignNewHitsToTrack(CDCTrack&) method of the HitsProcessor).
      static void assignNewHits(const std::vector<const CDCWireHit*>& allWireHits, std::list<CDCTrack>& cdcTrackList);

      /// Perform all track postprocessing - return whether the track is considered good after the postprocessing
      static bool postprocessTrack(CDCTrack& track, const std::vector<const CDCWireHit*>& allAxialWireHits);

      /// Finalize the tracks after the legendre track finder is done - includes a merging step of found tracks
      static void mergeAndFinalizeTracks(std::list<CDCTrack>& cdcTrackList,
                                         const std::vector<const CDCWireHit*>& allWireHits);

      /// Check an (improper) p-values of the tracks. If they are below the given value, delete the track from the list.
      static void deleteTracksWithLowFitProbability(std::list<CDCTrack>& cdcTrackList,
                                                    double minimal_probability_for_good_fit = 0.4);

    private:
      /// Check track quality -- currently based on number of hits only.
      static bool checkTrackQuality(const CDCTrack& track);
    };
  }
}
