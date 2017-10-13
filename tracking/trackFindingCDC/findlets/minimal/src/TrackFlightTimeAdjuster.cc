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

#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <vector>
#include <string>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackFlightTimeAdjuster::getDescription()
{
  return "Adjusts the flight time of tracks relative to the flight time zero location";
}

void TrackFlightTimeAdjuster::apply(std::vector<CDCTrack>& tracks)
{
  for (CDCTrack& track : tracks) {
    CDCTrajectory3D startTrajectory3D = track.getStartTrajectory3D();
    const Vector2D pos2D = startTrajectory3D.getSupport().xy();
    const Vector2D dir2D = startTrajectory3D.getFlightDirection3DAtSupport().xy();
    const double alpha = pos2D.angleWith(dir2D);
    const double beta = 1;
    const double flightTime2D =
      FlightTimeEstimator::instance().getFlightTime2D(pos2D, alpha, beta);
    const double flightTime3D = flightTime2D * hypot2(1, startTrajectory3D.getTanLambda());
    startTrajectory3D.setFlightTime(flightTime3D);
    track.setStartTrajectory3D(startTrajectory3D);
  }
}
