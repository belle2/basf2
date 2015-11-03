#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <framework/dataobjects/Helix.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace std;


void TrackQualityTools::moveToNextAxialLayer(CDCTrack& track) const
{
  for (const CDCRecoHit3D& recoHit : track) {
    if (recoHit.getStereoType() == StereoType::c_Axial) {
      break;
    } else {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
    }
  }
}

void TrackQualityTools::splitSecondHalfOfTrack(CDCTrack& track, std::vector<CDCTrack>& tracks) const
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const double radius = trajectory2D.getLocalCircle().absRadius();
  const Vector2D& apogee = trajectory2D.getGlobalCircle().apogee();
  double arcLength2DOfApogee = trajectory2D.calcArcLength2D(apogee);
  if (arcLength2DOfApogee < 0) {
    arcLength2DOfApogee += 2 * TMath::Pi() * radius;
  }

  CDCTrack splittedCDCTrack;
  splittedCDCTrack.setStartTrajectory3D(trajectory3D);

  for (CDCRecoHit3D& recoHit : track) {
    double currentArcLength2D = recoHit.getArcLength2D();
    if (currentArcLength2D < 0) B2INFO("Below 0");
    if (currentArcLength2D > arcLength2DOfApogee) {
      splittedCDCTrack.push_back(recoHit);
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
    }
  }

  if (splittedCDCTrack.size() > 0) {
    tracks.push_back(splittedCDCTrack);
  }
}

void TrackQualityTools::unmaskHitsInTrack(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
  }
}

void TrackQualityTools::normalizeTrack(CDCTrack& track) const
{
  // Set the start point of the trajectory to the first hit
  if (track.size() < 5) return;

  unmaskHitsInTrack(track);

  bool m_usePosition = true;
  CDCObservations2D observations2D;
  observations2D.setUseRecoPos(m_usePosition);
  for (const CDCRecoHit3D& item : track.items()) {
    observations2D.append(item, m_usePosition);
  }

  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
  CDCTrajectory2D trackTrajectory2D = fitter.fit(observations2D);

  if (trackTrajectory2D.getChargeSign() != HitProcessor::getChargeSign(track)) trackTrajectory2D.reverse();

  trackTrajectory2D.setLocalOrigin(trackTrajectory2D.getGlobalPerigee());
  for (CDCRecoHit3D& recoHit : track) {
    HitProcessor::updateRecoHit3D(trackTrajectory2D, recoHit);
  }

  track.sortByArcLength2D();

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  track.setStartTrajectory3D(trajectory3D);

  unmaskHitsInTrack(track);
}

void TrackQualityTools::normalizeHitsAndResetTrajectory(CDCTrack& track) const
{
  if (track.size() <= 1) {
    return;
  }

  // We start with the most inner hit
  track.sortByRadius();

  // We then set the trajectory to start with this point
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const short charge = track.getStartChargeSign();

  if (abs(charge) != 1)
    return;

  Belle2::Helix helix(trajectory3D.getSupport(), trajectory3D.getMom3DAtSupport() , charge, 1.5);
  const Vector2D& startPosition = track.front().getRecoPos2D();
  const double perpSAtFront = helix.getArcLength2DAtXY(startPosition.x(), startPosition.y());

  if (std::isnan(perpSAtFront)) {
    B2WARNING("Strange trajectory. Can not do much about it.");
  } else {
    Vector3D newStartPosition(helix.getPositionAtArcLength2D(perpSAtFront));
    Vector3D newStartMomentum(helix.getMomentumAtArcLength2D(perpSAtFront, 1.5));
    if (newStartMomentum.hasNAN() or newStartPosition.hasNAN()) {
      B2WARNING("Strange trajectory. Can not do much about it.");
    } else {
      const CDCTrajectory2D currentTrajectory2D(newStartPosition.xy(), newStartMomentum.xy(), charge);

      // Check if we have to reverse the trajectory. This is done by counting the number of hits
      // with positive an with negative perpS
      unsigned int numberOfPositiveHits = 0;
      for (CDCRecoHit3D& recoHit : track) {
        const double currentPerpS = currentTrajectory2D.calcArcLength2D(recoHit.getRecoPos2D());
        if (currentPerpS > 0) {
          numberOfPositiveHits++;
        }
      }
      // ... or by looking at the arcLength of the origin
      const double arcLength2DOfOrigin = currentTrajectory2D.calcArcLength2D(Vector2D(0, 0));

      const bool reverseTrajectory = 2 * numberOfPositiveHits < track.size() or arcLength2DOfOrigin > 0;

      // We reset the trajectory here to start at the newStartPosition of the first hit
      if (reverseTrajectory)
        track.setStartTrajectory3D(CDCTrajectory3D(newStartPosition, -newStartMomentum, -charge));
      else
        track.setStartTrajectory3D(CDCTrajectory3D(newStartPosition, newStartMomentum, charge));
    }
  }

  track.setEndTrajectory3D(track.getStartTrajectory3D());
  const CDCTrajectory2D& startTrajectory = track.getStartTrajectory3D().getTrajectory2D();

  for (CDCRecoHit3D& recoHit : track) {
    // The 0.1 is to prevent hits "before" the first hit to be sorted at the end of the track.
    recoHit.setArcLength2D(startTrajectory.calcArcLength2D(recoHit.getRecoPos2D()) + 0.1);
    recoHit.getWireHit().getAutomatonCell().unsetAssignedFlag();
    recoHit.getWireHit().getAutomatonCell().setTakenFlag();
  }

  // The first hit has - per definition of the trajectory2D - a perpS of 0. We want every other hit to have a perpS greater than 0,
  // especially for curlers. For this, we go through all hits and look for negative perpS.
  // If we have found one, we shift it to positive values
  track.shiftToPositiveArcLengths2D(true);

  // We can now sort by perpS
  track.sortByArcLength2D();
}


// Remove all hits which can not belong to the track, as the particle can not exit and enter the CDC again.
// Works not very good
void TrackQualityTools::removeHitsAfterCDCWall(CDCTrack& track) const
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getLocalCircle().absRadius();

  // Curler are allowed to have hits on both arms
  if (trajectory2D.isCurler(m_outerCylindricalRFactor)) {
    return;
  }

  const Vector2D& outerExitWithFactor = trajectory2D.getOuterExit(m_outerCylindricalRFactor);

  double arcLength2DOfExitWithFactor = trajectory2D.calcArcLength2D(outerExitWithFactor);
  if (arcLength2DOfExitWithFactor < 0) {
    arcLength2DOfExitWithFactor += 2 * TMath::Pi() * radius;
  }
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : track) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      continue;
    }

    double currentArcLength2D = recoHit.getArcLength2D();
    if (currentArcLength2D < 0) {
      currentArcLength2D += 2 * TMath::Pi() * radius;
    }

    if (currentArcLength2D > arcLength2DOfExitWithFactor) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      removeAfterThis = true;
    }
  }
}

void TrackQualityTools::removeHitsAfterLayerBreak2(CDCTrack& track) const
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
          recoHit->getWireHit().getAutomatonCell().setAssignedFlag();
        }
      }
    }
  }
}


// Search for consecutive hits with 2D-distance below 50 and layer-delta below 4. If the number of consecutive hits is below 7, remove them.
// Works quite well for finding large "breaks" in the track.
void TrackQualityTools::removeHitsAfterLayerBreak(CDCTrack& track) const
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const double radius = trajectory2D.getLocalCircle().absRadius();

  if (std::isnan(radius)) {
    return;
  }

  std::vector<std::vector<const CDCRecoHit3D*>> trackletList;

  double lastArcLength2D = std::nan("");

  std::vector<const CDCRecoHit3D*>* currentTracklet = nullptr;

  for (const CDCRecoHit3D& recoHit : track) {
    if (currentTracklet == nullptr) {
      trackletList.emplace_back();
      currentTracklet = &(trackletList.back());
    }

    const double currentArcLength2D = recoHit.getArcLength2D();
    if (not std::isnan(lastArcLength2D)) {
      const double delta = (currentArcLength2D - lastArcLength2D);
      if (abs(delta) > m_maximumArcLength2DDistance) {
        trackletList.emplace_back();
        currentTracklet = &(trackletList.back());
      }
    }

    lastArcLength2D = currentArcLength2D;

    currentTracklet->push_back(&recoHit);
  }

  if (trackletList.size() > 1) {
    // Throw away the ends if they are too small
    while (trackletList.back().size() < 5 and trackletList.size() > 0) {
      for (const CDCRecoHit3D* recoHit : trackletList.back()) {
        recoHit->getWireHit().getAutomatonCell().setAssignedFlag();
      }

      trackletList.pop_back();
    }

    std::reverse(trackletList.begin(), trackletList.end());

    while (trackletList.back().size() < 5 and trackletList.size() > 0) {
      for (const CDCRecoHit3D* recoHit : trackletList.back()) {
        recoHit->getWireHit().getAutomatonCell().setAssignedFlag();
      }

      trackletList.pop_back();
    }
  }
}

void TrackQualityTools::removeHitsIfSmall(CDCTrack& track) const
{
  const bool deleteTrack = track.size() < m_minimalHits;

  if (deleteTrack) {
    for (const CDCRecoHit3D& recoHit : track) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
    }
  }
}

void TrackQualityTools::removeHitsInTheBeginningIfAngleLarge(CDCTrack& track) const
{
  double lastAngle = NAN;
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : boost::adaptors::reverse(track)) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      continue;
    }

    const double currentAngle = recoHit.getRecoPos2D().phi();
    if (not std::isnan(lastAngle)) {
      const double delta = currentAngle - lastAngle;
      const double normalizedDelta = std::min(TVector2::Phi_0_2pi(delta), TVector2::Phi_0_2pi(-delta));
      if (fabs(normalizedDelta) > m_maximalAngle) {
        removeAfterThis = true;
        recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      }
    }

    lastAngle = currentAngle;
  }
}


void TrackQualityTools::removeHitsIfOnlyOneSuperLayer(CDCTrack& track) const
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
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
    }
  }
}

void TrackQualityTools::removeHitsOnTheWrongSide(CDCTrack& track) const
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();

  // Curler are allowed to have negative arc lengths
  if (trajectory2D.isCurler()) {
    return;
  }
  for (const CDCRecoHit3D& recoHit : track) {
    if (trajectory2D.calcArcLength2D(recoHit.getRecoPos2D()) < 0) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
    }
  }
}

void TrackQualityTools::removeArcLength2DHoles(CDCTrack& track) const
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const double radius = trajectory2D.getLocalCircle().absRadius();

  if (std::isnan(radius)) {
    return;
  }

  double lastArcLength2D = std::nan("");
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : track) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      continue;
    }

    const double currentArcLength2D = recoHit.getArcLength2D();
    if (not std::isnan(lastArcLength2D)) {
      const double delta = (currentArcLength2D - lastArcLength2D) / radius;
      if (fabs(delta) > m_maximumArcLength2DDistance) {
        removeAfterThis = true;
        recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
        continue;
      }
    }

    lastArcLength2D = currentArcLength2D;
  }
}
