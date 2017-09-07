/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSVDSpacePoint/SimpleCKFCDCToSVDStateFilter.h>

#include <tracking/ckf/utilities/StateAlgorithms.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  template <class ARhs, class ALhs>
  decltype(ARhs() % ALhs()) mod(ARhs a, ALhs b)
  {
    return (a % b + b) % b;
  }

  unsigned int getPTRange(const Vector3D& momentum)
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

constexpr const double SimpleCKFCDCToSVDStateFilter::m_param_maximumHelixDistance[][3];
constexpr const double SimpleCKFCDCToSVDStateFilter::m_param_maximumResidual[][3];
constexpr const double SimpleCKFCDCToSVDStateFilter::m_param_maximumResidual2[][3];
constexpr const double SimpleCKFCDCToSVDStateFilter::m_param_maximumChi2[][3];

Weight SimpleCKFCDCToSVDStateFilter::operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState)
{
  const auto* spacePoint = currentState.getHit();

  if (not spacePoint) {
    // lets use a very small number here, to always have the empty state in the game
    return 0;
  }

  const Vector3D position(currentState.getMSoPPosition());
  const Vector3D hitPosition(spacePoint->getPosition());

  const double sameHemisphere = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();
  if (sameHemisphere != 1) {
    return NAN;
  }

  const unsigned int layer = extractGeometryLayer(currentState);
  const Vector3D momentum(currentState.getMSoPMomentum());

  double valueToCheck;
  const MaximalValueArray* maximumValues;

  if (not currentState.isFitted() and not currentState.isAdvanced()) {
    // Filter 1
    const RecoTrack* cdcTrack = currentState.getSeedRecoTrack();
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
    const auto& measuredStateOnPlane = currentState.getMeasuredStateOnPlane();
    double residual = 0;

    for (const auto& svdCluster : spacePoint->getRelationsTo<SVDCluster>()) {
      SVDRecoHit recoHit(&svdCluster);
      residual += m_fitter.calculateResidualDistance<SVDRecoHit, 1>(measuredStateOnPlane, recoHit);
    }

    valueToCheck = residual;
    if (currentState.isFitted()) {
      maximumValues = &m_param_maximumResidual2;
    } else {
      maximumValues = &m_param_maximumResidual;
    }
  }/* else {
    // Filter 3
    valueToCheck = currentState.getChi2();
    maximumValues = &m_param_maximumChi2;
  }*/

  if (valueToCheck > (*maximumValues)[layer - 3][getPTRange(momentum)]) {
    if (currentState.getTruthInformation()) {
      B2WARNING("Throwing away a truth hit with " << valueToCheck << " instead of " << (*maximumValues)[layer - 3][getPTRange(momentum)]);
      B2WARNING("on layer " << layer << " and momentum " << momentum);
    }
    return NAN;
  }

  return valueToCheck;
}