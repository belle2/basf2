/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/general/filters/NonIPCrossingStateFilter.dcl.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.icc.h>

#include <vector>

namespace Belle2 {
  template <class AllStateFilter>
  TrackFindingCDC::Weight NonIPCrossingStateFilter<AllStateFilter>::operator()(const Object& pair)
  {
    if (std::isnan(AllStateFilter::operator()(pair))) {
      return NAN;
    }

    const auto& previousStates = pair.first;
    auto* state = pair.second;

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

    const TrackFindingCDC::Vector3D& position = static_cast<TrackFindingCDC::Vector3D>(firstMeasurement.getPos());
    const TrackFindingCDC::Vector3D& momentum = static_cast<TrackFindingCDC::Vector3D>(firstMeasurement.getMom());

    const TrackFindingCDC::CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed());

    const TrackFindingCDC::Vector3D& hitPosition = static_cast<TrackFindingCDC::Vector3D>(spacePoint->getPosition());

    const double arcLengthDifference = trajectory.getTrajectory2D().calcArcLength2DBetween(hitPosition.xy(),
                                       TrackFindingCDC::Vector2D(0, 0));
    if (m_param_direction * arcLengthDifference > 0) {
      return NAN;
    }

    return 1.0;
  }

  template <class AllStateFilter>
  void NonIPCrossingStateFilter<AllStateFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "direction"), m_param_direction,
                                  "The direction where the extrapolation will happen.", m_param_direction);
  }
}