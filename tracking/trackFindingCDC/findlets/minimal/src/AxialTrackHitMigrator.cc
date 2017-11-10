/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackHitMigrator.h>

#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string AxialTrackHitMigrator::getDescription()
{
  return "Exchanges hits between axial tracks based on their distance to the respective "
         "trajectory.";
}

void AxialTrackHitMigrator::exposeParameters(ModuleParamList* moduleParamList,
                                             const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "dropDistance"),
                                m_param_dropDistance,
                                "Distance for a hit to be removed.",
                                m_param_dropDistance);

  moduleParamList->addParameter(prefixed(prefix, "addDistance"),
                                m_param_addDistance,
                                "Distance for a hit to be added.",
                                m_param_addDistance);
}

void AxialTrackHitMigrator::apply(const std::vector<const CDCWireHit*>& axialWireHits,
                                  std::vector<CDCTrack>& axialTracks)
{
  // First release some hits
  for (CDCTrack& track : axialTracks) {
    AxialTrackUtil::deleteHitsFarAwayFromTrajectory(track, m_param_dropDistance);
    AxialTrackUtil::normalizeTrack(track);
  }

  // Now add new ones
  for (CDCTrack& track : axialTracks) {
    if (track.size() < 5) continue;
    AxialTrackUtil::assignNewHitsToTrack(track, axialWireHits, m_param_addDistance);
    AxialTrackUtil::normalizeTrack(track);

    AxialTrackUtil::splitBack2BackTrack(track);
    AxialTrackUtil::normalizeTrack(track);
  }
}
