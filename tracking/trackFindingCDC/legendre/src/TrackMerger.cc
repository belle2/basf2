/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/TrackMerger.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/legendre/LegendreHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>

#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void TrackMergerNew::resetHits(CDCTrack& otherTrackCandidate)
{
  for (const CDCRecoHit3D& hit : otherTrackCandidate.items()) {
    hit->getWireHit().getAutomatonCell().setTakenFlag(false);
  }
}

CDCTrack& TrackMergerNew::splitBack2BackTrack(CDCTrack& trackCandidate)
{
  assert(trackCandidate);

  // If the trackCandidate goes more or less through the IP, we have a problem with back-to-back tracks. These can be assigned to only on track.
  // If this is the case, we delete the smaller fraction here and let the track-finder find the remaining track again
//    std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();


  if (checkBack2BackTrack(trackCandidate)) {

    SignType trackCharge = getChargeSign(trackCandidate);

    for (const CDCRecoHit3D& hit : trackCandidate.items()) {

      if (getCurvatureSignWrt(hit, trackCandidate.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) {
        hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
        hit.getWireHit().getAutomatonCell().setTakenFlag(false);
      }

    }

    deleteAllMarkedHits(trackCandidate);

    /*
    // TODO
    std::sort(trackHits.begin(), trackHits.end(), [](TrackHit * hit1, TrackHit * hit2) {
      return hit1->getWirePosition().Mag2() < hit2->getWirePosition().Mag2();
    });

    unsigned int number_of_hits_in_one_half = 0;
    unsigned int number_of_hits_in_other_half = 0;

    Vector2D momentum = trackCandidate->getMomentumEstimation();
    double phiOfTrack = momentum.phi();

    for (TrackHit* hit : trackHits) {
      double phiOfHit = hit->getWirePosition().Phi();
      if (std::abs(TVector2::Phi_mpi_pi(phiOfTrack - phiOfHit)) < TMath::PiOver2()) {
        number_of_hits_in_one_half++;
      } else {
        number_of_hits_in_other_half++;
      }
    }

    if (number_of_hits_in_one_half > 0 and number_of_hits_in_other_half > 0) {

      TrackCandidate* secondTrackCandidate = new TrackCandidate(*trackCandidate);
      std::vector<TrackHit*>& secondTrackHits = secondTrackCandidate->getTrackHits();
      secondTrackHits.clear();

      // Small trick: mark the hits which should belong to the other track candidate as bad,
      // add them to the other track candidate and delete all marked hits from the first. Then unmark the hits again.

      for (TrackHit* hit : trackHits) {
        double phiOfHit = hit->getWirePosition().Phi();
        if (std::abs(TVector2::Phi_mpi_pi(phiOfTrack - phiOfHit)) < TMath::PiOver2()) {
          secondTrackHits.push_back(hit);
          hit->setHitUsage(TrackHit::c_bad);
        }
      }

      SimpleFilter::deleteAllMarkedHits(trackCandidate);

      for (TrackHit* hit : secondTrackHits) {
        hit->setHitUsage(TrackHit::c_usedInTrack);
      }

      return secondTrackCandidate;
    }
    */
  }

  return trackCandidate;
}


bool TrackMergerNew::checkBack2BackTrack(CDCTrack& track)
{
  int vote_pos = 0;
  int vote_neg = 0;

  for (const CDCRecoHit3D& hit : track.items()) {
    int curve_sign = getCurvatureSignWrt(hit, track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().center());

    if (curve_sign == PLUS)
      ++vote_pos;
    else if (curve_sign == MINUS)
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

void TrackMergerNew::deleteAllMarkedHits(CDCTrack& track)
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


SignType TrackMergerNew::getChargeSign(CDCTrack& track)
{
  SignType trackCharge;
  int vote_pos(0), vote_neg(0);

  for (const CDCRecoHit3D& hit : track.items()) {
    int curve_sign = getCurvatureSignWrt(hit, track.getStartTrajectory3D().getGlobalCircle().center());

    if (curve_sign == PLUS)
      ++vote_pos;
    else if (curve_sign == MINUS)
      ++vote_neg;
    else {
      B2ERROR(
        "Strange behaviour of TrackHit::getCurvatureSignWrt");
      exit(EXIT_FAILURE);
    }
  }

  if (vote_pos > vote_neg)
    trackCharge = PLUS;
  else
    trackCharge = MINUS;


  return trackCharge;
}

int TrackMergerNew::getCurvatureSignWrt(const CDCRecoHit3D& hit, Vector2D xy)
{
  double phi_diff = atan2(xy.y(), xy.x()) - getPhi(hit);

  while (phi_diff > 2 * TMath::Pi())
    phi_diff -= 2 * TMath::Pi();
  while (phi_diff < 0)
    phi_diff += 2 * TMath::Pi();

  if (phi_diff > TMath::Pi())
    return PLUS;
  else
    return MINUS;
}

double TrackMergerNew::getPhi(const CDCRecoHit3D& hit)
{


  double phi = atan2(hit.getRecoPos2D().y() , hit.getRecoPos2D().x());

  while (phi > 2 * TMath::Pi())
    phi -= 2 * TMath::Pi();
  while (phi < 0)
    phi += 2 * TMath::Pi();

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
