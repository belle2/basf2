/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/vxdToCDC/state/CKFVXDToWireHitStateTruthVarSet.h>
#include <tracking/ckf/utilities/CKFMCUtils.h>
#include <tracking/ckf/utilities/StateAlgorithms.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool CKFVXDToWireHitStateTruthVarSet::extract(const BaseCKFCKFVXDToWireHitStateFilter::Object* result)
{
  RecoTrack* vxdTrack = result->getSeedRecoTrack();
  const CDCRLWireHit* wireHit = result->getHit();

  if (not vxdTrack) return false;

  StoreObjPtr<EventMetaData> eventMetaData;
  var<named("truth_event_id")>() = eventMetaData->getEvent();
  var<named("truth_vxd_number")>() = vxdTrack->getArrayIndex();

  const std::string& vxdTrackStoreArrayName = vxdTrack->getArrayName();

  TrackMatchLookUp mcVXDMatchLookUp("MCRecoTracks", vxdTrackStoreArrayName);
  const RecoTrack* vxdMCTrack = mcVXDMatchLookUp.getMatchedMCRecoTrack(*vxdTrack);

  // Default to 0, -1 or false (depending on context)
  var<named("truth_position_x")>() = 0;
  var<named("truth_position_y")>() = 0;
  var<named("truth_position_z")>() = 0;
  var<named("truth_momentum_x")>() = 0;
  var<named("truth_momentum_y")>() = 0;
  var<named("truth_momentum_z")>() = 0;
  var<named("truth")>() = false;
  var<named("truth_cdc_hit_number")>() = -1;

  // In case the CDC track is a fake, return false always
  if (not vxdMCTrack) {
    return true;
  }

  if (not wireHit) {
    var<named("truth")>() = true;
    return true;
  }

  var<named("truth_cdc_hit_number")>() = wireHit->getHit()->getArrayIndex();

  if (not isCorrectHit(*wireHit, *vxdMCTrack)) {
    // Keep all variables set to false and return.
    return true;
  }

  var<named("truth")>() = true;

  var<named("truth_position_x")>() = vxdMCTrack->getPositionSeed().X();
  var<named("truth_position_y")>() = vxdMCTrack->getPositionSeed().Y();
  var<named("truth_position_z")>() = vxdMCTrack->getPositionSeed().Z();
  var<named("truth_momentum_x")>() = vxdMCTrack->getMomentumSeed().X();
  var<named("truth_momentum_y")>() = vxdMCTrack->getMomentumSeed().Y();
  var<named("truth_momentum_z")>() = vxdMCTrack->getMomentumSeed().Z();

  return true;
}
