#include <tracking/trackFindingCDC/collectors/stereo_segments/StereoSegmentTrackAdder.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <utility>

using namespace Belle2;
using namespace TrackFindingCDC;

void StereoSegmentTrackAdder::add(CDCTrack& track, const std::vector<WithWeight<const CDCRecoSegment2D*>>& matchedSegments)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  for (const auto& recoSegmentWithWeight : matchedSegments) {
    B2ASSERT("A stereo segment should not be added twice!", not recoSegmentWithWeight->getAutomatonCell().hasTakenFlag());
    recoSegmentWithWeight->getAutomatonCell().setTakenFlag();

    for (const CDCRecoHit2D& recoHit : recoSegmentWithWeight->items()) {
      Vector3D recoPos3D = recoHit.reconstruct3D(trajectory2D);
      double arcLength2D = trajectory2D.calcArcLength2D(recoPos3D.xy());
      if (isCurler and arcLength2D < 0) {
        arcLength2D += 2 * TMath::Pi() * radius;
      }

      B2ASSERT("This stereo hit has already a taken flag!", not recoHit.getWireHit().getAutomatonCell().hasTakenFlag()); {

        track.emplace_back(recoHit.getRLWireHit(), recoPos3D, arcLength2D);
        recoHit.getWireHit().getAutomatonCell().setTakenFlag();
      }
    }
  }
}
