/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/SimpleCDCTrackSpacePointCombinationFilter.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void SimpleCDCTrackSpacePointCombinationFilter::exposeParameters(ModuleParamList* moduleParamList,
    const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHelixChi2XYZ"),
                                m_param_maximumHelixChi2XYZ, "", m_param_maximumHelixChi2XYZ);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumChi2XY"),
                                m_param_maximumChi2XY, "", m_param_maximumChi2XY);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumChi2"),
                                m_param_maximumChi2, "", m_param_maximumChi2);
}

void SimpleCDCTrackSpacePointCombinationFilter::initialize()
{
  BaseCDCTrackSpacePointCombinationFilter::initialize();

  B2ASSERT("You need to provide exactly 4 maximal norms (for four layers).", m_param_maximumHelixChi2XYZ.size() == 4);
  B2ASSERT("You need to provide exactly 4 maximal norms (for four layers).", m_param_maximumChi2XY.size() == 4);
  B2ASSERT("You need to provide exactly 4 maximal norms (for four layers).", m_param_maximumChi2.size() == 4);
}

Weight SimpleCDCTrackSpacePointCombinationFilter::operator()(const BaseCDCTrackSpacePointCombinationFilter::Object& currentState)
{

  const SpacePoint* spacePoint = currentState.getHit();
  const RecoTrack* cdcTrack = currentState.getSeedRecoTrack();

  // Allow overlap layers to have no hit
  // TODO: do only allow this in some cases where we actually have no overlap!
  if (not spacePoint) {
    if (currentState.isOnOverlapLayer()) {
      return 1;
    } else {
      return std::nan("");
    }
  }

  const genfit::MeasuredStateOnPlane& mSoP = currentState.getMeasuredStateOnPlaneSavely();

  Vector3D position = TrackFindingCDC::Vector3D(mSoP.getPos());
  Vector3D momentum = TrackFindingCDC::Vector3D(mSoP.getMom());
  Vector3D hitPosition = TrackFindingCDC::Vector3D(spacePoint->getPosition());

  const double& sameHemisphere = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();

  if (sameHemisphere != 1) {
    return std::nan("");
  }

  const double& layer = currentState.extractGeometryLayer();

  const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed());

  const double arcLength = trajectory.calcArcLength2D(hitPosition);
  const auto& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
  const auto& trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);

  Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
  Vector3D differenceHelix = trackPositionAtHit - hitPosition;
  Vector3D difference = position - hitPosition;

  const TMatrixDSym& cov6D = mSoP.get6DCov();

  if (not currentState.isFitted() and not currentState.isAdvanced()) {
    // Filter 1
    // TODO: this is wrong!
    const double& helix_chi2_xyz = (differenceHelix.x() * differenceHelix.x() / sqrt(cov6D(0, 0)) +
                                    differenceHelix.y() * differenceHelix.y() / sqrt(cov6D(1, 1)) +
                                    differenceHelix.z() * differenceHelix.z() / sqrt(cov6D(2, 2)));
    if (helix_chi2_xyz > m_param_maximumHelixChi2XYZ[layer - 3]) {
      return std::nan("");
    } else {
      return helix_chi2_xyz;
    }
  } else if (not currentState.isFitted()) {
    // Filter 2
    const double& chi2_xy = (difference.x() * difference.x() / sqrt(cov6D(0, 0)) +
                             difference.y() * difference.y() / sqrt(cov6D(1, 1)));
    const double& chi2_xyz = chi2_xy + difference.z() * difference.z() / sqrt(cov6D(2, 2));
    if (chi2_xy > m_param_maximumChi2XY[layer - 3]) {
      return std::nan("");
    } else {
      return chi2_xyz;
    }
  } else {
    // Filter 3
    const double& chi2 = currentState.getChi2();
    if (chi2 > m_param_maximumChi2[layer - 3]) {
      return std::nan("");
    } else {
      return chi2;
    }
  }
}
