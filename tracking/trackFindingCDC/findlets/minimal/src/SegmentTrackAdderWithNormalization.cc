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

/// Constructor for registering the sub.findlets
SegmentTrackAdderWithNormalization::SegmentTrackAdderWithNormalization() : Super()
{
  addProcessingSignalListener(&m_trackNormalizer);
}

/// Expose the parameters of the sub-findlets.
void SegmentTrackAdderWithNormalization::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
}

/// Short description of the findlet
std::string SegmentTrackAdderWithNormalization::getDescription()
{
  return "Add the matched segments to the tracks and normalize the tracks afterwards. Also deletes all "
         "hits from tracks, that are now part in another track (or should not be part in any).";
}

/// Apply the findlet
void SegmentTrackAdderWithNormalization::apply(std::vector<WeightedRelation<CDCTrack, const CDCSegment2D>>& relations,
                                               std::vector<CDCTrack>& tracks, const std::vector<CDCSegment2D>& segments)
{
  std::vector<std::tuple<std::pair<const CDCWireHit*, double>, CDCTrack*, CDCRecoHit3D>> hitTrackRelations;

  // Establish the ordering
  for (CDCTrack& track : tracks) {
    track.sortByArcLength2D();
  }

  // Add the original hit content of the track with low priority
  for (CDCTrack& track : tracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      hitTrackRelations.push_back({{&recoHit3D.getWireHit(), -INFINITY},  &track, recoHit3D});
    }
  }

  // Add the relations for the matched segments
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

  // Add also those segments, that have no track-partner (and therefore do not have a taken flag)
  for (const CDCSegment2D& segment : segments) {
    // hits were already used in the step before
    if (segment.getAutomatonCell().hasTakenFlag()) continue;

    for (const CDCRecoHit2D& recoHit : segment) {
      hitTrackRelations.push_back({{&recoHit.getWireHit(), 0},  nullptr, CDCRecoHit3D()});
    }
  }

  std::sort(hitTrackRelations.begin(), hitTrackRelations.end(), GreaterOf<First>());

  // Thin out those weighted relations, by selecting only the best matching track for each hit
  erase_unique(hitTrackRelations, EqualOf<FirstOf<First>>());

  // Now go through all the tracks and delete those hits, that are now part of another track or
  // not matched to any track at all
  for (CDCTrack& track : tracks) {
    // Will call hitIsInOtherTrack(hit) for each hit in the track and remove those, where
    // hitIsInOtherTrack yields true.
    const auto& hitIsInOtherTrack = [&hitTrackRelations, &track](const CDCRecoHit3D & recoHit) {

      // Look for the destination track of the given hit
      const CDCWireHit* wireHit = &(recoHit.getWireHit());
      auto itHitTrackRelation = std::partition_point(hitTrackRelations.begin(),
                                                     hitTrackRelations.end(),
                                                     FirstOf<First>() > wireHit);

      assert(itHitTrackRelation != hitTrackRelations.end());
      assert(std::get<0>(*itHitTrackRelation).first == wireHit);

      const CDCTrack* matchedTrack = std::get<1>(*itHitTrackRelation);

      // If the segment it belonged to, was not matched to any track, the matched track is a nullptr.
      // This means we delete the hit from the track and untick its taken flag.
      recoHit.getWireHit().getAutomatonCell().setTakenFlag(matchedTrack != nullptr);

      // If the track, this hit should belong to (because the segment was matched to this track),
      // is the track we are currently looking on, the hit can stay. If not, the hit should be deleted from
      // this track. We do not have to untick the taken flag, because the hit is still used (by the other track).
      return matchedTrack != &track;
    };
    erase_remove_if(track, hitIsInOtherTrack);
  }

  // Now add the hits to their destination tracks
  for (const auto& relation : hitTrackRelations) {
    const CDCWireHit& wireHit = *std::get<0>(relation).first;
    CDCTrack* track = std::get<1>(relation);
    const CDCRecoHit3D& recoHit3D = std::get<2>(relation);

    if (not track) continue;

    auto itRecoHit3D = std::find_if(track->begin(), track->end(), GetWireHit() == wireHit);

    // Do only add the hit, if it is not already present in the track.
    if (itRecoHit3D != track->end()) {
      assert(*itRecoHit3D == recoHit3D);
      track->erase(itRecoHit3D);
    }

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
