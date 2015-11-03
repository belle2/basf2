#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombinerModule.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

/// Do the combination work. See the SegmentTrackCombiner methods for full details.
void SegmentTrackCombinerImplModule::generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments,
                                              std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  // Resort the perpS information
  for (CDCTrack& track : tracks) {
    TrackQualityTools::normalizeHitsAndResetTrajectory(track);
  }

  m_combiner.fillWith(tracks, segments);
  m_combiner.match(*m_ptrSegmentTrackFilterFirstStep);
  m_combiner.filterSegments(*m_ptrBackgroundSegmentFilter);
  m_combiner.filterOutNewSegments(*m_ptrNewSegmentFilter);
  m_combiner.combine(*m_ptrSegmentTrackFilterSecondStep, *m_ptrSegmentTrainFilter, *m_ptrSegmentInformationListTrackFilter);
  m_combiner.filterTracks(tracks, *m_ptrTrackFilter);
  m_combiner.clearAndRecover();

  // Resort the perpS information
  for (CDCTrack& track : tracks) {
    TrackQualityTools::normalizeHitsAndResetTrajectory(track);
  }

  // Delete all taken segments
  segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCRecoSegment2D & segment) {
    return segment.getAutomatonCell().hasTakenFlag();
  }), segments.end());

}
