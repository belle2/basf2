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
  class RecoTrack;

  extern bool isCorrectSpacePoint(const SpacePoint& spacePoint, const RecoTrack& mcRecoTrack);

  extern unsigned int getNumberOfCorrectHits(const RecoTrack* mcTrack, const std::vector<const SpacePoint*> spacePoints);
}