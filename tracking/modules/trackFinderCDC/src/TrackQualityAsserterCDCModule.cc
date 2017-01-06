#include <tracking/modules/trackFinderCDC/TrackQualityAsserterCDCModule.h>

#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/Helix.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackQualityAsserterCDC);

void TrackQualityAsserterCDCModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  // Only use the not fitted tracks if set - was unused
  /*
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

      return false;
    }), tracks.end());
  }
  */

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

      // Delete all hits that were marked
      erase_remove_if(track, [](const CDCRecoHit3D & recoHit3D) -> bool {
        AutomatonCell& automatonCell = recoHit3D.getWireHit().getAutomatonCell();
        if (automatonCell.hasAssignedFlag())
        {
          automatonCell.unsetTakenFlag();
          return true;
        }
        return false;
      });

      TrackQualityTools::normalizeHitsAndResetTrajectory(track);
    } // correctorFunction
  } // track

  erase_remove_if(tracks, [](const CDCTrack & track) -> bool { return track.size() < 3; });

  for (const CDCTrack& splittedTrack : splittedTracks) {
    tracks.push_back(splittedTrack);
  }
}
