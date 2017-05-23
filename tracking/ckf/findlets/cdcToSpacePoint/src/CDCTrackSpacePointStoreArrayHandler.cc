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

  // Create a list of cdc-track - svd cluster list pairs
  std::vector<std::pair<const RecoTrack*, std::pair<TVector3, std::vector<const SVDCluster*>>>> seedsWithPositionAndHits;
  seedsWithPositionAndHits.reserve(cdcTracksWithMatchedSpacePoints.size());

  // Create new VXD tracks out of the found VXD space points and store them into a store array
  for (const auto& cdcTrackWithMatchedSpacePoints : cdcTracksWithMatchedSpacePoints) {
    RecoTrack* cdcRecoTrack = cdcTrackWithMatchedSpacePoints.first;
    if (not cdcRecoTrack) {
      continue;
    }

    const std::vector<const SpacePoint*> matchedSpacePoints = cdcTrackWithMatchedSpacePoints.second;
    if (matchedSpacePoints.empty()) {
      continue;
    }

    const TVector3& vxdPosition = matchedSpacePoints.front()->getPosition();
    seedsWithPositionAndHits.emplace_back(cdcRecoTrack, std::make_pair(vxdPosition, std::vector<const SVDCluster*>()));
    auto& hits = seedsWithPositionAndHits.back().second.second;

    for (const auto& spacePoint : matchedSpacePoints) {
      if (spacePoint) {
        const auto& relatedClusters = spacePoint->getRelationsWith<SVDCluster>();
        for (const SVDCluster& relatedCluster : relatedClusters) {
          hits.push_back(&relatedCluster);
        }
      }
    }
  }

  combine(seedsWithPositionAndHits);
}