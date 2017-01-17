#include <tracking/trackFindingCDC/collectors/adders/SegmentTrackAdder.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void SegmentTrackAdder::add(CDCTrack& track, const CDCSegment2D& segment, Weight weight __attribute__((unused)))
{
  B2ASSERT("Segment has already a taken flag. Something is wrong in the matcher routine!",
           not segment.getAutomatonCell().hasTakenFlag());

  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  // Reconstruct and add all hits from the segment to the track
  for (const CDCRecoHit2D& recoHit : segment) {
    const CDCWireHit& wireHit = recoHit.getWireHit();
    const CDCHit* cdcHit = wireHit.getHit();
    AutomatonCell& automatonCell = wireHit.getAutomatonCell();

    if (m_hitMatchedTrackMap.find(cdcHit) != m_hitMatchedTrackMap.end()) {
      // The hit was already matched to another track
      // (which means there is another segment, which shares this hit and which was already matched to the track)
      // Segments sharing hits can only occur (hopefully!), when two orientations are present. As we only add hits
      // here, there is no difference and we can just skip adding the hit at all.
      if (m_hitMatchedTrackMap[cdcHit] != &track) {
        B2DEBUG(100, "Same hit was matched to two different tracks");
      }
      continue;
    }

    B2ASSERT("Hit was already used in the adding!", not automatonCell.hasMaskedFlag());

    m_hitMatchedTrackMap.insert({cdcHit, &track});

    if (not automatonCell.hasTakenFlag()) {
      // Only add the hit, if it is not already taken.
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit.getRLWireHit(), trajectory2D);
      track.push_back(recoHit3D);
    }

    automatonCell.setTakenFlag();
    automatonCell.setMaskedFlag();
  }

  track.setHasMatchingSegment();
  segment.getAutomatonCell().setTakenFlag();
}
