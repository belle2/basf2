/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/cosmicsTrackMerger/PhiRecoTrackRelationFilter.h>

#include <tracking/trackingUtilities/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackingUtilities/geometry/Vector3D.h>

#include <tracking/trackingUtilities/numerics/Angle.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackingUtilities;

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

  const CDCTrajectory2D fromTrajectory(VectorUtil::get2DVector(fromTrack->getPositionSeed()), 0,
                                       VectorUtil::get2DVector(fromTrack->getMomentumSeed()), fromTrack->getChargeSeed());
  const CDCTrajectory2D toTrajectory(VectorUtil::get2DVector(toTrack->getPositionSeed()), 0,
                                     VectorUtil::get2DVector(toTrack->getMomentumSeed()), toTrack->getChargeSeed());

  const Vector2D origin;

  const double fromPhi = fromTrajectory.getFlightDirection2D(origin).Phi();
  const double toPhi = toTrajectory.getFlightDirection2D(origin).Phi();

  const double phiDistance = std::fabs(AngleUtil::normalised(fromPhi - toPhi));

  if (phiDistance > m_param_maximalPhiDistance) {
    return NAN;
  } else {
    return phiDistance;
  }
}
