/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Simon Kurz, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/cdc/entities/CDCCKFPath.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <vector>

namespace Belle2 {

  /// Create a CKF seed based on RecoTrack (presumably from VXDTF2)
  class CDCfromEclCKFSeedCreator : public TrackFindingCDC::Findlet<RecoTrack* const, CDCCKFPath> {
  public:
    /// Main method of the findlet, loop over reco tracks, create seeds for each of them.
    void apply(const std::vector<RecoTrack*>& recoTracks, std::vector<CDCCKFPath>& seeds) override
    {
      for (RecoTrack* recoTrack : recoTracks) {
        // This does not work as CDCCKFStateFilter needs genfit::MeasuredStateOnPlane to create TrackFindingCDC::CDCTrajectory3D
        CDCCKFState seedState(recoTrack, nullptr);
        seeds.push_back({seedState});
      }
    }
  };
}
