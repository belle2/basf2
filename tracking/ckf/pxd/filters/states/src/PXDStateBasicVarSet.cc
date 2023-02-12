/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/pxd/filters/states/PXDStateBasicVarSet.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <pxd/dataobjects/PXDCluster.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool PXDStateBasicVarSet::extract(const BasePXDStateFilter::Object* pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>& previousStates = pair->first;
  CKFToPXDState* state = pair->second;

  const RecoTrack* seedTrack = previousStates.front()->getSeed();
  B2ASSERT("Path without seed?", seedTrack);

  const SpacePoint* spacePoint = state->getHit();
  B2ASSERT("Path without hit?", spacePoint);

  genfit::MeasuredStateOnPlane firstMeasurement;
  if (state->mSoPSet()) {
    firstMeasurement = state->getMeasuredStateOnPlane();
  } else {
    firstMeasurement = previousStates.back()->getMeasuredStateOnPlane();
  }

  Vector3D position = Vector3D(firstMeasurement.getPos());
  Vector3D momentum = Vector3D(firstMeasurement.getMom());

  const CDCTrajectory3D trajectory(position, 0, momentum, seedTrack->getChargeSeed());

  const Vector3D& hitPosition = static_cast<Vector3D>(spacePoint->getPosition());

  const double arcLength = trajectory.calcArcLength2D(hitPosition);
  const Vector2D& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
  double trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);

  Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
  Vector3D distance = trackPositionAtHit - hitPosition;

  var<named("distance")>() = static_cast<Float_t>(distance.norm());
  var<named("xy_distance")>() = static_cast<Float_t>(distance.xy().norm());
  var<named("z_distance")>() = static_cast<Float_t>(distance.z());

  Vector3D mSoP_distance = position - hitPosition;

  var<named("mSoP_distance")>() = static_cast<Float_t>(mSoP_distance.norm());
  var<named("mSoP_xy_distance")>() = static_cast<Float_t>(mSoP_distance.xy().norm());
  var<named("mSoP_z_distance")>() = static_cast<Float_t>(mSoP_distance.z());

  var<named("same_hemisphere")>() = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();

  var<named("arcLengthOfHitPosition")>() = static_cast<Float_t>(trajectory.calcArcLength2D(hitPosition));
  var<named("arcLengthOfCenterPosition")>() = static_cast<Float_t>(trajectory.calcArcLength2D(Vector3D(0, 0, 0)));

  var<named("layer")>() = spacePoint->getVxdID().getLayerNumber();
  var<named("number")>() = previousStates.size();

  var<named("pt")>() = static_cast<Float_t>(momentum.xy().norm());
  var<named("tan_lambda")>() = static_cast<Float_t>(trajectory.getTanLambda());
  var<named("phi")>() = static_cast<Float_t>(momentum.phi());

  const VxdID& sensorInfo = spacePoint->getVxdID();

  var<named("ladder")>() = sensorInfo.getLadderNumber();
  var<named("sensor")>() = sensorInfo.getSensorNumber();
  var<named("segment")>() = sensorInfo.getSegmentNumber();
  var<named("id")>() = sensorInfo.getID();

  const auto& clusters = spacePoint->getRelationsTo<PXDCluster>();
  B2ASSERT("Must be related to exactly 1 cluster", clusters.size() == 1);
  var<named("cluster_charge")>() = clusters[0]->getCharge();
  var<named("cluster_seed_charge")>() = clusters[0]->getSeedCharge();
  var<named("cluster_size")>() = clusters[0]->getSize();
  var<named("cluster_size_u")>() = clusters[0]->getUSize();
  var<named("cluster_size_v")>() = clusters[0]->getVSize();

  var<named("last_layer")>() = 0;
  var<named("last_ladder")>() = 0;
  var<named("last_sensor")>() = 0;
  var<named("last_segment")>() = 0;
  var<named("last_id")>() = 0;
  var<named("last_cluster_charge")>() = 0;
  var<named("last_cluster_seed_charge")>() = 0;
  var<named("last_cluster_size")>() = 0;
  var<named("last_cluster_size_u")>() = 0;
  var<named("last_cluster_size_v")>() = 0;

  const CKFToPXDState* parent = previousStates.back();
  const SpacePoint* parentSpacePoint = parent->getHit();
  if (parentSpacePoint) {
    const VxdID& parentSensorInfo = parentSpacePoint->getVxdID();

    var<named("last_layer")>() = parentSensorInfo.getLayerNumber();
    var<named("last_ladder")>() = parentSensorInfo.getLadderNumber();
    var<named("last_sensor")>() = parentSensorInfo.getSensorNumber();
    var<named("last_segment")>() = parentSensorInfo.getSegmentNumber();
    var<named("last_id")>() = parentSensorInfo.getID();

    const auto& parentclusters = parentSpacePoint->getRelationsTo<PXDCluster>();
    B2ASSERT("Must be related to exactly 1 cluster", parentclusters.size() == 1);
    var<named("last_cluster_charge")>() = parentclusters[0]->getCharge();
    var<named("last_cluster_seed_charge")>() = parentclusters[0]->getSeedCharge();
    var<named("last_cluster_size")>() = parentclusters[0]->getSize();
    var<named("last_cluster_size_u")>() = parentclusters[0]->getUSize();
    var<named("last_cluster_size_v")>() = parentclusters[0]->getVSize();
  }

  const double residual = m_kalmanStepper.calculateResidual(firstMeasurement, *state);
  var<named("residual")>() = residual;

  if (state->isFitted()) {
    var<named("chi2")>() = static_cast<Float_t>(state->getChi2());
  } else {
    var<named("chi2")>() = -999;
  }

  const TMatrixDSym& cov5 = firstMeasurement.getCov();
  const Float_t sigmaUV = std::sqrt(std::max(cov5(4, 4), cov5(3, 3)));
  var<named("sigma_uv")>() = sigmaUV;
  var<named("residual_over_sigma")>() = residual / sigmaUV;

  return true;
}
