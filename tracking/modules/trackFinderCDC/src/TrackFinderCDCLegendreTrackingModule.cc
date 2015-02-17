/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreTrackingModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/TrackFitResult.h>

/*
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackMerger.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackFitter.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendrePatternChecker.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackDrawer.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreConformalPosition.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreQuadTreeCandidateCreator.h>
#include "tracking/trackFindingCDC/legendre/CDCLegendreQuadTreeNeighborFinder.h"
#include <tracking/trackFindingCDC/legendre/CDCLegendreWireCenter.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>
*/

#include "TH1F.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"


#include "cdc/dataobjects/CDCSimHit.h"

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/MeasurementFactory.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <TDatabasePDG.h>

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>
#include <genfit/FieldManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>
#include <TGeoManager.h>
#include <TDatabasePDG.h>
#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>


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
using namespace CDC;
using namespace TrackFindingCDC;

#define SQR(x) ((x)*(x)) //we will use it in least squares fit

//ROOT macro
REG_MODULE(CDCLegendreTracking)

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  Module(), m_rMin(-0.15), m_rMax(0.15), eventnr(0)
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
           "Maximal level of recursive calling of FastHough algorithm", 11);

  addParam("ReconstructCurler", m_reconstructCurler,
           "Flag, whether curlers should be reconstructed", false);

  addParam("FitTracks", m_fitTracks,
           "Flag, whether candidates should be fitted with circle", true);

  addParam("EarlyTrackFitting", m_fitTracksEarly,
           "Flag, whether candidates should be fitted with circle at early stage", true);


  addParam("DeleteHitsInTheEnd", m_deleteHitsInTheEnd,
           "Try to delete bad hits from track candidate in the end.", false);
  addParam("MergeTracksInTheEnd", m_mergeTracksInTheEnd,
           "Try to merge tracks in the end.", false);
  addParam("AppendHitsInTheEnd", m_appendHitsInTheEnd,
           "Try to append new hits to track candidate in the end.", false);

  addParam("DeleteHitsWhileFinding", m_deleteHitsWhileFinding,
           "Try to delete bad hits from track candidate", false);
  addParam("AppendHitsWhileFinding", m_appendHitsWhileFinding,
           "Try to append new hits to track candidate while finding.", false);
  addParam("MergeTracksWhileFinding", m_mergeTracksWhileFinding,
           "Try to merge tracks while finding.", false);




  addParam("DrawCandidates", m_drawCandidates,
           "Draw candidate after finding", false);

  addParam("EnableDrawing", m_drawCandInfo,
           "Enable in-module drawing", false);

  addParam("EnableBatchMode", m_batchMode,
           "Enable batch mode for track drawer. (Done with gROOT->SetBatch())", false);

  addParam("MultipleCandidateSearch", m_multipleCandidateSearch,
           "Search multiple track candidates per run of FastHough algorithm", true);

  addParam("UseHitPrecalculatedR", m_useHitPrecalculatedR,
           "To store r values inside hit objects or recalculate it each step", true);

  addParam("AssignStereoHits", m_assignStereoHits,
           "Flag, whether stereohits should be assigned to tracks", false);


}

CDCLegendreTrackingModule::~CDCLegendreTrackingModule()
{

}

void CDCLegendreTrackingModule::setupGeometry()
{
  CDCRecoHit::setTranslators(new LinearGlobalADCCountTranslator(),
                             new IdealCDCGeometryTranslator(), new SimpleTDCCountTranslator());
  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2WARNING(
      "Material effects not set up, doing this myself with default values.  Please use SetupGenfitExtrapolationModule.");

    if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
      geometry::GeometryManager& geoManager =
        geometry::GeometryManager::getInstance();
      geoManager.createTGeoRepresentation();
    }
    genfit::MaterialEffects::getInstance()->init(
      new genfit::TGeoMaterialInterface());

    // activate / deactivate material effects in genfit
    genfit::MaterialEffects::getInstance()->setEnergyLossBetheBloch(true);
    genfit::MaterialEffects::getInstance()->setNoiseBetheBloch(true);
    genfit::MaterialEffects::getInstance()->setNoiseCoulomb(true);
    genfit::MaterialEffects::getInstance()->setEnergyLossBrems(true);
    genfit::MaterialEffects::getInstance()->setNoiseBrems(true);

    genfit::MaterialEffects::getInstance()->setMscModel("Highland");
  }
  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2WARNING("Magnetic field not set up, doing this myself.");

    //pass the magnetic field to genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::FieldManager::getInstance()->useCache();
  }
}

void CDCLegendreTrackingModule::initialize()
{
  //StoreArray for genfit::TrackCandidates
  StoreArray<genfit::TrackCand>::registerPersistent(m_gfTrackCandsColName);

  m_nbinsTheta = 8192; //hardcoded value!!! temporary solution, for avoiding segfaults only  //static_cast<int>(std::pow(2.0, m_maxLevel + 3)); //+3 needed for make bin overlapping;

  m_cdcLegendreTrackFitter = new TrackFitter(m_nbinsTheta, m_rMax, m_rMin, m_fitTracks);

  m_cdcLegendreTrackDrawer = new TrackDrawer(m_drawCandInfo, m_drawCandidates, m_batchMode);
  m_cdcLegendreTrackDrawer->initialize();

  m_axialHitList.reserve(2048);
  //m_StereoHitList.reserve(2048);

  m_cdcLegendreFastHough = new FastHough(m_reconstructCurler, m_maxLevel, m_nbinsTheta, m_rMax);

  m_cdcLegendreTrackProcessor = new TrackProcessor(m_axialHitList, m_StereoHitList, m_trackList, m_trackletList, m_stereoTrackletList, m_cdcLegendreTrackFitter, m_cdcLegendreTrackDrawer);

  m_cdcLegendreTrackMerger = new TrackMerger(m_trackList, m_trackletList, m_stereoTrackletList, m_cdcLegendreTrackFitter, m_cdcLegendreFastHough, m_cdcLegendreTrackProcessor);

  //m_cdcLegendrePatternChecker = new PatternChecker(m_cdcLegendreTrackProcessor);

//  m_cdcLegendreConformalPosition = new ConformalPosition();

//  m_cdcLegendreQuadTree = new QuadTree(-1.*m_rc, m_rc, 0, m_nbinsTheta, 0, NULL);
  m_cdcLegendreQuadTree = new QuadTree(m_rMin, m_rMax,  /*-1.*m_rc, m_rc,*/ 0, m_nbinsTheta, 0, NULL);
  m_cdcLegendreQuadTree->setLastLevel(m_maxLevel);
//  m_cdcLegendreQuadTree->buildNeighborhood(m_maxLevel);

  m_cdcLegendreQuadTreeCandidateCreator = new QuadTreeCandidateCreator();
  QuadTreeCandidateCreator m_cdcLegendreQuadTreeCandidateCreator_temp __attribute__((unused)) =  QuadTreeCandidateCreator::Instance();
  QuadTreeCandidateCreator::setCandidateCreator(m_cdcLegendreTrackProcessor);
  QuadTreeCandidateCreator::setFitter(m_cdcLegendreTrackFitter);
  QuadTreeCandidateCreator::setMerger(m_cdcLegendreTrackMerger);
  QuadTreeCandidateCreator::setAppendHitsWhileFinding(m_appendHitsWhileFinding);
  QuadTreeCandidateCreator::setMergeTracksWhileFinding(m_mergeTracksWhileFinding);
  QuadTreeCandidateCreator::setDeleteHitsWhileFinding(m_deleteHitsWhileFinding);

  //QuadTreeNeighborFinder m_cdcLegendreQuadTreeNeighborFinder_temp __attribute__((unused)) = QuadTreeNeighborFinder::Instance();

  m_treeFinder = 0;
  m_steppedFinder = 0;

  setupGeometry();
}

void CDCLegendreTrackingModule::event()
{

  eventnr++;

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
    TrackHit* trackHit = new TrackHit(cdcHits[iHit], iHit);
    if (trackHit->checkHitDriftLength() and trackHit->getIsAxial()) {
      m_axialHitList.push_back(trackHit);
      //else
      //  m_StereoHitList.push_back(trackHit);
    } else {
      delete trackHit;
    }
  }

  unsigned int size = m_axialHitList.size();

  QuadTreeCandidateCreator::setHits(m_axialHitList);

  B2DEBUG(100, "Perform track finding");

  DoTreeTrackFinding();

//  for (TrackCandidate * cand : m_trackList) {
//     if (cand->getCandidateType() != TrackCandidate::tracklet) continue;
//    m_cdcLegendreTrackMerger->extendTracklet(cand, m_AxialHitList);
//  }

  DoTreeTrackFindingFinal();
  DoTreeTrackFindingFinal();

  postprocessTracks();

//  if(m_AxialHitList.size() > 100) DoTreeTrackFinding();
//  else DoSteppedTrackFinding();

//  DoTreeStereoTrackletFinding();

//  m_cdcLegendreTrackMerger->addStereoTracklesToTrack();

//  doVXDTrackCandidatesMerging();
//  doStereoHitsAssignment();
//  doStereoHitsAssignmentInClasses();
//  doStereoHitsAssignmentWithDAF();

//1if(m_assignStereoHits) AsignStereoHits();
//  checkHitPattern();

  for (TrackCandidate * cand : m_trackList) {
    B2DEBUG(100, "R value: " << cand->getR() << "; theta: " << cand->getTheta() << "; radius: " << cand->getRadius() << "; phi: " << cand->getPhi() << "; charge: " << cand->getChargeSign() << "; Xc = " << cand->getXc() << "; Yc = " << cand->getYc());
  }

  //create GenFit Track candidates
  m_cdcLegendreTrackProcessor->createGFTrackCandidates(m_gfTrackCandsColName);

  m_cdcLegendreTrackDrawer->finalizeFile();

  unsigned int size2 = m_axialHitList.size();

  if (size != size2)
    B2ERROR("Lost hits!")

    //memory management
    clear_pointer_vectors();
}

void CDCLegendreTrackingModule::DoTreeTrackFinding()
{
  B2INFO("Tree track finding");
  m_treeFinder++;
  std::sort(m_axialHitList.begin(), m_axialHitList.end());

  std::set<TrackHit*> hits_set;
  std::set<TrackHit*>::iterator it = hits_set.begin();
  for (TrackHit * trackHit : m_axialHitList) {
    it = hits_set.insert(it, trackHit);
  }

  double limit = 40;
  double rThreshold = 0.07;
  m_cdcLegendreQuadTree->provideHitSet(hits_set);
  m_cdcLegendreQuadTree->setRThreshold(rThreshold);
  int nSteps = 0;

  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations
  QuadTree::CandidateProcessorLambda lmdCandidateProcessing = [](QuadTree * qt) -> void {
    QuadTreeCandidateCreator::Instance().createCandidateDirect(qt);
  };

  //  Start loop, where tracks are searched for
  do {
    m_cdcLegendreQuadTree->setRThreshold(rThreshold);
    m_cdcLegendreQuadTree->setHitsThreshold(limit);
    m_cdcLegendreQuadTree->startFillingTree(lmdCandidateProcessing);

    limit = limit * m_stepScale;
//    rThreshold *= 2.;
//    if (rThreshold > 0.15) rThreshold = 0.15;
    nSteps++;
    //perform search until found track has too few hits or threshold is too small and no tracks are found
  } while (limit >= m_threshold
           && hits_set.size() >= (unsigned) m_threshold);

  //sort tracks by value of curvature
  m_trackList.sort([](const TrackCandidate * a, const TrackCandidate * b) {
    return static_cast <bool>(a->getRadius() > b->getRadius());
  });

  std::vector<TrackHit*> hits_vector; //temporary array;
  m_cdcLegendreTrackDrawer->finalizeROOTFile(hits_vector);

  B2DEBUG(100, "Number of steps in tree track finding: " << nSteps);
  B2DEBUG(100, "Threshold on number of hits: " << limit);
}

void CDCLegendreTrackingModule::DoTreeTrackFindingFinal()
{
  B2INFO("Final tree track finding");
  m_treeFinder++;
  std::sort(m_axialHitList.begin(), m_axialHitList.end());

  for (TrackCandidate * cand : m_trackList) {
    for (TrackHit * hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }
  }


  std::set<TrackHit*> hits_set;
  std::set<TrackHit*>::iterator it = hits_set.begin();
  for (TrackHit * trackHit : m_axialHitList) {
    if ((trackHit->getHitUsage() != TrackHit::used_in_track) && (trackHit->getHitUsage() != TrackHit::background)) it = hits_set.insert(it, trackHit);
  }

  double limit = 20;
  double rThreshold = 0.15;
  m_cdcLegendreQuadTree->clearTree();
  m_cdcLegendreQuadTree->provideHitSet(hits_set);
  m_cdcLegendreQuadTree->setRThreshold(rThreshold);
  int nSteps = 0;

  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations
  QuadTree::CandidateProcessorLambda lmdCandidateProcessing = [](QuadTree * qt) -> void {
    QuadTreeCandidateCreator::Instance().createCandidateDirect(qt);
  };

//  Start loop, where tracks are searched for
  do {
    m_cdcLegendreQuadTree->setRThreshold(rThreshold);
    m_cdcLegendreQuadTree->setHitsThreshold(limit);
    m_cdcLegendreQuadTree->startFillingTree(lmdCandidateProcessing);

    limit = limit * m_stepScale;
    rThreshold *= 2.;
    if (rThreshold > 0.15) rThreshold = 0.15;
    nSteps++;
    //perform search until found track has too few hits or threshold is too small and no tracks are found
  } while (limit >= m_threshold
           && hits_set.size() >= (unsigned) m_threshold);

  //sort tracks by value of curvature
  m_trackList.sort([](const TrackCandidate * a, const TrackCandidate * b) {
    return static_cast <bool>(a->getRadius() > b->getRadius());
  });

  std::vector<TrackHit*> hits_vector; //temporary array;
  m_cdcLegendreTrackDrawer->finalizeROOTFile(hits_vector);
  B2DEBUG(100, "Number of steps in tree track finding: " << nSteps);
  B2DEBUG(100, "Threshold on number of hits: " << limit);
}

void CDCLegendreTrackingModule::postprocessTracks()
{

  //Track processing done in such way:
  //*try to merge tracks;
  //*split curlers and try to merge again - it allows to release tracklets produced by particles from non-IP
  //*reassign hit
  //*add unused hits to tracks

  /*
    for (TrackCandidate * cand : m_trackList) {
      m_cdcLegendreTrackFitter->performRiemannFit(cand);
    }
   */

  B2DEBUG(100, "Number of tracks: " << m_trackList.size());
  //B2DEBUG(100, "Number of trackslets: " << m_trackletList.size());

  fitAllTracks();

  if (m_deleteHitsInTheEnd) {
    for (TrackCandidate * trackCandidate : m_trackList) {
      SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8, m_cdcLegendreTrackFitter);
    }
    fitAllTracks();
  }

  if (m_appendHitsInTheEnd) {
    SimpleFilter::processTracks(m_trackList);
    fitAllTracks();
    SimpleFilter::appendUnusedHits(m_trackList, m_axialHitList, 0.8);
    fitAllTracks();
    for (TrackCandidate * trackCandidate : m_trackList) {
      SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8, m_cdcLegendreTrackFitter);
    }
  }

  fitAllTracks();

  if (m_mergeTracksInTheEnd) {
    for (TrackCandidate * trackCandidate : m_trackList) {
      m_cdcLegendreTrackMerger->tryToMergeTrackWithOtherTracks(trackCandidate);
    }

    //  m_cdcLegendreTrackMerger->splitTracks();

    //  for (TrackCandidate * cand : m_trackList) {
    //     if (cand->getCandidateType() != TrackCandidate::tracklet) continue;
    //    m_cdcLegendreTrackMerger->extendTracklet(cand, m_AxialHitList);
    //  }

    //sort tracks by value of curvature
    /*m_trackList.sort([](TrackCandidate * a, TrackCandidate * b) {
      return static_cast <bool>(a->getTrackHits().size() > b->getTrackHits().size());
    });


    m_cdcLegendreTrackMerger->doTracksMerging();*/
  }

  fitAllTracks();

  /*


      PatternChecker cdcLegendrePatternChecker(m_cdcLegendreTrackProcessor);
      for (TrackCandidate * cand : m_trackList) {
        cand->reestimateCharge();
        cdcLegendrePatternChecker.checkCandidate(cand);
        m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand);
      }
  */


//  m_cdcLegendreTrackMerger->doTracksMerging();


}

void CDCLegendreTrackingModule::endRun()
{
  B2INFO("Tree finder had " << m_treeFinder << " calls");
  B2INFO("Stepped finder had " << m_steppedFinder << " calls");
}

void CDCLegendreTrackingModule::terminate()
{
  delete m_cdcLegendreQuadTree;
  delete m_cdcLegendreQuadTreeCandidateCreator;
  delete m_cdcLegendreTrackDrawer;
  delete m_cdcLegendreFastHough;
  delete m_cdcLegendreTrackMerger;
  delete m_cdcLegendreTrackProcessor;

  //  delete m_cdcLegendreConformalPosition;
  //  delete m_cdcLegendreTrackFitter;
  //delete m_cdcLegendrePatternChecker;
  //  m_cdcLegendreQuadTree->terminate();
}

void CDCLegendreTrackingModule::clear_pointer_vectors()
{
  m_cdcLegendreQuadTree->clearTree();
  QuadTreeCandidateCreator::Instance().clearNodes();
  QuadTreeCandidateCreator::Instance().clearCandidates();

  for (TrackHit * hit : m_axialHitList) {
    delete hit;
  }
  m_axialHitList.clear();

  /*for (TrackHit * hit : m_StereoHitList) {
    delete hit;
  }
  m_StereoHitList.clear();*/

  for (TrackCandidate * track : m_trackList) {
    delete track;
  }
  m_trackList.clear();

  /*for (TrackCandidate * track : m_trackletList) {
    delete track;
  }
  m_trackletList.clear();*/

  /*for (TrackCandidate * track : m_stereoTrackletList) {
    delete track;
  }
  m_stereoTrackletList.clear();*/
}

void CDCLegendreTrackingModule::fitAllTracks()
{
  for (TrackCandidate * cand : m_trackList) {
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand);
    cand->reestimateCharge();
  }
}

// NOT USED IN THE MOMENT

/*void CDCLegendreTrackingModule::DoTreeStereoTrackletFinding()
{
  B2INFO("Tree stereo tracklet finding");
  std::sort(m_StereoHitList.begin(), m_StereoHitList.end());

  m_cdcLegendreQuadTree->setLastLevel(10);

  std::set<TrackHit*> hits_set;
  std::set<TrackHit*>::iterator it = hits_set.begin();
  for (TrackHit * trackHit : m_StereoHitList) {
    it = hits_set.insert(it, trackHit);
  }

  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations
  QuadTree::CandidateProcessorLambda lmdCandidateProcessing = [](QuadTree * qt) -> void {
    QuadTreeCandidateCreator::Instance().createCandidateDirect(qt);
  };

  for (int iSLayer = 1; iSLayer <= 7; iSLayer += 2) {
    std::set<TrackHit*> hits_set;
    std::set<TrackHit*>::iterator it = hits_set.begin();
    for (TrackHit * trackHit : m_StereoHitList) {
      if (trackHit->getSuperlayerId() == iSLayer)
        it = hits_set.insert(it, trackHit);
    }

    m_cdcLegendreQuadTree->clearTree();
    QuadTreeCandidateCreator::Instance().clearNodes();
    QuadTreeCandidateCreator::Instance().clearCandidates();

    double limit = 6;
    double rThreshold = 0.15;
    m_cdcLegendreQuadTree->provideHitSet(hits_set);
    m_cdcLegendreQuadTree->setRThreshold(rThreshold);
    m_cdcLegendreQuadTree->setRThreshold(rThreshold);
    m_cdcLegendreQuadTree->setHitsThreshold(limit);
    m_cdcLegendreQuadTree->startFillingTree(lmdCandidateProcessing);

  }

}*/

/*
void CDCLegendreTrackingModule::AsignStereoHits()
{
  for (TrackHit * hit : m_StereoHitList) {
    TrackCandidate* best = NULL;
    double best_chi2 = 999;

    for (TrackCandidate * candidate : m_trackList) {
      //precut on distance between track and stereo hit
      if (candidate->DistanceTo(*hit) <= 5.) {
        //Hit needs to have the correct curvature
        if ((candidate->getCharge() == TrackCandidate::charge_curler) || hit->getCurvatureSignWrt(candidate->getXc(), candidate->getYc()) == candidate->getCharge()) {
          //check nearest position of the hit to the track
          if (hit->approach2(*candidate)) {
            double chi2 = candidate->DistanceTo(*hit) / sqrt(hit->getSigmaDriftLength());

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

  for (TrackCandidate * cand : m_trackList) {
    cand->CheckStereoHits();
  }

}
*/

/*void CDCLegendreTrackingModule::DoSteppedTrackFinding()
{
  B2INFO("Stepped track finding");
  m_steppedFinder++;
  std::sort(m_AxialHitList.begin(), m_AxialHitList.end());

  std::set<TrackHit*> hits_set;
  std::set<TrackHit*>::iterator it = hits_set.begin();
  for (TrackHit * trackHit : m_AxialHitList) {
    it = hits_set.insert(it, trackHit);
  }

  unsigned int n_hits = 999;
  double limit = m_initialAxialHits;

  //Start loop, where tracks are searched for
  do {
    B2DEBUG(100, "Copying hits set");

    std::vector<TrackHit*> hits_vector;
    std::copy_if(hits_set.begin(), hits_set.end(), std::back_inserter(hits_vector), [&](TrackHit * hit) {return (hit->getHitUsage() == TrackHit::not_used);});
    if (not m_multipleCandidateSearch) {
      std::vector<TrackHit*> c_list;
      std::pair<std::vector<TrackHit*>, std::pair<double, double> > candidate = std::make_pair(c_list, std::make_pair(-999, -999));

      m_cdcLegendreFastHough->FastHoughNormal(&candidate, hits_vector, 1, 0, m_nbinsTheta, m_rMin, m_rMax, static_cast<unsigned>(limit));

      n_hits = candidate.first.size();

      //if no track is found
      if (n_hits == 0) {
        limit *= m_stepScale;
        n_hits = 999;
      }
      // if track is found and has enough hits
      else if (n_hits >= (unsigned) m_threshold) {

        std::pair<double, double> ref_point = std::make_pair(0., 0.);

        bool merged = false;
        if (m_mergeTracksEarly) merged = m_cdcLegendreTrackMerger->earlyCandidateMerge(candidate, hits_set, m_fitTracksEarly);

        if (!merged) m_cdcLegendreTrackProcessor->createLegendreTrackCandidate(candidate, ref_point);

        if (m_drawCandidates) m_cdcLegendreTrackDrawer->showPicture();

//        limit = n_hits * m_stepScale;
      }
    } else {
      std::vector< std::pair<std::vector<TrackHit*>, std::pair<double, double> > > candidates;

      int level __attribute__((unused)) = 0;

      B2DEBUG(100, "Perform FastHough");

      m_cdcLegendreFastHough->initializeCandidatesVector(&candidates);
      m_cdcLegendreFastHough->setLimit(limit);
      m_cdcLegendreFastHough->setAxialHits(hits_vector);
      m_cdcLegendreFastHough->MaxFastHough(hits_vector, 0, 0, m_nbinsTheta, m_rMin, m_rMax);
//      m_cdcLegendreFastHough->MaxFastHoughHighPt(hits_vector, 0, m_nbinsTheta, -0.01667, 0.01667, level);
      if (candidates.size() == 0) {
        limit *= m_stepScale;
        n_hits = 999;
      } else {
        for (std::pair<std::vector<TrackHit*>, std::pair<double, double> > candidate_temp : candidates) {
          if (candidate_temp.first.size() >= (unsigned) m_threshold) {
            std::pair<double, double> ref_point = std::make_pair(0., 0.);

            bool merged = false;
            if (m_mergeTracksEarly) merged = m_cdcLegendreTrackMerger->earlyCandidateMerge(candidate_temp, hits_set, m_fitTracksEarly);

            if (!merged) m_cdcLegendreTrackProcessor->createLegendreTrackCandidate(candidate_temp, ref_point);

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


  std::vector<TrackHit*> hits_vector_unused; //temporary array;
  std::copy_if(hits_set.begin(), hits_set.end(), std::back_inserter(hits_vector_unused), [&](TrackHit * hit) {return (hit->getHitUsage() != TrackHit::used_in_track);});
  m_cdcLegendreTrackDrawer->drawConformalHits(hits_vector_unused, -1, false);
  m_cdcLegendreTrackDrawer->drawLegendreHits(hits_vector_unused, -1, false);

  m_cdcLegendreTrackDrawer->finalizeROOTFile(hits_vector_unused);

}*/

