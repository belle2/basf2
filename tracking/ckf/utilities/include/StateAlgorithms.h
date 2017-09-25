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
  /// Shortcut to make compilation dependent on state content for space point-like states (SFINAE)
  template<class AState>
  using EnableIfSpacePoint =
    TrackFindingCDC::EnableIf<std::is_same<decltype(std::declval<AState>().getHit()), const SpacePoint*>::value>;

  /// Shortcut to make compilation dependent on state content for cdc wire hit-like states (SFINAE)
  template<class AState>
  using EnableIfCDCWireHit =
    TrackFindingCDC::EnableIf<std::is_same<decltype(std::declval<AState>().getHit()), const TrackFindingCDC::CDCRLWireHit*>::value>;

  /// Functor for extracting the geometrical layer of a state
  struct GeometryLayerExtractor {
    /// Marker function for the isFunctor test
    operator TrackFindingCDC::FunctorTag();

    /// Calculate the layer this state is located on from a space point. (SFINAE)
    template<class AState>
    unsigned int operator()(const AState& stateObject, EnableIfSpacePoint<AState>* = 0)
    {
      return static_cast<unsigned int>((static_cast<double>(stateObject.getNumber()) / 2) + 1);
    }

    /// Calculate the layer this state is located on for wire hits. (SFINAE)
    template<class AState>
    unsigned int operator()(const AState& stateObject, EnableIfCDCWireHit<AState>* = 0)
    {
      return 56 - stateObject.getNumber();
    }
  };

  /// Functor for returning true, if the state is on an overlap layer
  struct OverlapExtractor {
    /// Marker function for the isFunctor test
    operator TrackFindingCDC::FunctorTag();

    /// Check if this state should describe an overlap hit. (SFINAE)
    template<class AState>
    bool operator()(const AState& stateObject, EnableIfSpacePoint<AState>* = 0)
    {
      return stateObject.getNumber() % 2 == 0;
    }

    /// Check if this state should describe an overlap hit. Always false for CDC hits (SFINAE)
    template<class AState>
    bool operator()(const AState& stateObject __attribute__((unused)), EnableIfCDCWireHit<AState>* = 0)
    {
      return false;
    }
  };

  /// Helper functor for extracting useful ID information
  struct HitIDExtractor {
    /// Marker function for the isFunctor test
    operator TrackFindingCDC::FunctorTag();

    /// ... from a space point (SFINAE)
    template<class AState>
    auto operator()(const AState* state, EnableIfSpacePoint<AState>* = 0) const
    {
      const auto* hit = state->getHit();
      if (hit) {
        return hit->getVxdID();
      } else {
        return VxdID();
      }
    }

    /// ... from a wire hit (SFINAE)
    template<class AState>
    auto operator()(const AState* state, EnableIfCDCWireHit<AState>* = 0) const
    {
      return state->getHit()->getWireID();
    }
  };

  /// Helper function to extract the numbered pt-range out of a momentum vector
  inline unsigned int getPTRange(const TrackFindingCDC::Vector3D& momentum)
  {
    const double pT = momentum.xy().norm();
    if (pT > 0.4) {
      return 0;
    } else if (pT > 0.2) {
      return 1;
    } else {
      return 2;
    }
  }
}