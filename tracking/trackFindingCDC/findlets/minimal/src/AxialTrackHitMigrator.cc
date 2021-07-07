/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
