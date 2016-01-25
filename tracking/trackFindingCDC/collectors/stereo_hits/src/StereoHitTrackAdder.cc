#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitTrackAdder.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void StereoHitTrackAdder::add(CDCTrack& track, const std::vector<WithWeight<const CDCRLTaggedWireHit*>>& matchedHits)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  for (const auto& rlWireHitWithWeight : matchedHits) {
    //const CDCRLTaggedWireHit* rlWireHit = rlWireHitWithWeight;
    Vector3D recoPos3D = rlWireHitWithWeight->reconstruct3D(trajectory2D);
    double arcLength2D = trajectory2D.calcArcLength2D(recoPos3D.xy());
    if (isCurler and arcLength2D < 0) {
      arcLength2D += 2 * TMath::Pi() * radius;
    }

    B2ASSERT(not rlWireHitWithWeight->getWireHit().getAutomatonCell().hasTakenFlag(), "A stereo hit should not be added twice!");
    track.emplace_back(*rlWireHitWithWeight, recoPos3D, arcLength2D);
    rlWireHitWithWeight->getWireHit().getAutomatonCell().setTakenFlag();
  }
}
