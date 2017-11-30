/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/states/NonIPCrossingSVDStateFilter.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;


Weight NonIPCrossingSVDStateFilter::operator()(const AllSVDStateFilter::Object& pair)
{
  if (std::isnan(AllSVDStateFilter::operator()(pair))) {
    return NAN;
  }

  const std::vector<TrackFindingCDC::WithWeight<const CKFToSVDState*>>& previousStates = pair.first;
  CKFToSVDState* state = pair.second;

  const RecoTrack* cdcTrack = previousStates.front()->getSeed();
  B2ASSERT("Path without seed?", cdcTrack);

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

  const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed());

  const Vector3D& hitPosition = static_cast<Vector3D>(spacePoint->getPosition());

  const double arcLengthOfHit = trajectory.calcArcLength2D(hitPosition);
  const double arcLengthOfOrigin = trajectory.calcArcLength2D(Vector3D(0, 0, 0));

  if (arcLengthOfHit > 0 or arcLengthOfOrigin > 0) {
    return NAN;
  }

  if (arcLengthOfOrigin > arcLengthOfHit) {
    return NAN;
  }

  return 1.0;
}