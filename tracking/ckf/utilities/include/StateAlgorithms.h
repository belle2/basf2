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
#include <tracking/trackFindingCDC/utilities/EnableIf.h>

namespace Belle2 {
  /// Shortcut to make compilation dependent on state content for space point-like states
  template<class AState>
  using EnableIfSpacePoint =
    TrackFindingCDC::EnableIf<std::is_same<decltype(std::declval<AState>().getHit()), const SpacePoint*>::value>;

  /// Shortcut to make compilation dependent on state content for cdc wire hit-like states
  template<class AState>
  using EnableIfCDCWireHit =
    TrackFindingCDC::EnableIf<std::is_same<decltype(std::declval<AState>().getHit()), const TrackFindingCDC::CDCRLWireHit*>::value>;

  struct GeometryLayerExtractor {
    /// Marker function for the isFunctor test
    operator TrackFindingCDC::FunctorTag();

    /// Calculate the layer this state is located on from a space point.
    template<class AState>
    unsigned int operator()(const AState& stateObject, EnableIfSpacePoint<AState>* = 0)
    {
      return static_cast<unsigned int>((static_cast<double>(stateObject.getNumber()) / 2) + 1);
    }

    /// Calculate the layer this state is located on for wire hits.
    template<class AState>
    unsigned int operator()(const AState& stateObject, EnableIfCDCWireHit<AState>* = 0)
    {
      return 56 - stateObject.getNumber();
    }
  };

  struct OverlapExtractor {
    /// Marker function for the isFunctor test
    operator TrackFindingCDC::FunctorTag();

    /// Check if this state should describe an overlap hit.
    template<class AState>
    bool operator()(const AState& stateObject, EnableIfSpacePoint<AState>* = 0)
    {
      return stateObject.getNumber() % 2 == 0;
    }

    /// Check if this state should describe an overlap hit. Always false for CDC hits
    template<class AState>
    bool operator()(const AState& stateObject __attribute__((unused)), EnableIfCDCWireHit<AState>* = 0)
    {
      return false;
    }
  };

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

  /// Helper functor for extracting useful ID information
  struct HitIDExtractor {
    /// Marker function for the isFunctor test
    operator TrackFindingCDC::FunctorTag();

    /// ... from a space point
    template<class AState, class SFINAE = EnableIfSpacePoint<AState>>
    auto operator()(const AState* state) const -> decltype(state->getHit()->getVxdID())
    {
      const auto* hit = state->getHit();
      if (hit) {
        return hit->getVxdID();
      } else {
        return VxdID();
      }
    }

    /// ... from a wire hit
    template<class AState, class SFINAE = EnableIfCDCWireHit<AState>>
    auto operator()(const AState* state) const -> decltype(state->getHit()->getWireID())
    {
      return state->getHit()->getWireID();
    }
  };
}