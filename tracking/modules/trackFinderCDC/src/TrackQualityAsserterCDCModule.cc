#include <tracking/modules/trackFinderCDC/TrackQualityAsserterCDCModule.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackQualityAsserterCDC);

unsigned int removeSecondHalfOfTrack(CDCTrack& track)
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const Vector2D origin(0, 0);

  const double perpSOfOrigin = trajectory2D.calcPerpS(origin);

  unsigned int hitsWithPositivePerpS = 0;
  unsigned int hitsWithNegativePerpS = 0;

  for (CDCRecoHit3D& recoHit : track) {
    const double currentPerpS = recoHit.getPerpS(trajectory2D) - perpSOfOrigin;
    recoHit.setPerpS(currentPerpS);

    if (currentPerpS > 0) {
      hitsWithPositivePerpS++;
    } else {
      hitsWithNegativePerpS++;
    }
  }

  bool negativeHitsAreMore = hitsWithNegativePerpS > hitsWithPositivePerpS;

  for (const CDCRecoHit3D& recoHit : track) {
    const double currentPerpS = recoHit.getPerpS();
    if ((negativeHitsAreMore and currentPerpS > 0) or (not negativeHitsAreMore and currentPerpS < 0)) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
    }
  }

  track.sortByPerpS();

  if (negativeHitsAreMore) {
    return hitsWithPositivePerpS;
  } else {
    return hitsWithNegativePerpS;
  }
}

void TrackQualityAsserterCDCModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  std::vector<CDCTrack> newTracks;

  for (CDCTrack& track : tracks) {
    // Reset all hits to not have a background hit (what they should not have anyway)
    for (const CDCRecoHit3D& recoHit : track) {
      recoHit.getWireHit().getAutomatonCell().unsetBackgroundFlag();
    }

    unsigned int removableHits = removeSecondHalfOfTrack(track);

    m_numberOfHits += track.size();
    m_numberOfDeletedHits += removableHits;

    SortableVector<CDCRecoHit3D> newTrackHits;
    newTrackHits.reserve(track.size());

    // Delete all hits that were marked
    track.erase(std::remove_if(track.begin(), track.end(), [&newTrackHits](const CDCRecoHit3D & recoHit) -> bool {
      if (recoHit.getWireHit().getAutomatonCell().hasBackgroundFlag())
      {
        newTrackHits.push_back(move(recoHit));
        return true;
      } else {
        return false;
      }
    }), track.end());

    if (newTrackHits.size() > 20) {
      CDCTrack newTrack;
      newTrack.swap(newTrackHits);

      CDCTrajectory3D newTrajectory = track.getStartTrajectory3D();
      newTrajectory.setLocalOrigin(newTrack.front().getRecoPos3D());
      newTrack.setStartTrajectory3D(newTrajectory);

      newTracks.push_back(move(newTrack));
      B2INFO("Adding new track")
    }
  }

  /*for(CDCTrack & newTrack : newTracks)
    tracks.push_back(move(newTrack));*/

  tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const CDCTrack & track) -> bool {
    return track.size() < 3;
  }));
}
