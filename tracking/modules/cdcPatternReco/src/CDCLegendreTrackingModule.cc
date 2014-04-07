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
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCreator.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackMerger.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <tracking/cdcLegendreTracking/CDCLegendrePatternChecker.h>
#include <tracking/cdcLegendreTracking/CDCLegendreFastHough.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackDrawer.h>
#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>
#include <tracking/cdcLegendreTracking/CDCLegendreConformalPosition.h>

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
           "Starting value of axial hits for the stepped Hough", 30);

  addParam("StepScale", m_stepScale, "Scale size for Stepped Hough", 0.75);

  addParam("Resolution StereoHits", m_resolutionStereo,
           "Total resolution, used for the assignment of stereo hits to tracks (in sigma)",
           2.);

  addParam("MaxLevel", m_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm", 10);

  addParam("Reconstruct Curler", m_reconstructCurler,
           "Flag, whether curlers should be reconstructed", false);

  addParam("FitTracks", m_fitTracks,
           "Flag, whether candidates should be fitted with circle", true);

  addParam("EarlyTrackFitting", m_fitTracksEarly,
           "Flag, whether candidates should be fitted with circle at early stage", false);

  addParam("EarlyTrackMerge", m_mergeTracksEarly,
           "Try to merge hit pattern after FastHough with any found track candidate", false);

  addParam("AppendHits", m_appendHits,
           "Try to append new hits to track candidate", false);

  addParam("DrawCandidates", m_drawCandidates,
           "Draw candidate after finding", false);

  addParam("EnableDrawing", m_drawCandInfo,
           "Enable in-module drawing", false);

  addParam("MultipleCandidateSearch", m_multipleCandidateSearch,
           "Search multiple track candidates per run of FastHough algorithm", true);

  addParam("UseHitPrecalculatedR", m_useHitPrecalculatedR,
           "To store r values inside hit objects or recalculate it each step", true);

}

CDCLegendreTrackingModule::~CDCLegendreTrackingModule()
{

}

void CDCLegendreTrackingModule::initialize()
{
  //StoreArray for genfit::TrackCandidates
  StoreArray<genfit::TrackCand>::registerPersistent(m_gfTrackCandsColName);

  m_nbinsTheta = 8192; //hardcoded value!!! temporary solution, for avoiding segfaults only  //static_cast<int>(std::pow(2.0, m_maxLevel + 3)); //+3 needed for make bin overlapping;

  m_cdcLegendreTrackFitter = new CDCLegendreTrackFitter(m_nbinsTheta, m_rMax, m_rMin, m_fitTracks);
  m_cdcLegendrePatternChecker = new CDCLegendrePatternChecker();

  m_AxialHitList.reserve(2048);
  m_StereoHitList.reserve(2048);

  m_cdcLegendreFastHough = new CDCLegendreFastHough(m_useHitPrecalculatedR, m_reconstructCurler, m_maxLevel, m_nbinsTheta, m_rMin, m_rMax);

  m_cdcLegendreTrackMerger = new CDCLegendreTrackMerger(m_trackList, m_cdcLegendreTrackFitter);

  m_cdcLegendreTrackDrawer = new CDCLegendreTrackDrawer(m_drawCandInfo, m_drawCandidates);
  m_cdcLegendreTrackDrawer->initialize();

  m_cdcLegendreTrackCreator = new CDCLegendreTrackCreator(m_AxialHitList, m_trackList, m_appendHits, m_cdcLegendreTrackFitter, m_cdcLegendreTrackDrawer);

  m_cdcLegendreConformalPosition = new CDCLegendreConformalPosition();

  CDCLegendreConformalPosition& m_cdcLegendreConformalPosition_temp __attribute__((unused)) = CDCLegendreConformalPosition::Instance();

//  m_cdcLegendreQuadTree = new CDCLegendreQuadTree(-1.*m_rc, m_rc, 0, m_nbinsTheta, 0, NULL);
}

void CDCLegendreTrackingModule::beginRun()
{

}

void CDCLegendreTrackingModule::event()
{

  m_cdcLegendreTrackDrawer->event();

  B2INFO("**********   CDCTrackingModule  ************");

  B2DEBUG(100, "Initializing hits");
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
    CDCLegendreTrackHit* trackHit = new CDCLegendreTrackHit(cdcHits[iHit], iHit);
    if (trackHit->getIsAxial())
      m_AxialHitList.push_back(trackHit);
    else
      m_StereoHitList.push_back(trackHit);
  }

  B2DEBUG(100, "Perform track finding");

  //perform track finding
  DoSteppedTrackFinding();
//  MergeTracks();
//    MergeCurler();
//  AsignStereoHits();

//  checkHitPattern();

  //create GenFit Track candidates
  m_cdcLegendreTrackCreator->createGFTrackCandidates(m_gfTrackCandsColName);

  //memory management
  clear_pointer_vectors();


  m_cdcLegendreTrackDrawer->finalizeFile();
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
    B2DEBUG(100, "Copying hits set");

    std::vector<CDCLegendreTrackHit*> hits_vector;
    std::copy_if(hits_set.begin(), hits_set.end(), std::back_inserter(hits_vector), [](CDCLegendreTrackHit * hit) {return (hit->isUsed() == CDCLegendreTrackHit::not_used);});
    if (not m_multipleCandidateSearch) {
      std::vector<CDCLegendreTrackHit*> c_list;
      std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate = std::make_pair(c_list, std::make_pair(-999, -999));

      m_cdcLegendreFastHough->FastHoughNormal(&candidate, hits_vector, 1, 0, m_nbinsTheta, m_rMin, m_rMax, static_cast<unsigned>(limit));

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
        if (m_mergeTracksEarly) merged = m_cdcLegendreTrackMerger->earlyCandidateMerge(candidate, hits_set, m_fitTracksEarly);

        if (!merged) m_cdcLegendreTrackCreator->createLegendreTrackCandidate(candidate, &hits_set, ref_point);

        if (m_drawCandidates) m_cdcLegendreTrackDrawer->showPicture();

//        limit = n_hits * m_stepScale;
      }
    } else {
      std::vector< std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > > candidates;

      int level = 0;

      B2DEBUG(100, "Perform FastHough");

      m_cdcLegendreFastHough->initializeCandidatesVector(&candidates);
      m_cdcLegendreFastHough->setLimit(limit);
      m_cdcLegendreFastHough->setAxialHits(hits_vector);
//      m_cdcLegendreFastHough->MaxFastHough(hits_vector, 0, 0, m_nbinsTheta, m_rMin, m_rMax);
//      m_cdcLegendreFastHough->MaxFastHoughHighPtHeap(hits_vector, 0, m_nbinsTheta, m_rMin, m_rMax, level);
      m_cdcLegendreFastHough->MaxFastHoughHighPtStack(hits_vector, 0, m_nbinsTheta, -0.01667, 0.01667, level);
      if (candidates.size() == 0) {
        limit *= m_stepScale;
        n_hits = 999;
      } else {
        for (std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp : candidates) {
          if (candidate_temp.first.size() >= m_threshold) {
            std::pair<double, double> ref_point = std::make_pair(0., 0.);

            bool merged = false;
            if (m_mergeTracksEarly) merged = m_cdcLegendreTrackMerger->earlyCandidateMerge(candidate_temp, hits_set, m_fitTracksEarly);

            if (!merged) m_cdcLegendreTrackCreator->createLegendreTrackCandidate(candidate_temp, &hits_set, ref_point);

            if (m_drawCandidates) m_cdcLegendreTrackDrawer->showPicture();
          }
        }
        limit = limit * m_stepScale;
      }
    }

    //perform search until found track has too few hits or threshold is too small and no tracks are found
  } while (limit >= m_threshold
           && (limit / m_stepScale >= m_threshold || n_hits != 999)
           && hits_set.size() >= (unsigned) m_threshold);


  std::vector<CDCLegendreTrackHit*> hits_vector; //temporary array;
  m_cdcLegendreTrackDrawer->finalizeROOTFile(hits_vector);

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

void CDCLegendreTrackingModule::endRun()
{
}

void CDCLegendreTrackingModule::terminate()
{
  delete m_cdcLegendreConformalPosition;
//  delete m_cdcLegendreTrackFitter;
  delete m_cdcLegendreTrackDrawer;
  delete m_cdcLegendrePatternChecker;
  delete m_cdcLegendreFastHough;
  delete m_cdcLegendreTrackMerger;
  delete m_cdcLegendreTrackCreator;
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

}

