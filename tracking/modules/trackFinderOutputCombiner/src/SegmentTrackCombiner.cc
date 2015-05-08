#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentTrackCombiner);

void SegmentTrackCombinerModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  m_combiner.fillWith(tracks, segments);
  m_combiner.combine();

  // Delete all used segments
  segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCRecoSegment2D & segment) -> bool {
    return segment.getAutomatonCell().hasTakenFlag();
  }), segments.end());

  B2WARNING("After all there are " << segments.size() << " Segments left in this event.")

  // Reset the taken flag for the hits of all the unused segments
  for (const CDCRecoSegment2D& segment : segments) {
    for (const CDCRecoHit2D& recoHit : segment) {
      recoHit.getWireHit().getAutomatonCell().unsetTakenFlag();
    }
  }

  m_combiner.clear();
}
