/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/utilities/PXDMCUtil.h>

#include <tracking/ckf/pxd/entities/CKFToPXDState.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <functional>

using namespace Belle2;

bool MCUtil::pxdHitIsCorrect(const RecoTrack* mcRecoTrack, const SpacePoint* spacePoint) const
{
  const auto isSameMCTrack = [&mcRecoTrack](const RecoTrack & clusterRecoTrack) {
    return &clusterRecoTrack == mcRecoTrack;
  };


  const std::string& mcRecoTrackStoreArrayName = mcRecoTrack->getArrayName();

  for (const PXDCluster& relatedCluster : spacePoint->getRelationsTo<PXDCluster>()) {
    const auto& relatedMCTracksToCluster = relatedCluster.getRelationsTo<RecoTrack>(mcRecoTrackStoreArrayName);
    if (not TrackFindingCDC::any(relatedMCTracksToCluster, isSameMCTrack)) {
      return false;
    }
  }

  // Test if these clusters are on the first half of the track
  // For this, get the reco hit information of the related hit
  const RecoHitInformation* recoHitInformationOfHit = mcRecoTrack->getRecoHitInformation(spacePoint->getRelatedTo<PXDCluster>());
  B2ASSERT("Invalid MC information", recoHitInformationOfHit);

  // then we also need the first exit out of this detector
  const std::vector<RecoHitInformation*> recoHitInformationList = mcRecoTrack->getRecoHitInformations(true);
  // For this, we get an iterator into the first entry (there must be at least one hit in this detector, so we are safe)
  const auto& detector = recoHitInformationOfHit->getTrackingDetector();

  const auto& itToFirstEntryInDetector = std::find_if(recoHitInformationList.begin(), recoHitInformationList.end(),
  [detector](RecoHitInformation * hitInformation) {
    return hitInformation->getTrackingDetector() == detector;
  });
  const auto& itToFirstEntryAfterDetector = std::find_if(itToFirstEntryInDetector, recoHitInformationList.end(),
  [detector](RecoHitInformation * hitInformation) {
    return hitInformation->getTrackingDetector() != detector;
  });

  if (itToFirstEntryAfterDetector == recoHitInformationList.end()) {
    // This is a really strange case: it should actually not be possible to find such a hit. Better
    // remove it.
    return false;
  }

  const auto* firstHitAfterDetector = *itToFirstEntryAfterDetector;
  return firstHitAfterDetector->getSortingParameter() >= recoHitInformationOfHit->getSortingParameter();
}

unsigned int MCUtil::getNumberOfCorrectPXDHits(const RecoTrack* mcRecoTrack, const std::vector<const SpacePoint*>& hits) const
{

  const auto hitIsCorrectSpecialisation = [this, mcRecoTrack](const SpacePoint * spacePoint) {
    return pxdHitIsCorrect(mcRecoTrack, spacePoint);
  };

  const unsigned int numberOfCorrectHits = std::count_if(hits.begin(), hits.end(), hitIsCorrectSpecialisation);
  return numberOfCorrectHits;
}

bool MCUtil::allStatesCorrect(const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>& states) const
{
  const RecoTrack* seed = states.front()->getSeed();

  B2ASSERT("Path without a seed?", seed);

  if (states.size() <= 1) {
    // just the seed? this can not be correct...
    return false;
  }

  const std::string& seedTrackStoreArrayName = seed->getArrayName();

  TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", seedTrackStoreArrayName);
  const RecoTrack* mcTrack = mcCDCMatchLookUp.getRelatedMCRecoTrack(*seed);

  if (not mcTrack) {
    // Track is a fake
    B2DEBUG(100, "Seed is a fake");
    return false;
  }

  std::vector<const SpacePoint*> spacePoints;
  for (const CKFToPXDState* state : states) {
    const SpacePoint* spacePoint = state->getHit();
    if (spacePoint) {
      spacePoints.push_back(spacePoint);
    }
  }

  const unsigned int numberOfCorrectHits = getNumberOfCorrectPXDHits(mcTrack, spacePoints);

  B2DEBUG(100, "Have found " << numberOfCorrectHits << " correct out of  " << spacePoints.size() << " hits");
  return numberOfCorrectHits == spacePoints.size();
}