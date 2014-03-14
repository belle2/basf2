/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreTrackCreator.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <cmath>

using namespace Belle2;
using namespace std;

CDCLegendreTrackCreator::CDCLegendreTrackCreator(
  std::list<CDCLegendreTrackCandidate*>& trackList, CDCLegendreTrackFitter* cdcLegendreTrackFitter):
  m_trackList(trackList), m_cdcLegendreTrackFitter(cdcLegendreTrackFitter)
{

}

void CDCLegendreTrackCreator::createLegendreTrackCandidate(
  const std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track,
  std::set<CDCLegendreTrackHit*>* trackHitList, std::pair<double, double>& ref_point)
{

  //get theta and r values for each track candidate
  double track_theta = track.second.first;
  double track_r = track.second.second;

  //get charge estimation for the track candidate
  int charge = CDCLegendreTrackCandidate::getChargeAssumption(track_theta,
                                                              track_r, track.first);

  //for curlers, negative, and positive tracks we want to create one track candidate
  if (charge == CDCLegendreTrackCandidate::charge_positive
      || charge == CDCLegendreTrackCandidate::charge_negative
      || charge == CDCLegendreTrackCandidate::charge_curler) {
    CDCLegendreTrackCandidate* trackCandidate = new CDCLegendreTrackCandidate(
      track_theta, track_r, charge, track.first);
    trackCandidate->setReferencePoint(ref_point.first, ref_point.second);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate, ref_point);
//    trackCandidate->clearBadHits(ref_point);

    processTrack(trackCandidate, trackHitList);

  }

  //here we create two oppositely charged tracks (with the same theta and r value)
  else if (charge == CDCLegendreTrackCandidate::charge_two_tracks) {
    CDCLegendreTrackCandidate* trackCandidate_pos =
      new CDCLegendreTrackCandidate(track_theta, track_r,
                                    CDCLegendreTrackCandidate::charge_positive, track.first);
    trackCandidate_pos->setReferencePoint(ref_point.first, ref_point.second);

    CDCLegendreTrackCandidate* trackCandidate_neg =
      new CDCLegendreTrackCandidate(track_theta, track_r,
                                    CDCLegendreTrackCandidate::charge_negative, track.first);
    trackCandidate_neg->setReferencePoint(ref_point.first, ref_point.second);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate_pos, ref_point);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate_neg, ref_point);
//    trackCandidate_pos->clearBadHits(ref_point);
//    trackCandidate_neg->clearBadHits(ref_point);

    processTrack(trackCandidate_pos, trackHitList);

    processTrack(trackCandidate_neg, trackHitList);
  }
  //This shouldn't happen, check CDCLegendreTrackCandidate::getChargeAssumption()
  else {
    B2ERROR(
      "Strange behavior of CDCLegendreTrackCandidate::getChargeAssumption");
    exit(EXIT_FAILURE);
  }
}

void CDCLegendreTrackCreator::processTrack(
  CDCLegendreTrackCandidate* trackCandidate,
  std::set<CDCLegendreTrackHit*>* trackHitList)
{
  //check if the number has enough axial hits (might be less due to the curvature check).
  if (fullfillsQualityCriteria(trackCandidate)) {
    /*    int candType = trackCandidate->getCandidateType();
        if(candType == CDCLegendreTrackCandidate::fullTrack)m_fullTrackList.push_back(trackCandidate);
        else if(candType == CDCLegendreTrackCandidate::curlerTrack)m_shortTrackList.push_back(trackCandidate);
        else if(candType == CDCLegendreTrackCandidate::tracklet)m_trackletTrackList.push_back(trackCandidate);
        else {
          for(CDCLegendreTrackHit * hit: trackCandidate->getTrackHits()) {
            trackHitList->erase(hit);
          }


          //memory management, since we cannot use smart pointers in function interfaces
          delete trackCandidate;
          trackCandidate = NULL;
        }
    */
    m_trackList.push_back(trackCandidate);

    for (CDCLegendreTrackHit * hit : trackCandidate->getTrackHits()) {
//      trackHitList->erase(hit);
      hit->setUsed(CDCLegendreTrackHit::used_in_track);
    }
  }

  else {
    for (CDCLegendreTrackHit * hit : trackCandidate->getTrackHits()) {
//      trackHitList->erase(hit);
      hit->setUsed(CDCLegendreTrackHit::used_bad);
    }

    //memory management, since we cannot use smart pointers in function interfaces
    delete trackCandidate;
    trackCandidate = NULL;
  }

}


bool CDCLegendreTrackCreator::fullfillsQualityCriteria(CDCLegendreTrackCandidate* trackCandidate)
{
//  if (trackCandidate->getNAxialHits() < m_threshold)
//    return false;

//  if (trackCandidate->getLayerWaight() < 1)
//    return false;

  return true;
}


