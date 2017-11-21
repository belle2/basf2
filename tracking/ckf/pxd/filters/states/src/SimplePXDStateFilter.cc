/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/states/SimplePXDStateFilter.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBFieldUtil.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Helper function to extract the numbered pt-range out of a momentum vector
  unsigned int getPTRange(const TrackFindingCDC::Vector3D& momentum)
  {
    const double pT = momentum.xy().norm();
    if (pT > 0.4) {
      return 0;
    } else if (pT > 0.2) {
      return 1;
    } else {
      return 2;
    }
  }
}

constexpr const SimplePXDStateFilter::MaximalValueArray SimplePXDStateFilter::m_param_maximumHelixDistanceXY;
constexpr const SimplePXDStateFilter::MaximalValueArray SimplePXDStateFilter::m_param_maximumResidual;
constexpr const SimplePXDStateFilter::MaximalValueArray SimplePXDStateFilter::m_param_maximumChi2;

void SimplePXDStateFilter::beginRun()
{
  m_cachedBField = TrackFindingCDC::CDCBFieldUtil::getBFieldZ();
}

Weight SimplePXDStateFilter::operator()(const BasePXDStateFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>& previousStates = pair.first;
  CKFToPXDState* currentState = pair.second;

  const auto* spacePoint = currentState->getHit();

  genfit::MeasuredStateOnPlane firstMeasurement;
  if (currentState->mSoPSet()) {
    firstMeasurement = currentState->getMeasuredStateOnPlane();
  } else {
    firstMeasurement = previousStates.back()->getMeasuredStateOnPlane();
  }

  Vector3D position = Vector3D(firstMeasurement.getPos());
  Vector3D momentum = Vector3D(firstMeasurement.getMom());

  const Vector3D hitPosition = static_cast<Vector3D>(spacePoint->getPosition());

  const double sameHemisphere = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();
  if (sameHemisphere != 1) {
    return NAN;
  }

  double valueToCheck;
  const MaximalValueArray* maximumValues;

  if (not currentState->isFitted() and not currentState->mSoPSet()) {
    // Filter 1
    const RecoTrack* cdcTrack = previousStates.front()->getSeed();

    B2ASSERT("A path without a seed?", cdcTrack);

    const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed(), m_cachedBField);

    const double arcLength = trajectory.calcArcLength2D(hitPosition);
    const Vector2D& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
    const double trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);
    const Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
    const Vector3D differenceHelix = trackPositionAtHit - hitPosition;

    valueToCheck = differenceHelix.xy().norm();
    maximumValues = &m_param_maximumHelixDistanceXY;
  } else if (not currentState->isFitted()) {
    // Filter 2
    const double residual = m_kalmanStepper.calculateResidual(firstMeasurement, *currentState);

    valueToCheck = residual;
    maximumValues = &m_param_maximumResidual;
  } else {
    // Filter 3
    valueToCheck = currentState->getChi2();
    maximumValues = &m_param_maximumChi2;
  }

  const unsigned int layer = currentState->getGeometricalLayer();
  if (valueToCheck > (*maximumValues)[layer - 1][getPTRange(momentum)]) {
    return NAN;
  }

  return valueToCheck;
}