/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/general/filters/NonIPCrossingStateFilter.dcl.h>

#include <tracking/trackingUtilities/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackingUtilities/geometry/VectorUtil.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>

#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.h>

#include <Math/Vector2D.h>
#include <Math/Vector3D.h>

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

    const ROOT::Math::XYZVector& position = static_cast<ROOT::Math::XYZVector>(firstMeasurement.getPos());
    const ROOT::Math::XYZVector& momentum = static_cast<ROOT::Math::XYZVector>(firstMeasurement.getMom());

    const TrackingUtilities::CDCTrajectory2D trajectory2D(VectorUtil::getXYVector(position), 0, VectorUtil::getXYVector(momentum),
                                                          cdcTrack->getChargeSeed());

    const ROOT::Math::XYZVector& hitPosition = spacePoint->getPosition();
    const ROOT::Math::XYVector origin(0, 0);

    const double deltaArcLengthHitOrigin = trajectory2D.calcArcLength2DBetween(VectorUtil::getXYVector(hitPosition), origin);
    const double deltaArcLengthTrackHit = trajectory2D.calcArcLength2D(VectorUtil::getXYVector(hitPosition));

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
