#include <tracking/trackFindingCDC/collectors/adders/SegmentTrackAdder.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <utility>

using namespace Belle2;
using namespace TrackFindingCDC;

void SegmentTrackAdder::add(CDCTrack& track, const CDCSegment2D& segment, Weight weight __attribute__((unused)))
{
  B2ASSERT("Segment has already a taken flag. Something is wrong in the matcher routine!",
           not segment.getAutomatonCell().hasTakenFlag());

  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  // Reconstruct and add all hits from the segment to the track
  for (const CDCRecoHit2D& recoHit : segment) {
    if (recoHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
      continue;
    }
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit.getRLWireHit(), trajectory2D);
    track.push_back(recoHit3D);
    recoHit.getWireHit().getAutomatonCell().setTakenFlag();
  }

  track.setHasMatchingSegment();
  segment.getAutomatonCell().setTakenFlag();
}
