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

#include <tracking/ckf/utilities/StateAlgorithms.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <framework/logging/Logger.h>

#include <string>

namespace Belle2 {
  class SpacePoint;
  class RecoTrack;
  template <class ASeedObject, class AHitObject> class CKFStateObject;

  namespace TrackFindingCDC {
    class CDCRLWireHit;
  }

  /// Test a given state for correctness.
  template <class AHitObject>
  bool isStateCorrect(const CKFStateObject<RecoTrack, AHitObject>& state)
  {
    RecoTrack* seedTrack = state.getSeedRecoTrack();
    const auto* hit = state.getHit();

    B2ASSERT("State must have a seed", seedTrack);

    const std::string& seedTrackStoreArrayName = seedTrack->getArrayName();

    TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", seedTrackStoreArrayName);
    const RecoTrack* mcTrack = mcCDCMatchLookUp.getMatchedMCRecoTrack(*seedTrack);

    // In case the CDC track is a fake, return false always
    if (not mcTrack) {
      return false;
    }

    if (not hit) {
      // on every second layer (the overlap layers) it is fine to have no space point
      if (isOnOverlapLayer(state)) {
        return true;
      }

      // TODO: include CDC here also
      // it is also fine, if the MC track does not have this layer
      const auto& svdHits = mcTrack->getSVDHitList();
      const auto& pxdHits = mcTrack->getPXDHitList();

      const bool hasSVDLayer = TrackFindingCDC::any(svdHits, [&state](const SVDCluster * cluster) {
        return cluster->getSensorID().getLayerNumber() == extractGeometryLayer(state);
      });
      const bool hasPXDLayer = TrackFindingCDC::any(pxdHits, [&state](const PXDCluster * cluster) {
        return cluster->getSensorID().getLayerNumber() == extractGeometryLayer(state);
      });

      return (not hasSVDLayer and not hasPXDLayer);
    }

    return isCorrectHit(*hit, *mcTrack);
  }

  /// Test this and all parent states for correctness
  template <class AHitObject>
  bool allStatesCorrect(const CKFStateObject<RecoTrack, AHitObject>& state)
  {
    bool oneIsWrong = false;

    const auto& findFalseHit = [&oneIsWrong](const CKFStateObject<RecoTrack, AHitObject>* walkState) {
      if (oneIsWrong) {
        return;
      }

      const bool stateIsCorrect = isStateCorrect(*walkState);
      if (not stateIsCorrect) {
        oneIsWrong = true;
      }
    };

    state.walk(findFalseHit);
    return not oneIsWrong;
  }

  /// Test if a given hit is really attached to this MC track
  extern bool isCorrectHit(const SpacePoint& spacePoint, const RecoTrack& mcRecoTrack);

  /// Test if a given hit is really attached to this MC track
  extern bool isCorrectHit(const TrackFindingCDC::CDCRLWireHit& wireHit, const RecoTrack& mcRecoTrack);

  /// return the number of correct hits in he given hit list (assuming the given MC track)
  template <class AHitType>
  unsigned int getNumberOfCorrectHits(const RecoTrack* mcTrack,
                                      const std::vector<AHitType> hits)
  {
    if (not mcTrack) {
      return 0;
    }

    const unsigned int numberOfCorrectHits = std::count_if(hits.begin(), hits.end(), [mcTrack](const auto * spacePoint) {
      return isCorrectHit(*spacePoint, *mcTrack);
    });

    return numberOfCorrectHits;
  }
}