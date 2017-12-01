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
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.icc.h>

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

  const genfit::MeasuredStateOnPlane& firstMeasurement = [&state, &previousStates]() {
    if (state->mSoPSet()) {
      return state->getMeasuredStateOnPlane();
    } else {
      return previousStates.back()->getMeasuredStateOnPlane();
    }
  }();

  const Vector3D& position = static_cast<Vector3D>(firstMeasurement.getPos());
  const Vector3D& momentum = static_cast<Vector3D>(firstMeasurement.getMom());

  const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed());

  const Vector3D& hitPosition = static_cast<Vector3D>(spacePoint->getPosition());

  const double arcLengthDifference = trajectory.getTrajectory2D().calcArcLength2DBetween(hitPosition.xy(), Vector2D(0, 0));
  if (m_param_direction * arcLengthDifference > 0) {
    return NAN;
  }

  return 1.0;
}

void NonIPCrossingSVDStateFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "direction"), m_param_direction,
                                "The direction where the extrapolation will happen.", m_param_direction);
}