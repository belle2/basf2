#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentTrackCombiner);

void SegmentTrackCombinerModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  m_lookUp.fillWith(tracks, segments);
  m_lookUp.combine();

  // Delete all used segments
  segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCRecoSegment2D & segment) -> bool {
    return segment.size() == 0 or segment.getAutomatonCell().hasTakenFlag();
  }), segments.end());

  B2WARNING("After all there are " << segments.size() << " Segments left in this event.")
}
