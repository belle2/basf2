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
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumXYDistance"),
                                m_param_maximumXYDistance, "", m_param_maximumXYDistance);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumDistance"),
                                m_param_maximumDistance, "", m_param_maximumDistance);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumChi2Difference"),
                                m_param_maximumChi2Difference, "", m_param_maximumChi2Difference);
}

void SimpleCDCTrackSpacePointCombinationFilter::initialize()
{
  BaseCDCTrackSpacePointCombinationFilter::initialize();

  B2ASSERT("You need to provide exactly 4 maximal norms (for four layers).", m_param_maximumXYDistance.size() == 4);
  B2ASSERT("You need to provide exactly 4 maximal norms (for four layers).", m_param_maximumDistance.size() == 4);
  B2ASSERT("You need to provide exactly 4 maximal norms (for four layers).", m_param_maximumChi2Difference.size() == 4);
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
  // TODO: Cache this also
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
  Vector3D difference = trackPositionAtHit - hitPosition;

  if (not currentState.isFitted() and not currentState.isAdvanced()) {
    const double& xy_distance = difference.xy().norm();
    if (xy_distance > m_param_maximumXYDistance[layer - 3]) {
      return std::nan("");
    } else {
      return xy_distance;
    }
  } else if (not currentState.isFitted()) {
    const double& distance = difference.norm();
    if (distance > m_param_maximumDistance[layer - 3]) {
      return std::nan("");
    } else {
      return distance;
    }
  } else {
    const double& chi2 = currentState.getChi2();
    if (chi2 > m_param_maximumChi2Difference[layer - 3]) {
      return std::nan("");
    } else {
      return chi2;
    }
  }
}
