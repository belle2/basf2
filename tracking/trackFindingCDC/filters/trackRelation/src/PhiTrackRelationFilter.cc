/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/PhiTrackRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void PhiTrackRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                              const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "maximalPhiDistance"),
                                m_param_maximalPhiDistance,
                                "Maximal Phi distance below to tracks should be merged.",
                                m_param_maximalPhiDistance);
}

Weight PhiTrackRelationFilter::operator()(const CDCTrack& fromTrack, const CDCTrack& toTrack)
{
  // Make sure we only have one relation out of A -> B and B -> A
  if (fromTrack.getStartRecoPos3D().y() < toTrack.getStartRecoPos3D().y()) {
    return NAN;
  }

  const double lhsPhi = fromTrack.getStartTrajectory3D().getFlightDirection3DAtSupport().phi();
  const double rhsPhi = toTrack.getStartTrajectory3D().getFlightDirection3DAtSupport().phi();

  const double phiDistance = std::fabs(AngleUtil::normalised(lhsPhi - rhsPhi));

  if (phiDistance > m_param_maximalPhiDistance) {
    return NAN;
  } else {
    return phiDistance;
  }
}
