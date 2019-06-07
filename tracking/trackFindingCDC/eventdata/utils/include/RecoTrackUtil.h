/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Dmitrii Neverov                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class CDCHit;
  class RecoTrack;

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCTrajectory3D;

    /// Structure to summarize utility function to output a list of hits into a RecoTrack.
    struct RecoTrackUtil {

      // For magnetic monopoles; estimates charge sign from all stereo hits, momentum direction from hits in closest superlayer, fixing magnitude with given value. Ignores building CDCTrajectory3D since it is based on a helix.
      static RecoTrack* storeInto(const CDCTrack& track, StoreArray<RecoTrack>& recoTracks, const double momentumSeedMagnitude);

      static RecoTrack* storeInto(const CDCTrack& track, StoreArray<RecoTrack>& recoTracks);

      /// Copies the trajectory information to the RecoTrack
      static RecoTrack* storeInto(const CDCTrajectory3D& traj3D, StoreArray<RecoTrack>& recoTracks);

      /// Copies the trajectory information to the RecoTrack
      static RecoTrack*
      storeInto(const CDCTrajectory3D& traj3D, const double bZ, StoreArray<RecoTrack>& recoTracks);

      /**
       *  Translates a range of hits and inserts them in the reco track.
       *
       *  Currently only instantiated for RecoTrack.
       *
       *  @param rlWireHitHolders  A range of hits, which elements support ->getRLWireHit()
       *                           to access a wire hit including a right left passage hypotheses.
       *  @param[out] recoTrack    RecoTrack to be filled.
       */
      template<class ARLHitHolderRange>
      static void fill(const ARLHitHolderRange& rlWireHitHolders, RecoTrack& recoTrack);

    };
  }
}
