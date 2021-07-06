/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/svd/filters/states/SimpleSVDStateFilter.h>

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

constexpr const double SimpleSVDStateFilter::m_param_maximumHelixDistance[][3];
constexpr const double SimpleSVDStateFilter::m_param_maximumResidual[][3];
constexpr const double SimpleSVDStateFilter::m_param_maximumResidual2[][3];

void SimpleSVDStateFilter::beginRun()
{
  m_cachedBField = TrackFindingCDC::CDCBFieldUtil::getBFieldZ();
}

Weight SimpleSVDStateFilter::operator()(const BaseSVDStateFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const CKFToSVDState*>>& previousStates = pair.first;
  CKFToSVDState* currentState = pair.second;

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

  const bool sameHemisphere = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();
  if (not sameHemisphere) {
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

    valueToCheck = differenceHelix.norm();
    maximumValues = &m_param_maximumHelixDistance;
  } else {
    // Filter 2 + 3
    const double residual = m_kalmanStepper.calculateResidual(firstMeasurement, *currentState);
    valueToCheck = residual;
    if (currentState->isFitted()) {
      maximumValues = &m_param_maximumResidual2;
    } else {
      maximumValues = &m_param_maximumResidual;
    }
  }

  const unsigned int layer = currentState->getGeometricalLayer();
  if (valueToCheck > (*maximumValues)[layer - 3][getPTRange(momentum)]) {
    return NAN;
  }

  return valueToCheck;
}
