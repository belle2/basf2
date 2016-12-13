/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackCombiner.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/utilities/Range.h>

#include <map>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackCombiner::TrackCombiner()
{
}

std::string TrackCombiner::getDescription()
{
  return "Combines two sets of tracks to one final set by merging tracks that have large overlaps.";
}

void TrackCombiner::exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                                     const std::string& prefix __attribute__((unused)))
{
}

void TrackCombiner::apply(const std::vector<CDCTrack>& inputTracks,
                          const std::vector<CDCTrack>& secondInputTracks,
                          std::vector<CDCTrack>& tracks)
{
  std::multimap<const CDCWireHit*, const CDCTrack*> trackByWireHit;
  for (const CDCTrack& track : inputTracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      const CDCWireHit& wireHit = recoHit3D.getWireHit();
      trackByWireHit.emplace(&wireHit, &track);
    }
  }

  // Incoporate the second input tracks in to the first input tracks by looking for large overlaps
  // Very simple approach use the first tracks and add the ones from the second tracks with no overlap to the first
  tracks.insert(tracks.end(), inputTracks.begin(), inputTracks.end());

  for (const CDCTrack& secondTrack : secondInputTracks) {
    std::map<const CDCTrack*, int> overlappingTracks;
    for (const CDCRecoHit3D& recoHit3D : secondTrack) {
      const CDCWireHit& wireHit = recoHit3D.getWireHit();

      auto tracksForWireHit = asRange(trackByWireHit.equal_range(&wireHit));
      for (const std::pair<const CDCWireHit*, const CDCTrack*>& trackForWireHit : tracksForWireHit) {
        const CDCTrack* track = trackForWireHit.second;
        overlappingTracks[track]++;
      }
    }

    if (overlappingTracks.size() == 0) {
      tracks.push_back(secondTrack);
    }
  }
}
