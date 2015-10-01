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

#include <tracking/trackFindingCDC/legendre/quadtree/precision_functions/OriginPrecisionFunction.h>
#include <tracking/trackFindingCDC/legendre/quadtree/precision_functions/NonOriginPrecisionFunction.h>


#include <genfit/TrackCand.h>
#include <cdc/dataobjects/CDCHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(CDCLegendreTracking)

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  TrackFinderCDCBaseModule(), m_cdcLegendreTrackProcessor(), m_cdcLegendreTrackDrawer(nullptr), m_trackProcessor()
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

  m_track1Run = 0;
  m_track2Run = 0;
  m_track3Run = 0;

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
//  m_cdcLegendreTrackProcessor.initializeHitListFromWireHitTopology();
  m_trackProcessor.initializeQuadTreeHitWrappers();
}

void CDCLegendreTrackingModule::findTracks()
{
  B2DEBUG(100, "Perform track finding");

  int nTracksBefore = m_trackProcessor.getTracks().size();
  // The first case is somewhat special
  doTreeTrackFinding(50, 0.07, false);
//  if (m_treeFindingNumber == 1 || m_doPostprocessingOften)
//    postprocessTracks();
  int nTracksAfter = m_trackProcessor.getTracks().size();
//  m_trackProcessor.mergeTracks();

  m_track1Run += nTracksAfter - nTracksBefore;

  nTracksBefore = nTracksAfter;
  doTreeTrackFinding(50, 0.07, true);
//  if (m_treeFindingNumber == 1 || m_doPostprocessingOften)
//    postprocessTracks();
  nTracksAfter = m_trackProcessor.getTracks().size();
//  m_trackProcessor.mergeTracks();

  m_track2Run += nTracksAfter - nTracksBefore;

  nTracksBefore = nTracksAfter;
  for (int counter = 1; counter < m_treeFindingNumber; counter++) {
    doTreeTrackFinding((m_treeFindingNumber - counter) * 20, 0.15, true);
//    if (counter == m_treeFindingNumber - 1 || m_doPostprocessingOften)
//      postprocessTracks();
  }
  nTracksAfter = m_trackProcessor.getTracks().size();
//  m_trackProcessor.mergeTracks();

  m_track3Run += nTracksAfter - nTracksBefore;


//  m_trackProcessor.mergeTracks();
//  m_trackProcessor.assignNewHits();
}

void CDCLegendreTrackingModule::outputObjects(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  m_trackProcessor.createCDCTrackCandidates(tracks);
  m_cdcLegendreTrackDrawer->finalizeFile();
}


void CDCLegendreTrackingModule::doTreeTrackFinding(unsigned int limitInitial, double rThreshold, bool increaseThreshold)
{
  B2DEBUG(100, "Performing tree track finding");

  unsigned int limit = limitInitial;

  int maxLevel = 1;

  if (not increaseThreshold) maxLevel = m_maxLevel;
  else maxLevel = m_maxLevel - 3;


  double rCDC = 113.;

  m_trackProcessor.resetMaskedHits();


  std::vector<QuadTreeHitWrapper*> hitsVector = m_trackProcessor.createQuadTreeHitWrappersForQT();


  OriginPrecisionFunction originPrecisionFunction;
  NonOriginPrecisionFunction nonOriginPrecisionFunction;


  BasePrecisionFunction::PrecisionFunction currentFunct;
  currentFunct = originPrecisionFunction.getFunction();

  AxialHitQuadTreeProcessor::ChildRanges ranges;
  ranges = std::make_pair(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2, 16)), AxialHitQuadTreeProcessor::rangeY(0,  0.3));


  if ((rThreshold == 0.07) and (not increaseThreshold)) {
    currentFunct = originPrecisionFunction.getFunction();
    ranges = std::make_pair(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2, 16)), AxialHitQuadTreeProcessor::rangeY(0., 0.075));
  } else if ((rThreshold == 0.07) and increaseThreshold) {
    currentFunct = nonOriginPrecisionFunction.getFunction();
    ranges = std::make_pair(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2, 16)), AxialHitQuadTreeProcessor::rangeY(0., 0.15));
  } else {
    currentFunct = nonOriginPrecisionFunction.getFunction();
    ranges = std::make_pair(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2, 16)), AxialHitQuadTreeProcessor::rangeY(0, 0.15));
  }
  std::vector<AxialHitQuadTreeProcessor::ReturnList> candidates;


  AxialHitQuadTreeProcessor qtProcessor(maxLevel, ranges, currentFunct, (increaseThreshold and (rThreshold < 0.15)));
  qtProcessor.provideItemsSet(hitsVector);
//  qtProcessor.drawNode();


  AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdAdvancedProcessing = [&](const AxialHitQuadTreeProcessor::ReturnList &
  __attribute__((unused)) hits, AxialHitQuadTreeProcessor::QuadTree * qt) -> void {
    double rRes = currentFunct(qt->getYMean());
    unsigned long thetaRes = abs(m_nbinsTheta* rRes / 0.3);

    unsigned long meanTheta = qt->getXMean();
    double meanR = qt->getYMean();


    std::vector< AxialHitQuadTreeProcessor::QuadTree*> nodesWithPossibleCands;

    for (int ii = -1; ii <= 1; ii = +2)
    {
      for (int jj = -1; jj <= 1; jj = +2) {
        AxialHitQuadTreeProcessor::ChildRanges rangesTmp(AxialHitQuadTreeProcessor::rangeX(meanTheta + static_cast<unsigned long>((
          ii - 1)*thetaRes / 2),
        meanTheta + static_cast<unsigned long>((ii + 1)*thetaRes / 2)),
        AxialHitQuadTreeProcessor::rangeY(meanR + (jj - 1)*rRes / 2.,
        meanR + (jj + 1)*rRes / 2.));

        nodesWithPossibleCands.push_back(qtProcessor.createSingleNode(rangesTmp));
      }
    }

    for (int ii = -1; ii <= 1; ii++)
    {
      for (int jj = -1; jj <= 1; jj++) {
        AxialHitQuadTreeProcessor::ChildRanges rangesTmp(AxialHitQuadTreeProcessor::rangeX(meanTheta + static_cast<unsigned long>((
                                                           2 * ii - 1)*thetaRes / 2),
                                                         meanTheta + static_cast<unsigned long>((2 * ii + 1)*thetaRes / 2)),
                                                         AxialHitQuadTreeProcessor::rangeY(meanR + (2 * jj - 1)*rRes / 2.,
                                                             meanR + (2 * jj + 1)*rRes / 2.));

        nodesWithPossibleCands.push_back(qtProcessor.createSingleNode(rangesTmp));
      }
    }


    std::sort(nodesWithPossibleCands.begin(), nodesWithPossibleCands.end(), [](const AxialHitQuadTreeProcessor::QuadTree * a, const AxialHitQuadTreeProcessor::QuadTree * b)
    {
      return static_cast <bool>(a->getNItems() > b->getNItems());
    });

    qt->clearNode();
    for (AxialHitQuadTreeProcessor::ItemType* hit : nodesWithPossibleCands.front()->getItemsVector())
    {
      qt->insertItem(hit);
    }

    for (AxialHitQuadTreeProcessor::QuadTree* node : nodesWithPossibleCands)
    {
      delete node;
    }
  };



  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations

  AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdCandidateProcessingFinal = [&](const AxialHitQuadTreeProcessor::ReturnList &
  hits, AxialHitQuadTreeProcessor::QuadTree * qt) -> void {

    for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItemsVector())
    {
      hit->setUsedFlag(false);
      //hit->setUsedFlag(TrackHit::c_notUsed);
    }
    lmdAdvancedProcessing(hits, qt);

    std::vector<QuadTreeHitWrapper*> candidateHits;

    for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItemsVector())
    {
      hit->setUsedFlag(false);
//      hit->setUsedFlag(TrackHit::c_notUsed);
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
      if (rThreshold > ranges.second.second) rThreshold = ranges.second.second;
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


void CDCLegendreTrackingModule::postprocessSingleNode(std::vector<TrackFindingCDC::QuadTreeHitWrapper*>& candidateHits,
                                                      bool __attribute__((unused)) increaseThreshold, AxialHitQuadTreeProcessor::QuadTree* __attribute__((unused)) qt)
{

  for (QuadTreeHitWrapper* hit : candidateHits) {
    hit->setUsedFlag(false);// setHitUsage(TrackHit::c_notUsed);
  }

  m_trackProcessor.createCDCTrackCandidates(candidateHits);
  return;

}

void CDCLegendreTrackingModule::postprocessTracks()
{
  if (m_deleteHitsInTheEnd) {
    m_cdcLegendreTrackProcessor.deleteHitsOfAllBadTracks();
  }

  m_cdcLegendreTrackProcessor.fitAllTracks();



  if (m_mergeTracksInTheEnd) {
    m_cdcLegendreTrackProcessor.mergeAllTracks();
  }

  if (m_appendHitsInTheEnd) {
    m_cdcLegendreTrackProcessor.appendHitsOfAllTracks();
  }
}

void CDCLegendreTrackingModule::terminate()
{
  B2INFO("Tracks in runs:");
  B2INFO("	run 1: " << m_track1Run);
  B2INFO("	run 2: " << m_track2Run);
  B2INFO("	run 3: " << m_track3Run);

  delete m_cdcLegendreTrackDrawer;
}

void CDCLegendreTrackingModule::clearVectors()
{
  m_cdcLegendreTrackProcessor.clearVectors();
  m_trackProcessor.clearVectors();
}
