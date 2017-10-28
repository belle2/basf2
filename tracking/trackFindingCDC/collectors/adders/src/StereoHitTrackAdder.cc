#include <tracking/trackFindingCDC/collectors/adders/StereoHitTrackAdder.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <utility>

using namespace Belle2;
using namespace TrackFindingCDC;

void StereoHitTrackAdder::add(CDCTrack& track, const CDCRLWireHit& hit, Weight weight __attribute__((unused)))
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  Vector3D recoPos3D = hit.reconstruct3D(trajectory2D);
  double arcLength2D = trajectory2D.calcArcLength2D(recoPos3D.xy());
  if (isCurler and arcLength2D < 0) {
    arcLength2D += 2 * TMath::Pi() * radius;
  }

  B2ASSERT("A stereo hit should not be added twice!", not hit.getWireHit().getAutomatonCell().hasTakenFlag());
  track.emplace_back(hit, recoPos3D, arcLength2D);
  hit.getWireHit().getAutomatonCell().setTakenFlag();
}
