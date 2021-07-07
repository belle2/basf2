/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
