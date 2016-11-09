#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <utility>
#include <tracking/trackFindingCDC/collectors/stereo_segments/StereoSegmentTrackMatcherFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::vector<WithWeight<const CDCRecoSegment2D*> >
StereoSegmentTrackMatcherFilter::match(const CDCTrack& track,
                                       const std::vector<CDCRecoSegment2D>& recoSegments)
{
  if (m_filter.needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  // List of matches
  std::vector<WithWeight<const CDCRecoSegment2D*>> matches;

  // Reconstruct the hits to the track and check their filter result
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  for (const CDCRecoSegment2D& recoSegment2D : recoSegments) {
    if (recoSegment2D.getStereoKind() != EStereoKind::c_Axial and not recoSegment2D.getAutomatonCell().hasTakenFlag()
        and not recoSegment2D.isFullyTaken(2)) {
      CDCRecoSegment3D recoSegment3D = CDCRecoSegment3D::reconstruct(recoSegment2D, trajectory2D);
      const Weight weight = m_filter({{&recoSegment2D, recoSegment3D}, track});
      if (not std::isnan(weight)) {
        matches.emplace_back(&recoSegment2D, weight);
      }
    }
  }

  return matches;
}
