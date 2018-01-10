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
  class SpacePoint;
  class CKFToSVDState;
  class RecoTrack;

  class MCUtil {
  public:
    unsigned int getNumberOfCorrectHits(const RecoTrack* mcRecoTrack, const std::vector<const SpacePoint*>& hits) const;

    bool allStatesCorrect(const std::vector<TrackFindingCDC::WithWeight<const CKFToSVDState*>>& states) const;

    bool hitIsCorrect(const RecoTrack* mcRecoTrack, const SpacePoint* spacePoint) const;
  };
}