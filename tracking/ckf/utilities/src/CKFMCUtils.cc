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
#include <tracking/dataobjects/RecoHitInformation.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/geometry/SensorInfoBase.h>

namespace Belle2 {
  using namespace TrackFindingCDC;

  bool isCorrectHit(const SpacePoint& spacePoint, const RecoTrack& mcRecoTrack)
  {
    const auto& isSameMCTrack = [&mcRecoTrack](const RecoTrack & clusterRecoTrack) {
      return &clusterRecoTrack == &mcRecoTrack;
    };

    const std::string& mcRecoTrackStoreArrayName = mcRecoTrack.getArrayName();

    if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::SVD) {
      for (const SVDCluster& relatedCluster : spacePoint.getRelationsTo<SVDCluster>()) {
        const auto& relatedMCTracksToCluster = relatedCluster.getRelationsTo<RecoTrack>(mcRecoTrackStoreArrayName);
        if (not TrackFindingCDC::any(relatedMCTracksToCluster, isSameMCTrack)) {
          return false;
        }
      }
    } else if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::PXD) {
      for (const PXDCluster& relatedCluster : spacePoint.getRelationsTo<PXDCluster>()) {
        const auto& relatedMCTracksToCluster = relatedCluster.getRelationsTo<RecoTrack>(mcRecoTrackStoreArrayName);
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
      recoHitInformationOfHit = mcRecoTrack.getRecoHitInformation(spacePoint.getRelatedTo<SVDCluster>());
    } else if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::PXD) {
      recoHitInformationOfHit = mcRecoTrack.getRecoHitInformation(spacePoint.getRelatedTo<PXDCluster>());
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
    return firstHitAfterDetector->getSortingParameter() >= recoHitInformationOfHit->getSortingParameter();
  }

  bool isCorrectHit(const TrackFindingCDC::CDCRLWireHit& wireHit, const RecoTrack& mcRecoTrack)
  {
    // TODO: also check for RL here
    return (mcRecoTrack.getRelated<MCParticle>() and
            mcRecoTrack.getRelated<MCParticle>() == wireHit.getHit()->getRelated<MCParticle>());
  }
}