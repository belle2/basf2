/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/filters/cdcTrackSpacePointCombination/CDCTrackSpacePointCombinationVarSet.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <TMath.h>

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

  const auto& sensorInfo = spacePoint->getVxdID();

  var<named("track_position_x")>() = position.x();
  var<named("track_position_y")>() = position.y();
  var<named("track_position_z")>() = position.z();

  var<named("hit_position_x")>() = hitPosition.x();
  var<named("hit_position_y")>() = hitPosition.y();
  var<named("hit_position_z")>() = hitPosition.z();

  var<named("track_position_at_hit_x")>() = trackPositionAtHit.x();
  var<named("track_position_at_hit_y")>() = trackPositionAtHit.y();
  var<named("track_position_at_hit_z")>() = trackPositionAtHit.z();

  const auto& cov = firstMeasurement.get6DCov();
  const auto& cov5 = firstMeasurement.getCov();

  var<named("C_00")>() = cov(0, 0);
  var<named("C_01")>() = cov(0, 1);
  var<named("C_02")>() = cov(0, 2);
  var<named("C_03")>() = cov(0, 3);
  var<named("C_04")>() = cov(0, 4);
  var<named("C_05")>() = cov(0, 4);

  var<named("C_10")>() = cov(1, 0);
  var<named("C_11")>() = cov(1, 1);
  var<named("C_12")>() = cov(1, 2);
  var<named("C_13")>() = cov(1, 3);
  var<named("C_14")>() = cov(1, 4);
  var<named("C_15")>() = cov(1, 4);

  var<named("C_20")>() = cov(2, 0);
  var<named("C_21")>() = cov(2, 1);
  var<named("C_22")>() = cov(2, 2);
  var<named("C_23")>() = cov(2, 3);
  var<named("C_24")>() = cov(2, 4);
  var<named("C_25")>() = cov(2, 4);

  var<named("C_30")>() = cov(3, 0);
  var<named("C_31")>() = cov(3, 1);
  var<named("C_32")>() = cov(3, 2);
  var<named("C_33")>() = cov(3, 3);
  var<named("C_34")>() = cov(3, 4);
  var<named("C_35")>() = cov(3, 4);

  var<named("C_40")>() = cov(4, 0);
  var<named("C_41")>() = cov(4, 1);
  var<named("C_42")>() = cov(4, 2);
  var<named("C_43")>() = cov(4, 3);
  var<named("C_44")>() = cov(4, 4);
  var<named("C_45")>() = cov(4, 4);

  var<named("C_50")>() = cov(5, 0);
  var<named("C_51")>() = cov(5, 1);
  var<named("C_52")>() = cov(5, 2);
  var<named("C_53")>() = cov(5, 3);
  var<named("C_54")>() = cov(5, 4);
  var<named("C_55")>() = cov(5, 4);

  var<named("C5_00")>() = cov5(0, 0);
  var<named("C5_01")>() = cov5(0, 1);
  var<named("C5_02")>() = cov5(0, 2);
  var<named("C5_03")>() = cov5(0, 3);
  var<named("C5_04")>() = cov5(0, 4);

  var<named("C5_10")>() = cov5(1, 0);
  var<named("C5_11")>() = cov5(1, 1);
  var<named("C5_12")>() = cov5(1, 2);
  var<named("C5_13")>() = cov5(1, 3);
  var<named("C5_14")>() = cov5(1, 4);

  var<named("C5_20")>() = cov5(2, 0);
  var<named("C5_21")>() = cov5(2, 1);
  var<named("C5_22")>() = cov5(2, 2);
  var<named("C5_23")>() = cov5(2, 3);
  var<named("C5_24")>() = cov5(2, 4);

  var<named("C5_30")>() = cov5(3, 0);
  var<named("C5_31")>() = cov5(3, 1);
  var<named("C5_32")>() = cov5(3, 2);
  var<named("C5_33")>() = cov5(3, 3);
  var<named("C5_34")>() = cov5(3, 4);

  var<named("C5_40")>() = cov5(4, 0);
  var<named("C5_41")>() = cov5(4, 1);
  var<named("C5_42")>() = cov5(4, 2);
  var<named("C5_43")>() = cov5(4, 3);
  var<named("C5_44")>() = cov5(4, 4);

  const auto& state = firstMeasurement.getState();

  var<named("state_0")>() = state(0);
  var<named("state_1")>() = state(1);
  var<named("state_2")>() = state(2);
  var<named("state_3")>() = state(3);
  var<named("state_4")>() = state(4);

  unsigned int clusterNumber = 0;
  for (const SVDCluster& relatedCluster : spacePoint->getRelationsTo<SVDCluster>()) {
    SVDRecoHit clusterMeasurement(&relatedCluster);
    const std::vector<genfit::MeasurementOnPlane*> measurementsOnPlane = clusterMeasurement.constructMeasurementsOnPlane(
          firstMeasurement);

    const genfit::MeasurementOnPlane& measurementOnPlane = *(measurementsOnPlane.front());

    const auto& m_k = measurementOnPlane.getState();
    const auto& V_k = measurementOnPlane.getCov();

    if (clusterNumber == 0) {
      var<named("m_0_state")>() = m_k(0);
      var<named("m_0_cov")>() = V_k(0, 0);
      var<named("is_u_0")>() = clusterMeasurement.isU();
    } else {
      var<named("m_1_state")>() = m_k(0);
      var<named("m_1_cov")>() = V_k(0, 0);
      var<named("is_u_1")>() = clusterMeasurement.isU();
    }

    clusterNumber++;
  }

  var<named("ladder")>() = sensorInfo.getLadderNumber();
  var<named("sensor")>() = sensorInfo.getSensorNumber();
  var<named("segment")>() = sensorInfo.getSegmentNumber();
  var<named("id")>() = sensorInfo.getID();

  var<named("pt")>() = momentum.xy().norm();
  var<named("tan_lambda")>() = trajectory.getTanLambda();
  var<named("phi")>() = momentum.phi();

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
