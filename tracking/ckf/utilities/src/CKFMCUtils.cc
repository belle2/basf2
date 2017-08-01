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

    return true;
  }

  bool isCorrectHit(const TrackFindingCDC::CDCRLWireHit& wireHit, const RecoTrack& mcRecoTrack)
  {
    return mcRecoTrack.getRelated<MCParticle>() == CDCMCHitLookUp::getInstance().getMCParticle(wireHit.getHit()) and
           CDCMCHitLookUp::getInstance().getRLInfo(wireHit.getHit()) == wireHit.getRLInfo();
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
          numberOfCorrectHits += 2;
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