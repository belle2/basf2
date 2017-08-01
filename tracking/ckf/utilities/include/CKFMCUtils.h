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

  namespace TrackFindingCDC {
    class CDCRLWireHit;
  }

  /// Test if a given hit is really attached to this MC track
  extern bool isCorrectHit(const SpacePoint& spacePoint, const RecoTrack& mcRecoTrack);

  /// Test if a given hit is really attached to this MC track
  extern bool isCorrectHit(const TrackFindingCDC::CDCRLWireHit& spacePoint, const RecoTrack& mcRecoTrack);

  /// return the number of correct hits in he given hit list (assuming the given MC track)
  extern unsigned int getNumberOfCorrectHits(const RecoTrack* mcTrack, const std::vector<const SpacePoint*> spacePoints);

  /// return the number of correct hits in he given hit list (assuming the given MC track)
  extern unsigned int getNumberOfCorrectHits(const RecoTrack* mcTrack,
                                             const std::vector<const TrackFindingCDC::CDCRLWireHit*> wireHits);
}