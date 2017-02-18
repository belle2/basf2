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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;
    class CDCTrajectory2D;

    /// Class with high-level track (post)processing.
    class TrackProcessor {

    public:
      /// Create CDCTrack using CDCWireHit hits and store it in the list. Then call the postprocessing on it.
      static void addCandidateFromHitsWithPostprocessing(const std::vector<const CDCWireHit*>& foundAxialWireHits,
                                                         const std::vector<const CDCWireHit*>& allAxialWireHits,
                                                         std::vector<CDCTrack>& axialTracks);

      /// Perform all track postprocessing - return whether the track is considered good after the postprocessing
      static bool postprocessTrack(CDCTrack& track, const std::vector<const CDCWireHit*>& allAxialWireHits);

      /// Check an (improper) p-values of the tracks. If they are below the given value, delete the track from the list.
      static void deleteTracksWithLowFitProbability(std::vector<CDCTrack>& axialTracks,
                                                    double minimal_probability_for_good_fit = 0.4);

      /// Remove tracks that are shorter than the given number of hits.
      static void deleteShortTracks(std::vector<CDCTrack>& axialTracks, double minimal_size = 5);

    private:
      /// Check track quality -- currently based on number of hits only.
      static bool checkTrackQuality(const CDCTrack& track);
    };
  }
}
