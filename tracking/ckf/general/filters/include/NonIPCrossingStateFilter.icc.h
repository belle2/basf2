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

#include <tracking/ckf/general/utilities/SearchDirection.h>

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

    const TrackFindingCDC::CDCTrajectory2D trajectory2D(position.xy(), 0, momentum.xy(), cdcTrack->getChargeSeed());

    const TrackFindingCDC::Vector3D& hitPosition = static_cast<TrackFindingCDC::Vector3D>(spacePoint->getPosition());
    const TrackFindingCDC::Vector2D origin(0, 0);

    const double deltaArcLengthHitOrigin = trajectory2D.calcArcLength2DBetween(hitPosition.xy(), origin);
    const double deltaArcLengthTrackHit = trajectory2D.calcArcLength2D(hitPosition.xy());

    if (not arcLengthInRightDirection(deltaArcLengthTrackHit, m_param_direction) or
        not arcLengthInRightDirection(deltaArcLengthHitOrigin, m_param_direction)) {
      return NAN;
    }

    return 1.0;
  }

  template <class AllStateFilter>
  void NonIPCrossingStateFilter<AllStateFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "direction"), m_param_directionAsString,
                                  "The direction where the extrapolation will happen.");
  }

  template <class AllStateFilter>
  void NonIPCrossingStateFilter<AllStateFilter>::initialize()
  {
    Super::initialize();
    m_param_direction = fromString(m_param_directionAsString);
  }
}