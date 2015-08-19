#include <tracking/modules/trackFinderCDC/TrackQualityAsserterCDCModule.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <framework/dataobjects/Helix.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackQualityAsserterCDC);

void removeSecondHalfOfTrack(CDCTrack& track)
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
}

void removeHitsAfterLayerBreak(CDCTrack& track)
{
  ILayerType lastLayer = -1;
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : track) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      continue;
    }

    const ILayerType currentLayer = recoHit.getWire().getICLayer();
    if (lastLayer != -1) {
      const ILayerType delta = currentLayer - lastLayer;
      if (abs(delta) > 5) {
        removeAfterThis = true;
        recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      }
    }

    lastLayer = currentLayer;
  }
}

void removeHitsIfOnlyOneSuperLayer(CDCTrack& track)
{
  ISuperLayerType lastLayer = -1;
  bool deleteTrack = true;

  for (const CDCRecoHit3D& recoHit : track) {
    const ISuperLayerType currentLayer = recoHit.getISuperLayer();
    if (lastLayer != -1 and lastLayer != currentLayer) {
      deleteTrack = false;
      break;
    }

    lastLayer = currentLayer;
  }

  if (deleteTrack) {
    for (const CDCRecoHit3D& recoHit : track) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
    }
  }
}

void removeHitsIfSmall(CDCTrack& track)
{
  bool deleteTrack = track.size() < 3;

  if (deleteTrack) {
    for (const CDCRecoHit3D& recoHit : track) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
    }
  }
}

void resetTrack(CDCTrack& track)
{
  for (const CDCRecoHit3D& recoHit : track) {
    recoHit.getWireHit().getAutomatonCell().unsetBackgroundFlag();
  }
}

void removeHitsInTheBeginningIfAngleLarge(CDCTrack& track)
{
  double lastAngle = std::nan("");
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : track.reverseRange()) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      continue;
    }

    const double currentAngle = recoHit.getRecoPos2D().phi();
    if (not std::isnan(lastAngle)) {
      const double delta = currentAngle - lastAngle;
      const double normalizedDelta = std::min(TVector2::Phi_0_2pi(delta), TVector2::Phi_0_2pi(-delta));
      if (fabs(normalizedDelta) > 0.5) {
        removeAfterThis = true;
        recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      }
    }

    lastAngle = currentAngle;
  }
}

void removeAllMarkedHits(CDCTrack& track)
{
  // Delete all hits that were marked
  track.erase(std::remove_if(track.begin(), track.end(), [](const CDCRecoHit3D & recoHit) -> bool {
    if (recoHit.getWireHit().getAutomatonCell().hasBackgroundFlag())
    {
      recoHit.getWireHit().getAutomatonCell().unsetTakenFlag();
      return true;
    } else {
      return false;
    }
  }), track.end());
}

void resetTrajectoryOfTrack(CDCTrack& track)
{
  const CDCTrajectory3D& cdcTrajectory = track.getStartTrajectory3D();

  const short charge = track.getStartChargeSign();
  if (abs(charge) == 1) {
    Belle2::Helix trajectory(cdcTrajectory.getSupport(), cdcTrajectory.getMom3DAtSupport() , charge, 1.5);
    const Vector2D& startPosition = track.front().getRecoPos2D();
    const double perpSAtFront = trajectory.getArcLength2DAtXY(startPosition.x(), startPosition.y());
    const Vector3D& position(trajectory.getPositionAtArcLength2D(perpSAtFront));
    const Vector3D& momentum(trajectory.getMomentumAtArcLength2D(perpSAtFront, 1.5));

    track.setStartTrajectory3D(CDCTrajectory3D(position, momentum, charge));
  }
}

void TrackQualityAsserterCDCModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  for (CDCTrack& track : tracks) {
    // Reset all hits to not have a background hit (what they should not have anyway)
    resetTrack(track);
    if (track.getStartTrajectory3D().getAbsMom3D() > m_param_minimalMomentum)
      continue;

    //removeSecondHalfOfTrack(track);
    removeHitsAfterLayerBreak(track);
    removeAllMarkedHits(track);

    removeHitsInTheBeginningIfAngleLarge(track);
    removeAllMarkedHits(track);

    removeHitsIfOnlyOneSuperLayer(track);
    removeAllMarkedHits(track);

    removeHitsIfSmall(track);
    removeAllMarkedHits(track);

    if (track.size() == 0)
      continue;

    track.sortByPerpS();

    resetTrajectoryOfTrack(track);
  }

  tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const CDCTrack & track) -> bool {
    return track.size() < 3;
  }), tracks.end());
}
