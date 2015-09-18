#include <tracking/modules/trackFinderCDC/TrackQualityAsserterCDCModule.h>

#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <framework/dataobjects/Helix.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

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

  const double ArcLength2DOfOrigin = trajectory2D.calcArcLength2D(origin);

  unsigned int hitsWithPositiveArcLength2D = 0;
  unsigned int hitsWithNegativeArcLength2D = 0;

  for (CDCRecoHit3D& recoHit : track) {
    const double currentArcLength2D = trajectory2D.calcArcLength2D(recoHit.getRecoPos2D()) - ArcLength2DOfOrigin;
    recoHit.setArcLength2D(currentArcLength2D);

    if (currentArcLength2D > 0) {
      hitsWithPositiveArcLength2D++;
    } else {
      hitsWithNegativeArcLength2D++;
    }
  }

  bool negativeHitsAreMore = hitsWithNegativeArcLength2D > hitsWithPositiveArcLength2D;

  for (const CDCRecoHit3D& recoHit : track) {
    const double currentArcLength2D = recoHit.getArcLength2D();
    if ((negativeHitsAreMore and currentArcLength2D > 0) or (not negativeHitsAreMore and currentArcLength2D < 0)) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
    }
  }
}

void removeHitsAfterLayerBreak(CDCTrack& track)
{
  ILayerType lastLayer = -1;
  Vector2D lastWirePosition;

  std::vector<std::vector<const CDCRecoHit3D*>> trackletList;
  trackletList.reserve(3);

  std::vector<const CDCRecoHit3D*>* currentTracklet = nullptr;

  for (const CDCRecoHit3D& recoHit : track) {
    if (currentTracklet == nullptr) {
      trackletList.emplace_back();
      currentTracklet = &(trackletList.back());
    }

    const ILayerType currentLayer = recoHit.getWire().getICLayer();
    const Vector2D& currentPosition = recoHit.getRecoPos2D();
    if (lastLayer != -1) {
      const ILayerType delta = currentLayer - lastLayer;
      const double distance = (currentPosition - lastWirePosition).norm();
      if (abs(delta) > 4 or distance > 50) {
        trackletList.emplace_back();
        currentTracklet = &(trackletList.back());
      }
    }

    lastWirePosition = currentPosition;
    lastLayer = currentLayer;

    currentTracklet->push_back(&recoHit);
  }

  if (trackletList.size() > 1) {
    for (const std::vector<const CDCRecoHit3D*>& tracklet : trackletList) {
      if (tracklet.size() < 5) {
        for (const CDCRecoHit3D* recoHit : tracklet) {
          recoHit->getWireHit().getAutomatonCell().setBackgroundFlag();
        }
      }
    }
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
  double ArcLength2DOfInnermostHit = track.front().getArcLength2D();
  track.sortByArcLength2D();

  for (const CDCRecoHit3D& recoHit : track) {
    if (recoHit.getArcLength2D() - ArcLength2DOfInnermostHit < 0) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
    }
  }
}

void removeArcLength2DHoles(CDCTrack& track)
{
  double lastArcLength2D = std::nan("");

  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : track) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      continue;
    }

    const double currentArcLength2D = recoHit.getArcLength2D();
    if (not std::isnan(lastArcLength2D)) {
      const double delta = currentArcLength2D - lastArcLength2D;
      if (delta > 100) {
        removeAfterThis = true;
        recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      }
    }

    lastArcLength2D = currentArcLength2D;
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



  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  const CDCWireLayer& innerMostLayer = topology.getWireLayers().front();
  const double innerCylindricalR = innerMostLayer.getInnerCylindricalR();
  const CDCWireLayer& outerMostLayer = topology.getWireLayers().back();
  const double outerCylindricalR = outerMostLayer.getOuterCylindricalR();

  const Vector2D support = trajectory2D.getSupport();
  const GeneralizedCircle globalCircle = trajectory2D.getGlobalCircle();
  // Trick: If the hit is too near to the inner circle, we scale it away a bit
  const Vector2D& innerExit = globalCircle.sameCylindricalRForwardOf(support, innerCylindricalR);
  if (innerExit.hasNAN()) {
    return;
  }
  const Vector2D& nextInnerExit = globalCircle.sameCylindricalRForwardOf(innerExit, innerCylindricalR);

  if (nextInnerExit.hasNAN()) {
    return;
  }

  B2INFO("Here")

  const double ArcLength2DOfInnerExit = trajectory2D.calcArcLength2D(innerExit);
  const double ArcLength2DOfNextInnerExit = trajectory2D.calcArcLength2D(nextInnerExit);

  // Count the number of clusters from the beginning to the first inner exit
  unsigned int numberOfHitsBeforeVXD = 0;
  unsigned int numberOfHitsAfterVXD = 0;
  for (const CDCRecoHit3D& recoHit : track) {
    const double currentArcLength2D = recoHit.getArcLength2D();
    if (currentArcLength2D < ArcLength2DOfInnerExit) {
      numberOfHitsBeforeVXD++;
    }

    else if (currentArcLength2D > ArcLength2DOfNextInnerExit) {
      numberOfHitsAfterVXD++;
    }
  }

  if (numberOfHitsBeforeVXD > 0) {
    for (const CDCRecoHit3D& recoHit : track) {
      const double currentArcLength2D = recoHit.getArcLength2D();
      if (currentArcLength2D < ArcLength2DOfInnerExit) {
        recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      }
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

  const double ArcLength2DOfExit = trajectory2D.calcArcLength2D(outerExit);
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : track) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setBackgroundFlag();
      continue;
    }

    const double currentArcLength2D = recoHit.getArcLength2D();
    if (currentArcLength2D > ArcLength2DOfExit) {
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
      } else if (correctorFunction == "ArcLength2D") {
        removeArcLength2DHoles(track);
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

    trackQualityTools.normalizeHitsAndResetTrajectory(track);
  }

  tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const CDCTrack & track) -> bool {
    return track.size() < 3;
  }), tracks.end());
}
