/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcToVXDExtrapolator/filterBased/CDCTrackSpacePointCombinationVarSet.h>
#include <tracking/trackFindingCDC/numerics/ToFinite.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <TMath.h>

#include <framework/dataobjects/Helix.h>
#include <geometry/bfieldmap/BFieldMap.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool CDCTrackSpacePointCombinationVarSet::extract(const BaseCDCTrackSpacePointCombinationFilter::Object* result)
{
  RecoTrack* cdcTrack = result->getSeedRecoTrack();
  const SpacePoint* spacePoint = result->getSpacePoint();

  // TODO: Do not dismiss spacePoint = 0 cases!
  if (not cdcTrack or not spacePoint) {
    return false;
  }

  if (not cdcTrack->wasFitSuccessful()) {
    return false;
  }

  const auto& firstMeasurement = result->getMeasuredStateOnPlane();
  Vector3D position = Vector3D(firstMeasurement.getPos());
  Vector3D momentum = Vector3D(firstMeasurement.getMom());

  const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed());

  const auto& hitPosition = Vector3D(spacePoint->getPosition());

  const double arcLength = trajectory.calcArcLength2D(hitPosition);
  const auto& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
  const auto& trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);

  Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
  Vector3D distance = trackPositionAtHit - hitPosition;

  const auto& sensorInfo = spacePoint->getVxdID();

  double arcLengthOfHitPosition = trajectory.calcArcLength2D(hitPosition);
  double arcLengthOfCenterPosition = trajectory.calcArcLength2D(Vector3D(0, 0, 0));

  var<named("distance")>() = distance.norm();
  var<named("xy_distance")>() = distance.xy().norm();
  var<named("z_distance")>() = distance.z();
  var<named("layer")>() = sensorInfo.getLayerNumber();
  var<named("ladder")>() = sensorInfo.getLadderNumber();
  var<named("pt")>() = momentum.xy().norm();
  var<named("tan_lambda")>() = trajectory.getTanLambda();
  var<named("phi")>() = momentum.phi();
  var<named("arcLengthOfHitPosition")>() = arcLengthOfHitPosition;
  var<named("arcLengthOfCenterPosition")>() = arcLengthOfCenterPosition;
  var<named("numberOfHoles")>() = result->getNumberOfHoles();
  return true;
}
