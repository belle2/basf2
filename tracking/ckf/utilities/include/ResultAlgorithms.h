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

#include <tracking/ckf/states/CKFStateObject.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
   * Helper functor for adding hits of a certain type to a given reco hit.
   */
  class RecoTrackHitsAdder {
  public:
    /// Add the hits in the list to the given reco track - no matter what type of hits.
    template<class Ts>
    static void addHitsToRecoTrack(const Ts& matchedHits, RecoTrack& newRecoTrack)
    {
      unsigned int sortingParameter = 0;
      for (const auto& hit : matchedHits) {
        addHitToRecoTrack(hit, newRecoTrack, sortingParameter);
      }
    }
  private:
    /// Helper function to add a single hit to a given RecoTrack - general form
    template <class AHit>
    static void addHitToRecoTrack(const AHit& hit, RecoTrack& newRecoTrack, unsigned int& sortingParameter);
  };


  /// Specialisation for space points.
  template <>
  void RecoTrackHitsAdder::addHitToRecoTrack(const SpacePoint* const& spacePoint, RecoTrack& newRecoTrack,
                                             unsigned int& sortingParameter);
}
