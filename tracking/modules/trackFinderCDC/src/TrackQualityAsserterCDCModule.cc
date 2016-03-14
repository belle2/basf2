#include <tracking/modules/trackFinderCDC/TrackQualityAsserterCDCModule.h>

#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <framework/dataobjects/Helix.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackQualityAsserterCDC);

void TrackQualityAsserterCDCModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  // Only use the not fitted tracks if set
  if (m_param_onlyNotFittedTracks) {
    tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const CDCTrack & track) {
      const genfit::TrackCand* trackCand = track.getRelatedGenfitTrackCandidate();
      if (trackCand == nullptr) {
        B2WARNING("Can not decide whether to correct this track or not, as it has no related genfit::TrackCand. Skipping.");
        return true;
      }
      RecoTrack* recoTrack = DataStore::Instance().getRelated<RecoTrack>(trackCand, "GF2Tracks");
      if (recoTrack == nullptr) {
        B2WARNING("Can not decide whether to correct this track or not, as it has no related RecoTrack. Skipping.");
        return true;
      }

      if (recoTrack->wasLastFitSucessfull()) {
        return true;
      }

      return false;
    }), tracks.end());
  }

  std::vector<CDCTrack> splittedTracks;

  for (CDCTrack& track : tracks) {
    // Reset all hits to not have a background hit (what they should not have anyway)
    TrackQualityTools::normalizeHitsAndResetTrajectory(track);

    for (const std::string& correctorFunction : m_param_corrections) {
      if (correctorFunction == "LayerBreak") {
        // GOOD
        TrackQualityTools::removeHitsAfterLayerBreak(track);
      } else if (correctorFunction == "LargeAngle") {
        // GOOD
        TrackQualityTools::removeHitsInTheBeginningIfAngleLarge(track);
      } else if (correctorFunction == "LargeBreak2") {
        // GOOD
        TrackQualityTools::removeHitsAfterLayerBreak2(track);
      } else if (correctorFunction == "OneSuperlayer") {
        // GOOD
        TrackQualityTools::removeHitsIfOnlyOneSuperLayer(track);
      } else if (correctorFunction == "Small") {
        // GOOD
        TrackQualityTools::removeHitsIfSmall(track);
      } else if (correctorFunction == "B2B") {
        // GOOD
        TrackQualityTools::removeHitsOnTheWrongSide(track);
      } else if (correctorFunction == "MoveToNextAxial") {
        // GOOD
        TrackQualityTools::moveToNextAxialLayer(track);
      } else if (correctorFunction == "None") {
        // GOOD :-)
        ;
      } else if (correctorFunction == "Split") {
        // Working, but makes it not better
        TrackQualityTools::splitSecondHalfOfTrack(track, splittedTracks);
      } else if (correctorFunction == "ArcLength2D") {
        // ???
        TrackQualityTools::removeArcLength2DHoles(track);
      } else if (correctorFunction == "CDCWall") {
        // BAD
        B2FATAL("Do not use this function as it is not working probably.");
        TrackQualityTools::removeHitsAfterCDCWall(track);
      } else {
        B2FATAL("Do not know corrector function " << correctorFunction);
      }

      track.removeAllAssignedMarkedHits();
      TrackQualityTools::normalizeHitsAndResetTrajectory(track);
    }

    if (track.size() == 0)
      continue;
  }

  tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const CDCTrack & track) -> bool {
    return track.size() < 3;
  }), tracks.end());

  for (const CDCTrack& splittedTrack : splittedTracks) {
    tracks.push_back(splittedTrack);
  }
}
