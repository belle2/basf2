/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/general/filters/NonIPCrossingStateFilter.dcl.h>

#include <tracking/trackingUtilities/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackingUtilities/geometry/Vector3D.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>

#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  template <class AllStateFilter>
  TrackingUtilities::Weight NonIPCrossingStateFilter<AllStateFilter>::operator()(const Object& pair)
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
        B2ASSERT("Previous state was not fitted?", previousStates.back()->mSoPSet());
        return previousStates.back()->getMeasuredStateOnPlane();
      }
    }();

    const TrackingUtilities::Vector3D& position = static_cast<TrackingUtilities::Vector3D>(firstMeasurement.getPos());
    const TrackingUtilities::Vector3D& momentum = static_cast<TrackingUtilities::Vector3D>(firstMeasurement.getMom());

    const TrackingUtilities::CDCTrajectory2D trajectory2D(position.xy(), 0, momentum.xy(), cdcTrack->getChargeSeed());

    const TrackingUtilities::Vector3D& hitPosition = static_cast<TrackingUtilities::Vector3D>(spacePoint->getPosition());
    const TrackingUtilities::Vector2D origin(0, 0);

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
    moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "direction"), m_param_directionAsString,
                                  "The direction where the extrapolation will happen.");
  }

  template <class AllStateFilter>
  void NonIPCrossingStateFilter<AllStateFilter>::initialize()
  {
    Super::initialize();
    m_param_direction = fromString(m_param_directionAsString);
  }
}
