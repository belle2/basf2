#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectories.h>
#include <framework/dataobjects/Helix.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace std;

void TrackQualityTools::normalizeHitsAndResetTrajectory(CDCTrack& track) const
{
  if (track.size() <= 1) {
    return;
  }

  // We start with the most inner hit
  track.sort();

  // We then set the trajectory to start with this point
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const short charge = track.getStartChargeSign();

  if (abs(charge) != 1)
    return;

  Belle2::Helix helix(trajectory3D.getSupport(), trajectory3D.getMom3DAtSupport() , charge, 1.5);
  const Vector2D& startPosition = track.front().getRecoPos2D();
  const double perpSAtFront = helix.getArcLength2DAtXY(startPosition.x(), startPosition.y());
  Vector3D newStartPosition(helix.getPositionAtArcLength2D(perpSAtFront));
  Vector3D newStartMomentum(helix.getMomentumAtArcLength2D(perpSAtFront, 1.5));
  const CDCTrajectory2D currentTrajectory2D(newStartPosition.xy(), newStartMomentum.xy(), charge);

  // Check if we have to reverse the trajectory. This is done by counting the number of hits
  // with positive an with negative perpS
  bool reverseTrajectory = false;
  unsigned int numberOfPositiveHits = 0;
  for (CDCRecoHit3D& recoHit : track) {
    const double currentPerpS = recoHit.getPerpS(currentTrajectory2D);
    if (currentPerpS > 0) {
      numberOfPositiveHits++;
    }
  }
  // We reset the trajectory here to start at the newStartPosition of the first hit
  reverseTrajectory = 2 * numberOfPositiveHits < track.size();

  if (reverseTrajectory)
    track.setStartTrajectory3D(CDCTrajectory3D(newStartPosition, -newStartMomentum, -charge));
  else
    track.setStartTrajectory3D(CDCTrajectory3D(newStartPosition, newStartMomentum, charge));

  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = fabs(trajectory2D.getGlobalCircle().radius());

  // The first hit has - per definition of the trajectory2D - a perpS of 0. We want every other hit to have a perpS greater than 0,
  // especially for curlers. For this, we go through all hits and look for negative perpS.
  // If we have found one, we shift it to positive values
  for (CDCRecoHit3D& recoHit : track) {
    recoHit.getWireHit().getAutomatonCell().unsetBackgroundFlag();
    recoHit.getWireHit().getAutomatonCell().setTakenFlag();
    double currentPerpS = recoHit.getPerpS(trajectory2D);
    if (currentPerpS < 0) {
      recoHit.setPerpS(2 * TMath::Pi() * radius + currentPerpS);
    } else {
      recoHit.setPerpS(currentPerpS);
    }
  }

  // We can now sort by perpS
  track.sortByPerpS();
}
