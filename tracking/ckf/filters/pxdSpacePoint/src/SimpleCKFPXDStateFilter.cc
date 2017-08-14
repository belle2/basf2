/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/pxdSpacePoint/SimpleCKFPXDStateFilter.h>

#include <tracking/ckf/utilities/StateAlgorithms.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

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

constexpr const double SimpleCKFPXDStateFilter::m_param_maximumHelixDistanceXY[][3];
constexpr const double SimpleCKFPXDStateFilter::m_param_maximumDistanceXY[][3];
constexpr const double SimpleCKFPXDStateFilter::m_param_maximumDistance[][3];
constexpr const double SimpleCKFPXDStateFilter::m_param_maximumChi2[][3];

Weight SimpleCKFPXDStateFilter::operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState)
{
  if (not checkOverlapAndHoles(currentState)) {
    return NAN;
  }

  const auto* spacePoint = currentState.getHit();

  if (not spacePoint) {
    // lets use a very small number here, to always have the empty state in the game
    return 0;
  }

  const unsigned int layer = extractGeometryLayer(currentState);
  const Vector3D momentum(currentState.getMSoPMomentum());

  if (not currentState.isFitted() and not currentState.isAdvanced()) {
    // TODO: Use the sensor information here and cache this!
    // Filter 1
    const RecoTrack* cdcTrack = currentState.getSeedRecoTrack();

    const Vector3D position(currentState.getMSoPPosition());
    const Vector3D hitPosition(spacePoint->getPosition());
    const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed(), m_cachedBField);

    const double arcLength = trajectory.calcArcLength2D(hitPosition);
    const Vector2D& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
    const double trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);
    const Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
    const Vector3D differenceHelix = trackPositionAtHit - hitPosition;

    const double helixDifferenceXY = differenceHelix.xy().norm();

    if (helixDifferenceXY > m_param_maximumHelixDistanceXY[layer - 1][getPTRange(momentum)]) {
      return NAN;
    } else {
      return helixDifferenceXY;
    }
  } else if (not currentState.isFitted()) {
    // Filter 2
    const Vector3D position(currentState.getMSoPPosition());
    const Vector3D hitPosition(spacePoint->getPosition());
    const Vector3D& difference = position - hitPosition;

    if (layer == 2) {
      const double differenceXY = difference.xy().norm();
      if (differenceXY > m_param_maximumDistanceXY[layer - 1][getPTRange(momentum)]) {
        return NAN;
      } else {
        return differenceXY;
      }
    } else {
      if (difference.norm() > m_param_maximumDistance[layer - 1][getPTRange(momentum)]) {
        return NAN;
      } else {
        return difference.norm();
      }
    }
  } else {
    const double chi2 = currentState.getChi2();
    if (chi2 > m_param_maximumChi2[layer - 1][getPTRange(momentum)]) {
      return NAN;
    } else {
      return chi2;
    }
  }
}