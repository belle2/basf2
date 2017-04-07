/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/cdcToVXDExtrapolator/CDCTrackSpacePointCombinationTruthVarSet.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool CDCTrackSpacePointCombinationTruthVarSet::extract(const BaseCDCTrackSpacePointCombinationFilter::Object* result)
{
  RecoTrack* cdcTrack = result->getSeedRecoTrack();
  const SpacePoint* spacePoint = result->getSpacePoint();

  if (not cdcTrack or not spacePoint) return false;

  const std::string& cdcTrackStoreArrayName = cdcTrack->getArrayName();

  TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", cdcTrackStoreArrayName);
  const RecoTrack* cdcMCTrack = mcCDCMatchLookUp.getMatchedMCRecoTrack(*cdcTrack);

  // In case the CDC track is a fake, return false always
  if (not cdcMCTrack) {
    var<named("truth")>() = false;
    return true;
  }

  // Next we have to get all MC tracks for the two SVD Clusters matches to the SpacePoint
  const auto& relatedSVDClusters = spacePoint->getRelationsWith<SVDCluster>();

  B2ASSERT("SpacePoint is related to more than two SVDClusters!", relatedSVDClusters.size() == 2);

  for (const auto& relatedSVDCluster : relatedSVDClusters) {
    const auto& relatedMCRecoTracksToOneCluster = relatedSVDCluster.getRelationsWith<RecoTrack>("MCRecoTracks");

    bool cdcTrackMatchedToCluster = std::any_of(relatedMCRecoTracksToOneCluster.begin(),
    relatedMCRecoTracksToOneCluster.end(), [cdcMCTrack](const RecoTrack & mcRecoTrack) {
      return &mcRecoTrack == cdcMCTrack;
    });

    if (not cdcTrackMatchedToCluster) {
      var<named("truth")>() = false;
      return true;
    }
  }

  // Test if these clusters are on the first half of the track
  // The track needs to have some SVD hits, so we can savely access the first element
  const auto& recoHitInformations = cdcMCTrack->getRecoHitInformations(true);

  const auto& isCDCHit = [](RecoHitInformation * recoHitInformation) {
    return recoHitInformation->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_CDC;
  };

  const RecoHitInformation* firstCDCHitInformation = *(std::find_if(recoHitInformations.begin(), recoHitInformations.end(),
                                                       isCDCHit));
  const RecoHitInformation* firstClusterInformation = cdcMCTrack->getRecoHitInformation(relatedSVDClusters[0]);

  if (firstCDCHitInformation->getSortingParameter() < firstClusterInformation->getSortingParameter()) {
    var<named("truth")>() = false;
    return true;
  }

  var<named("truth")>() = true;
  return true;
}
