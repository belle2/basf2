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
#include <deque>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentTrackAdderWithNormalization::SegmentTrackAdderWithNormalization()
  : Super()
{
  addProcessingSignalListener(&m_trackNormalizer);
  addProcessingSignalListener(&m_singleHitSelector);
}

void SegmentTrackAdderWithNormalization::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_singleHitSelector.exposeParameters(moduleParamList, prefixed(prefix, "hitSelector"));
}

std::string SegmentTrackAdderWithNormalization::getDescription()
{
  return "Add the matched segments to the tracks and normalize the tracks afterwards. Also deletes all "
         "hits from tracks, that are now part in another track (or should not be part in any).";
}

void SegmentTrackAdderWithNormalization::apply(std::vector<WeightedRelation<CDCTrack, const CDCSegment2D>>& relations,
                                               std::vector<CDCTrack>& tracks, const std::vector<CDCSegment2D>& segments)
{
  // Storage space for the hits - deque for address persistence
  std::deque<CDCRecoHit3D> recoHits3D;

  // Relations for the matching tracks
  std::vector<WeightedRelation<CDCTrack, const CDCRecoHit3D>> trackHitRelations;
  trackHitRelations.reserve(2500);

  // Add the original hit content of the track with low priority
  for (CDCTrack& track : tracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      recoHits3D.push_back(recoHit3D);
      trackHitRelations.push_back({&track, -INFINITY, &recoHits3D.back()});
    }
  }

  // Add the relations for the matched segments with the match weight
  for (const auto& relation : relations) {
    CDCTrack* track = relation.getFrom();
    const CDCSegment2D& segment = *(relation.getTo());
    const Weight weight = relation.getWeight();
    const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();

    for (const CDCRecoHit2D& recoHit : segment) {
      MayBePtr<const CDCRecoHit3D> ptrRecoHit3D = track->find(recoHit.getWireHit());
      if (ptrRecoHit3D == nullptr) {
        CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit.getRLWireHit(), trajectory2D);
        assert(recoHit3D.getRLInfo() == recoHit.getRLInfo());
        recoHits3D.push_back(recoHit3D);
        trackHitRelations.push_back({track, weight, &recoHits3D.back()});
      } else {
        recoHits3D.push_back(*ptrRecoHit3D);
        trackHitRelations.push_back({track, weight, &recoHits3D.back()});
      }
    }
    segment.getAutomatonCell().setTakenFlag();
  }

  // Add also those segments, that have no track-partner and schedule them for removal
  for (const CDCSegment2D& segment : segments) {
    // Skip segment already used in the steps before or marked outside as already taken.
    if (segment.getAutomatonCell().hasTakenFlag()) continue;

    // Add hit with destination track nullptr
    for (const CDCRecoHit2D& recoHit : segment) {
      recoHits3D.push_back({recoHit.getRLWireHit(), Vector3D(recoHit.getRecoPos2D()), 0});
      trackHitRelations.push_back({nullptr, 0, &recoHits3D.back()});
    }
  }

  // Thin out those weighted relations, by selecting only the best matching track for each hit.
  std::sort(trackHitRelations.begin(), trackHitRelations.end());
  m_singleHitSelector.apply(trackHitRelations);

  // Remove all hits from the tracks in order to rebuild them completely
  for (CDCTrack& track : tracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      recoHit3D.getWireHit().getAutomatonCell().unsetTakenFlag();
    }
    track.clear();
  }

  // Now add the hits to their destination tracks
  for (const auto& trackHitRelation : trackHitRelations) {
    CDCTrack* track = trackHitRelation.getFrom();
    const CDCRecoHit3D* recoHit3D = trackHitRelation.getTo();

    if (track == nullptr) continue;

    track->push_back(*recoHit3D);
    recoHit3D->getWireHit()->setTakenFlag();
  }

  // Establish the ordering
  for (CDCTrack& track : tracks) {
    track.sortByArcLength2D();
  }

  // Normalize the trajectory and hit contents of the tracks
  m_trackNormalizer.apply(tracks);
}
