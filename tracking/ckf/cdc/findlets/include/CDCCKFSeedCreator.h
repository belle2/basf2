/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/cdc/entities/CDCCKFPath.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <vector>

namespace Belle2 {

  /// Create a CKF seed based on RecoTrack (presumably from VXDTF2)
  class CDCCKFSeedCreator : public TrackFindingCDC::Findlet<RecoTrack* const, CDCCKFPath> {
  public:
    /// Main method of the findlet, loop over reco tracks, create seeds for each of them.
    void apply(const std::vector<RecoTrack*>& recoTracks, std::vector<CDCCKFPath>& seeds) override
    {
      for (RecoTrack* recoTrack : recoTracks) {
        CDCCKFState seedState(recoTrack, recoTrack->getMeasuredStateOnPlaneFromLastHit());
        seeds.push_back({seedState});
      }
    }
  };
}
