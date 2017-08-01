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
}

Weight SimpleCKFPXDStateFilter::operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState)
{
  if (not checkOverlapAndHoles(currentState)) {
    return NAN;
  }

  const Vector3D position(currentState.getMSoPPosition());
  const Vector3D hitPosition(currentState.getHit()->getPosition());

  const double sameHemisphere = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();

  if (sameHemisphere != 1) {
    return NAN;
  }

  const TMatrixDSym& cov = currentState.getMSoPCovariance();
  const double layer = extractGeometryLayer(currentState);

  if (not currentState.isFitted() and not currentState.isAdvanced()) {
    // Filter 1
    const RecoTrack* cdcTrack = currentState.getSeedRecoTrack();

    const Vector3D momentum(currentState.getMSoPMomentum());
    const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed(), m_cachedBField);

    const double arcLength = trajectory.calcArcLength2D(hitPosition);
    const Vector2D& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
    const double trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);
    const Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
    const Vector3D differenceHelix = trackPositionAtHit - hitPosition;

    const double helix_chi2_xyz = (differenceHelix.x() * differenceHelix.x() / sqrt(cov(0, 0)) +
                                   differenceHelix.y() * differenceHelix.y() / sqrt(cov(1, 1)) +
                                   differenceHelix.z() * differenceHelix.z() / sqrt(cov(2, 2)));

    if (helix_chi2_xyz > m_param_maximumHelixChi2XYZ[layer]) {
      return NAN;
    } else {
      return helix_chi2_xyz;
    }
  } else if (not currentState.isFitted()) {
    // Filter 2
    const Vector3D& difference = position - hitPosition;

    const double chi2_xy = (difference.x() * difference.x() / sqrt(cov(0, 0)) +
                            difference.y() * difference.y() / sqrt(cov(1, 1)));
    const double chi2_xyz = chi2_xy + difference.z() * difference.z() / sqrt(cov(2, 2));

    if (chi2_xy > m_param_maximumChi2XY[layer]) {
      return NAN;
    } else {
      return chi2_xyz;
    }
  } else {
    // Filter 3
    const double chi2 = currentState.getChi2();
    if (chi2 > m_param_maximumChi2[layer]) {
      return NAN;
    } else {
      return chi2;
    }
  }
}