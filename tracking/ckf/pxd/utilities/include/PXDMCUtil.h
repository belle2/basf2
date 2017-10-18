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

namespace Belle2 {
  class SpacePoint;
  class CKFToPXDState;
  class RecoTrack;

  class MCUtil {
  public:
    unsigned int getNumberOfCorrectPXDHits(const RecoTrack* mcRecoTrack, const std::vector<const SpacePoint*>& hits) const;

    bool allStatesCorrect(const std::vector<const CKFToPXDState*>& states) const;

    bool pxdHitIsCorrect(const RecoTrack* mcRecoTrack, const SpacePoint* spacePoint) const;
  };
}