/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreTrackingModule.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackDrawer.h>
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>

#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessorWithNewReferencePoint.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>

#include <genfit/TrackCand.h>
#include <cdc/dataobjects/CDCHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(CDCLegendreTracking)

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  TrackFinderCDCBaseModule(), m_cdcLegendreQuadTree(0, m_nbinsTheta, m_rMin, m_rMax, 0, nullptr),
  m_cdcLegendreTrackProcessor(), m_cdcLegendreTrackDrawer(nullptr)
{
  setDescription(
    "Performs the pattern recognition in the CDC with the conformal finder: digitized CDCHits are combined to track candidates (genfit::TrackCand)");

  addParam("Threshold", m_param_threshold, "Threshold for peak finder", static_cast<unsigned int>(10));

  addParam("StepScale", m_param_stepScale, "Scale size for Stepped Hough", 0.75);

  addParam("MaxLevel", m_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm", 12);

  addParam("DeleteHitsInTheEnd", m_deleteHitsInTheEnd,
           "Try to delete bad hits from track candidate in the end.", true);
  addParam("MergeTracksInTheEnd", m_mergeTracksInTheEnd,
           "Try to merge tracks in the end.", true);
  addParam("AppendHitsInTheEnd", m_appendHitsInTheEnd,
           "Try to append new hits to track candidate in the end.", true);

  addParam("DeleteHitsWhileFinding", m_deleteHitsWhileFinding,
           "Try to delete bad hits from track candidate", true);
  addParam("MergeTracksWhileFinding", m_mergeTracksWhileFinding,
           "Try to merge tracks while finding.", false);
  addParam("AppendHitsWhileFinding", m_appendHitsWhileFinding,
           "Try to append new hits to track candidate while finding.", false);
  addParam("DoPostprocessingOften", m_doPostprocessingOften,
           "Repeat the postprocessing mode after every tree search.", true);
  addParam("TreeFindingNumber", m_treeFindingNumber,
           "Repeat the whole process that many times.", 3);

  addParam("DrawCandidates", m_drawCandidates,
           "Draw candidate after finding", false);

  addParam("EnableDrawing", m_drawCandInfo,
           "Enable in-module drawing", false);

  addParam("EnableBatchMode", m_batchMode,
           "Enable batch mode for track drawer. (Done with gROOT->SetBatch())", false);
}

void CDCLegendreTrackingModule::initialize()
{
  TrackFinderCDCBaseModule::initialize();

  // initialize track drawer
  m_cdcLegendreTrackDrawer = new TrackDrawer(m_drawCandInfo, m_drawCandidates, m_batchMode);
  m_cdcLegendreTrackDrawer->initialize();

  // set parameters of track processor
  m_cdcLegendreTrackProcessor.setTrackDrawer(m_cdcLegendreTrackDrawer);
}

void CDCLegendreTrackingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  startNewEvent();
  findTracks();
  outputObjects(tracks);
  clearVectors();
}

void CDCLegendreTrackingModule::startNewEvent()
{
  B2DEBUG(100, "**********   CDCTrackingModule  ************");
  m_cdcLegendreTrackDrawer->event();

  B2DEBUG(100, "Initializing hits");
  m_cdcLegendreTrackProcessor.initializeHitListFromWireHitTopology();
}

void CDCLegendreTrackingModule::findTracks()
{
  B2DEBUG(100, "Perform track finding");

  // The first case is somewhat special
  doTreeTrackFinding(50, 0.07, false);
  if (m_treeFindingNumber == 1 || m_doPostprocessingOften)
    postprocessTracks();

  for (int counter = 1; counter < m_treeFindingNumber; counter++) {
    doTreeTrackFinding((m_treeFindingNumber - counter) * 20, 0.15, true);
    if (counter == m_treeFindingNumber - 1 || m_doPostprocessingOften)
      postprocessTracks();
  }
}

void CDCLegendreTrackingModule::outputObjects(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  //create GenFit Track candidates
  m_cdcLegendreTrackProcessor.createCDCTrackCandidates(tracks);
  m_cdcLegendreTrackDrawer->finalizeFile();
}


void CDCLegendreTrackingModule::doTreeTrackFinding(unsigned int limitInitial, double rThreshold, bool increaseThreshold)
{
  B2DEBUG(100, "Performing tree track finding");

  unsigned int limit = limitInitial;

  int maxLevel = 1;

  if (not increaseThreshold) maxLevel = m_maxLevel;
  else maxLevel = m_maxLevel - 2;


  double rCDC = 113.;

  if (increaseThreshold) {
    m_cdcLegendreTrackProcessor.resetBadHits();
  }

  std::set<TrackHit*> hits_set = m_cdcLegendreTrackProcessor.createHitSet();

  std::vector<TrackHit*> hitsVector;

  for (TrackHit* hit : hits_set) {
    hitsVector.push_back(hit);
  }


  AxialHitQuadTreeProcessor::ChildRanges ranges(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2, 13)),
                                                AxialHitQuadTreeProcessor::rangeY(-0.15, 0.15));
  std::vector<AxialHitQuadTreeProcessor::ReturnList> candidates;

  AxialHitQuadTreeProcessor qtProcessor(maxLevel, ranges);
  qtProcessor.provideItemsSet(hitsVector);


  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations

  AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdCandidateProcessingFinal = [&](const AxialHitQuadTreeProcessor::ReturnList &
  hits, AxialHitQuadTreeProcessor::QuadTree * qt) -> void {


    std::vector<TrackHit*> candidateHits;

    for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItemsVector())
    {
      hit->setUsedFlag(TrackHit::c_notUsed);
      candidateHits.push_back(hit->getPointer());
    }

    postprocessSingleNode(candidateHits, increaseThreshold, qt);

  };


  //find high-pt tracks (not-curlers: diameter of the track higher than radius of CDC -- 2*Rtrk > rCDC => Rtrk < 2./rCDC, r(legendre) = 1/Rtrk =>  r(legendre) < 2./rCDC)
  qtProcessor.fillGivenTree(lmdCandidateProcessingFinal, 50, 2. / rCDC);

  //find curlers with diameter higher than half of radius of CDC (see calculations above)
  qtProcessor.fillGivenTree(lmdCandidateProcessingFinal, 70, 4. / rCDC);


  int nSteps(0);
  // Start loop, where tracks are searched for
  do {
    qtProcessor.fillGivenTree(lmdCandidateProcessingFinal, limit, rThreshold);

    limit = limit * m_param_stepScale;

    if (increaseThreshold) {
      rThreshold *= 2.;
      if (rThreshold > 0.15) rThreshold = 0.15;
    }


    nSteps++;
    //perform search until found track has too few hits or threshold is too small and no tracks are found
  } while (limit >= m_param_threshold && hitsVector.size() >= m_param_threshold);

  //sort tracks by value of curvature
  m_cdcLegendreTrackProcessor.sortTrackList();

  // ?????
  std::vector<TrackHit*> hits_vector; //temporary array;
  m_cdcLegendreTrackDrawer->finalizeROOTFile(hits_vector);

  B2DEBUG(90, "Number of steps in tree track finding: " << nSteps);
  B2DEBUG(90, "Threshold on number of hits: " << limit);
  B2DEBUG(90, "Threshold on r: " << rThreshold);
}


void CDCLegendreTrackingModule::postprocessSingleNode(std::vector<TrackFindingCDC::TrackHit*>& candidateHits,
                                                      bool increaseThreshold, AxialHitQuadTreeProcessor::QuadTree* qt)
{

  for (TrackHit* hit : candidateHits) {
    hit->setHitUsage(TrackHit::c_notUsed);
  }

//  m_cdcLegendreTrackProcessor.createLegendreTrackCandidateFromHits(candidateHits);

//  return;

  std::pair<double, double> track_par;
  std::pair<double, double> ref_point;
  TrackFitter cdcLegendreTrackFitter;
  double chi2 = cdcLegendreTrackFitter.fitTrackCandidateFast(candidateHits, track_par, ref_point);
  double D = pow(ref_point.first * ref_point.first + ref_point.second * ref_point.second, 0.5);

  if (not increaseThreshold) {
    if (TrackCandidate::convertRhoToPt(fabs(qt->getYMean())) > 0.7 && (D > 0.4)) {
      for (TrackHit* hit : candidateHits) {
        hit->setHitUsage(TrackHit::c_bad);
      }

      return;
    }
  }

  double levelPrecision = 10.5 - 0.24 * exp(-4.13118 * TrackCandidate::convertRhoToPt(fabs(track_par.second)) + 2.74);

  double precision_r, precision_theta;
  precision_theta = 3.1415 / (pow(2., levelPrecision + 1));
  precision_r = 0.15 / (pow(2., levelPrecision));

  double B = -1.*track_par.second * (ref_point.first) - (1 - track_par.second * D) * cos(track_par.first);
  double C = track_par.second * (ref_point.second) + (1 - track_par.second * D) * sin(track_par.first);
  double theta_new = atan2(B, C) + boost::math::constants::pi<double>() / 2.;
//    if(theta_new < 0) theta_new += boost::math::constants::pi<double>();


  AxialHitQuadTreeProcessorWithNewReferencePoint::ChildRanges
  ranges(AxialHitQuadTreeProcessorWithNewReferencePoint::rangeX(static_cast<float>(theta_new - precision_theta),
         static_cast<float>(theta_new + precision_theta)),
         AxialHitQuadTreeProcessorWithNewReferencePoint::rangeY(static_cast<float>(track_par.second - precision_r),
             static_cast<float>(track_par.second + precision_r)));

  std::vector<AxialHitQuadTreeProcessorWithNewReferencePoint::ReturnList> candidates;

  for (TrackHit* hit : candidateHits) {
    hit->setHitUsage(TrackHit::c_usedInTrack);
  }

  std::set<TrackHit*> hits_set = m_cdcLegendreTrackProcessor.createHitSet();

  std::vector<TrackHit*> tmpHitsVector;

  for (TrackHit* hit : hits_set) {
    tmpHitsVector.push_back(hit);
  }

  AxialHitQuadTreeProcessorWithNewReferencePoint qtProcessor(ranges, ref_point);
  qtProcessor.provideItemsSet(tmpHitsVector);

  std::vector<TrackHit*> newAssignedHits = qtProcessor.getAssignedHits();

  for (TrackHit* hit : candidateHits) {
    hit->setHitUsage(TrackHit::c_notUsed);
  }


  if (newAssignedHits.size() > 0) {
    std::vector<TrackHit*> trackHitsTemp;

    for (TrackHit* hit : candidateHits) {
      trackHitsTemp.push_back(hit);
    }
    for (TrackHit* hit : newAssignedHits) {
      trackHitsTemp.push_back(hit);
    }
    double chi2New = cdcLegendreTrackFitter.fitTrackCandidateFast(trackHitsTemp, track_par, ref_point);

    if (chi2New * 2. < chi2) {
      for (TrackHit* hit : newAssignedHits) {
        candidateHits.push_back(hit);
      }
    }
  }

  TrackCandidate* trackCandidate = m_cdcLegendreTrackProcessor.createLegendreTrackCandidateFromHits(candidateHits);

  // Postprocessing of one track candidate
  m_cdcLegendreTrackProcessor.fitOneTrack(trackCandidate);

  if (m_deleteHitsWhileFinding) {
    m_cdcLegendreTrackProcessor.deleteBadHitsOfOneTrack(trackCandidate);
  }

  if (m_mergeTracksWhileFinding) {
    m_cdcLegendreTrackProcessor.mergeOneTrack(trackCandidate);
  }

  if (m_appendHitsWhileFinding) {
    m_cdcLegendreTrackProcessor.appendHitsOfAllTracks();
  }

  m_cdcLegendreTrackProcessor.deleteTracksWithASmallNumberOfHits();


  m_cdcLegendreTrackProcessor.sortTrackList();


}

void CDCLegendreTrackingModule::postprocessTracks()
{
  m_cdcLegendreTrackProcessor.fitAllTracks();

  if (m_deleteHitsInTheEnd) {
    m_cdcLegendreTrackProcessor.deleteHitsOfAllBadTracks();
  }

  if (m_mergeTracksInTheEnd) {
    m_cdcLegendreTrackProcessor.mergeAllTracks();
  }

  if (m_appendHitsInTheEnd) {
    m_cdcLegendreTrackProcessor.appendHitsOfAllTracks();
  }
}

void CDCLegendreTrackingModule::terminate()
{
  delete m_cdcLegendreTrackDrawer;
}

void CDCLegendreTrackingModule::clearVectors()
{
  m_cdcLegendreTrackProcessor.clearVectors();
}
