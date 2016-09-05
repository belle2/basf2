/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackFlightTimeAdjuster.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/gearbox/Const.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackFlightTimeAdjuster::getDescription()
{
  return "Adjusts the flight time of tracks relative to a trigger point";
}

void TrackFlightTimeAdjuster::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "triggerPoint"),
                                m_param_triggerPoint,
                                "Point relative to which the flight times of tracks should be adjusted",
                                m_param_triggerPoint);
}

void TrackFlightTimeAdjuster::initialize()
{
  m_triggerPoint = Vector3D(std::get<0>(m_param_triggerPoint),
                            std::get<1>(m_param_triggerPoint),
                            std::get<2>(m_param_triggerPoint));
}

void TrackFlightTimeAdjuster::apply(std::vector<CDCTrack>& tracks)
{
  for (CDCTrack& track : tracks) {
    CDCTrajectory3D startTrajectory3D = track.getStartTrajectory3D();
    double arcLength2D = startTrajectory3D.calcArcLength2D(m_triggerPoint);
    double arcLength3D = arcLength2D * hypot2(1, startTrajectory3D.getTanLambda());
    double flightTime = arcLength3D / Const::speedOfLight;
    // Insert negative drift time such that the time at the trigger point is 0.
    startTrajectory3D.setFlightTime(-flightTime);
    track.setStartTrajectory3D(startTrajectory3D);
  }
}
