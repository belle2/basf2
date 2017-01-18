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
  addProcessingSignalListener(&m_singleHitSelector);
}

/// Expose the parameters of the sub-findlets.
void SegmentTrackAdderWithNormalization::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_trackNormalizer.exposeParameters(moduleParamList, prefix);
  m_singleHitSelector.exposeParameters(moduleParamList, prefixed(prefix, "HitSelector"));

  moduleParamList->getParameter<bool>("HitSelectorUseOnlySingleBestCandidate").setDefaultValue(false);
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
  // Create weighted relations between all hits in the segments and their tracks (matched over the segment-track-relation)
  for (const auto& relation : relations) {
    CDCTrack* track = relation.getFrom();
    const CDCSegment2D& segment = *(relation.getTo());
    const Weight weight = relation.getWeight();

    segment.getAutomatonCell().setTakenFlag();

    for (const CDCRecoHit2D& recoHit : segment) {
      m_relationsFromTracksToHits.emplace_back(track, weight, &recoHit);
    }
  }

  // Add also those segments, that have no track-partner (and therefore do not have a taken flag)
  for (const CDCSegment2D& segment : segments) {
    // hits were already used in the step before
    if (segment.getAutomatonCell().hasTakenFlag()) {
      continue;
    }

    for (const CDCRecoHit2D& recoHit : segment) {
      m_relationsFromTracksToHits.emplace_back(nullptr, 0, &recoHit);
    }
  }

  std::sort(m_relationsFromTracksToHits.begin(), m_relationsFromTracksToHits.end());

  // Thin out those weighted relations, by selecting only the best matching track for each hit
  m_singleHitSelector.apply(m_relationsFromTracksToHits);

  // Now we have a list of relations between hits and track pointers
  for (const auto& relation : m_relationsFromTracksToHits) {
    const CDCRecoHit2D* recoHit = relation.getTo();
    const CDCWireHit* cdcWireHit = &(recoHit->getWireHit());
    CDCTrack* track = relation.getFrom();

    if (track) {
      const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();

      AutomatonCell& automatonCell = cdcWireHit->getAutomatonCell();

      const auto& trackHitAndSegmentHitAreTheSame = [&cdcWireHit](const CDCRecoHit3D & recoHit3D) {
        return &(recoHit3D.getWireHit()) == cdcWireHit;
      };

      // Do only add the hit, if it is not already present in the track.
      if (not any(*track, trackHitAndSegmentHitAreTheSame)) {
        CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit->getRLWireHit(), trajectory2D);
        track->push_back(recoHit3D);
        automatonCell.setTakenFlag();
      }
    }

    m_mapHitsToMatchedTracks.insert({cdcWireHit, track});
  }

  // Now go through all the tracks and delete those hits, that are now part of another track or not matched to any
  // track at all
  const auto& hitIsInOtherTrack = [this](const CDCTrack & thisTrack, const CDCRecoHit3D & recoHit) {
    const CDCWireHit* cdcWireHit = &(recoHit.getWireHit());
    // If the hit is not part of any segments, it should stay
    if (m_mapHitsToMatchedTracks.find(cdcWireHit) == m_mapHitsToMatchedTracks.end()) {
      return false;
    }

    const CDCTrack* matchedTrack = m_mapHitsToMatchedTracks[cdcWireHit];
    // If the segment it belonged to, was not matched to any track, the matched track is a nullptr.
    // This means we delete the hit from the track and untick its taken flag.
    if (not matchedTrack) {
      recoHit.getWireHit().getAutomatonCell().unsetTakenFlag();
      return true;
    }

    // If the track, this hit should belong to (because the segment was matched to this track),
    // is the track we are currently looking on, the hit can stay. If not, the hit should be deleted from
    // this track. We do not have to untick the taken flag, because the hit is still used (by the other track).
    return matchedTrack != &thisTrack;
  };

  for (CDCTrack& track : tracks) {
    // Will call hitIsInOtherTrack(track, hit) for each hit in the track and remove those, where
    // hitIsInOtherTrack yields true.
    erase_remove_if(track,
                    std::bind(hitIsInOtherTrack, std::cref(track), std::placeholders::_1));
  }

  // Normalize the trajectory and hit contents of the tracks
  m_trackNormalizer.apply(tracks);
}