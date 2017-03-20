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

#include <boost/range/adaptor/map.hpp>
#include <map>
#include <deque>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  std::array<int, ISuperLayerUtil::c_N>  getNHitsByISuperLayer(const CDCTrack& track)
  {
    std::array<int, ISuperLayerUtil::c_N> nHitsBySLayer = {0};
    for (const CDCRecoHit3D& hit : track) {
      nHitsBySLayer[hit.getISuperLayer()]++;
    }
    return nHitsBySLayer;
  }

  CDCTrack condense(const Path<const CDCSegment3D>& segmentPath)
  {
    CDCTrack result;
    for (const CDCSegment3D* segment : segmentPath) {
      for (const CDCRecoHit3D& recoHit3D : *segment) {
        result.push_back(recoHit3D);
      }
    }
    if (segmentPath.size()) {
      result.setStartTrajectory3D(segmentPath.front()->getTrajectory3D());
    }
    return result;
  }
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
                          std::vector<CDCTrack>& outputTracks)
{
  using InTracks = std::array<const CDCTrack*, 2>;

  // Constructing map of back links
  std::map<const CDCWireHit*, InTracks> inTracksByWireHit;
  for (const CDCTrack& track : inputTracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      const CDCWireHit& wireHit = recoHit3D.getWireHit();
      if (inTracksByWireHit.count(&wireHit) == 0) inTracksByWireHit[&wireHit] = {nullptr, nullptr};
      inTracksByWireHit[&wireHit][0] = &track;
      // Prepare hits for the cellular automaton
      wireHit->unsetTemporaryFlags();
      wireHit->unsetMaskedFlag();
    }
  }

  for (const CDCTrack& track : secondInputTracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      const CDCWireHit& wireHit = recoHit3D.getWireHit();
      if (inTracksByWireHit.count(&wireHit) == 0) inTracksByWireHit[&wireHit] = {nullptr, nullptr};
      inTracksByWireHit[&wireHit][1] = &track;
      // Prepare hits for the cellular automaton
      wireHit->unsetTemporaryFlags();
      wireHit->unsetMaskedFlag();
    }
  }

  // Rank tracks by number of superlayers touched first and number of hits second
  std::vector<std::pair<std::pair<int, size_t>, const CDCTrack*> > rankedTracks;
  for (const CDCTrack& track : inputTracks) {
    std::array<int, ISuperLayerUtil::c_N> nHitsBySLayer = getNHitsByISuperLayer(track);
    int nSL = std::count_if(nHitsBySLayer.begin(), nHitsBySLayer.end(), Id() > 0);
    rankedTracks.push_back({{nSL, track.size()}, &track});
  }
  for (const CDCTrack& track : secondInputTracks) {
    std::array<int, ISuperLayerUtil::c_N> nHitsBySLayer = getNHitsByISuperLayer(track);
    int nSL = std::count_if(nHitsBySLayer.begin(), nHitsBySLayer.end(), Id() > 0);
    rankedTracks.push_back({{nSL, track.size()}, &track});
  }
  std::sort(rankedTracks.begin(), rankedTracks.end(), GreaterOf<First>());

  // Memory for the splitted segments
  std::deque<CDCSegment3D> segments;

  // Memory for the relations between tracks to be followed on linking
  std::vector<WeightedRelation<const CDCSegment3D>> segmentRelations;

  // Split tracks into segments - break segment such that there will be matching pieces with the other tracks
  for (const std::pair<std::pair<int, size_t>, const CDCTrack*>  rankedTrack : rankedTracks) {
    // const std::pair<ISuperLayer, size_t> rank = rankedTrack.first;
    const CDCTrack* track = rankedTrack.second;
    std::vector<std::pair<InTracks, CDCSegment3D> > segmentsInTrack;

    // Split track into segments
    CDCSegment3D* currentSegment = nullptr;
    ISuperLayer lastISuperLayer = -1;
    InTracks lastTracksForHit{{nullptr, nullptr}};
    for (const CDCRecoHit3D& recoHit3D : *track) {
      ISuperLayer iSuperLayer = recoHit3D.getISuperLayer();
      std::array<const CDCTrack*, 2> tracksForWireHit = inTracksByWireHit[&recoHit3D.getWireHit()];
      if (not currentSegment or iSuperLayer != lastISuperLayer or tracksForWireHit != lastTracksForHit) {
        // Make new segments
        segmentsInTrack.push_back({tracksForWireHit, CDCSegment3D()});
        currentSegment = &segmentsInTrack.back().second;
        currentSegment->setTrajectory3D(track->getStartTrajectory3D());
      }
      currentSegment->push_back(recoHit3D);
      lastISuperLayer = iSuperLayer;
      lastTracksForHit = tracksForWireHit;
    }

    // Merge small segments
    auto absorbsSegment = [](std::pair<InTracks, CDCSegment3D>& segment1,
    std::pair<InTracks, CDCSegment3D>& segment2) {

      if (segment1.second.getISuperLayer() != segment2.second.getISuperLayer()) return false;
      if (segment1.second.size() < 3) {
        segment1.first = segment2.first;
      }
      // Absorb segment1 into segment2
      if (segment1.second.size() < 3 or segment2.second.size() < 3 or
          segment1.first == segment2.first) {
        segment1.second.insert(segment1.second.end(),
                               segment2.second.begin(),
                               segment2.second.end());
        return true;
      }
      return false;
    };
    auto itSegmentToDrop = std::unique(segmentsInTrack.begin(), segmentsInTrack.end(), absorbsSegment);
    segmentsInTrack.erase(itSegmentToDrop, segmentsInTrack.end());

    size_t nHits = 0;
    for (std::pair<InTracks, CDCSegment3D>& segment : segmentsInTrack) {
      nHits += segment.second.size();
    }
    B2ASSERT("Expected the number of hits to be the same", nHits == track->size());

    // Push segments to the common pool
    const CDCSegment3D* lastSegment = nullptr;
    for (std::pair<InTracks, CDCSegment3D>& segmentInTrack : segmentsInTrack) {
      const CDCSegment3D& segment = segmentInTrack.second;
      segment->setCellWeight(segment.size());
      segments.push_back(std::move(segment));
      if (lastSegment != nullptr) {
        segmentRelations.push_back({lastSegment, 0, &segments.back()});
      }
      lastSegment = &segments.back();
    }
  }

  // Sort the relations for the lookup
  std::sort(segmentRelations.begin(), segmentRelations.end());

  // Identify common segments and check orientation of tracks

  // Create edges

  // Analyse branches and add additional links where there is no contention between alternatives

  // Extract paths
  // Memory for the track paths generated from the graph.
  std::vector<Path<const CDCSegment3D>> segmentPaths;
  WeightedNeighborhood<const CDCSegment3D> segmentNeighborhood(segmentRelations);
  m_cellularPathFinder.apply(segments, segmentNeighborhood, segmentPaths);

  // Put the linked segments together
  outputTracks.clear();
  for (const std::vector<const CDCSegment3D*>& segmentPath : segmentPaths) {
    // Reject single left over segments
    if (segmentPath.size() < 2) continue;
    outputTracks.push_back(condense(segmentPath));
  }
  return;

  // Incoporate the second input tracks in to the first input tracks by looking for large overlaps
  // Very simple approach use the first tracks and add the ones from the second tracks with no overlap to the first
  outputTracks.insert(outputTracks.end(), inputTracks.begin(), inputTracks.end());

  for (const CDCTrack& secondTrack : secondInputTracks) {
    std::map<const CDCTrack*, int> overlappingTracks;
    for (const CDCRecoHit3D& recoHit3D : secondTrack) {
      const CDCWireHit& wireHit = recoHit3D.getWireHit();
      inTracksByWireHit.equal_range(&recoHit3D.getWireHit());
      auto tracksForWireHit = asRange(inTracksByWireHit.equal_range(&wireHit));
      for (const std::pair<const CDCWireHit*, InTracks>& trackForWireHit : inTracksByWireHit) {
        const CDCTrack* track = trackForWireHit.second[0];
        if (not track) continue;
        overlappingTracks[track]++;
      }
    }
    if (overlappingTracks.size() == 0) {
      outputTracks.push_back(secondTrack);
    }
  }
}
