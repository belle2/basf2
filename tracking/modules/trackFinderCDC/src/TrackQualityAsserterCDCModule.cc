#include <tracking/modules/trackFinderCDC/TrackQualityAsserterCDCModule.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackQualityAsserterCDC);

void TrackQualityAsserterCDCModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  for (CDCTrack& track : tracks) {
    // Reset all hits to not have a background hit (what they should not have anyway)
    for (const CDCRecoHit3D& recoHit : track) {
      recoHit.getWireHit().getAutomatonCell().unsetBackgroundFlag();
    }

    const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
    const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
    const double radius = fabs(trajectory2D.getGlobalCircle().radius());

    // The first hit has - per definition of the trajectory2D - a perpS of 0. We want every other hit to have a perpS greater than 0,
    // especially for curlers. For this, we go through all hits and look for negative perpS. If we have found one, we shift it to positive values
    for (CDCRecoHit3D& recoHit : track) {
      double currentPerpS = recoHit.getPerpS();
      if (currentPerpS < 0) {
        recoHit.setPerpS(2 * TMath::Pi() * radius + currentPerpS);
      }
    }

    track.sortByPerpS();

    // Skip this track if it is no curler
    if (not trajectory2D.getOuterExit().hasNAN()) {
      continue;
    }

    // Find large holes in the perpS of the tracks
    double lastPerpS = std::nan("");
    bool deleteHitsAfterThat = false;

    for (const CDCRecoHit3D& recoHit : track) {
      if (deleteHitsAfterThat) {
        recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      } else {
        double currentPerpS = recoHit.getPerpS() / radius;
        if (not std::isnan(lastPerpS)) {
          if (fabs(currentPerpS - lastPerpS) > m_param_minimalPerpSCut) {
            deleteHitsAfterThat = true;
            recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
          }
        }
        lastPerpS = currentPerpS;
      }
    }

    // Delete all hits that were marked
    track.erase(std::remove_if(track.begin(), track.end(), [](const CDCRecoHit3D & recoHit) -> bool {
      return recoHit.getWireHit().getAutomatonCell().hasBackgroundFlag();
    }), track.end());
  }
}
