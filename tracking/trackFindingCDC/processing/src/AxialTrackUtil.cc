/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <numeric>

using namespace Belle2;
using namespace TrackFindingCDC;

void AxialTrackUtil::addCandidateFromHits(const std::vector<const CDCWireHit*>& foundAxialWireHits,
                                          const std::vector<const CDCWireHit*>& allAxialWireHits,
                                          std::vector<CDCTrack>& axialTracks,
                                          bool withPostprocessing)
{
  if (foundAxialWireHits.empty()) return;

  // New track
  CDCTrack track;

  // Fit trajectory
  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
  CDCTrajectory2D trajectory2D = fitter.fit(foundAxialWireHits);
  trajectory2D.setLocalOrigin(Vector2D(0, 0));
  track.setStartTrajectory3D(CDCTrajectory3D(trajectory2D, CDCTrajectorySZ::basicAssumption()));

  // Reconstruct and add hits
  for (const CDCWireHit* wireHit : foundAxialWireHits) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstructNearest(wireHit, trajectory2D);
    track.push_back(std::move(recoHit3D));
  }
  track.sortByArcLength2D();

  // Change everything again in the postprocessing, if desired
  bool success = withPostprocessing ? postprocessTrack(track, allAxialWireHits) : true;
  if (success) {
    /// Mark hits as taken and add the new track to the track list
    for (const CDCRecoHit3D& recoHit3D : track) {
      recoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
    }
    axialTracks.emplace_back(std::move(track));
  } else {
    /// Masked bad hits
    for (const CDCRecoHit3D& recoHit3D : track) {
      recoHit3D.getWireHit().getAutomatonCell().setMaskedFlag(true);
      recoHit3D.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
  }
}

bool AxialTrackUtil::postprocessTrack(CDCTrack& track, const std::vector<const CDCWireHit*>& allAxialWireHits)
{
  AxialTrackUtil::normalizeTrack(track);

  AxialTrackUtil::deleteHitsFarAwayFromTrajectory(track);
  AxialTrackUtil::normalizeTrack(track);
  if (not checkTrackQuality(track)) {
    return false;
  }

  AxialTrackUtil::assignNewHitsToTrack(track, allAxialWireHits);
  AxialTrackUtil::normalizeTrack(track);

  AxialTrackUtil::splitBack2BackTrack(track);
  AxialTrackUtil::normalizeTrack(track);
  if (not checkTrackQuality(track)) {
    return false;
  }
  return true;
}

bool AxialTrackUtil::checkTrackQuality(const CDCTrack& track)
{
  return not(track.size() < 5);
}

void AxialTrackUtil::normalizeTrack(CDCTrack& track)
{
  // Set the start point of the trajectory to the first hit
  if (track.size() < 5) return;

  CDCObservations2D observations2D(EFitPos::c_RecoPos);
  for (const CDCRecoHit3D& item : track) {
    observations2D.append(item);
  }

  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
  CDCTrajectory2D trackTrajectory2D = fitter.fit(observations2D);
  Vector2D center = trackTrajectory2D.getGlobalCenter();

  // Arm used as a proxy for the charge of the track
  // Correct if the track originates close to the origin
  ESign expectedCharge = AxialTrackUtil::getMajorArmSign(track, center);

  if (trackTrajectory2D.getChargeSign() != expectedCharge) trackTrajectory2D.reverse();

  trackTrajectory2D.setLocalOrigin(trackTrajectory2D.getGlobalPerigee());
  for (CDCRecoHit3D& recoHit : track) {
    AxialTrackUtil::updateRecoHit3D(trackTrajectory2D, recoHit);
  }

  track.sortByArcLength2D();

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  track.setStartTrajectory3D(trajectory3D);

  Vector3D backPosition = track.back().getRecoPos3D();
  trajectory3D.setLocalOrigin(backPosition);
  track.setEndTrajectory3D(trajectory3D);
}

void AxialTrackUtil::updateRecoHit3D(const CDCTrajectory2D& trajectory2D, CDCRecoHit3D& hit)
{
  hit = CDCRecoHit3D::reconstructNearest(&hit.getWireHit(), trajectory2D);

  // Recalculate the perpS of the hits
  double arcLength2D = hit.getArcLength2D();
  if (arcLength2D < -trajectory2D.getArcLength2DPeriod() / 4.0) {
    arcLength2D += trajectory2D.getArcLength2DPeriod();
  }
  hit.setArcLength2D(arcLength2D);
}

void AxialTrackUtil::deleteHitsFarAwayFromTrajectory(CDCTrack& track, double maximumDistance)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  auto farFromTrajectory = [&trajectory2D, &maximumDistance](CDCRecoHit3D & recoHit3D) {
    Vector2D refPos2D = recoHit3D.getRefPos2D();
    double distance = trajectory2D.getDist2D(refPos2D) - recoHit3D.getSignedRecoDriftLength();
    if (std::fabs(distance) > maximumDistance) {
      recoHit3D.getWireHit().getAutomatonCell().setTakenFlag(false);
      // This must be here as the deleted hits must not participate in the hough search again.
      recoHit3D.getWireHit().getAutomatonCell().setMaskedFlag(true);
      return true;
    }
    return false;
  };
  erase_remove_if(track, farFromTrajectory);
}

void AxialTrackUtil::deleteTracksWithLowFitProbability(std::vector<CDCTrack>& axialTracks,
                                                       double minimal_probability_for_good_fit)
{
  const CDCKarimakiFitter& trackFitter = CDCKarimakiFitter::getNoDriftVarianceFitter();
  const auto lowPValue = [&](const CDCTrack & track) {
    CDCTrajectory2D fittedTrajectory = trackFitter.fit(track);
    // Keep good fits - p-value is not really a probability,
    // but what can you do if the original author did not mind...
    if (not(fittedTrajectory.getPValue() >= minimal_probability_for_good_fit)) {
      // Release hits
      track.forwardTakenFlag(false);
      return true;
    }
    return false;
  };
  erase_remove_if(axialTracks, lowPValue);
}

void AxialTrackUtil::assignNewHitsToTrack(CDCTrack& track,
                                          const std::vector<const CDCWireHit*>& allAxialWireHits,
                                          double minimalDistance)
{
  if (track.size() < 10) return;

  const CDCTrajectory2D& trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  for (const CDCWireHit* wireHit : allAxialWireHits) {
    if (wireHit->getAutomatonCell().hasTakenFlag()) continue;

    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstructNearest(wireHit, trackTrajectory2D);
    const Vector2D& recoPos2D = recoHit3D.getRecoPos2D();

    if (fabs(trackTrajectory2D.getDist2D(recoPos2D)) < minimalDistance) {
      track.push_back(std::move(recoHit3D));
      recoHit3D.getWireHit().getAutomatonCell().setTakenFlag();
    }
  }
}

void AxialTrackUtil::deleteShortTracks(std::vector<CDCTrack>& axialTracks, double minimal_size)
{
  const auto isShort = [&](const CDCTrack & track) {
    if (track.size() < minimal_size) {
      // Release hits
      track.forwardTakenFlag(false);
      return true;
    }
    return false;
  };
  erase_remove_if(axialTracks, isShort);
}

std::vector<CDCRecoHit3D> AxialTrackUtil::splitBack2BackTrack(CDCTrack& track)
{
  std::vector<CDCRecoHit3D> removedHits;

  if (track.size() < 5) return removedHits;
  if (not isBack2BackTrack(track)) return removedHits;

  Vector2D center = track.getStartTrajectory3D().getGlobalCenter();
  ESign majorArmSign = getMajorArmSign(track, center);

  auto isOnMajorArm = [&center, &majorArmSign](const CDCRecoHit3D & hit) {
    return getArmSign(hit, center) == majorArmSign;
  };

  auto itFirstMinorArmHit = std::stable_partition(track.begin(),
                                                  track.end(),
                                                  isOnMajorArm);

  for (const CDCRecoHit3D& recoHit3D : asRange(itFirstMinorArmHit, track.end())) {
    recoHit3D.getWireHit().getAutomatonCell().setTakenFlag(false);
    removedHits.push_back(recoHit3D);
  }
  track.erase(itFirstMinorArmHit, track.end());

  return removedHits;
}

bool AxialTrackUtil::isBack2BackTrack(CDCTrack& track)
{
  Vector2D center = track.getStartTrajectory3D().getGlobalCenter();
  int armSignVote = getArmSignVote(track, center);
  if (std::abs(armSignVote) < int(track.size()) and std::fabs(center.cylindricalR()) > 60.) {
    return true;
  }
  return false;
}

ESign AxialTrackUtil::getMajorArmSign(const CDCTrack& track, const Vector2D& center)
{
  int armSignVote = getArmSignVote(track, center);
  if (armSignVote > 0) {
    return ESign::c_Plus;
  } else {
    return ESign::c_Minus;
  }
}

int AxialTrackUtil::getArmSignVote(const CDCTrack& track, const Vector2D& center)
{
  int votePos = 0;
  int voteNeg = 0;

  if (center.hasNAN()) {
    B2WARNING("Trajectory is not setted or wrong!");
    return 0;
  }

  for (const CDCRecoHit3D& hit : track) {
    ESign armSign = getArmSign(hit, center);

    if (armSign == ESign::c_Plus) {
      ++votePos;
    } else if (armSign == ESign::c_Minus) {
      ++voteNeg;
    } else {
      B2ERROR("Strange behaviour of getArmSignVote");
    }
  }
  int armSignVote = votePos - voteNeg;
  return armSignVote;
}

ESign AxialTrackUtil::getArmSign(const CDCRecoHit3D& hit, const Vector2D& center)
{
  return sign(center.isRightOrLeftOf(hit.getRecoPos2D()));
}

void AxialTrackUtil::removeHitsAfterSuperLayerBreak(CDCTrack& track)
{
  double apogeeArcLength = fabs(track.getStartTrajectory3D().getGlobalCircle().perimeter()) / 2.;

  std::array<int, ISuperLayerUtil::c_N> nForwardArmHitsBySLayer = {0};
  std::array<int, ISuperLayerUtil::c_N> nBackwardArmHitsBySLayer = {0};

  // Count the number of hits in the outgoing and ingoing arm per superlayer.
  for (const CDCRecoHit3D& hit : track) {
    if ((hit.getArcLength2D() <= apogeeArcLength) and (hit.getArcLength2D() > 0)) {
      nForwardArmHitsBySLayer[hit.getISuperLayer()]++;
    } else {
      nBackwardArmHitsBySLayer[hit.getISuperLayer()]++;
    }
  }

  std::vector<ISuperLayer> forwardSLayerHoles = getSLayerHoles(nForwardArmHitsBySLayer);
  std::vector<ISuperLayer> backwardSLayerHoles = getSLayerHoles(nBackwardArmHitsBySLayer);

  // No missing layers
  if (forwardSLayerHoles.empty() and backwardSLayerHoles.empty()) return;

  // We only check for holes in the forward arm for now
  // We do not use emptyEndingSLayers here, as it would leave to a severy efficiency drop.
  assert(std::is_sorted(forwardSLayerHoles.begin(), forwardSLayerHoles.end()));
  if (forwardSLayerHoles.empty()) return;

  const ISuperLayer breakSLayer = forwardSLayerHoles.front();

  auto isInBackwardArm = [apogeeArcLength](const CDCRecoHit3D & recoHit3D) {
    if ((recoHit3D.getArcLength2D() >= apogeeArcLength) or (recoHit3D.getArcLength2D() < 0)) {
      recoHit3D.getWireHit().getAutomatonCell().unsetTakenFlag();
      return true;
    }
    return false;
  };

  erase_remove_if(track, isInBackwardArm);

  auto isAfterSLayerBreak = [breakSLayer](const CDCRecoHit3D & recoHit3D) {
    recoHit3D.getWireHit().getAutomatonCell().unsetTakenFlag();
    if (recoHit3D.getISuperLayer() >= breakSLayer) {
      recoHit3D.getWireHit().getAutomatonCell().unsetTakenFlag();
      return true;
    }
    return false;
  };

  erase_remove_if(track, isAfterSLayerBreak);
}

std::vector<ISuperLayer> AxialTrackUtil::getSLayerHoles(const std::array<int, ISuperLayerUtil::c_N>& nHitsBySLayer)
{
  std::vector<ISuperLayer> sLayerHoles;

  // Find the start end end point.
  ISuperLayer firstSlayer = getFirstOccupiedISuperLayer(nHitsBySLayer);
  ISuperLayer lastSlayer = getLastOccupiedISuperLayer(nHitsBySLayer);

  if (ISuperLayerUtil::isInvalid(firstSlayer) or ISuperLayerUtil::isInvalid(lastSlayer)) {
    return sLayerHoles;
  }

  for (ISuperLayer iSLayer = firstSlayer; iSLayer <= lastSlayer; iSLayer += 2) {
    if (nHitsBySLayer[iSLayer] == 0) {
      sLayerHoles.push_back(iSLayer);
    }
  }
  return sLayerHoles;
}

ISuperLayer AxialTrackUtil::getFirstOccupiedISuperLayer(const std::array<int, ISuperLayerUtil::c_N>& nHitsBySLayer)
{
  for (ISuperLayer iSLayer = 0; iSLayer < ISuperLayerUtil::c_N; ++iSLayer) {
    if (nHitsBySLayer[iSLayer] > 0) return iSLayer;
  }
  return ISuperLayerUtil::c_Invalid;
}

ISuperLayer AxialTrackUtil::getLastOccupiedISuperLayer(const std::array<int, ISuperLayerUtil::c_N>& nHitsBySLayer)
{
  for (ISuperLayer iSLayer = ISuperLayerUtil::c_N - 1; iSLayer >= 0; --iSLayer) {
    if (nHitsBySLayer[iSLayer] > 0) return iSLayer;
  }
  return ISuperLayerUtil::c_Invalid;
}
