/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/AxialTrackFinderLegendre.h>

#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace Belle2;
using namespace TrackFindingCDC;

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
  return "Performs the pattern recognition in the CDC with the legendre hough finder";
}

void AxialTrackFinderLegendre::exposeParameters(ModuleParamList* moduleParamList,
                                                const std::string& prefix)
{
  m_axialTrackHitMigrator.exposeParameters(moduleParamList, prefix);
  m_axialTrackMerger.exposeParameters(moduleParamList, prefixed("merge", prefix));
  // No parameters exposed for the legendre passes
}

void AxialTrackFinderLegendre::apply(const std::vector<CDCWireHit>& wireHits,
                                     std::vector<CDCTrack>& tracks)
{
  B2DEBUG(100, "**********   CDCTrackingModule  ************");

  // Acquire the axial hits
  std::vector<const CDCWireHit*> axialWireHits;
  axialWireHits.reserve(wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    wireHit->unsetTemporaryFlags();
    wireHit->unsetMaskedFlag();
    if (not wireHit.isAxial()) continue;
    if (wireHit->hasBackgroundFlag()) continue;
    axialWireHits.emplace_back(&wireHit);
  }

  // First legendre pass
  m_nonCurlerAxialTrackCreatorHitLegendre.apply(axialWireHits, tracks);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(axialWireHits, tracks);

  // Second legendre pass
  m_nonCurlersWithIncreasingThresholdAxialTrackCreatorHitLegendre.apply(axialWireHits, tracks);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(axialWireHits, tracks);

  // Iterate the last finding pass until no track is found anymore

  // Loop counter to guard against infinit loop
  for (int iPass = 0; iPass < 20; ++iPass) {
    int nCandsAdded = tracks.size();

    // Third legendre pass
    m_fullRangeAxialTrackCreatorHitLegendre.apply(axialWireHits, tracks);

    // Assign new hits to the tracks
    m_axialTrackHitMigrator.apply(axialWireHits, tracks);

    nCandsAdded = tracks.size() - nCandsAdded;

    if (iPass == 19) B2WARNING("Reached maximal number of legendre search passes");
    if (nCandsAdded == 0) break;
  }

  // Merge found tracks
  m_axialTrackMerger.apply(tracks, axialWireHits);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(axialWireHits, tracks);

  AxialTrackUtil::deleteShortTracks(tracks);
}
