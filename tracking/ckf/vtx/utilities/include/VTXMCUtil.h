/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <vector>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  class SpacePoint;
  class CKFToVTXState;
  class RecoTrack;

  /// Class bundling all helper functions for the MC information used in the VTX CKF.
  class MCUtil {
  public:
    /// How many of the given space points are also in the MC track? Runs vtxHitIsCorrect on all of them.
    unsigned int getNumberOfCorrectVTXHits(const RecoTrack* mcRecoTrack, const std::vector<const SpacePoint*>& hits) const;

    /// Are all hits related to the same MC track the seed is related to?
    bool allStatesCorrect(const std::vector<TrackFindingCDC::WithWeight<const CKFToVTXState*>>& states) const;

    /// Returns true, of the space point is related to the mc track and if it is on the first half.
    bool vtxHitIsCorrect(const RecoTrack* mcRecoTrack, const SpacePoint* spacePoint) const;
  };
}
