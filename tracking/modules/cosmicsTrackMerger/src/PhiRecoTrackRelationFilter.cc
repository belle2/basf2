/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/cosmicsTrackMerger/PhiRecoTrackRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void PhiRecoTrackRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                                  const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "maximalPhiDistance"),
                                m_param_maximalPhiDistance,
                                "Maximal Phi distance below to tracks should be merged.",
                                m_param_maximalPhiDistance);
}

Weight PhiRecoTrackRelationFilter::operator()(const CellularRecoTrack& fromTrack, const CellularRecoTrack& toTrack)
{
  // Make sure we only have one relation out of A -> B and B -> A
  if (fromTrack->getPositionSeed().Y() < toTrack->getPositionSeed().Y()) {
    return NAN;
  }

  const CDCTrajectory2D fromTrajectory(Vector3D(fromTrack->getPositionSeed()).xy(), 0,
                                       Vector3D(fromTrack->getMomentumSeed()).xy(), fromTrack->getChargeSeed());
  const CDCTrajectory2D toTrajectory(Vector3D(toTrack->getPositionSeed()).xy(), 0,
                                     Vector3D(toTrack->getMomentumSeed()).xy(), toTrack->getChargeSeed());

  const Vector2D origin;

  const double fromPhi = fromTrajectory.getFlightDirection2D(origin).phi();
  const double toPhi = toTrajectory.getFlightDirection2D(origin).phi();

  const double phiDistance = std::fabs(AngleUtil::normalised(fromPhi - toPhi));

  if (phiDistance > m_param_maximalPhiDistance) {
    return NAN;
  } else {
    return phiDistance;
  }
}
