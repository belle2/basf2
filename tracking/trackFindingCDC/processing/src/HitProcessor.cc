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

#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/stable_partition.hpp>
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

void HitProcessor::appendUnusedHits(std::list<CDCTrack>& tracks,
                                    const std::vector<const CDCWireHit*>& axialWireHits)
{
  for (CDCTrack& track : tracks) {
    if (track.size() < 5) continue;

    // ESign trackCharge = TrackMergerNew::getChargeSign(track);
    CDCTrajectory2D trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

    for (const CDCWireHit* wireHit : axialWireHits) {
      if (wireHit->getAutomatonCell().hasTakenFlag() or wireHit->getAutomatonCell().hasMaskedFlag()) continue;

      ERightLeft rlInfo = trackTrajectory2D.isRightOrLeft(wireHit->getRefPos2D());
      CDCRLWireHit rlWireHit(wireHit, rlInfo, wireHit->getRefDriftLength());

      // if(fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
      // if(getArmSign(recoHit3D, track.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) continue;

      const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trackTrajectory2D);

      if (fabs(trackTrajectory2D.getDist2D(recoHit3D.getRecoPos2D())) < 0.1) {
        track.push_back(std::move(recoHit3D));
        recoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
      }
    }

  }
}

void HitProcessor::reassignHitsFromOtherTracks(std::list<CDCTrack>& trackList)
{
  std::vector<std::pair<CDCRecoHit3D, CDCTrack>> assignedHits;
  for (CDCTrack& cand : trackList) {
    for (CDCRecoHit3D& recoHit : cand) {
      recoHit.getWireHit().getAutomatonCell().setTakenFlag(true);
      recoHit.getWireHit().getAutomatonCell().setMaskedFlag(false);

      assignedHits.push_back(std::make_pair(recoHit, cand));
    }
  }

  B2DEBUG(100, "NCands = " << trackList.size());

  for (std::pair<CDCRecoHit3D, CDCTrack>& itemWithCand : assignedHits) {

    CDCRecoHit3D& item = itemWithCand.first;
    CDCTrack& cand = itemWithCand.second;


    CDCTrajectory2D trajectory = cand.getStartTrajectory3D().getTrajectory2D();

    HitProcessor::updateRecoHit3D(trajectory, item);
    double dist = fabs(trajectory.getDist2D(item.getRecoPos2D()));

    double bestHitDist = dist;
    CDCTrack* bestCandidate = nullptr;


    for (CDCTrack& candInner : trackList) {
      if (candInner == cand) continue;
      CDCTrajectory2D trajectoryInner = candInner.getStartTrajectory3D().getTrajectory2D();

      HitProcessor::updateRecoHit3D(trajectoryInner, item);
      double distTemp = fabs(trajectoryInner.getDist2D(item.getRecoPos2D()));

      if (distTemp < bestHitDist) {
        bestCandidate = &candInner;
        bestHitDist = distTemp;
      }
    }

    if (bestHitDist < dist) {
      const CDCRecoHit3D& recoHit3D =
        CDCRecoHit3D::reconstruct(item.getRLWireHit(),
                                  bestCandidate->getStartTrajectory3D().getTrajectory2D());

      bestCandidate->push_back(std::move(recoHit3D));
      item.getWireHit().getAutomatonCell().setMaskedFlag(true);
      deleteAllMarkedHits(cand);
      recoHit3D.getWireHit().getAutomatonCell().setMaskedFlag(false);
    }
  }
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
  int votePos = 0;
  int voteNeg = 0;

  Vector2D center = track.getStartTrajectory3D().getGlobalCenter();
  for (const CDCRecoHit3D& hit : track) {
    ESign curve_sign = getArmSign(hit, center);

    if (curve_sign == ESign::c_Plus) {
      ++votePos;
    } else if (curve_sign == ESign::c_Minus) {
      ++voteNeg;
    } else {
      B2ERROR("Unexpected value from getArmSign");
      return false;
    }
  }

  if (std::abs(votePos - voteNeg) < (votePos + voteNeg) and std::fabs(center.cylindricalR()) > 60.) {
    return true;
  }
  return false;
}

void HitProcessor::deleteAllMarkedHits(CDCTrack& track)
{
  auto hasMaskedFlag = [](const CDCRecoHit3D & hit) { return hit.getWireHit().getAutomatonCell().hasMaskedFlag(); };
  boost::remove_erase_if(track, hasMaskedFlag);
}

void HitProcessor::deleteAllMarkedHits(std::vector<const CDCWireHit*>& wireHits)
{
  auto hasMaskedFlag = [](const CDCWireHit * hit) { return hit->getAutomatonCell().hasMaskedFlag(); };
  boost::remove_erase_if(wireHits, hasMaskedFlag);
}

ESign HitProcessor::getMajorArmSign(const CDCTrack& track, const Vector2D& center)
{
  int votePos = 0;
  int voteNeg = 0;

  if (std::isnan(center.x())) {
    B2WARNING("Trajectory is not setted or wrong!");
    return track.getStartTrajectory3D().getChargeSign();
  }

  for (const CDCRecoHit3D& hit : track) {
    ESign curve_sign = getArmSign(hit, center);

    if (curve_sign == ESign::c_Plus) {
      ++votePos;
    } else if (curve_sign == ESign::c_Minus) {
      ++voteNeg;
    } else {
      B2ERROR("Strange behaviour of getArmSign");
    }
  }

  if (votePos > voteNeg) {
    return ESign::c_Plus;
  } else {
    return ESign::c_Minus;
  }
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
      // This would be correct but worsens efficiency...
      // recoHit3D->getWireHit().getAutomatonCell().setTakenFlag(false);
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

void HitProcessor::maskHitsWithPoorQuality(CDCTrack& track)
{
  double apogeeArcLenght = fabs(track.getStartTrajectory3D().getGlobalCircle().perimeter()) / 4.;

  std::vector<double> startingArmSLayers;
  std::vector<double> endingArmSLayers;

  for (int ii = 0; ii <= 8; ii++) {
    startingArmSLayers.push_back(0);
    endingArmSLayers.push_back(0);
  }

  // Count the number of hits in the outgoing and ingoing arm per superlayer.
  for (const CDCRecoHit3D& hit : track) {
    if ((hit.getArcLength2D() <= apogeeArcLenght) && (hit.getArcLength2D() > 0)) {
      startingArmSLayers[hit->getISuperLayer()]++;
    } else {
      endingArmSLayers[hit->getISuperLayer()]++;
    }
  }

  std::vector<int> emptyStartingSLayers;
  std::vector<int> emptyEndingSLayers;

  if (hasHoles(startingArmSLayers, endingArmSLayers, emptyStartingSLayers, emptyEndingSLayers)) {
    sort(emptyStartingSLayers.begin(), emptyStartingSLayers.end());

    if (emptyStartingSLayers.size() > 0) {
      const int breakSLayer = emptyStartingSLayers.front();
      for (CDCRecoHit3D& hit : track) {
        if (hit.getArcLength2D() >= apogeeArcLenght || hit.getArcLength2D() < 0) {
          hit.getWireHit().getAutomatonCell().setMaskedFlag();
        }
        if (hit.getISuperLayer() >= breakSLayer) {
          hit.getWireHit().getAutomatonCell().setMaskedFlag();
        }
      }
    }
    // We do not use emptyEndingSLayers here, as it would leave to a severy efficiency drop.
  }
  deleteAllMarkedHits(track);
}

int HitProcessor::startingSLayer(const std::vector<double>& startingArmSLayers)
{
  std::vector<double>::const_iterator startSlIt = std::find_if(startingArmSLayers.begin(),
                                                  startingArmSLayers.end(),
  [](double val) { return val > 0; });

  if (startSlIt != startingArmSLayers.end()) {
    return startSlIt - startingArmSLayers.begin();
  } else {
    return 8;
  }
}


int HitProcessor::endingSLayer(const std::vector<double>& startingArmSLayers)
{
  std::vector<double>::const_reverse_iterator endSlIt;
  endSlIt = std::find_if(startingArmSLayers.rbegin(), startingArmSLayers.rend(), [](double val) {
    return val > 0;
  });

  if (endSlIt != startingArmSLayers.rend()) {
    return 8 - (endSlIt - startingArmSLayers.rbegin());
  } else {
    return 0;
  }
}

bool HitProcessor::isTwoSided(const std::vector<double>& startingArmSLayers,
                              const std::vector<double>& endingArmSLayers)
{
  if ((std::accumulate(startingArmSLayers.begin(), startingArmSLayers.end(), 0) > 0) &&
      (std::accumulate(endingArmSLayers.begin(), endingArmSLayers.end(), 0) > 0)) {
    return true;
  } else {
    return false;
  }
}

bool HitProcessor::hasHoles(const std::vector<double>& startingArmSLayers,
                            const std::vector<double>& endingArmSLayers,
                            std::vector<int>& emptyStartingSLayers,
                            std::vector<int>& emptyEndingSLayers)
{
  // Find the start end end point.
  int startingSlayer = startingSLayer(startingArmSLayers);
  int endingSlayer = endingSLayer(startingArmSLayers);

  std::vector<double>::const_iterator first = startingArmSLayers.begin() + startingSlayer;
  std::vector<double>::const_iterator last = startingArmSLayers.begin() + endingSlayer;

  for (; first <= last; first += 2) {
    if (*first == 0) {
      emptyStartingSLayers.push_back(first - startingArmSLayers.begin());
    }
  }

  if (isTwoSided(startingArmSLayers, endingArmSLayers)) {

    // Find the start end end point.
    startingSlayer = startingSLayer(endingArmSLayers);
    endingSlayer = endingSLayer(endingArmSLayers);

    std::vector<double>::const_iterator rfirst = endingArmSLayers.begin() + startingSlayer;
    std::vector<double>::const_iterator rlast = endingArmSLayers.begin() + endingSlayer;

    for (; rfirst <= rlast; rfirst += 2) {
      if (*rfirst == 0) {
        emptyEndingSLayers.push_back(rfirst - endingArmSLayers.begin());
      }
    }
  }

  return emptyStartingSLayers.size() > 0 or emptyEndingSLayers.size() > 0;
}
