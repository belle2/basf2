/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun, Oliver Frost                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackMerger.h>

#include <tracking/trackFindingCDC/processing/TrackMerger.h>
#include <tracking/trackFindingCDC/processing/TrackProcessor.h>
#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/processing/HitProcessor.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string AxialTrackMerger::getDescription()
{
  return "Merges axial tracks found in the legendre search";
}

void AxialTrackMerger::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "minFitProb"),
                                m_param_minFitProb,
                                "Minimal fit probability of the common fit "
                                "of two tracks to be eligible for merging",
                                m_param_minFitProb);
}

void AxialTrackMerger::apply(std::vector<CDCTrack>& axialTracks,
                             const std::vector<const CDCWireHit*>& allAxialWireHits)
{
  // Check quality of the track basing on holes on the trajectory;
  // if holes exist then track is splitted
  for (CDCTrack& track : axialTracks) {
    if (track.size() < 5) continue;
    HitProcessor::removeHitsAfterSuperLayerBreak(track);
    TrackQualityTools::normalizeTrack(track);
  }

  // Update tracks before storing to DataStore
  for (CDCTrack& track : axialTracks) {
    TrackQualityTools::normalizeTrack(track);
  }

  // Remove bad tracks
  TrackProcessor::deleteShortTracks(axialTracks);
  TrackProcessor::deleteTracksWithLowFitProbability(axialTracks);

  // Perform tracks merging
  TrackMerger::doTracksMerging(axialTracks, allAxialWireHits);

  HitProcessor::resetMaskedHits(axialTracks, allAxialWireHits);
  erase_remove_if(axialTracks, Size() == 0u);
}
