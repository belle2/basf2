/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/findlets/cdcToSpacePoint/CDCTrackSpacePointStoreArrayHandler.h>

using namespace Belle2;

void CDCTrackSpacePointStoreArrayHandler::apply(const std::vector<CKFCDCToVXDStateObject::ResultObject>&
                                                cdcTracksWithMatchedSpacePoints)
{
  if (not m_param_exportTracks) {
    return;
  }

  // Create new VXD tracks out of the found VXD space points and store them into a store array
  for (const auto& cdcTrackWithMatchedSpacePoints : cdcTracksWithMatchedSpacePoints) {
    RecoTrack* cdcRecoTrack = cdcTrackWithMatchedSpacePoints.first;

    const std::vector<const SpacePoint*> matchedSpacePoints = cdcTrackWithMatchedSpacePoints.second;
    if (matchedSpacePoints.empty()) {
      continue;
    }

    const TVector3& vxdPosition = matchedSpacePoints.front()->getPosition();

    TVector3 trackMomentum;
    int trackCharge;

    extrapolateMomentum(*cdcRecoTrack, vxdPosition, trackMomentum, trackCharge);

    RecoTrack* newRecoTrack = m_vxdRecoTracks.appendNew(vxdPosition, trackMomentum, trackCharge);

    unsigned int hitCounter = 0;
    for (const auto& spacePoint : matchedSpacePoints) {
      if (spacePoint) {
        const auto& relatedClusters = spacePoint->getRelationsWith<SVDCluster>();
        newRecoTrack->addSVDHit(relatedClusters[0], hitCounter);
        newRecoTrack->addSVDHit(relatedClusters[1], hitCounter + 1);
        hitCounter += 2;
      }
    }

    cdcRecoTrack->addRelationTo(newRecoTrack);
  }

  Super::relateAndCombineTracks();
}