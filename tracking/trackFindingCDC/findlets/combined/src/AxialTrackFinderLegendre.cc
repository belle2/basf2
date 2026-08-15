/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/AxialTrackFinderLegendre.h>

#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>

#include <tracking/trackingUtilities/utilities/StringManipulation.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

AxialTrackFinderLegendre::AxialTrackFinderLegendre()
{
  addProcessingSignalListener(&m_nonCurlerAxialTrackCreatorHitLegendre);
  addProcessingSignalListener(&m_nonCurlersWithIncreasingThresholdAxialTrackCreatorHitLegendre);
  addProcessingSignalListener(&m_fullRangeAxialTrackCreatorHitLegendre);
  addProcessingSignalListener(&m_axialTrackHitMigrator);
  addProcessingSignalListener(&m_axialTrackMerger);
}

std::string AxialTrackFinderLegendre::getDescription()
{
  return "Performs the pattern recognition in the CDC with the Legendre hough finder";
}

void AxialTrackFinderLegendre::exposeParameters(ModuleParamList* moduleParamList,
                                                const std::string& prefix)
{
  m_axialTrackHitMigrator.exposeParameters(moduleParamList, prefix);
  m_axialTrackMerger.exposeParameters(moduleParamList, prefixed("merge", prefix));
  // No parameters exposed for the Legendre passes
}

void AxialTrackFinderLegendre::apply(const std::vector<TrackingUtilities::CDCWireHit>& wireHits,
                                     std::vector<TrackingUtilities::CDCTrack>& tracks)
{
  B2DEBUG(25, "**********   CDCTrackingModule  ************");

  // Acquire the axial hits
  std::vector<const TrackingUtilities::CDCWireHit*> axialWireHits;
  axialWireHits.reserve(wireHits.size());
  for (const TrackingUtilities::CDCWireHit& wireHit : wireHits) {
    wireHit->unsetTemporaryFlags();
    wireHit->unsetMaskedFlag();
    if (not wireHit.isAxial()) continue;
    if (wireHit->hasBackgroundFlag()) continue;
    axialWireHits.emplace_back(&wireHit);
  }

  // First Legendre pass
  m_nonCurlerAxialTrackCreatorHitLegendre.apply(axialWireHits, tracks);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(axialWireHits, tracks);

  // Second Legendre pass
  m_nonCurlersWithIncreasingThresholdAxialTrackCreatorHitLegendre.apply(axialWireHits, tracks);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(axialWireHits, tracks);

  // Iterate the last finding pass until no track is found anymore

  // Loop counter to guard against infinite loop
  for (int iPass = 0; iPass < 20; ++iPass) {
    int nCandsAdded = tracks.size();

    // Third Legendre pass
    m_fullRangeAxialTrackCreatorHitLegendre.apply(axialWireHits, tracks);

    // Assign new hits to the tracks
    m_axialTrackHitMigrator.apply(axialWireHits, tracks);

    nCandsAdded = tracks.size() - nCandsAdded;

    if (iPass == 19) B2WARNING("Reached maximal number of Legendre search passes");
    if (nCandsAdded == 0) break;
  }

  // Merge found tracks
  m_axialTrackMerger.apply(tracks, axialWireHits);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(axialWireHits, tracks);

  AxialTrackUtil::deleteShortTracks(tracks);
}
