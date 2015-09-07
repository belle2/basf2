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
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <TMath.h>
#include "../include/QuadTreeHitWrapper.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool TrackMergerNew::mergeTwoTracks(CDCTrack& track1, CDCTrack& track2)
{
  CDCRiemannFitter trackFitter;

  if ((track1.size() < 5) || (track2.size() < 5)) return false;

  bool usePosition(false);


  CDCTrajectory2D result1;
  CDCObservations2D observations1;
  observations1.setUseRecoPos(usePosition);
  size_t nAppendedHits1 = 0;
  for (const CDCRecoHit3D& item : track1.items()) {
    nAppendedHits1 += observations1.append(item, usePosition);
  }
  trackFitter.update(result1, observations1);



  CDCTrajectory2D result2;
  CDCObservations2D observations2;
  observations2.setUseRecoPos(usePosition);
  size_t nAppendedHits2 = 0;
  for (const CDCRecoHit3D& item : track2.items()) {
    nAppendedHits2 += observations2.append(item, usePosition);
  }
  trackFitter.update(result2, observations2);



  CDCTrajectory2D resultMerge;
  CDCObservations2D observationsMerge;
  observationsMerge.setUseRecoPos(usePosition);
  size_t nAppendedHitsMerge = 0;

  for (const CDCRecoHit3D& item : track1.items()) {
    nAppendedHitsMerge += observationsMerge.append(item, usePosition);
  }

  for (const CDCRecoHit3D& item : track2.items()) {
    nAppendedHitsMerge += observationsMerge.append(item, usePosition);
  }

  trackFitter.update(resultMerge, observationsMerge);


//  B2INFO("chi2_1: " << result1.getChi2() << "; chi2_2: " << result2.getChi2() << "; chi2_merge: " << resultMerge.getChi2());

  double distCriteria1(0);
  for (CDCRecoHit3D& item : track1) {
    HitProcessor::updateRecoHit3D(result1, item);
    distCriteria1 += fabs(result1.getDist2D(item.getRecoPos2D()));
  }
  distCriteria1 = distCriteria1 / track1.size();

  double distCriteria2(0);
  for (CDCRecoHit3D& item : track2) {
    HitProcessor::updateRecoHit3D(result2, item);
    distCriteria2 += fabs(result2.getDist2D(item.getRecoPos2D()));
  }
  distCriteria2 = distCriteria2 / track2.size();


  double distCriteriaMerge(0);
  for (CDCRecoHit3D& item : track1) {
    HitProcessor::updateRecoHit3D(resultMerge, item);
    distCriteriaMerge += fabs(resultMerge.getDist2D(item.getRecoPos2D()));
  }
  for (CDCRecoHit3D& item : track2) {
    HitProcessor::updateRecoHit3D(resultMerge, item);
    distCriteriaMerge += fabs(resultMerge.getDist2D(item.getRecoPos2D()));
  }
  distCriteriaMerge = distCriteriaMerge / (track1.size() + track2.size());

//  B2INFO("criteria1: " << distCriteria1 << "; criteria2: " << distCriteria2 << "; criteriaMerge: " << distCriteriaMerge);

//  if(resultMerge.getChi2()*2./resultMerge.getNDF() <= (result1.getChi2()/result1.getNDF() + result2.getChi2()/result2.getNDF())) return true;

  if (distCriteriaMerge < 0.3) return true;

  return false;
}

CDCTrack& TrackMergerNew::splitBack2BackTrack(CDCTrack& trackCandidate)
{
  assert(trackCandidate);
  if (trackCandidate.size() < 5) return trackCandidate;

  // If the trackCandidate goes more or less through the IP, we have a problem with back-to-back tracks. These can be assigned to only on track.
  // If this is the case, we delete the smaller fraction here and let the track-finder find the remaining track again
//    std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();


  if (checkBack2BackTrack(trackCandidate)) {

    ESign trackCharge = getChargeSign(trackCandidate);

    for (const CDCRecoHit3D& hit : trackCandidate.items()) {

      if (getCurvatureSignWrt(hit, trackCandidate.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) {
        hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
        hit.getWireHit().getAutomatonCell().setTakenFlag(false);
      }

    }

    deleteAllMarkedHits(trackCandidate);

  }

  return trackCandidate;
}


bool TrackMergerNew::checkBack2BackTrack(CDCTrack& track)
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

void TrackMergerNew::resetHits(CDCTrack& otherTrackCandidate)
{
  for (const CDCRecoHit3D& hit : otherTrackCandidate.items()) {
    hit->getWireHit().getAutomatonCell().setTakenFlag(false);
  }
}


ESign TrackMergerNew::getChargeSign(CDCTrack& track)
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

ESign TrackMergerNew::getCurvatureSignWrt(const CDCRecoHit3D& hit, Vector2D xy)
{
  double phi_diff = atan2(xy.y(), xy.x()) - getPhi(hit);

  while (phi_diff > /*2 */ TMath::Pi())
    phi_diff -= 2 * TMath::Pi();
  while (phi_diff < -1. * TMath::Pi())
    phi_diff += 2 * TMath::Pi();

  if (phi_diff > 0 /*TMath::Pi()*/)
    return ESign::c_Plus;
  else
    return ESign::c_Minus;
}

double TrackMergerNew::getPhi(const CDCRecoHit3D& hit)
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
