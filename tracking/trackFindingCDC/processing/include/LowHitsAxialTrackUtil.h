/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;

    /**
     * Utility structure gathering heuristic functions used during the search for non-helix or low hit count tracks
     */
    struct LowHitsAxialTrackUtil {
    public:
      /**
       * Create CDCTrack using CDCWireHit hits and store it in the list. Then call the postprocessing on it if need be.
       * Uses extra assumptions like track coming from origin
       */
      static void addCandidateFromHits(const std::vector<const CDCWireHit*>& foundAxialWireHits,
                                       const std::vector<const CDCWireHit*>& allAxialWireHits,
                                       std::vector<CDCTrack>& axialTracks,
                                       bool fromOrigin = true,
                                       bool straight = true,
                                       bool withPostprocessing = true);

      /// Perform all track postprocessing - return whether the track is considered good after the postprocessing
      static bool postprocessTrack(CDCTrack& track, const std::vector<const CDCWireHit*>& allAxialWireHits);
    };
  }
}
