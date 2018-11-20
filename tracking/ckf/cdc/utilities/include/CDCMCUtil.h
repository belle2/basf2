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

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <vector>

namespace Belle2 {
  class CDCCKFState;
  class RecoTrack;

  namespace TrackFindingCDC {
    class CDCRLWireHit;
  }

  /// Class bundling all helper functions for the MC information used in the CDC CKF.
  class MCUtil {
  public:
    /// How many of the given CDC wire hits are also in the MC track? Runs hitIsCorrect on all of them.
    unsigned int getNumberOfCorrectHits(const RecoTrack* mcRecoTrack,
                                        const std::vector<const TrackFindingCDC::CDCRLWireHit>& hits) const;

    /// Are all hits related to the same MC track the seed is related to?
    bool allStatesCorrect(const std::vector<CDCCKFState>& states) const;

    /// Returns true, of the wire hit is related to the mc track and if it is on the first half.
    bool hitIsCorrect(const RecoTrack* mcRecoTrack, const TrackFindingCDC::CDCRLWireHit& rlWireHit) const;
  };
}
