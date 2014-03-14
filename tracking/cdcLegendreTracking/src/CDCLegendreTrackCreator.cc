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

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <cdc/dataobjects/CDCHit.h>
#include "genfit/TrackCand.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <string>

using namespace Belle2;
using namespace std;

CDCLegendreTrackCreator::CDCLegendreTrackCreator(std::vector<CDCLegendreTrackHit*>& AxialHitList, std::list<CDCLegendreTrackCandidate*>& trackList,
                                                 bool appendHits, CDCLegendreTrackFitter* cdcLegendreTrackFitter, CDCLegendreTrackDrawer* cdcLegendreTrackDrawer):
  m_AxialHitList(AxialHitList), m_trackList(trackList), m_appendHits(appendHits),
  m_cdcLegendreTrackFitter(cdcLegendreTrackFitter), m_cdcLegendreTrackDrawer(cdcLegendreTrackDrawer)
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
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate, ref_point);
    trackCandidate->setReferencePoint(ref_point.first, ref_point.second);
    appendNewHits(trackCandidate);
//    trackCandidate->clearBadHits(ref_point);

    processTrack(trackCandidate, trackHitList);

  }

  //here we create two oppositely charged tracks (with the same theta and r value)
  else if (charge == CDCLegendreTrackCandidate::charge_two_tracks) {
    CDCLegendreTrackCandidate* trackCandidate_pos =
      new CDCLegendreTrackCandidate(track_theta, track_r,
                                    CDCLegendreTrackCandidate::charge_positive, track.first);

    CDCLegendreTrackCandidate* trackCandidate_neg =
      new CDCLegendreTrackCandidate(track_theta, track_r,
                                    CDCLegendreTrackCandidate::charge_negative, track.first);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate_pos, ref_point);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate_neg, ref_point);
    trackCandidate_neg->setReferencePoint(ref_point.first, ref_point.second);
    trackCandidate_pos->setReferencePoint(ref_point.first, ref_point.second);
    appendNewHits(trackCandidate_pos);
    appendNewHits(trackCandidate_neg);
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

void CDCLegendreTrackCreator::appendNewHits(CDCLegendreTrackCandidate* track)
{
  if (not m_appendHits) return;
  double x0_track = cos(track->getTheta()) / track->getR() + track->getReferencePoint().X();
  double y0_track = sin(track->getTheta()) / track->getR() + track->getReferencePoint().Y();
  double R = fabs(1. / track->getR());

  for (CDCLegendreTrackHit * hit : m_AxialHitList) {
    if (hit->isUsed() != CDCLegendreTrackHit::used_in_track) {
      double x0_hit = hit->getOriginalWirePosition().X();
      double y0_hit = hit->getOriginalWirePosition().Y();
      double dist = fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftTime();
      if (dist < hit->getDriftTime() * 0.75) track->addHit(hit);
    }
  }
}

void CDCLegendreTrackCreator::processTrack(
  CDCLegendreTrackCandidate* trackCandidate,
  std::set<CDCLegendreTrackHit*>* /*trackHitList*/)
{
  //check if the number has enough axial hits (might be less due to the curvature check).
  if (fullfillsQualityCriteria(trackCandidate)) {
    m_trackList.push_back(trackCandidate);

    m_cdcLegendreTrackDrawer->drawTrackCand(trackCandidate);

    for (CDCLegendreTrackHit * hit : trackCandidate->getTrackHits()) {
      hit->setUsed(CDCLegendreTrackHit::used_in_track);
    }
  }

  else {
    for (CDCLegendreTrackHit * hit : trackCandidate->getTrackHits()) {
      hit->setUsed(CDCLegendreTrackHit::used_bad);
    }

    //memory management, since we cannot use smart pointers in function interfaces
    delete trackCandidate;
    trackCandidate = NULL;
  }

}


bool CDCLegendreTrackCreator::fullfillsQualityCriteria(CDCLegendreTrackCandidate* /*trackCandidate*/)
{
//  if (trackCandidate->getNAxialHits() < m_threshold)
//    return false;

//  if (trackCandidate->getLayerWaight() < 1)
//    return false;

  return true;
}


void CDCLegendreTrackCreator::createGFTrackCandidates(string& m_gfTrackCandsColName)
{
  //StoreArray for genfit::TrackCandidates: interface class to Genfit
  StoreArray<genfit::TrackCand> gfTrackCandidates(m_gfTrackCandsColName);
  gfTrackCandidates.create();

  int i = 0;

  for (CDCLegendreTrackCandidate * trackCand : m_trackList) {
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
    std::vector<CDCLegendreTrackHit*> trackHitVector = trackCand->getTrackHits();

    sortHits(trackHitVector, trackCand->getChargeSign());

    B2DEBUG(100, " Add Hits: hitId rho planeId")

    for (CDCLegendreTrackHit * trackHit : trackHitVector) {
      int hitID = trackHit->getStoreIndex();
      gfTrackCandidates[i]->addHit(Const::CDC, hitID);
    }
    ++i;
  }
}


void CDCLegendreTrackCreator::sortHits(
  std::vector<CDCLegendreTrackHit*>& hits, int charge)
{
  CDCLegendreTrackingSortHit sorter(charge);
  stable_sort(hits.begin(), hits.end(), sorter);
}
