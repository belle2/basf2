#include <tracking/modules/trackFinderCDC/TrackQualityAsserterCDCModule.h>

#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <framework/dataobjects/Helix.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackQualityAsserterCDC);

/* Corrector functions */
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
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
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
        recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      }
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
        trackQualityTools.removeHitsAfterLayerBreak(track);
      } else if (correctorFunction == "LargeAngle") {
        trackQualityTools.removeHitsInTheBeginningIfAngleLarge(track);
      } else if (correctorFunction == "LargeBreak2") {
        trackQualityTools.removeHitsAfterLayerBreak2(track);
      } else if (correctorFunction == "OneSuperlayer") {
        trackQualityTools.removeHitsIfOnlyOneSuperLayer(track);
      } else if (correctorFunction == "Small") {
        trackQualityTools.removeHitsIfSmall(track);
      } else if (correctorFunction == "B2B") {
        trackQualityTools.removeHitsOnTheWrongSide(track);
      } else if (correctorFunction == "None") {
        ;
      }

      else if (correctorFunction == "ArcLength2D") {
        trackQualityTools.removeArcLength2DHoles(track);
      } else if (correctorFunction == "CDCWall") {
        B2FATAL("Do not use this function as it is not working probably.");
        trackQualityTools.removeHitsAfterCDCWall(track);
      } else if (correctorFunction == "CenterPointing") {
        B2FATAL("Do not use this function as it is not working probably.");
        revertTrajectoriesPointingToTheCenter(track);
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
