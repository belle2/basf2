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
#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackMerger.h>
#include <tracking/cdcLegendreTracking/CDCLegendrePatternChecker.h>
#include <tracking/cdcLegendreTracking/CDCLegendreFastHough.h>

#include "genfit/TrackCand.h"

#include <cstdlib>
#include <iomanip>
#include <string>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>

using namespace std;
using namespace Belle2;

#define SQR(x) ((x)*(x)) //we will use it in least squares fit

//ROOT macro
REG_MODULE(CDCLegendreTracking)

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  Module(), m_rMin(-0.15), m_rMax(0.15)
{
  setDescription(
    "Performs the pattern recognition in the CDC with the conformal finder: digitized CDCHits are combined to track candidates (genfit::TrackCand)");

  addParam("CDCHitsColName", m_cdcHitsColName,
           "Input CDCHits collection (should be created by CDCDigi module)",
           string("CDCHits"));

  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Output GFTrackCandidates collection",
           string("TrackCands"));

  addParam("Threshold", m_threshold, "Threshold for peak finder", 10);

  addParam("InitialAxialHits", m_initialAxialHits,
           "Starting value of axial hits for the stepped Hough", 48);

  addParam("StepScale", m_stepScale, "Scale size for Stepped Hough", 0.75);

  addParam("Resolution StereoHits", m_resolutionStereo,
           "Total resolution, used for the assignment of stereo hits to tracks (in sigma)",
           2.);

  addParam("MaxLevel", m_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm", 12);

  addParam("Reconstruct Curler", m_reconstructCurler,
           "Flag, whether curlers should be reconstructed", true);

  addParam("Fit tracks", m_fitTracks,
           "Flag, whether candidates should be fitted with circle", false);

  addParam("Early track merging", m_earlyMerge,
           "Try to merge hit pattern after FastHough with any found track candidate", false);
}

CDCLegendreTrackingModule::~CDCLegendreTrackingModule()
{

}

void CDCLegendreTrackingModule::initialize()
{
  //StoreArray for genfit::TrackCandidates
  StoreArray<genfit::TrackCand>::registerPersistent(m_gfTrackCandsColName);

  m_nbinsTheta = static_cast<int>(std::pow(2.0, m_maxLevel + 3)); //+3 needed for make bin overlapping;

  m_cdcLegendreTrackFitter = new CDCLegendreTrackFitter(m_nbinsTheta, m_rMax, m_rMin);
  m_cdcLegendrePatternChecker = new CDCLegendrePatternChecker();

  m_AxialHitList.reserve(1024);
  m_StereoHitList.reserve(1024);

  m_cdcLegendreFastHough = new CDCLegendreFastHough(m_reconstructCurler, m_maxLevel, m_nbinsTheta, m_rMin, m_rMax);

  m_cdcLegendreTrackMerger = new CDCLegendreTrackMerger(m_trackList, m_cdcLegendreTrackFitter);
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

//  if (cdcHits.getEntries() > 1500) {
//    B2INFO("** Skipping track finding due to too large number of hits **");
//    return;
//  }

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
//  MergeTracks();
//    MergeCurler();
//  AsignStereoHits();

//  checkHitPattern();

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
  for (CDCLegendreTrackHit * trackHit : m_AxialHitList) {
    it = hits_set.insert(it, trackHit);
  }

  int n_hits = 999;
  double limit = m_initialAxialHits;

  //Start loop, where tracks are searched for
  do {
    std::vector<CDCLegendreTrackHit*> hits_vector;
    std::copy_if(hits_set.begin(), hits_set.end(), std::back_inserter(hits_vector), [](CDCLegendreTrackHit * hit) {return (hit->isUsed() == CDCLegendreTrackHit::not_used);});

    std::vector<CDCLegendreTrackHit*> c_list;
    std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate =
      std::make_pair(c_list, std::make_pair(-999, -999));

    m_cdcLegendreFastHough->MaxFastHough(&candidate, hits_vector, 1, 0, m_nbinsTheta, m_rMin, m_rMax,
                                         static_cast<unsigned>(limit));

    n_hits = candidate.first.size();

    //if no track is found
    if (n_hits == 0) {
      limit *= m_stepScale;
      n_hits = 999;
    }


    // if track is found and has enough hits
    else if (n_hits >= m_threshold) {

      std::pair<double, double> ref_point = std::make_pair(0., 0.);

      bool merged = false;
      merged = m_cdcLegendreTrackMerger->earlyCandidateMerge(candidate, hits_set);

      if (!merged) createLegendreTrackCandidate(candidate, &hits_set, ref_point);

      limit = n_hits * m_stepScale;
    }

    //perform search until found track has too few hits or threshold is too small and no tracks are found
  } while (n_hits >= m_threshold
           && (limit / m_stepScale >= m_threshold || n_hits != 999)
           && hits_set.size() >= (unsigned) m_threshold);

}


void CDCLegendreTrackingModule::AsignStereoHits()
{
  for (CDCLegendreTrackHit * hit : m_StereoHitList) {
    CDCLegendreTrackCandidate* best = NULL;
    double best_chi2 = 999;

    for (CDCLegendreTrackCandidate * candidate : m_trackList) {
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

  for (CDCLegendreTrackCandidate * cand : m_trackList) {
    cand->CheckStereoHits();
  }

}


void CDCLegendreTrackingModule::checkHitPattern()
{
  int candType;
  for (CDCLegendreTrackCandidate * trackCand : m_trackList) {
    cout << "pattern:" << trackCand->getHitPatternAxial().getHitPattern() << endl;
    candType = trackCand->getCandidateType();
    cout << "candType = " << candType << endl;
//    if(candType == 1)

  }
}

void CDCLegendreTrackingModule::createLegendreTrackCandidate(
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

void CDCLegendreTrackingModule::processTrack(
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

bool CDCLegendreTrackingModule::fullfillsQualityCriteria(CDCLegendreTrackCandidate* trackCandidate)
{
//  if (trackCandidate->getNAxialHits() < m_threshold)
//    return false;

//  if (trackCandidate->getLayerWaight() < 1)
//    return false;

  return true;
}

void CDCLegendreTrackingModule::createGFTrackCandidates()
{
  //StoreArray for genfit::TrackCandidates: interface class to Genfit
  StoreArray<genfit::TrackCand> gfTrackCandidates(m_gfTrackCandsColName);
  gfTrackCandidates.create();

  int i = 0;

  for (CDCLegendreTrackCandidate * trackCand : m_trackList) {
    gfTrackCandidates.appendNew();
    std::pair<double, double> ref_point_temp = std::make_pair(0., 0.);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCand, ref_point_temp);

//  testing of track's hit pattern
//    cout << "pattern:" << trackCand->getHitPattern().getHitPattern() << endl;

    //set the values needed as start values for the fit in the genfit::TrackCandidate from the CDCTrackCandidate information
    //variables stored in the genfit::TrackCandidates are: vertex position + error, momentum + error, pdg value, indices for the Hits
    TVector3 position;
//    position.SetXYZ(0.0, 0.0, 0.0);//at the moment there is no vertex determination in the ConformalFinder, but maybe the origin or the innermost hit are good enough as start values...
    //position = cdcTrackCandidates[i]->getInnerMostHit().getWirePosition();
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

void CDCLegendreTrackingModule::endRun()
{

}

void CDCLegendreTrackingModule::terminate()
{
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

  for (CDCLegendreTrackHit * hit : m_AxialHitList) {
    delete hit;
  }
  m_AxialHitList.clear();

  for (CDCLegendreTrackHit * hit : m_StereoHitList) {
    delete hit;
  }
  m_StereoHitList.clear();

  for (CDCLegendreTrackCandidate * track : m_trackList) {
    delete track;
  }
  m_trackList.clear();

  for (CDCLegendreTrackCandidate * track : m_fullTrackList) {
    delete track;
  }
  m_fullTrackList.clear();

  for (CDCLegendreTrackCandidate * track : m_shortTrackList) {
    delete track;
  }
  m_shortTrackList.clear();

  for (CDCLegendreTrackCandidate * track : m_trackletTrackList) {
    delete track;
  }
  m_trackletTrackList.clear();

}

