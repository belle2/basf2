/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackNormalizer.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackNormalizer::getDescription()
{
  return "Findlet for normalizing the track (trajectory) into common requirements (e.g. let it start at the first hit etc.)";
}

void TrackNormalizer::apply(std::vector<CDCTrack>& tracks)
{
  for (CDCTrack& track : tracks) {
    TrackQualityTools::normalizeHitsAndResetTrajectory(track);
  }
}