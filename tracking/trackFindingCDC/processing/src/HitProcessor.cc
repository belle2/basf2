/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/processing/HitProcessor.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <boost/range/algorithm/stable_partition.hpp>
#include <boost/range/algorithm_ext/is_sorted.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

#include <numeric>

using namespace Belle2;
using namespace TrackFindingCDC;

void HitProcessor::updateRecoHit3D(CDCTrajectory2D& trajectory2D, CDCRecoHit3D& hit)
{
  hit.setRecoPos3D(hit.getRecoHit2D().getRLWireHit().reconstruct3D(trajectory2D));

  double perpS = trajectory2D.calcArcLength2D(hit.getRecoPos2D());
  if (perpS < 0.) {
    double perimeter = fabs(trajectory2D.getGlobalCircle().perimeter()) / 2.;
    perpS += perimeter;
  }
  // Recalculate the perpS of the hits
  hit.setArcLength2D(perpS);
}

std::vector<const CDCWireHit*> HitProcessor::splitBack2BackTrack(CDCTrack& track)
{
  std::vector<const CDCWireHit*> removedHits;

  if (track.size() < 5) return removedHits;
  if (not isBack2BackTrack(track)) return removedHits;

  Vector2D center = track.getStartTrajectory3D().getGlobalCenter();
  ESign majorArmSign = getMajorArmSign(track, center);

  auto isOnMajorArm = [&center, &majorArmSign](const CDCRecoHit3D & hit) {
    return getArmSign(hit, center) == majorArmSign;
  };

  auto minorArmHits = boost::stable_partition<boost::return_found_end>(track, isOnMajorArm);

  for (const CDCRecoHit3D& recoHit3D : minorArmHits) {
    const CDCWireHit* wireHit = &recoHit3D.getWireHit();
    wireHit->getAutomatonCell().setTakenFlag(false);
    removedHits.push_back(wireHit);
  }

  boost::erase(track, minorArmHits);

  return removedHits;
}

bool HitProcessor::isBack2BackTrack(CDCTrack& track)
{
  Vector2D center = track.getStartTrajectory3D().getGlobalCenter();
  int armSignVote = getArmSignVote(track, center);
  if (std::abs(armSignVote) < track.size() and std::fabs(center.cylindricalR()) > 60.) {
    return true;
  }
  return false;
}

ESign HitProcessor::getMajorArmSign(const CDCTrack& track, const Vector2D& center)
{
  int armSignVote = getArmSignVote(track, center);
  if (armSignVote > 0) {
    return ESign::c_Plus;
  } else {
    return ESign::c_Minus;
  }
}

int HitProcessor::getArmSignVote(const CDCTrack& track, const Vector2D& center)
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

ESign HitProcessor::getArmSign(const CDCRecoHit3D& hit, const Vector2D& center)
{
  return sign(center.isRightOrLeftOf(hit.getRecoPos2D()));
}

void HitProcessor::resetMaskedHits(std::list<CDCTrack>& trackList,
                                   std::vector<const CDCWireHit*>& allAxialWireHits)
{
  for (const CDCWireHit* wireHit : allAxialWireHits) {
    wireHit->getAutomatonCell().setMaskedFlag(false);
    wireHit->getAutomatonCell().setTakenFlag(false);
  }

  for (const CDCTrack& track : trackList) {
    track.forwardTakenFlag();
  }
}

void HitProcessor::unmaskHitsInTrack(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
  }
}

void HitProcessor::deleteHitsFarAwayFromTrajectory(CDCTrack& track, double maximumDistance)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  auto farFromTrajectory = [&trajectory2D, &maximumDistance](CDCRecoHit3D & recoHit3D) {
    Vector2D recoPos2D = recoHit3D.getRecoPos2D();
    if (fabs(trajectory2D.getDist2D(recoPos2D)) > maximumDistance) {
      recoHit3D.getWireHit().getAutomatonCell().setTakenFlag(false);
      // This must be here as the deleted hits must not participate in the hough search again.
      recoHit3D.getWireHit().getAutomatonCell().setMaskedFlag(true);
      return true;
    }
    return false;
  };
  boost::remove_erase_if(track, farFromTrajectory);
}

void HitProcessor::assignNewHitsToTrack(CDCTrack& track,
                                        const std::vector<const CDCWireHit*>& allAxialWireHits,
                                        double minimalDistance)
{
  if (track.size() < 10) return;

  const CDCTrajectory2D& trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  for (const CDCWireHit* wireHit : allAxialWireHits) {
    if (wireHit->getAutomatonCell().hasTakenFlag() or wireHit->getAutomatonCell().hasMaskedFlag()) continue;

    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstructNearest(wireHit, trackTrajectory2D);
    const Vector2D& recoPos2D = recoHit3D.getRecoPos2D();

    if (fabs(trackTrajectory2D.getDist2D(recoPos2D)) < minimalDistance) {
      track.push_back(std::move(recoHit3D));
      recoHit3D.getWireHit().getAutomatonCell().setTakenFlag();
    }
  }
}

void HitProcessor::removeHitsAfterSuperLayerBreak(CDCTrack& track)
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
  assert(boost::is_sorted(forwardSLayerHoles));
  if (forwardSLayerHoles.empty()) return;

  const ISuperLayer breakSLayer = forwardSLayerHoles.front();

  auto isInBackwardArm = [apogeeArcLength](const CDCRecoHit3D & recoHit3D) {
    recoHit3D.getWireHit().getAutomatonCell().unsetTakenFlag();
    return (recoHit3D.getArcLength2D() >= apogeeArcLength) or (recoHit3D.getArcLength2D() < 0);
  };
  boost::remove_erase_if(track, isInBackwardArm);

  auto isAfterSLayerBreak = [breakSLayer](const CDCRecoHit3D & recoHit3D) {
    recoHit3D.getWireHit().getAutomatonCell().unsetTakenFlag();
    return recoHit3D.getISuperLayer() >= breakSLayer;
  };
  boost::remove_erase_if(track, isAfterSLayerBreak);
}

std::vector<ISuperLayer> HitProcessor::getSLayerHoles(const std::array<int, ISuperLayerUtil::c_N>& nHitsBySLayer)
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

ISuperLayer HitProcessor::getFirstOccupiedISuperLayer(const std::array<int, ISuperLayerUtil::c_N>& nHitsBySLayer)
{
  for (ISuperLayer iSLayer = 0; iSLayer < ISuperLayerUtil::c_N; ++iSLayer) {
    if (nHitsBySLayer[iSLayer] > 0) return iSLayer;
  }
  return ISuperLayerUtil::c_Invalid;
}

ISuperLayer HitProcessor::getLastOccupiedISuperLayer(const std::array<int, ISuperLayerUtil::c_N>& nHitsBySLayer)
{
  for (ISuperLayer iSLayer = ISuperLayerUtil::c_N - 1; iSLayer >= 0; --iSLayer) {
    if (nHitsBySLayer[iSLayer] > 0) return iSLayer;
  }
  return ISuperLayerUtil::c_Invalid;
}
