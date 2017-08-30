/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/utilities/CKFMCUtils.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

namespace Belle2 {
  using namespace TrackFindingCDC;

  bool isCorrectHit(const SpacePoint& spacePoint, const RecoTrack& mcRecoTrack)
  {
    const auto& isSameMCTrack = [&mcRecoTrack](const RecoTrack & clusterRecoTrack) {
      return &clusterRecoTrack == &mcRecoTrack;
    };

    const std::string& mcRecoTrackStoreArrayName = mcRecoTrack.getArrayName();

    if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::SVD) {
      for (const SVDCluster& relatedCluster : spacePoint.getRelationsWith<SVDCluster>()) {
        const auto& relatedMCTracksToCluster = relatedCluster.getRelationsWith<RecoTrack>(mcRecoTrackStoreArrayName);
        if (not TrackFindingCDC::any(relatedMCTracksToCluster, isSameMCTrack)) {
          return false;
        }
      }
    } else if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::PXD) {
      for (const PXDCluster& relatedCluster : spacePoint.getRelationsWith<PXDCluster>()) {
        const auto& relatedMCTracksToCluster = relatedCluster.getRelationsWith<RecoTrack>(mcRecoTrackStoreArrayName);
        if (not TrackFindingCDC::any(relatedMCTracksToCluster, isSameMCTrack)) {
          return false;
        }
      }
    } else {
      B2FATAL("Can not handle unknown type " << spacePoint.getType() << "!");
    }

    // Test if these clusters are on the first half of the track
    // For this, get the reco hit information of the related hit
    const RecoHitInformation* recoHitInformationOfHit;
    if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::SVD) {
      recoHitInformationOfHit = mcRecoTrack.getRecoHitInformation(spacePoint.getRelated<SVDCluster>());
    } else if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::PXD) {
      recoHitInformationOfHit = mcRecoTrack.getRecoHitInformation(spacePoint.getRelated<PXDCluster>());
    } else {
      B2FATAL("Can not handle unknown type " << spacePoint.getType() << "!");
    }

    B2ASSERT("Invalid MC information", recoHitInformationOfHit);

    // then we also need the first exit out of this detector
    const std::vector<RecoHitInformation*> recoHitInformationList = mcRecoTrack.getRecoHitInformations(true);
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
    if (firstHitAfterDetector->getSortingParameter() < recoHitInformationOfHit->getSortingParameter()) {
      // The found hit is a curler! Remove it
      return false;
    }

    return true;
  }

  bool isCorrectHit(const TrackFindingCDC::CDCRLWireHit& wireHit, const RecoTrack& mcRecoTrack)
  {
    // TODO: also check for RL here
    return (mcRecoTrack.getRelated<MCParticle>() and
            mcRecoTrack.getRelated<MCParticle>() == wireHit.getHit()->getRelated<MCParticle>());
  }

  unsigned int getNumberOfCorrectHits(const RecoTrack* mcTrack, const std::vector<const SpacePoint*> spacePoints)
  {
    if (not mcTrack) {
      return 0;
    }

    unsigned int numberOfCorrectHits = 0;

    for (const SpacePoint* spacePoint : spacePoints) {
      if (isCorrectHit(*spacePoint, *mcTrack)) {
        if (spacePoint->getType() == VXD::SensorInfoBase::SensorType::SVD) {
          numberOfCorrectHits += 1;
        } else {
          numberOfCorrectHits += 1;
        }
      }
    }
    return numberOfCorrectHits;
  }

  unsigned int getNumberOfCorrectHits(const RecoTrack* mcTrack, const std::vector<const TrackFindingCDC::CDCRLWireHit*> wireHits)
  {
    if (not mcTrack) {
      return 0;
    }

    unsigned int numberOfCorrectHits = 0;

    for (const CDCRLWireHit* wireHit : wireHits) {
      if (isCorrectHit(*wireHit, *mcTrack)) {
        numberOfCorrectHits++;
      }
    }
    return numberOfCorrectHits;
  }
}