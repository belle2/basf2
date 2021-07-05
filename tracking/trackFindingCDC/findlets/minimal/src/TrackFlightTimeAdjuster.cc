/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
