/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectBasicVarSet.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/ckf/utilities/StateAlgorithms.h>
#include <tracking/ckf/findlets/base/KalmanUpdateFitter.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool CKFCDCToSpacePointStateObjectBasicVarSet::extract(const BaseCKFCDCToSpacePointStateObjectFilter::Object* result)
{
  RecoTrack* cdcTrack = result->getSeedRecoTrack();
  const SpacePoint* spacePoint = result->getHit();

  if (not cdcTrack or not spacePoint) {
    // TODO: Can we also extract meaningful things of the spacePoint is null?
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

  var<named("numberOfHoles")>() = result->getNumberOfHoles();

  OverlapExtractor isOnOverlapLayer;

  var<named("layer")>() = spacePoint->getVxdID().getLayerNumber();
  var<named("number")>() = result->getNumber();
  var<named("overlap")>() = isOnOverlapLayer(*result);

  var<named("pt")>() = static_cast<Float_t>(momentum.xy().norm());
  var<named("tan_lambda")>() = static_cast<Float_t>(trajectory.getTanLambda());
  var<named("phi")>() = static_cast<Float_t>(momentum.phi());

  const auto& sensorInfo = spacePoint->getVxdID();

  var<named("ladder")>() = sensorInfo.getLadderNumber();
  var<named("sensor")>() = sensorInfo.getSensorNumber();
  var<named("segment")>() = sensorInfo.getSegmentNumber();
  var<named("id")>() = sensorInfo.getID();

  var<named("last_layer")>() = 0;
  var<named("last_ladder")>() = 0;
  var<named("last_sensor")>() = 0;
  var<named("last_segment")>() = 0;
  var<named("last_id")>() = 0;

  const auto* parent = result->getParent();
  if (parent) {
    // skip the overlap layers
    parent = parent->getParent();
    if (parent) {
      const auto* parentSpacePoint = parent->getHit();
      if (parentSpacePoint) {
        const auto& parentSensorInfo = parentSpacePoint->getVxdID();

        var<named("last_layer")>() = parentSensorInfo.getLayerNumber();
        var<named("last_ladder")>() = parentSensorInfo.getLadderNumber();
        var<named("last_sensor")>() = parentSensorInfo.getSensorNumber();
        var<named("last_segment")>() = parentSensorInfo.getSegmentNumber();
        var<named("last_id")>() = parentSensorInfo.getID();
      }
    }
  }

  KalmanUpdateFitter fitter;
  Float_t residual = 0;

  if (spacePoint->getType() == VXD::SensorInfoBase::SVD) {
    for (const auto& svdCluster : spacePoint->getRelationsTo<SVDCluster>()) {
      SVDRecoHit recoHit(&svdCluster);
      residual += fitter.calculateResidualDistance<SVDRecoHit, 1>(result->getMeasuredStateOnPlane(), recoHit);
    }
  } else {
    PXDRecoHit recoHit(spacePoint->getRelatedTo<PXDCluster>());
    // chi2 = fitter.calculateChi2<PXDRecoHit, 2>(result->getMeasuredStateOnPlane(), recoHit);
    residual = fitter.calculateResidualDistance<PXDRecoHit, 2>(result->getMeasuredStateOnPlane(), recoHit);
  }
  var<named("residual")>() = residual;

  if (result->isFitted()) {
    var<named("chi2")>() = static_cast<Float_t>(result->getChi2());
  } else {
    var<named("chi2")>() = -999;
  }

  const auto& cov5 = firstMeasurement.getCov();
  const Float_t sigmaUV = std::sqrt(std::max(cov5(4, 4), cov5(3, 3)));
  var<named("sigma_uv")>() = sigmaUV;
  var<named("residual_over_sigma")>() = residual / sigmaUV;

  return true;
}
