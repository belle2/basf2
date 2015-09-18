#include <tracking/modules/trackFinderCDC/TrackQualityAsserterCDCModule.h>

#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <framework/dataobjects/Helix.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackQualityAsserterCDC);

void TrackQualityAsserterCDCModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  for (CDCTrack& track : tracks) {
    // Reset all hits to not have a background hit (what they should not have anyway)
    trackQualityTools.normalizeHitsAndResetTrajectory(track);
    /*if (track.getStartTrajectory3D().getAbsMom3D() > m_param_minimalMomentum)
      continue;*/

    for (const std::string& correctorFunction : m_param_corrections) {
      if (correctorFunction == "LayerBreak") {
        // more or less GOOD
        trackQualityTools.removeHitsAfterLayerBreak(track);
      } else if (correctorFunction == "LargeAngle") {
        // more or less GOOD
        trackQualityTools.removeHitsInTheBeginningIfAngleLarge(track);
      } else if (correctorFunction == "LargeBreak2") {
        // more or less GOOD
        trackQualityTools.removeHitsAfterLayerBreak2(track);
      } else if (correctorFunction == "OneSuperlayer") {
        // GOOD
        trackQualityTools.removeHitsIfOnlyOneSuperLayer(track);
      } else if (correctorFunction == "Small") {
        // GOOD
        trackQualityTools.removeHitsIfSmall(track);
      } else if (correctorFunction == "B2B") {
        // GOOD
        trackQualityTools.removeHitsOnTheWrongSide(track);
      } else if (correctorFunction == "None") {
        // GOOD
        ;
      } else if (correctorFunction == "Split") {
        CDCTrack newSplittedTrack = trackQualityTools.splitSecondHalfOfTrack(track);
        /*if(newSplittedTrack.size() > 0) {
          tracks.push_back(newSplittedTrack);
        }*/
      } else if (correctorFunction == "ArcLength2D") {
        // ???
        trackQualityTools.removeArcLength2DHoles(track);
      } else if (correctorFunction == "CDCWall") {
        // BAD
        B2FATAL("Do not use this function as it is not working probably.");
        trackQualityTools.removeHitsAfterCDCWall(track);
      } else {
        B2FATAL("Do not know corrector function " << correctorFunction);
      }

      track.removeAllAssignedMarkedHits();
      trackQualityTools.normalizeHitsAndResetTrajectory(track);
    }

    if (track.size() == 0)
      continue;

    trackQualityTools.normalizeHitsAndResetTrajectory(track);
  }

  tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const CDCTrack & track) -> bool {
    return track.size() < 3;
  }), tracks.end());
}
