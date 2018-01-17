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
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/numerics/Index.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/Range.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

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

  std::vector<CDCRLWireHit> getRLWireHitSegment(const CDCSegment3D& segment3D)
  {
    std::vector<CDCRLWireHit> result;
    for (const CDCRecoHit3D& recoHit3D : segment3D) {
      result.push_back(recoHit3D.getRLWireHit());
    }
    return result;
  }

  std::vector<std::pair<std::pair<Index, Index>, CDCRLWireHit>> getCommonRLWireHits(const std::vector<CDCRLWireHit>& rlWireHits1,
                                                             const std::vector<CDCRLWireHit>& rlWireHits2)
  {
    std::vector<std::pair<std::pair<Index, Index>, CDCRLWireHit>> result;
    Index index1 = -1;
    for (const CDCRLWireHit& rlWireHit : rlWireHits1) {
      ++index1;
      auto itRLWireHits2 = std::find(rlWireHits2.begin(), rlWireHits2.end(), rlWireHit);
      if (itRLWireHits2 == rlWireHits2.end()) continue;
      Index index2 = std::distance(rlWireHits2.begin(), itRLWireHits2);
      assert(index2 >= 0);
      result.push_back({{index1, index2}, rlWireHit});
    }
    return result;
  }
}

TrackCombiner::TrackCombiner() = default;

std::string TrackCombiner::getDescription()
{
  return "Combines two sets of tracks to one final set by merging tracks that have large overlaps.";
}

void TrackCombiner::exposeParameters(ModuleParamList* moduleParamList,
                                     const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "identifyCommonSegments"),
                                m_param_identifyCommonSegments,
                                "Activate the identification of common segments",
                                m_param_identifyCommonSegments);
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

  // Also keep a record to which of the tracks the segment
  std::map<const CDCSegment3D*, InTracks> inTracksBySegment;

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
      const InTracks& inTracks = segmentInTrack.first;
      const CDCSegment3D& segment = segmentInTrack.second;
      segment->setCellWeight(segment.size());
      segments.push_back(std::move(segment));
      inTracksBySegment[&segments.back()] = inTracks;
      if (lastSegment != nullptr) {
        segmentRelations.push_back({lastSegment, 0, &segments.back()});
      }
      lastSegment = &segments.back();
    }
  }

  // Sort the relations for the lookup
  std::sort(segmentRelations.begin(), segmentRelations.end());

  // Identify common segments, also checking whether they have the same orientation
  if (m_param_identifyCommonSegments) {
    std::vector<std::pair<const CDCSegment3D*, const CDCSegment3D*>> segmentContainmentRelation;
    for (auto itSegment1 = segments.begin(); itSegment1 != segments.end(); ++itSegment1) {
      const CDCSegment3D& segment1 = *itSegment1;
      std::vector<CDCRLWireHit> rlWireHits1 = getRLWireHitSegment(segment1);
      ISuperLayer iSL1 = segment1.getISuperLayer();
      InTracks inTracks1 = inTracksBySegment[&segment1];
      for (auto itSegment2 = itSegment1 + 1; itSegment2 != segments.end(); ++itSegment2) {
        const CDCSegment3D& segment2 = *itSegment2;

        // Should not happen - here for safety reasons
        if (&segment1 == &segment2) continue;

        ISuperLayer iSL2 = segment2.getISuperLayer();
        if (iSL1 != iSL2) continue;

        InTracks inTracks2 = inTracksBySegment[&segment2];
        if (inTracks1 != inTracks2) continue;

        // Now answering the question if segment 1 is contained in segment 2
        std::vector<CDCRLWireHit> rlWireHits2 = getRLWireHitSegment(segment2);
        std::vector<std::pair<std::pair<int, int>, CDCRLWireHit>> commonRLWireHits =
                                                                 getCommonRLWireHits(rlWireHits1, rlWireHits2);

        // Check for containment, requiring that the majority of the wire hits of one is in two
        // However also require that the two is not too much larger compared to the first one
        bool contains = commonRLWireHits.size() > 2 and
                        commonRLWireHits.size() > rlWireHits1.size() * 0.8 and
                        commonRLWireHits.size() > rlWireHits2.size() * 0.8;
        if (not contains) continue;

        // Now finally check whether both segments point in the same direction
        double n = commonRLWireHits.size();
        double prod11 = 0;
        double prod12 = 0;
        double prod22 = 0;
        double sum1 = 0;
        double sum2 = 0;
        for (const auto& commonRLWireHit : commonRLWireHits) {
          const std::pair<int, int>& indices = commonRLWireHit.first;
          prod11 += indices.first * indices.first;
          prod12 += indices.first * indices.second;
          prod22 += indices.second * indices.second;

          sum1 += indices.first;
          sum2 += indices.second;
        }
        double var1 = (prod11 - sum1 * sum1 / n);
        double var2 = (prod22 - sum2 * sum2 / n);
        double cov12 = (prod12 - sum1 * sum2 / n);
        double cor = cov12 / std::sqrt(var1 * var2);
        if (not(cor > 0.75)) continue;

        segmentContainmentRelation.push_back({&segment1, &segment2});
      }
    }

    // Create additional edges
    std::vector<WeightedRelation<const CDCSegment3D>> additionalSegmentRelations;
    for (std::pair<const CDCSegment3D*, const CDCSegment3D*>& rel : segmentContainmentRelation) {
      for (WeightedRelation<const CDCSegment3D> rel1 :
           asRange(std::equal_range(segmentRelations.begin(), segmentRelations.end(), rel.first))) {
        additionalSegmentRelations.push_back({rel.second, 0, rel1.getTo()});
      }

      for (WeightedRelation<const CDCSegment3D> rel2 : asRange(
             std::equal_range(segmentRelations.begin(), segmentRelations.end(), rel.second))) {
        additionalSegmentRelations.push_back({rel.first, 0, rel2.getFrom()});
      }
    }
    segmentRelations.insert(segmentRelations.end(),
                            additionalSegmentRelations.begin(),
                            additionalSegmentRelations.end());
    std::sort(segmentRelations.begin(), segmentRelations.end());
  }

  // Extract paths
  // Obtain the segments as pointers
  std::vector<const CDCSegment3D*> segmentPtrs = as_pointers<const CDCSegment3D>(segments);

  // Memory for the track paths generated from the graph.
  std::vector<Path<const CDCSegment3D>> segmentPaths;
  m_cellularPathFinder.apply(segmentPtrs, segmentRelations, segmentPaths);

  // Put the linked segments together
  outputTracks.clear();
  for (const std::vector<const CDCSegment3D*>& segmentPath : segmentPaths) {
    // Reject single left over segments
    if (segmentPath.size() < 2) continue;
    outputTracks.push_back(condense(segmentPath));
  }

  // Simple approach
  // Incoporate the second input tracks in to the first input tracks by looking for large overlaps
  // Very simple approach use the first tracks and add the ones from the second tracks with no overlap to the first
  // outputTracks.insert(outputTracks.end(), inputTracks.begin(), inputTracks.end());
  // for (const CDCTrack& secondTrack : secondInputTracks) {
  //   std::map<const CDCTrack*, int> overlappingTracks;
  //   for (const CDCRecoHit3D& recoHit3D : secondTrack) {
  //     const CDCWireHit& wireHit = recoHit3D.getWireHit();
  //     inTracksByWireHit.equal_range(&recoHit3D.getWireHit());
  //     auto tracksForWireHit = asRange(inTracksByWireHit.equal_range(&wireHit));
  //     for (const std::pair<const CDCWireHit*, InTracks>& trackForWireHit : inTracksByWireHit) {
  //       const CDCTrack* track = trackForWireHit.second[0];
  //       if (not track) continue;
  //       overlappingTracks[track]++;
  //     }
  //   }
  //   if (overlappingTracks.size() == 0) {
  //     outputTracks.push_back(secondTrack);
  //   }
  // }
}
