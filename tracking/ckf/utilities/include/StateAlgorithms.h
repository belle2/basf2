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
  /// Calculate the layer this state is located on.
  inline unsigned int extractGeometryLayer(const CKFStateObject<RecoTrack, SpacePoint>& stateObject)
  {
    return static_cast<unsigned int>((static_cast<double>(stateObject.getNumber()) / 2) + 1);
  }

  /// Calculate the layer this state is located on.
  inline unsigned int extractGeometryLayer(const CKFStateObject<RecoTrack, TrackFindingCDC::CDCRLWireHit>& stateObject)
  {
    return 56 - stateObject.getNumber();
  }

  /// Check if this state should describe an overlap hit.
  inline bool isOnOverlapLayer(const CKFStateObject<RecoTrack, SpacePoint>& stateObject)
  {
    return stateObject.getNumber() % 2 == 0;
  }

  inline bool isOnOverlapLayer(const CKFStateObject<RecoTrack, TrackFindingCDC::CDCRLWireHit>& stateObject __attribute__((unused)))
  {
    return false;
  }

  /// Helper functor for extracting the hit
  struct HitGetter {
    /// Marker function for the isFunctor test
    operator TrackFindingCDC::FunctorTag();

    /// from a general state
    template <class AState>
    auto operator()(const AState& state) const -> decltype(state->getHit())
    {
      return state->getHit();
    }
  };

  /// Helper functor for extrating useful ID information
  struct HitIDExtractor {
    /// Marker function for the isFunctor test
    operator TrackFindingCDC::FunctorTag();

    /// ... from a space point
    VxdID operator()(const CKFStateObject<RecoTrack, SpacePoint>* state) const
    {
      const auto* hit = state->getHit();
      if (hit) {
        return hit->getVxdID();
      } else {
        return VxdID();
      }
    }

    /// ... from a wire hit
    const WireID& operator()(const CKFStateObject<RecoTrack, TrackFindingCDC::CDCRLWireHit>* state) const
    {
      return state->getHit()->getWireID();
    }
  };
}