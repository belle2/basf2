/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/states/NonIPCrossingPXDStateFilter.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;


Weight NonIPCrossingPXDStateFilter::operator()(const AllPXDStateFilter::Object& pair)
{
  if (std::isnan(AllPXDStateFilter::operator()(pair))) {
    return NAN;
  }

  const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>& previousStates = pair.first;
  CKFToPXDState* state = pair.second;

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

  const double arcLengthDifference = trajectory.getTrajectory2D().calcArcLength2DBetween(hitPosition.xy(), Vector2D(0, 0));
  if (m_param_direction * arcLengthDifference > 0) {
    return NAN;
  }

  return 1.0;
}

void NonIPCrossingPXDStateFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "direction"), m_param_direction,
                                "The direction where the extrapolation will happen.", m_param_direction);
}