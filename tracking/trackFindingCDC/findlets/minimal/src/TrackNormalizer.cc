/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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