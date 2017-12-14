#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/utilities/ReversedRange.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <framework/dataobjects/Helix.h>

using namespace Belle2;
using namespace TrackFindingCDC;


void TrackQualityTools::moveToNextAxialLayer(CDCTrack& track)
{
  for (const CDCRecoHit3D& recoHit : track) {
    if (recoHit.getStereoKind() == EStereoKind::c_Axial) {
      break;
    } else {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
    }
  }
}

void TrackQualityTools::splitSecondHalfOfTrack(CDCTrack& track, std::vector<CDCTrack>& tracks)
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const double radius = trajectory2D.getLocalCircle()->absRadius();
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

void TrackQualityTools::normalizeHitsAndResetTrajectory(CDCTrack& track)
{
  if (track.empty()) return;

  CDCTrajectory3D trajectory3D = track.getStartTrajectory3D();

  // We reset the trajectory here to start (later) at the newStartPosition of the first hit
  const Vector3D startPosition(0, 0, 0);
  trajectory3D.setLocalOrigin(startPosition);
  trajectory3D.setFlightTime(0);

  CDCTrajectory2D trajectory2D = trajectory3D.getTrajectory2D();

  // Check if we have to reverse the trajectory. This is done by counting the number of hits
  // with positive and with negative arc length
  unsigned int numberOfPositiveHits = 0;
  for (const CDCRecoHit3D& recoHit : track) {
    const double currentArcLength = trajectory2D.calcArcLength2D(recoHit.getRecoPos2D());
    if (currentArcLength > 0) {
      numberOfPositiveHits++;
    }
  }
  const bool reverseTrajectory = 2 * numberOfPositiveHits < track.size();

  if (reverseTrajectory) {
    trajectory3D.reverse();
    trajectory2D.reverse();
  }

  double arcLength2DPeriod = trajectory2D.getArcLength2DPeriod();
  for (CDCRecoHit3D& recoHit : track) {
    Vector2D recoPos2D = recoHit.getRLWireHit().reconstruct3D(trajectory2D).xy();
    double arcLength2D = trajectory2D.calcArcLength2D(recoPos2D);

    if (arcLength2D < 0) {
      arcLength2D += arcLength2DPeriod;
    }
    recoHit.setArcLength2D(arcLength2D);
    recoHit.getWireHit().getAutomatonCell().unsetAssignedFlag();
    recoHit.getWireHit().getAutomatonCell().setTakenFlag();
  }

  // We can now sort by arc length
  track.sortByArcLength2D();

  // Set the position to the first hit and let the hits start at arc length of 0
  Vector3D frontPosition = track.front().getRLWireHit().reconstruct3D(trajectory2D);
  double arcLengthOffset = trajectory3D.setLocalOrigin(frontPosition);
  track.setStartTrajectory3D(trajectory3D);
  for (CDCRecoHit3D& recoHit : track) {
    recoHit.shiftArcLength2D(-arcLengthOffset);
  }

  // Set the back trajectory to start at the last hit position (and shift if necessary)
  Vector3D backPosition = track.back().getRLWireHit().reconstruct3D(trajectory2D);
  double backArcLength2D = trajectory3D.setLocalOrigin(backPosition);
  if (backArcLength2D < 0) {
    trajectory3D.shiftPeriod(1);
  }
  track.setEndTrajectory3D(trajectory3D);

}

void TrackQualityTools::removeHitsAfterCDCWall(CDCTrack& track, double outerCylindricalRFactor)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getLocalCircle()->absRadius();

  // Curler are allowed to have hits on both arms
  if (trajectory2D.isCurler(outerCylindricalRFactor)) {
    return;
  }

  const Vector2D& outerExitWithFactor = trajectory2D.getOuterExit(outerCylindricalRFactor);

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

void TrackQualityTools::removeHitsAfterLayerBreak2(CDCTrack& track)
{
  ILayer lastLayer = -1;
  Vector2D lastWirePosition;

  std::vector<std::vector<const CDCRecoHit3D*>> trackletList;
  trackletList.reserve(3);

  std::vector<const CDCRecoHit3D*>* currentTracklet = nullptr;

  for (const CDCRecoHit3D& recoHit : track) {
    if (currentTracklet == nullptr) {
      trackletList.emplace_back();
      currentTracklet = &(trackletList.back());
    }

    const ILayer currentLayer = recoHit.getWire().getICLayer();
    const Vector2D& currentPosition = recoHit.getRecoPos2D();
    if (lastLayer != -1) {
      const ILayer delta = currentLayer - lastLayer;
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

void TrackQualityTools::removeHitsAfterLayerBreak(CDCTrack& track, double m_maximumArcLength2DDistance)
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const double radius = trajectory2D.getLocalCircle()->absRadius();

  if (std::isnan(radius)) {
    return;
  }

  std::vector<std::vector<const CDCRecoHit3D*>> trackletList;

  double lastArcLength2D = NAN;

  std::vector<const CDCRecoHit3D*>* currentTracklet = nullptr;

  for (const CDCRecoHit3D& recoHit : track) {
    if (currentTracklet == nullptr) {
      trackletList.emplace_back();
      currentTracklet = &(trackletList.back());
    }

    const double currentArcLength2D = recoHit.getArcLength2D();
    if (not std::isnan(lastArcLength2D)) {
      const double delta = (currentArcLength2D - lastArcLength2D);
      if (std::fabs(delta) > m_maximumArcLength2DDistance) {
        trackletList.emplace_back();
        currentTracklet = &(trackletList.back());
      }
    }

    lastArcLength2D = currentArcLength2D;

    currentTracklet->push_back(&recoHit);
  }

  if (trackletList.size() > 1) {
    // Throw away the ends if they are too small
    while (trackletList.size() > 0) {
      if (trackletList.back().size() > 4) {
        break;
      }

      for (const CDCRecoHit3D* recoHit : trackletList.back()) {
        recoHit->getWireHit().getAutomatonCell().setAssignedFlag();
      }

      trackletList.pop_back();
    }

    std::reverse(trackletList.begin(), trackletList.end());

    while (trackletList.size() > 0) {
      if (trackletList.back().size() > 4) {
        break;
      }

      for (const CDCRecoHit3D* recoHit : trackletList.back()) {
        recoHit->getWireHit().getAutomatonCell().setAssignedFlag();
      }

      trackletList.pop_back();
    }
  }
}

void TrackQualityTools::removeHitsIfSmall(CDCTrack& track, unsigned int minimalHits)
{
  const bool deleteTrack = track.size() < minimalHits;

  if (deleteTrack) {
    for (const CDCRecoHit3D& recoHit : track) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
    }
  }
}

void TrackQualityTools::removeHitsInTheBeginningIfAngleLarge(CDCTrack& track, double maximalAngle)
{
  double lastAngle = NAN;
  bool removeAfterThis = false;

  for (const CDCRecoHit3D& recoHit : reversedRange(track)) {
    if (removeAfterThis) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      continue;
    }

    const double currentAngle = recoHit.getRecoPos2D().phi();
    if (not std::isnan(lastAngle)) {
      const double delta = currentAngle - lastAngle;
      const double normalizedDelta = std::min(TVector2::Phi_0_2pi(delta), TVector2::Phi_0_2pi(-delta));
      if (fabs(normalizedDelta) > maximalAngle) {
        removeAfterThis = true;
        recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      }
    }

    lastAngle = currentAngle;
  }
}


void TrackQualityTools::removeHitsIfOnlyOneSuperLayer(CDCTrack& track)
{
  ISuperLayer lastLayer = -1;
  bool deleteTrack = true;

  for (const CDCRecoHit3D& recoHit : track) {
    const ISuperLayer currentLayer = recoHit.getISuperLayer();
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

void TrackQualityTools::removeHitsOnTheWrongSide(CDCTrack& track)
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

void TrackQualityTools::removeArcLength2DHoles(CDCTrack& track, double m_maximumArcLength2DDistance)
{
  const CDCTrajectory3D& trajectory3D = track.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const double radius = trajectory2D.getLocalCircle()->absRadius();

  if (std::isnan(radius)) {
    return;
  }

  double lastArcLength2D = NAN;
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
