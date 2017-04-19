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
#include <svd/reconstruction/SVDRecoHit.h>

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

  var<named("track_position_x")>() = position.x();
  var<named("track_position_y")>() = position.y();
  var<named("track_position_z")>() = position.z();

  var<named("hit_position_x")>() = hitPosition.x();
  var<named("hit_position_y")>() = hitPosition.y();
  var<named("hit_position_z")>() = hitPosition.z();

  var<named("track_position_at_hit_x")>() = trackPositionAtHit.x();
  var<named("track_position_at_hit_y")>() = trackPositionAtHit.y();
  var<named("track_position_at_hit_z")>() = trackPositionAtHit.z();

  var<named("same_hemisphere")>() = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();

  var<named("layer")>() = sensorInfo.getLayerNumber();
  var<named("ladder")>() = sensorInfo.getLadderNumber();
  var<named("sensor")>() = sensorInfo.getSensorNumber();
  var<named("segment")>() = sensorInfo.getSegmentNumber();
  var<named("id")>() = sensorInfo.getID();

  var<named("pt")>() = momentum.xy().norm();
  var<named("tan_lambda")>() = trajectory.getTanLambda();
  var<named("phi")>() = momentum.phi();

  var<named("arcLengthOfHitPosition")>() = arcLengthOfHitPosition;
  var<named("arcLengthOfCenterPosition")>() = arcLengthOfCenterPosition;

  var<named("numberOfHoles")>() = result->getNumberOfHoles();

  var<named("chi2")>() = result->getChi2();

  var<named("last_layer")>() = 0;
  var<named("last_ladder")>() = 0;
  var<named("last_sensor")>() = 0;
  var<named("last_segment")>() = 0;
  var<named("last_id")>() = 0;

  const auto* parent = result->getParent();
  if (parent) {
    const auto* parentSpacePoint = parent->getSpacePoint();
    if (parentSpacePoint) {
      const auto& parentSensorInfo = parentSpacePoint->getVxdID();

      var<named("last_layer")>() = parentSensorInfo.getLayerNumber();
      var<named("last_ladder")>() = parentSensorInfo.getLadderNumber();
      var<named("last_sensor")>() = parentSensorInfo.getSensorNumber();
      var<named("last_segment")>() = parentSensorInfo.getSegmentNumber();
      var<named("last_id")>() = parentSensorInfo.getID();
    }
  }

  return true;
}
