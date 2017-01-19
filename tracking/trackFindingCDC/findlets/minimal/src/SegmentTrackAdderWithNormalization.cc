/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentTrackAdderWithNormalization.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentTrackAdderWithNormalization::SegmentTrackAdderWithNormalization()
  : Super()
{
  addProcessingSignalListener(&m_trackNormalizer);
}

void SegmentTrackAdderWithNormalization::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
}

std::string SegmentTrackAdderWithNormalization::getDescription()
{
  return "Add the matched segments to the tracks and normalize the tracks afterwards. Also deletes all "
         "hits from tracks, that are now part in another track (or should not be part in any).";
}

void SegmentTrackAdderWithNormalization::apply(std::vector<WeightedRelation<CDCTrack, const CDCSegment2D>>& relations,
                                               std::vector<CDCTrack>& tracks, const std::vector<CDCSegment2D>& segments)
{
  std::vector<std::tuple<std::pair<const CDCWireHit*, double>, CDCTrack*, CDCRecoHit3D>> hitTrackRelations;
  hitTrackRelations.reserve(2500);

  // Add the original hit content of the track with low priority
  for (CDCTrack& track : tracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      hitTrackRelations.push_back({{&recoHit3D.getWireHit(), -INFINITY},  &track, recoHit3D});
    }
  }

  // Add the relations for the matched segments with the match weight
  for (const auto& relation : relations) {
    CDCTrack* track = relation.getFrom();
    const CDCSegment2D& segment = *(relation.getTo());
    const Weight weight = relation.getWeight();
    const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();

    for (const CDCRecoHit2D& recoHit : segment) {
      auto itRecoHit3D =
        std::find_if(track->begin(), track->end(), GetWireHit() == recoHit.getWireHit());

      if (itRecoHit3D == track->end()) {
        CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit.getRLWireHit(), trajectory2D);
        assert(recoHit3D.getRLInfo() == recoHit.getRLInfo());
        hitTrackRelations.push_back({{&recoHit.getWireHit(), weight},  track, recoHit3D});
      } else {
        CDCRecoHit3D recoHit3D = *itRecoHit3D;
        hitTrackRelations.push_back({{&recoHit.getWireHit(), weight},  track, recoHit3D});
      }
    }
    segment.getAutomatonCell().setTakenFlag();
  }

  // Add also those segments, that have no track-partner and schedule them for removal
  for (const CDCSegment2D& segment : segments) {
    // Skip segment already used in the steps before or marked outside as already taken.
    if (segment.getAutomatonCell().hasTakenFlag()) continue;

    // Add with destination track nullptr
    for (const CDCRecoHit2D& recoHit : segment) {
      hitTrackRelations.push_back({{&recoHit.getWireHit(), 0},  nullptr, CDCRecoHit3D()});
    }
  }

  // Sort such that wire hits are grouped together with the highest weight at the front.
  std::sort(hitTrackRelations.begin(), hitTrackRelations.end(), GreaterOf<First>());

  // Thin out those weighted relations, by selecting only the best matching track for each hit.
  erase_unique(hitTrackRelations, EqualOf<FirstOf<First>>());

  // Remove all hits from the tracks in order to rebuild them completely
  for (CDCTrack& track : tracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      recoHit3D.getWireHit().getAutomatonCell().unsetTakenFlag();
    }
    std::vector<CDCRecoHit3D>& recoHits3D = track;
    recoHits3D.clear();
  }

  // Now add the hits to their destination tracks
  for (const auto& relation : hitTrackRelations) {
    const CDCWireHit& wireHit = *std::get<0>(relation).first;
    CDCTrack* track = std::get<1>(relation);
    const CDCRecoHit3D& recoHit3D = std::get<2>(relation);

    if (track == nullptr) continue;

    track->push_back(recoHit3D);
    wireHit.getAutomatonCell().setTakenFlag();
  }

  // Establish the ordering
  for (CDCTrack& track : tracks) {
    track.sortByArcLength2D();
  }

  // Normalize the trajectory and hit contents of the tracks
  m_trackNormalizer.apply(tracks);
}
