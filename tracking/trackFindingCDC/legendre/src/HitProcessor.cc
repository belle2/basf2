/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/trackFindingCDC/legendre/HitProcessor.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/legendre/TrackProcessor.h>

#include <TMath.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

void HitProcessor::updateRecoHit3D(CDCTrajectory2D& trackTrajectory2D, CDCRecoHit3D& hit)
{
  hit.setRecoPos3D(hit.getRecoHit2D().getRLWireHit().reconstruct3D(trackTrajectory2D));

  double perpS = trackTrajectory2D.calcArcLength2D(hit.getRecoPos2D());
  if (perpS < 0.) {
    double perimeter = fabs(trackTrajectory2D.getGlobalCircle().perimeter()) / 2.;
    perpS += perimeter;
  }
  // Recalculate the perpS of the hits
  hit.setArcLength2D(perpS);

}


void HitProcessor::appendUnusedHits(std::vector<CDCTrack>& trackCandidates, const std::vector<ConformalCDCWireHit*>& axialHitList)
{
  for (CDCTrack& trackCandidate : trackCandidates) {
    if (trackCandidate.size() < 5) continue;

    const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
//    ESign trackCharge = TrackMergerNew::getChargeSign(trackCandidate);
    CDCTrajectory2D trackTrajectory2D = trackCandidate.getStartTrajectory3D().getTrajectory2D();


    for (const ConformalCDCWireHit* hit : axialHitList) {
      if (hit->getUsedFlag() || hit->getMaskedFlag()) continue;

      ERightLeft rlInfo = ERightLeft::c_Right;
      if (trackTrajectory2D.getDist2D(hit->getCDCWireHit()->getRefPos2D()) < 0)
        rlInfo = ERightLeft::c_Left;
      const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(hit->getCDCWireHit()->getHit(), rlInfo);
      if (rlWireHit->getWireHit().getAutomatonCell().hasTakenFlag())
        continue;

      //        if(fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
      //          if(TrackMergerNew::getCurvatureSignWrt(cdcRecoHit3D, track.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) continue;

      const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trackTrajectory2D);

      if (fabs(trackTrajectory2D.getDist2D(cdcRecoHit3D.getRecoPos2D())) < 0.1) {
        trackCandidate.push_back(std::move(cdcRecoHit3D));
        cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
      }
    }

  }
  /*
  for (TrackHit* hit : axialHitList) {
    if (hit->getHitUsage() == TrackHit::c_usedInTrack or
        hit->getHitUsage() == TrackHit::c_bad) continue;

    // Search for best candidate to assign to
    double bestHitProb = 0;
    TrackCandidate* bestCandidate = nullptr;

    for (TrackCandidate* cand : trackList) {
      double probTemp = getAssigmentProbability(hit, cand);

      if (probTemp > bestHitProb) {
        bestCandidate = cand;
        bestHitProb = probTemp;
      }
    }

    if (bestCandidate != nullptr and bestHitProb > minimal_assignment_probability) {
      bestCandidate->addHit(hit);
      hit->setHitUsage(TrackHit::c_usedInTrack);
    }

  }
  */
}

void HitProcessor::deleteWrongHitsOfTrack(CDCTrack& trackCandidate)
{
  /*
  assert(trackCandidate);

  std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();

  if (trackHits.size() == 0) return;

  for (TrackHit* hit : trackHits) {
    assert(hit);
    hit->setHitUsage(TrackHit::c_usedInTrack);
  }

  int ndf = trackHits.size() - 4;

  if (ndf <= 0) return;

  for (TrackHit* trackHit : trackHits) {
    assert(trackHit);
    double assignment_probability = getAssigmentProbability(trackHit, trackCandidate);

    if (assignment_probability < minimal_assignment_probability) {
      trackHit->setHitUsage(TrackHit::c_bad);
    }
  }
  */
  deleteAllMarkedHits(trackCandidate);

}


/*
double SimpleFilter::getAssigmentProbability(const TrackHit* hit, const TrackCandidate* track)
{
  double x0_track = track->getXc();
  double y0_track = track->getYc();
  double R = track->getRadius();

  double x0_hit = hit->getWirePosition().X();
  double y0_hit = hit->getWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) *
                                   (y0_track - y0_hit))) - hit->getDriftLength());

  return 1.0 - exp(-1 / dist);
}
*/


void HitProcessor::reassignHitsFromOtherTracks(std::list<CDCTrack>& trackCandidates)
{

  return;
  std::vector<std::pair<CDCRecoHit3D, CDCTrack>> assignedHits;
  for (CDCTrack& cand : trackCandidates) {

    for (CDCRecoHit3D& recoHit : cand) {
      recoHit.getWireHit().getAutomatonCell().setTakenFlag(true);
      recoHit.getWireHit().getAutomatonCell().setMaskedFlag(false);

      assignedHits.push_back(std::make_pair(recoHit, cand));
    }
  }


  B2DEBUG(100, "NCands = " << trackCandidates.size());

  for (std::pair<CDCRecoHit3D, CDCTrack>& itemWithCand : assignedHits) {

    CDCRecoHit3D& item = itemWithCand.first;
    CDCTrack& cand = itemWithCand.second;


    CDCTrajectory2D trajectory = cand.getStartTrajectory3D().getTrajectory2D();

    HitProcessor::updateRecoHit3D(trajectory, item);
    double dist = fabs(trajectory.getDist2D(item.getRecoPos2D()));

    double bestHitDist = dist;
    CDCTrack* bestCandidate = NULL;

    for (CDCTrack& candInner : trackCandidates) {
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
      const CDCRecoHit3D& cdcRecoHit3D  =  CDCRecoHit3D::reconstruct(item.getRLWireHit(),
                                           bestCandidate->getStartTrajectory3D().getTrajectory2D());

      bestCandidate->push_back(std::move(cdcRecoHit3D));
      item.getWireHit().getAutomatonCell().setMaskedFlag(true);
      deleteAllMarkedHits(cand);
      cdcRecoHit3D.getWireHit().getAutomatonCell().setMaskedFlag(false);
    }

  }


}

void HitProcessor::unmaskHitsInTrack(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
  }
}

std::vector<const CDCWireHit*> HitProcessor::splitBack2BackTrack(CDCTrack& trackCandidate)
{
  assert(trackCandidate);

  std::vector<const CDCWireHit*> removedHits;

//  return removedHits;
  if (trackCandidate.size() < 5) return removedHits;

  // If the trackCandidate goes more or less through the IP, we have a problem with back-to-back tracks. These can be assigned to only on track.
  // If this is the case, we delete the smaller fraction here and let the track-finder find the remaining track again
//    std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();

  for (CDCRecoHit3D& hit : trackCandidate) {
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
  }


  if (checkBack2BackTrack(trackCandidate)) {

    ESign trackCharge = getChargeSign(trackCandidate);

    for (const CDCRecoHit3D& hit : trackCandidate.items()) {

      if (getCurvatureSignWrt(hit, trackCandidate.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) {
        hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
        hit.getWireHit().getAutomatonCell().setTakenFlag(false);
      }

    }

    for (CDCRecoHit3D& hit : trackCandidate) {
      if (hit.getWireHit().getAutomatonCell().hasMaskedFlag())
        removedHits.push_back(&(hit.getWireHit()));
    }

    deleteAllMarkedHits(trackCandidate);

    for (const CDCWireHit* hit : removedHits) {
      hit->getAutomatonCell().setMaskedFlag(false);
      hit->getAutomatonCell().setTakenFlag(false);
    }


  }

  return removedHits;
}

CDCTrajectory2D HitProcessor::fitWhithoutRecoPos(CDCTrack& track)
{
  std::vector<const CDCWireHit*> wireHits;
  for (const CDCRecoHit3D& hit : track) {
    wireHits.push_back(&(hit.getWireHit()));
  }

  return fitWhithoutRecoPos(wireHits);
}

CDCTrajectory2D HitProcessor::fitWhithoutRecoPos(std::vector<const CDCWireHit*>& wireHits)
{
  CDCKarimakiFitter trackFitter;
  CDCObservations2D observations;
  for (const CDCWireHit* wireHit : wireHits) {
    observations.append(wireHit->getRefPos2D().x(), wireHit->getRefPos2D().y(), 0, 1. / fabs(wireHit->getRefDriftLength()));
  }
  CDCTrajectory2D trackTrajectory2D ;
  trackFitter.update(trackTrajectory2D, observations);

  return trackTrajectory2D;
}


bool HitProcessor::checkBack2BackTrack(CDCTrack& track)
{
  int vote_pos = 0;
  int vote_neg = 0;

  for (const CDCRecoHit3D& hit : track.items()) {
    int curve_sign = getCurvatureSignWrt(hit, track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().center());

    if (curve_sign == ESign::c_Plus)
      ++vote_pos;
    else if (curve_sign == ESign::c_Minus)
      ++vote_neg;
    else {
      B2ERROR(
        "Strange behaviour of TrackHit::getCurvatureSignWrt");
      exit(EXIT_FAILURE);
    }
  }

  if ((fabs(vote_pos - vote_neg) / (double)(vote_pos + vote_neg) < 1.)
      && fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
    return true;

  return false;
}

void HitProcessor::deleteAllMarkedHits(CDCTrack& track)
{


  track.erase(
  std::remove_if(track.begin(), track.end(), [](const CDCRecoHit3D & hit) {
    if (hit.getWireHit().getAutomatonCell().hasMaskedFlag()) {
      return true;
    }
    return false;
//    return hit.getWireHit().getAutomatonCell().hasMaskedFlag();
  }),
  track.end());

}

void HitProcessor::resetHits(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track.items()) {
    hit->getWireHit().getAutomatonCell().setTakenFlag(false);
  }
}


ESign HitProcessor::getChargeSign(CDCTrack& track)
{
  ESign trackCharge;
  int vote_pos(0), vote_neg(0);

  Vector2D center(track.getStartTrajectory3D().getGlobalCircle().center());

  if (std::isnan(center.x())) {
    B2WARNING("Trajectory is not setted or wrong!");
    return track.getStartTrajectory3D().getChargeSign();
  }

  for (const CDCRecoHit3D& hit : track.items()) {
    ESign curve_sign = getCurvatureSignWrt(hit, center);

    if (curve_sign == ESign::c_Plus)
      ++vote_pos;
    else if (curve_sign == ESign::c_Minus)
      ++vote_neg;
    else {
      B2ERROR(
        "Strange behaviour of TrackHit::getCurvatureSignWrt");
      exit(EXIT_FAILURE);
    }
  }

  if (vote_pos > vote_neg)
    trackCharge = ESign::c_Plus;
  else
    trackCharge = ESign::c_Minus;


  return trackCharge;
}

ESign HitProcessor::getCurvatureSignWrt(const CDCRecoHit3D& hit, Vector2D xy)
{
  double phi_diff = atan2(xy.y(), xy.x()) - getPhi(hit);

  while (phi_diff > /*2 */ TMath::Pi())
    phi_diff -= 2 * TMath::Pi();
  while (phi_diff < -1. * TMath::Pi())
    phi_diff += 2 * TMath::Pi();

  if (phi_diff > 0 /*TMath::Pi()*/)
    return ESign::c_Minus;
  else
    return ESign::c_Plus;

}

double HitProcessor::getPhi(const CDCRecoHit3D& hit)
{


  double phi = atan2(hit.getRecoPos2D().y() , hit.getRecoPos2D().x());
  /*
    while (phi > 2 * TMath::Pi())
      phi -= 2 * TMath::Pi();
    while (phi < 0)
      phi += 2 * TMath::Pi();
  */
  return phi;
  /*
    //the phi angle of the hit depends on the definition, so I try to use the wireId instead
    //however maybe this function might also be still useful...
    double phi = atan(hit.getRecoPos2D().y() / hit.getRecoPos2D().x());

    if (hit.getRecoPos2D().x() < 0) {
      phi += TMath::Pi();
    }

    if (hit.getRecoPos2D().x() >= 0 && hit.getRecoPos2D().y() < 0) {
      phi += 2 * TMath::Pi();
    }

    return phi;
  */
}

