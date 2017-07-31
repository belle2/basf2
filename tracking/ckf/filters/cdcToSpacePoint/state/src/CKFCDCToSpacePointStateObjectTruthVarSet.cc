/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectTruthVarSet.h>
#include <tracking/ckf/utilities/CKFMCUtils.h>
#include <tracking/ckf/utilities/StateAlgorithms.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool CKFCDCToSpacePointStateObjectTruthVarSet::extract(const BaseCKFCDCToSpacePointStateObjectFilter::Object* result)
{
  RecoTrack* cdcTrack = result->getSeedRecoTrack();
  const SpacePoint* spacePoint = result->getHit();

  if (not cdcTrack) return false;

  StoreObjPtr<EventMetaData> eventMetaData;
  var<named("event_id")>() = eventMetaData->getEvent();
  var<named("cdc_number")>() = cdcTrack->getArrayIndex();

  const std::string& cdcTrackStoreArrayName = cdcTrack->getArrayName();

  TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", cdcTrackStoreArrayName);
  const RecoTrack* cdcMCTrack = mcCDCMatchLookUp.getMatchedMCRecoTrack(*cdcTrack);

  // Default to 0, -1 or false (depending on context)
  var<named("truth_position_x")>() = 0;
  var<named("truth_position_y")>() = 0;
  var<named("truth_position_z")>() = 0;
  var<named("truth_momentum_x")>() = 0;
  var<named("truth_momentum_y")>() = 0;
  var<named("truth_momentum_z")>() = 0;
  var<named("truth")>() = false;
  var<named("truth_no_curler")>() = false;
  var<named("space_point_number")>() = -1;

  // In case the CDC track is a fake, return false always
  if (not cdcMCTrack) {
    return true;
  }

  if (not spacePoint) {
    // on every second layer (the overlap layers) it is fine to have no space point
    if (isOnOverlapLayer(*result)) {
      var<named("truth")>() = true;
      return true;
    } else {
      return false;
    }
  }

  var<named("space_point_number")>() = spacePoint->getArrayIndex();

  if (not isCorrectSpacePoint(*spacePoint, *cdcMCTrack)) {
    // Keep all variables set to false and return.
    return true;
  }

  var<named("truth_no_curler")>() = true;

  // Test if these clusters are on the first half of the track
  // The track needs to have some hits, so we can savely access the first element
  const auto& recoHitInformations = cdcMCTrack->getRecoHitInformations(true);

  const auto& isCDCHit = [](RecoHitInformation * recoHitInformation) {
    return recoHitInformation->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_CDC;
  };

  const RecoHitInformation* firstCDCHitInformation = *(std::find_if(recoHitInformations.begin(), recoHitInformations.end(),
                                                       isCDCHit));

  const RecoHitInformation* firstClusterInformation;
  if (spacePoint->getType() == VXD::SensorInfoBase::SensorType::SVD) {
    const auto& relatedSVDClusters = spacePoint->getRelationsWith<SVDCluster>();
    firstClusterInformation = cdcMCTrack->getRecoHitInformation(relatedSVDClusters[0]);
  } else {
    firstClusterInformation = cdcMCTrack->getRecoHitInformation(spacePoint->getRelated<PXDCluster>());
  }

  if (firstCDCHitInformation->getSortingParameter() < firstClusterInformation->getSortingParameter()) {
    return true;
  }

  var<named("truth")>() = true;

  var<named("truth_position_x")>() = cdcMCTrack->getPositionSeed().X();
  var<named("truth_position_y")>() = cdcMCTrack->getPositionSeed().Y();
  var<named("truth_position_z")>() = cdcMCTrack->getPositionSeed().Z();
  var<named("truth_momentum_x")>() = cdcMCTrack->getMomentumSeed().X();
  var<named("truth_momentum_y")>() = cdcMCTrack->getMomentumSeed().Y();
  var<named("truth_momentum_z")>() = cdcMCTrack->getMomentumSeed().Z();

  return true;
}
