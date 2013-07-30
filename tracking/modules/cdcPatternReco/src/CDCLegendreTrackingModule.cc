/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcPatternReco/CDCLegendreTrackingModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <cdc/dataobjects/CDCHit.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>

#include "GFTrackCand.h"

#include <cstdlib>
#include <iomanip>
#include <string>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>

#include <iostream>
#include <algorithm>
#include <memory>
#include <cmath>

using namespace std;
using namespace Belle2;

//ROOT macro
REG_MODULE(CDCLegendreTracking)

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  Module(), m_rMin(-0.15), m_rMax(0.15)
{
  setDescription(
    "Performs the pattern recognition in the CDC with the conformal finder: digitized CDCHits are combined to track candidates (GFTrackCand)");

  addParam("CDCHitsColName", m_cdcHitsColName,
           "Input CDCHits collection (should be created by CDCDigi module)",
           string("CDCHits"));

  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Output GFTrackCandidates collection",
           string("GFTrackCandidates_LegendreFinder"));

  addParam("Threshold", m_threshold, "Threshold for peak finder", 10);

  addParam("InitialAxialHits", m_initialAxialHits,
           "Starting value of axial hits for the stepped Hough", 48);

  addParam("StepScale", m_stepScale, "Scale size for Stepped Hough", 0.75);

  addParam("Resolution StereoHits", m_resolutionStereo,
           "Total resolution, used for the assignment of stereo hits to tracks (in sigma)",
           2.);

  addParam("MaxLevel", m_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm", 10);

  addParam("Reconstruct Curler", m_reconstructCurler,
           "Flag, whether curlers should be reconstructed", true);
}

CDCLegendreTrackingModule::~CDCLegendreTrackingModule()
{

}

void CDCLegendreTrackingModule::initialize()
{
  //StoreArray for GFTrackCandidates
  StoreArray<GFTrackCand>::registerPersistent(m_gfTrackCandsColName);

  //Initialize look-up table
  m_nbinsTheta = static_cast<int>(std::pow(2.0, m_maxLevel));

  double bin_width = m_PI / m_nbinsTheta;
  m_sin_theta = new double[m_nbinsTheta + 1];
  m_cos_theta = new double[m_nbinsTheta + 1];

  for (int i = 0; i <= m_nbinsTheta; ++i) {
    m_sin_theta[i] = sin(i * bin_width);
    m_cos_theta[i] = cos(i * bin_width);
  }

  m_AxialHitList.reserve(1024);
  m_StereoHitList.reserve(1024);

}

void CDCLegendreTrackingModule::beginRun()
{

}

void CDCLegendreTrackingModule::event()
{
  B2INFO("**********   CDCTrackingModule  ************");

  //StoreArray with digitized CDCHits, should already be created by CDCDigitizer module
  StoreArray<CDCHit> cdcHits(m_cdcHitsColName);
  B2DEBUG(100,
          "CDCTracking: Number of digitized Hits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0)
    B2WARNING("CDCTracking: cdcHitsCollection is empty!");

  if (cdcHits.getEntries() > 1500) {
    B2INFO("** Skipping track finding due to too large number of hits **");
    return;
  }

  //Convert CDCHits to own Hit class
  for (int iHit = 0; iHit < cdcHits.getEntries(); iHit++) {
    CDCLegendreTrackHit* trackHit = new CDCLegendreTrackHit(cdcHits[iHit],
                                                            iHit);
    if (trackHit->getIsAxial())
      m_AxialHitList.push_back(trackHit);
    else
      m_StereoHitList.push_back(trackHit);
  }

  //perform track finding
  DoSteppedTrackFinding();
  MergeCurler();
  AsignStereoHits();

  //create GenFit Track candidates
  createGFTrackCandidates();

  //memory management
  clear_pointer_vectors();
}

void CDCLegendreTrackingModule::DoSteppedTrackFinding()
{
  std::sort(m_AxialHitList.begin(), m_AxialHitList.end());

  std::set<CDCLegendreTrackHit*> hits_set;
  std::set<CDCLegendreTrackHit*>::iterator it = hits_set.begin();
  BOOST_FOREACH(CDCLegendreTrackHit * trackHit, m_AxialHitList) {
    it = hits_set.insert(it, trackHit);
  }

  int n_hits = 999;
  double limit = m_initialAxialHits;

  //Start loop, where tracks are searched for
  do {
    std::vector<CDCLegendreTrackHit*> hits_vector;
    std::copy(hits_set.begin(), hits_set.end(), std::back_inserter(hits_vector));

    std::vector<CDCLegendreTrackHit*> c_list;
    std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate =
      std::make_pair(c_list, std::make_pair(-999, -999));

    MaxFastHough(&candidate, hits_vector, 1, 0, m_nbinsTheta, m_rMin, m_rMax,
                 static_cast<unsigned>(limit));

    n_hits = candidate.first.size();

    //if no track is found
    if (n_hits == 0) {
      limit *= m_stepScale;
      n_hits = 999;
    }

    // if track is found and has enough hits
    else if (n_hits >= m_threshold) {
      createLegendreTrackCandidate(candidate, &hits_set);

      limit = n_hits * m_stepScale;
    }

    //perform search until found track has too few hits or threshold is too small and no tracks are found
  } while (n_hits >= m_threshold
           && (limit / m_stepScale >= m_threshold || n_hits != 999)
           && hits_set.size() >= (unsigned) m_threshold);

}

void CDCLegendreTrackingModule::MergeCurler()
{
  //loop over all candidates
  for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
         m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
    CDCLegendreTrackCandidate* cand1 = *it1;

    //check only curler
    if (fabs(cand1->getR()) > m_rc) {

      //loop over remaining candidates
      std::list<CDCLegendreTrackCandidate*>::iterator it2 = boost::next(it1);
      while (it2 != m_trackList.end()) {
        CDCLegendreTrackCandidate* cand2 = *it2;
        ++it2;

        if (fabs(cand2->getR()) > m_rc) {

          //check if the two tracks lie next to each other
          if (fabs(cand1->getR() - cand2->getR()) < 0.03
              && fabs(cand1->getTheta() - cand2->getTheta()) < 0.15)

            mergeTracks(cand1, cand2);
        }
      }
    }
  }
}

void CDCLegendreTrackingModule::AsignStereoHits()
{
  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_StereoHitList) {
    CDCLegendreTrackCandidate* best = NULL;
    double best_chi2 = 999;

    BOOST_FOREACH(CDCLegendreTrackCandidate * candidate, m_trackList) {
      //precut on distance between track and stereo hit
      if (candidate->DistanceTo(*hit) <= 5.) {
        //Hit needs to have the correct curvature
        if ((candidate->getCharge() == CDCLegendreTrackCandidate::charge_curler) || hit->getCurvatureSignWrt(candidate->getXc(), candidate->getYc()) == candidate->getCharge()) {
          //check nearest position of the hit to the track
          if (hit->approach2(*candidate)) {
            double chi2 = candidate->DistanceTo(*hit) / sqrt(hit->getDeltaDriftTime());

            if (chi2 < m_resolutionStereo) {
              //search for minimal distance
              if (chi2 < best_chi2) {
                best = candidate;
                best_chi2 = chi2;
              }
            }
          }
        }
      }
    }

    //if there is track near enough
    if (best != NULL) {
      best->addHit(hit);
    }
  }

  BOOST_FOREACH(CDCLegendreTrackCandidate * cand, m_trackList) {
    cand->CheckStereoHits();
  }

}

void CDCLegendreTrackingModule::mergeTracks(CDCLegendreTrackCandidate* cand1,
                                            CDCLegendreTrackCandidate* cand2)
{

  cand1->setR(
    (cand1->getR() * cand1->getNHits() + cand2->getR() * cand2->getNHits())
    / (cand1->getNHits() + cand2->getNHits()));
  cand1->setTheta(
    (cand1->getTheta() * cand1->getNHits()
     + cand2->getTheta() * cand2->getNHits())
    / (cand1->getNHits() + cand2->getNHits()));

  BOOST_FOREACH(CDCLegendreTrackHit * hit, cand2->getTrackHits()) {
    cand1->addHit(hit);
  }

  m_trackList.remove(cand2);
  delete cand2;
  cand2 = NULL;
}

void CDCLegendreTrackingModule::createLegendreTrackCandidate(
  const std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track,
  std::set<CDCLegendreTrackHit*>* trackHitList)
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

void CDCLegendreTrackingModule::processTrack(
  CDCLegendreTrackCandidate* trackCandidate,
  std::set<CDCLegendreTrackHit*>* trackHitList)
{
  //check if the number has enough axial hits (might be less due to the curvature check).
  if (fullfillsQualityCriteria(trackCandidate)) {
    m_trackList.push_back(trackCandidate);

    BOOST_FOREACH(CDCLegendreTrackHit * hit, trackCandidate->getTrackHits()) {
      trackHitList->erase(hit);
    }
  }

  else {
    BOOST_FOREACH(CDCLegendreTrackHit * hit, trackCandidate->getTrackHits()) {
      trackHitList->erase(hit);
    }

    //memory management, since we cannot use smart pointers in function interfaces
    delete trackCandidate;
    trackCandidate = NULL;
  }

}

bool CDCLegendreTrackingModule::fullfillsQualityCriteria(CDCLegendreTrackCandidate* trackCandidate)
{
  if (trackCandidate->getNAxialHits() < m_threshold)
    return false;

  if (trackCandidate->getLayerWaight() < 1)
    return false;

  return true;
}

void CDCLegendreTrackingModule::createGFTrackCandidates()
{
  //StoreArray for GFTrackCandidates: interface class to Genfit
  StoreArray<GFTrackCand> gfTrackCandidates(m_gfTrackCandsColName);
  gfTrackCandidates.create();

  int i = 0;

  BOOST_FOREACH(CDCLegendreTrackCandidate * trackCand, m_trackList) {
    gfTrackCandidates.appendNew();

    //set the values needed as start values for the fit in the GFTrackCandidate from the CDCTrackCandidate information
    //variables stored in the GFTrackCandidates are: vertex position + error, momentum + error, pdg value, indices for the Hits
    TVector3 position;
    position.SetXYZ(0.0, 0.0, 0.0);//at the moment there is no vertex determination in the ConformalFinder, but maybe the origin or the innermost hit are good enough as start values...
    //position = cdcTrackCandidates[i]->getInnerMostHit().getWirePosition();

    TVector3 momentum =
      trackCand->getMomentumEstimation(true);

    //Pattern recognition can determine only the charge, so here some dummy pdg value is set (with the correct charge), the pdg hypothesis can be then overwritten in the GenFitterModule
    int pdg = trackCand->getChargeSign() * (211);

    //The initial covariance matrix is calculated from these errors and it is important (!!) that it is not completely wrong
    TMatrixDSym covSeed(6);
    covSeed(0, 0) = 4; covSeed(1, 1) = 4; covSeed(2, 2) = 4;
    covSeed(3, 3) = 0.1 * 0.1; covSeed(4, 4) = 0.1 * 0.1; covSeed(5, 5) = 0.5 * 0.5;

    //set the start parameters
    gfTrackCandidates[i]->setPosMomSeedAndPdgCode(position, momentum, pdg, covSeed);


    B2DEBUG(100, "Create GFTrackCandidate " << i << "  with pdg " << pdg);
    B2DEBUG(100,
            "position seed:  (" << position.x() << ", " << position.y() << ", " << position.z() << ")   position variance: (" << covSeed(0, 0) << ", " << covSeed(1, 1) << ", " << covSeed(2, 2) << ") ");
    B2DEBUG(100,
            "momentum seed:  (" << momentum.x() << ", " << momentum.y() << ", " << momentum.z() << ")   position variance: (" << covSeed(3, 3) << ", " << covSeed(4, 4) << ", " << covSeed(5, 5) << ") ");

    //find indices of the Hits
    std::vector<CDCLegendreTrackHit*> trackHitVector = trackCand->getTrackHits();

    sortHits(trackHitVector, trackCand->getChargeSign());

    B2DEBUG(100, " Add Hits: hitId rho planeId")

    BOOST_FOREACH(CDCLegendreTrackHit * trackHit, trackHitVector) {
      int hitID = trackHit->getStoreIndex();
      gfTrackCandidates[i]->addHit(Const::CDC, hitID);
    }
    ++i;
  }
}

void CDCLegendreTrackingModule::endRun()
{

}

void CDCLegendreTrackingModule::terminate()
{
  delete[] m_cos_theta;
  delete[] m_sin_theta;
}

void CDCLegendreTrackingModule::MaxFastHough(
  std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* candidate,
  const std::vector<CDCLegendreTrackHit*>& hits, const int level,
  const int theta_min, const int theta_max, const double r_min,
  const double r_max, const unsigned limit)
{
  if (not m_reconstructCurler
      && (r_min * r_max >= 0 && fabs(r_min) > m_rc && fabs(r_max) > m_rc)) {
    return;
  }

  //calculate bin borders of 2x2 bin "histogram"
  int thetaBin[3];
  thetaBin[0] = theta_min;
  thetaBin[1] = theta_min + (theta_max - theta_min) / 2;
  thetaBin[2] = theta_max;

  double r[3];
  r[0] = r_min;
  r[1] = r_min + 0.5 * (r_max - r_min);
  r[2] = r_max;

  //2 x 2 voting plane
  std::vector<CDCLegendreTrackHit*> voted_hits[2][2];
  for (unsigned int i = 0; i < 2; ++i)
    for (unsigned int j = 0; j < 2; ++j)
      voted_hits[i][j].reserve(1024);

  double r_temp, r_1, r_2;
  double dist_1[3][3];
  double dist_2[3][3];

  //Voting within the four bins
  BOOST_FOREACH(CDCLegendreTrackHit * hit, hits) {
    for (int t_index = 0; t_index < 3; ++t_index) {
      r_temp = hit->getConformalX() * m_cos_theta[thetaBin[t_index]]
               + hit->getConformalY() * m_sin_theta[thetaBin[t_index]];
      r_1 = r_temp + hit->getConformalDriftTime();
      r_2 = r_temp - hit->getConformalDriftTime();

      //calculate distances of lines to horizontal bin border
      for (int r_index = 0; r_index < 3; ++r_index) {
        dist_1[t_index][r_index] = r[r_index] - r_1;
        dist_2[t_index][r_index] = r[r_index] - r_2;
      }
    }

    //actual voting, based on the distances (test, if line is passing though the bin)
    for (int t_index = 0; t_index < 2; ++t_index) {
      for (int r_index = 0; r_index < 2; ++r_index) {
        //curves are assumed to be straight lines, might be a reasonable assumption locally
        if (!sameSign(dist_1[t_index][r_index], dist_1[t_index][r_index + 1], dist_1[t_index + 1][r_index], dist_1[t_index + 1][r_index + 1]))
          voted_hits[t_index][r_index].push_back(hit);
        else if (!sameSign(dist_2[t_index][r_index], dist_2[t_index][r_index + 1], dist_2[t_index + 1][r_index], dist_2[t_index + 1][r_index + 1]))
          voted_hits[t_index][r_index].push_back(hit);
      }
    }

  }

//Processing, which bins are further investigated
  for (int t_index = 0; t_index < 2; ++t_index) {
    for (int r_index = 0; r_index < 2; ++r_index) {

      //bin must contain more hits than the limit and maximal found track candidate
      if (voted_hits[t_index][r_index].size() >= limit
          && voted_hits[t_index][r_index].size() > candidate->first.size()) {

        //if max level of fast Hough is reached, mark candidate and return
        if (level == m_maxLevel) {
          double theta = static_cast<double>(thetaBin[t_index]
                                             + thetaBin[t_index + 1]) / 2 * m_PI / m_nbinsTheta;

          if (not m_reconstructCurler
              && fabs((r[r_index] + r[r_index + 1]) / 2) > m_rc)
            return;

          candidate->first = voted_hits[t_index][r_index];
          candidate->second = std::make_pair(theta,
                                             (r[r_index] + r[r_index + 1]) / 2);
        } else {
          //Recursive calling of the function with higher level and smaller box
          MaxFastHough(candidate, voted_hits[t_index][r_index], level + 1,
                       thetaBin[t_index], thetaBin[t_index + 1], r[r_index],
                       r[r_index + 1], limit);
        }
      }
    }
  }
}

inline bool CDCLegendreTrackingModule::sameSign(double n1, double n2,
                                                double n3, double n4)
{
  if (n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0)
    return true;
  else if (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0)
    return true;
  else
    return false;
}

void CDCLegendreTrackingModule::sortHits(
  std::vector<CDCLegendreTrackHit*>& hits, int charge)
{
  CDCTracking_SortHit sorter(charge);
  stable_sort(hits.begin(), hits.end(), sorter);
}

bool CDCTracking_SortHit::operator()(CDCLegendreTrackHit* hit1,
                                     CDCLegendreTrackHit* hit2)
{
  bool result = true;
  boost::tuple<int, double, int, double> tuple1(hit1->getStoreIndex(),
                                                hit1->getWirePosition().Mag(), hit1->getWireId());
  boost::tuple<int, double, int, double> tuple2(hit2->getStoreIndex(),
                                                hit2->getWirePosition().Mag(), hit2->getWireId());

  //the comparison function for the tuples created by the sort function

  if ((int) tuple1.get<1>() == (int) tuple2.get<1>()) {
    //special case: several hits in the same layer
    //now we have to proceed differently for positive and negative tracks
    //in a common case we just have to check the wireIds and decide the order according to the charge
    //if however the track is crossing the wireId 0, we have again to treat it specially
    //the number 100 is just a more or less arbitrary number, assuming that no track will be 'crossing' 100 different wireIds

    //in general this solution does not look very elegant, so if you have some suggestions how to improve it, do not hesitate to tell me

    if (m_charge < 0) {
      //negative charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0 && tuple2.get<2>() > 100) {
        result = false;
      }
      if (tuple1.get<2>() > 100 && tuple2.get<2>() == 0) {
        result = true;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = true;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = false;
      }
    } //end negative charge

    else {
      //positive charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0 && tuple2.get<2>() > 100) {
        result = true;
      }
      if (tuple1.get<2>() > 100 && tuple2.get<2>() == 0) {
        result = false;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = false;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = true;
      }
    } //end positive charge

  }

  //usual case: hits sorted by the rho value
  else
    result = (tuple1.get<1>() < tuple2.get<1>());

  return result;

}

void CDCLegendreTrackingModule::clear_pointer_vectors()
{

  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_AxialHitList) {
    delete hit;
  }
  m_AxialHitList.clear();

  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_StereoHitList) {
    delete hit;
  }
  m_StereoHitList.clear();

  BOOST_FOREACH(CDCLegendreTrackCandidate * track, m_trackList) {
    delete track;
  }
  m_trackList.clear();
}

