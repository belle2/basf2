/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackSZFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackSZFitter::getDescription()
{
  return "Use an SZFitter to create the 3D trajectory out of the 2D one.";
}

void TrackSZFitter::apply(std::vector<CDCTrack>& tracks)
{
  // Postprocess each track (=fit)
  for (CDCTrack& track : tracks) {
    const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

    track.shiftToPositiveArcLengths2D();
    track.sortByArcLength2D();

    const CDCTrajectorySZ& szTrajectory = szFitter.fitWithStereoHits(track);
    CDCTrajectory3D newStartTrajectory(track.getStartTrajectory3D().getTrajectory2D(), szTrajectory);
    track.setStartTrajectory3D(newStartTrajectory);
  }
}