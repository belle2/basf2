/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcToVXDExtrapolator/filterBased/CDCTrackSpacePointCombinationBasicVarSet.h>
#include <tracking/trackFindingCDC/numerics/ToFinite.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <TMath.h>

#include <framework/dataobjects/Helix.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <svd/reconstruction/SVDRecoHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool CDCTrackSpacePointCombinationBasicVarSet::extract(const BaseCDCTrackSpacePointCombinationFilter::Object* result)
{
  RecoTrack* cdcTrack = result->getSeedRecoTrack();
  const SpacePoint* spacePoint = result->getSpacePoint();

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

  var<named("distance")>() = distance.norm();
  var<named("xy_distance")>() = distance.xy().norm();
  var<named("z_distance")>() = distance.z();

  Vector3D mSoP_distance = position - hitPosition;

  var<named("mSoP_distance")>() = mSoP_distance.norm();
  var<named("mSoP_xy_distance")>() = mSoP_distance.xy().norm();
  var<named("mSoP_z_distance")>() = mSoP_distance.z();

  var<named("same_hemisphere")>() = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();

  var<named("arcLengthOfHitPosition")>() = trajectory.calcArcLength2D(hitPosition);
  var<named("arcLengthOfCenterPosition")>() = trajectory.calcArcLength2D(Vector3D(0, 0, 0));

  var<named("numberOfHoles")>() = result->getNumberOfHoles();

  var<named("layer")>() = spacePoint->getVxdID().getLayerNumber();
  var<named("number")>() = result->getNumber();

  var<named("chi2")>() = result->getChi2();

  return true;
}
