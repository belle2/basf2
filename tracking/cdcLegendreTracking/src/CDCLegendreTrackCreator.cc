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
#include <tracking/cdcLegendreTracking/CDCLegendreTrackingSortHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackDrawer.h>
#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>
#include <tracking/cdcLegendreTracking/CDCLegendrePatternChecker.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <cdc/dataobjects/CDCHit.h>
#include "genfit/TrackCand.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <string>

using namespace std;
using namespace Belle2;
using namespace TrackFinderCDCLegendre;

TrackCreator::TrackCreator(std::vector<TrackHit*>& AxialHitList, std::vector<TrackHit*>& StereoHitList, std::list<TrackCandidate*>& trackList, std::list<TrackCandidate*>& trackletList,
                           std::list<TrackCandidate*>& stereoTrackletList, bool appendHits, TrackFitter* cdcLegendreTrackFitter, TrackDrawer* cdcLegendreTrackDrawer):
  m_AxialHitList(AxialHitList), m_StereoHitList(StereoHitList), m_trackList(trackList), m_trackletList(trackletList), m_stereoTrackletList(stereoTrackletList), m_appendHits(appendHits),
  m_cdcLegendreTrackFitter(cdcLegendreTrackFitter), m_cdcLegendreTrackDrawer(cdcLegendreTrackDrawer)
{

}

void TrackCreator::createLegendreTrackCandidate(
  const std::pair<std::vector<TrackHit*>, std::pair<double, double> >& track,
  std::pair<double, double>& ref_point)
{

  //get theta and r values for each track candidate
  double track_theta = track.second.first;
  double track_r = track.second.second;

  //get charge estimation for the track candidate
  int charge = TrackCandidate::getChargeAssumption(track_theta,
                                                   track_r, track.first);

  //for curlers, negative, and positive tracks we want to create one track candidate
  if (charge == TrackCandidate::charge_positive
      || charge == TrackCandidate::charge_negative
      || charge == TrackCandidate::charge_curler) {
    TrackCandidate* trackCandidate = new TrackCandidate(
      track_theta, track_r, charge, track.first);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate, ref_point);
    trackCandidate->setReferencePoint(ref_point.first, ref_point.second);
    appendNewHits(trackCandidate);
//    trackCandidate->clearBadHits(ref_point);

    processTrack(trackCandidate, m_trackList);

    trackCandidate->setCandidateType(TrackCandidate::goodTrack);

  }

  //here we create two oppositely charged tracks (with the same theta and r value)
  else if (charge == TrackCandidate::charge_two_tracks) {
    TrackCandidate* trackCandidate_pos =
      new TrackCandidate(track_theta, track_r,
                         TrackCandidate::charge_positive, track.first);

    TrackCandidate* trackCandidate_neg =
      new TrackCandidate(track_theta, track_r,
                         TrackCandidate::charge_negative, track.first);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate_pos, ref_point);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate_neg, ref_point);
    trackCandidate_neg->setReferencePoint(ref_point.first, ref_point.second);
    trackCandidate_pos->setReferencePoint(ref_point.first, ref_point.second);
    appendNewHits(trackCandidate_pos);
    appendNewHits(trackCandidate_neg);
//    trackCandidate_pos->clearBadHits(ref_point);
//    trackCandidate_neg->clearBadHits(ref_point);

    processTrack(trackCandidate_pos, m_trackList);

    processTrack(trackCandidate_neg, m_trackList);

    trackCandidate_pos->setCandidateType(TrackCandidate::goodTrack);
    trackCandidate_neg->setCandidateType(TrackCandidate::goodTrack);

  }
  //This shouldn't happen, check TrackCandidate::getChargeAssumption()
  else {
    B2ERROR(
      "Strange behavior of TrackCandidate::getChargeAssumption");
    exit(EXIT_FAILURE);
  }
}

TrackCandidate* TrackCreator::createLegendreTrackCandidate(std::vector<QuadTree*> nodeList)
{
  std::pair<double, double> ref_point = std::make_pair(0., 0.);;
  TrackCandidate* trackCandidate = new TrackCandidate(nodeList);
//  trackCandidate->clearBadHits(ref_point);
//    appendNewHits(trackCandidate);

//  trackCandidate->checkHitPattern();
  m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate, ref_point);
  trackCandidate->setReferencePoint(ref_point.first, ref_point.second);

  processTrack(trackCandidate, m_trackList);
  appendNewHits(trackCandidate);

  trackCandidate->setCandidateType(TrackCandidate::goodTrack);

  return trackCandidate;
}


TrackCandidate* TrackCreator::createLegendreTracklet(std::vector<TrackHit*>& hits)
{
  std::pair<double, double> ref_point = std::make_pair(0., 0.);
  std::pair<double, double> track_par = std::make_pair(-999, -999);
  double chi2;
  chi2 = m_cdcLegendreTrackFitter->fitTrackCandidateFast(hits, track_par, ref_point);
  int charge = TrackCandidate::getChargeAssumption(track_par.first,
                                                   track_par.second, hits);
  TrackCandidate* trackCandidate = new TrackCandidate(track_par.first, track_par.second, charge /*TrackCandidate::charge_tracklet*/, hits);
//  trackCandidate->clearBadHits(ref_point);
//    appendNewHits(trackCandidate);
  trackCandidate->setChi2(chi2);

  trackCandidate->setReferencePoint(ref_point.first, ref_point.second);

  processTrack(trackCandidate, m_trackList);

  for (TrackHit * hit : trackCandidate->getTrackHits()) {
    hit->setHitUsage(TrackHit::not_used);
  }

  trackCandidate->setCandidateType(TrackCandidate::tracklet);

  return trackCandidate;
}


TrackCandidate* TrackCreator::createLegendreStereoTracklet(std::vector<QuadTree*> nodeList)
{
  TrackCandidate* trackCandidate = new TrackCandidate(nodeList);
  std::pair<double, double> ref_point = std::make_pair(0., 0.);
  m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate, ref_point);
  //  trackCandidate->clearBadHits(ref_point);
//    appendNewHits(trackCandidate);


  processTrack(trackCandidate, m_stereoTrackletList);

  return trackCandidate;

}


void TrackCreator::appendNewHits(TrackCandidate* track)
{
  if (not m_appendHits) return;
  double x0_track = cos(track->getTheta()) / track->getR() + track->getReferencePoint().X();
  double y0_track = sin(track->getTheta()) / track->getR() + track->getReferencePoint().Y();
  double R = fabs(1. / track->getR());

  for (TrackHit * hit : m_AxialHitList) {
    if (hit->getHitUsage() != TrackHit::used_in_track || hit->getHitUsage() != TrackHit::background) {
      double x0_hit = hit->getOriginalWirePosition().X();
      double y0_hit = hit->getOriginalWirePosition().Y();
      double dist = fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftLength();
      if (dist < hit->getDriftLength() * 0.75) track->addHit(hit);
    }
  }
}

void TrackCreator::processTrack(TrackCandidate* trackCandidate, std::list<TrackCandidate*>& trackList)
{


  //check if the number has enough axial hits (might be less due to the curvature check).
  if (fullfillsQualityCriteria(trackCandidate)) {
    trackList.push_back(trackCandidate);

    m_cdcLegendreTrackDrawer->drawTrackCand(trackCandidate);

    for (TrackHit * hit : trackCandidate->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }

    PatternChecker cdcLegendrePatternChecker(this);
    cdcLegendrePatternChecker.checkCandidate(trackCandidate);

  }

  else {
    for (TrackHit * hit : trackCandidate->getTrackHits()) {
      hit->setHitUsage(TrackHit::bad);
    }

    //memory management, since we cannot use smart pointers in function interfaces
    delete trackCandidate;
    trackCandidate = NULL;
  }

}


bool TrackCreator::fullfillsQualityCriteria(TrackCandidate* /*trackCandidate*/)
{
//  if (trackCandidate->getNAxialHits() < m_threshold)
//    return false;

//  if (trackCandidate->getLayerWaight() < 1)
//    return false;

  return true;
}

void TrackCreator::moveCandidate(list<TrackCandidate*>& initialTrackList, list<TrackCandidate*>& resultTrackList, TrackCandidate* cand)
{
  initialTrackList.remove(cand);
  resultTrackList.push_back(cand);
}

void TrackCreator::removeFromList(list<TrackCandidate*>& trackList, TrackCandidate* cand)
{
  trackList.remove(cand);
}


void TrackCreator::createGFTrackCandidates(string& m_gfTrackCandsColName)
{
  //StoreArray for genfit::TrackCandidates: interface class to Genfit
  StoreArray<genfit::TrackCand> gfTrackCandidates(m_gfTrackCandsColName);
  gfTrackCandidates.create();

  int i = 0;
  /*  std::vector<TrackCandidate*> completeTracksList;
    std::copy(m_trackList.begin(), m_trackList.end(), std::back_inserter(completeTracksList));
    std::copy(m_trackletList.begin(), m_trackletList.end(), std::back_inserter(completeTracksList));
    std::copy(m_stereoTrackletList.begin(), m_stereoTrackletList.end(), std::back_inserter(completeTracksList));
  */
  for (TrackCandidate * trackCand : m_trackList) {
//    if (trackCand->getNHits() == 0) continue;
    if (trackCand->getTrackHits().size() < 5) continue;
//    B2INFO("Number of hits: " << trackCand->getNHits());
    gfTrackCandidates.appendNew();
    std::pair<double, double> ref_point_temp = std::make_pair(0., 0.);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCand, ref_point_temp);


    //set the values needed as start values for the fit in the genfit::TrackCandidate from the CDCTrackCandidate information
    //variables stored in the genfit::TrackCandidates are: vertex position + error, momentum + error, pdg value, indices for the Hits
    TVector3 position;
//    position.SetXYZ(0.0, 0.0, 0.0);//at the moment there is no vertex determination in the ConformalFinder, but maybe the origin or the innermost hit are good enough as start values...
    position = trackCand->getReferencePoint();

    TVector3 momentum =
      trackCand->getMomentumEstimation(true);

    //Pattern recognition can determine only the charge, so here some dummy pdg value is set (with the correct charge), the pdg hypothesis can be then overwritten in the GenFitterModule
    int pdg = trackCand->getChargeSign() * (211);

    //The initial covariance matrix is calculated from these errors and it is important (!!) that it is not completely wrong
    /*TMatrixDSym covSeed(6);
    covSeed(0, 0) = 4; covSeed(1, 1) = 4; covSeed(2, 2) = 4;
    covSeed(3, 3) = 0.1 * 0.1; covSeed(4, 4) = 0.1 * 0.1; covSeed(5, 5) = 0.5 * 0.5;*/

    //set the start parameters
    gfTrackCandidates[i]->setPosMomSeedAndPdgCode(position, momentum, pdg);


    B2DEBUG(100, "Create genfit::TrackCandidate " << i << "  with pdg " << pdg);
    B2DEBUG(100,
            "position seed:  (" << position.x() << ", " << position.y() << ", " << position.z() << ")");//   position variance: (" << covSeed(0, 0) << ", " << covSeed(1, 1) << ", " << covSeed(2, 2) << ") ");
    B2DEBUG(100,
            "momentum seed:  (" << momentum.x() << ", " << momentum.y() << ", " << momentum.z() << ")");//   position variance: (" << covSeed(3, 3) << ", " << covSeed(4, 4) << ", " << covSeed(5, 5) << ") ");

    //find indices of the Hits
    std::vector<TrackHit*>& trackHitVector = trackCand->getTrackHits();

    sortHits(trackHitVector, trackCand->getChargeSign());


    for (TrackHit * trackHit : trackHitVector) {
      int hitID = trackHit->getStoreIndex();
      gfTrackCandidates[i]->addHit(Const::CDC, hitID);
    }
    ++i;
  }
}


void TrackCreator::sortHits(
  std::vector<TrackHit*>& hits, int charge)
{
  SortHits sorter(charge);
  stable_sort(hits.begin(), hits.end(), sorter);
}
