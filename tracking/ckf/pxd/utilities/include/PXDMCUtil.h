/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  class SpacePoint;
  class CKFToPXDState;
  class RecoTrack;

  /// Class bundling all helper functions for the MC information used in the PXD CKF.
  class MCUtil {
  public:
    /// How many of the given space points are also in the MC track? Runs pxdHitIsCorrect on all of them.
    unsigned int getNumberOfCorrectPXDHits(const RecoTrack* mcRecoTrack, const std::vector<const SpacePoint*>& hits) const;

    /// Are all hits related to the same MC track the seed is related to?
    bool allStatesCorrect(const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>& states) const;

    /// Returns true, of the space point is related to the mc track and if it is on the first half.
    bool pxdHitIsCorrect(const RecoTrack* mcRecoTrack, const SpacePoint* spacePoint) const;
  };
}