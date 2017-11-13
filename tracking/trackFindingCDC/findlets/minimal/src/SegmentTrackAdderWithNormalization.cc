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

#include <framework/core/ModuleParamList.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
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
  moduleParamList->addParameter(prefixed(prefix, "removeUnmatchedSegments"),
                                m_param_removeUnmatchedSegments,
                                "Swtich to remove hits in segments that have no matching track from all tracks",
                                m_param_removeUnmatchedSegments);

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

  // We construct track hit relations denoting to which track each hit should belong from 3 sources
  // 1. From the given track segment matches with the weight of the match
  // 2. From the unmatch, untaken segments schedule the hits for removal with lower weight
  // 3. From the original track content with lowest weight
  // Hence if a hit is mentioned in source 1. it takes precedence over 2. and 3. to so on.

  // 1. Add the relations for the matched segments with the match weight
  for (const auto& relation : relations) {
    CDCTrack* track = relation.getFrom();
    const CDCSegment2D& segment = *(relation.getTo());
    const Weight weight = relation.getWeight();
    const CDCTrajectory3D& trajectory3D = track->getStartTrajectory3D();

    for (const CDCRecoHit2D& recoHit : segment) {

      // In case the hit is already in the matched track - keep its reconstructed position
      MayBePtr<const CDCRecoHit3D> ptrRecoHit3D = track->find(recoHit.getWireHit());
      if (ptrRecoHit3D != nullptr) {
        recoHits3D.push_back(*ptrRecoHit3D);
        trackHitRelations.push_back({track, weight, &recoHits3D.back()});
        continue;
      }

      // Otherwise reconstruct the position into the third dimension
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit, trajectory3D);
      if (std::isnan(recoHit3D.getArcLength2D())) {
        B2DEBUG(100, "Had to skip a NAN hit");
        continue;
      }
      recoHits3D.push_back(recoHit3D);
      trackHitRelations.push_back({track, weight, &recoHits3D.back()});
    }
    segment->setTakenFlag();
  }

  // 2. Add also those segments, that have no track-partner and schedule them for removal
  if (m_param_removeUnmatchedSegments) {
    for (const CDCSegment2D& segment : segments) {

      // Skip segment already used in the steps before or marked outside as already taken.
      if (segment->hasTakenFlag()) continue;

      // Add hit with destination track nullptr
      for (const CDCRecoHit2D& recoHit : segment) {
        recoHits3D.push_back({recoHit.getRLWireHit(), Vector3D(recoHit.getRecoPos2D()), 0});
        trackHitRelations.push_back({nullptr, 0, &recoHits3D.back()});
      }
    }
  }

  // 3. Add the original hit content of the track with lowest priority
  for (CDCTrack& track : tracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      recoHits3D.push_back(recoHit3D);
      trackHitRelations.push_back({&track, -INFINITY, &recoHits3D.back()});
    }
  }

  // Thin out the weighted relations by selecting only the best matching track for each hit.
  std::sort(trackHitRelations.begin(), trackHitRelations.end());
  m_singleHitSelector.apply(trackHitRelations);

  // Remove all hits from the tracks in order to rebuild them completely
  for (CDCTrack& track : tracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      recoHit3D.getWireHit()->unsetTakenFlag();
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

  // Drop tracks which have no hits
  TrackFindingCDC::erase_remove_if(tracks, [](const CDCTrack & track) { return track.empty(); });

  // Establish the ordering
  for (CDCTrack& track : tracks) {
    track.sortByArcLength2D();
    CDCTrajectory3D startTrajectory = track.getStartTrajectory3D();
    startTrajectory.setLocalOrigin(track.front().getRecoPos3D());
    track.setStartTrajectory3D(startTrajectory);

    CDCTrajectory3D endTrajectory = track.getEndTrajectory3D();
    endTrajectory.setLocalOrigin(track.back().getRecoPos3D());
    track.setEndTrajectory3D(endTrajectory);
  }

  // Normalize the trajectory and hit contents of the tracks
  m_trackNormalizer.apply(tracks);
}
