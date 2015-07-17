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

    /*// Check if the second hit has a negative perpS. If yes, the trajectory is reversed
    if(track.size() < 2) continue;
    const CDCRecoHit3D & secondHit = track[1];
    if(secondHit.getPerpS() < 0) {
      const CDCTrajectory2D & reversedTrajectory2D = trajectory2D.reversed();
      for(CDCRecoHit3D & recoHit : track) {
        recoHit.setPerpS(-recoHit.getPerpS());
      }

      CDCTrajectory3D reversedTrajectory3D(reversedTrajectory2D, trajectorySZ);
      track.setStartTrajectory3D(reversedTrajectory3D);
    }*/



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

  tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const CDCTrack & track) -> bool {
    return track.size() < 3;
  }));

  for (const CDCTrack& track : tracks) {
    B2INFO(track.size());
  }
}
