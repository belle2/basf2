#include <tracking/modules/trackFinderCDC/TrackQualityAsserterCDCModule.h>

#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <framework/dataobjects/Helix.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackQualityAsserterCDC);

void removeSecondHalfOfTrack(CDCTrack& track)
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const Vector2D origin(0, 0);

  const double perpSOfOrigin = trajectory2D.calcArcLength2D(origin);

  unsigned int hitsWithPositivePerpS = 0;
  unsigned int hitsWithNegativePerpS = 0;

  for (CDCRecoHit3D& recoHit : track) {
    const double currentPerpS = trajectory2D.calcArcLength2D(recoHit.getRecoPos2D()) - perpSOfOrigin;
    recoHit.setArcLength2D(currentPerpS);

    if (currentPerpS > 0) {
      hitsWithPositivePerpS++;
    } else {
      hitsWithNegativePerpS++;
    }
  }

  bool negativeHitsAreMore = hitsWithNegativePerpS > hitsWithPositivePerpS;

  for (const CDCRecoHit3D& recoHit : track) {
    const double currentPerpS = recoHit.getArcLength2D();
    if ((negativeHitsAreMore and currentPerpS > 0) or (not negativeHitsAreMore and currentPerpS < 0)) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
    }
  }
}

void removeHitsAfterLayerBreak(CDCTrack& track)
{
  ILayerType lastLayer = -1;
  Vector2D lastWirePosition;
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : track) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      continue;
    }

    const ILayerType currentLayer = recoHit.getWire().getICLayer();
    const Vector2D& currentPosition = recoHit.getRecoPos2D();
    if (lastLayer != -1) {
      const ILayerType delta = currentLayer - lastLayer;
      const double distance = (currentPosition - lastWirePosition).norm();
      if (abs(delta) > 4 or distance > 50) {
        removeAfterThis = true;
        recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      }
    }

    lastWirePosition = currentPosition;
    lastLayer = currentLayer;
  }
}

void removeBack2BackStuff(CDCTrack& track)
{
  const Vector2D origin(0, 0);

  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();

  if (trajectory2D.getOuterExit().hasNAN()) {
    return;
  }

  track.sort();
  double perpSOfInnermostHit = track.front().getPerpS();
  track.sortByPerpS();

  for (const CDCRecoHit3D& recoHit : track) {
    if (recoHit.getPerpS() - perpSOfInnermostHit < 0) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
    }
  }
}

void removePerpSHoles(CDCTrack& track)
{
  double lastPerpS = std::nan("");

  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : track) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      continue;
    }

    const double currentPerpS = recoHit.getPerpS();
    if (not std::isnan(lastPerpS)) {
      const double delta = currentPerpS - lastPerpS;
      if (delta > 100) {
        removeAfterThis = true;
        recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      }
    }

    lastPerpS = currentPerpS;
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
  bool deleteTrack = track.size() < 7;

  if (deleteTrack) {
    for (const CDCRecoHit3D& recoHit : track) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
    }
  }
}

void revertTrajectoriesPointingToTheCenter(CDCTrack& track)
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const Vector2D& outerExit = trajectory2D.getOuterExit();
  const double radius = trajectory2D.getLocalCircle().radius();

  const Vector2D& innerExit = trajectory2D.getInnerExit();
  if (innerExit.hasNAN()) {
    // Huh?
    B2WARNING("Strange track...")
    return;
  }

  const double perpSOfFirstInnerExit = trajectory2D.calcPerpS(innerExit);

  if (perpSOfFirstInnerExit > 0.2 * radius) {
    return;
  }

  // We want to determine the entry into the cdc
  const CDCTrajectory2D reversedTrajectory(trajectory2D.getSupport(), -trajectory2D.getMom2DAtSupport(),
                                           -trajectory2D.getChargeSign());
  const Vector2D& reentry = reversedTrajectory.getInnerExit();
  if (reentry.hasNAN()) {
    // The track seems to touch the border of the inner CDC. This is probably ok.
    return;
  }

  const double perpSOfSecondExit = trajectory2D.calcPerpS(reentry);


  for (const CDCRecoHit3D& recoHit : track) {
    const double currentPerpS = recoHit.getPerpS();
    if (currentPerpS < perpSOfSecondExit and currentPerpS >= perpSOfFirstInnerExit) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
    }
  }
}

void removeHitsInTheBeginningIfAngleLarge(CDCTrack& track)
{
  double lastAngle = NAN;
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : boost::adaptors::reverse(track)) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      continue;
    }

    const double currentAngle = recoHit.getRecoPos2D().phi();
    if (not std::isnan(lastAngle)) {
      const double delta = currentAngle - lastAngle;
      const double normalizedDelta = std::min(TVector2::Phi_0_2pi(delta), TVector2::Phi_0_2pi(-delta));
      if (fabs(normalizedDelta) > 0.7) {
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

void removeHitsAfterCDCWall(CDCTrack& track)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const Vector2D& outerExit = trajectory2D.getOuterExit();

  if (outerExit.hasNAN()) {
    return;
  }

  const double perpSOfExit = trajectory2D.calcPerpS(outerExit);
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : track) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      continue;
    }

    const double currentPerpS = recoHit.getPerpS();
    if (currentPerpS > perpSOfExit) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      removeAfterThis = true;
    }
  }
}

void TrackQualityAsserterCDCModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  for (CDCTrack& track : tracks) {
    // Reset all hits to not have a background hit (what they should not have anyway)
    trackQualityTools.normalizeHitsAndResetTrajectory(track);
    if (track.getStartTrajectory3D().getAbsMom3D() > m_param_minimalMomentum)
      continue;

    for (const std::string& correctorFunction : m_param_corrections) {
      if (correctorFunction == "LayerBreak") {
        removeHitsAfterLayerBreak(track);
      } else if (correctorFunction == "LargeAngle") {
        removeHitsInTheBeginningIfAngleLarge(track);
      } else if (correctorFunction == "OneSuperlayer") {
        removeHitsIfOnlyOneSuperLayer(track);
      } else if (correctorFunction == "Small") {
        removeHitsIfSmall(track);
      } else if (correctorFunction == "B2B") {
        removeBack2BackStuff(track);
      } else if (correctorFunction == "PerpS") {
        removePerpSHoles(track);
      } else if (correctorFunction == "CDCEnd") {
        removeHitsAfterCDCWall(track);
      } else if (correctorFunction == "CenterPointing") {
        revertTrajectoriesPointingToTheCenter(track);
      } else {
        B2FATAL("Do not know corrector function " << correctorFunction);
      }

      removeAllMarkedHits(track);
      trackQualityTools.normalizeHitsAndResetTrajectory(track);
    }

    if (track.size() == 0)
      continue;

    track.sortByArcLength2D();

    resetTrajectoryOfTrack(track);
  }

  tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const CDCTrack & track) -> bool {
    return track.size() < 3;
  }), tracks.end());
}
