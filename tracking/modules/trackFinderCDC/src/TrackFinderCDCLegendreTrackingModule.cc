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

#include <genfit/TrackCand.h>
#include <cdc/dataobjects/CDCHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(CDCLegendreTracking)

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  TrackFinderCDCBaseModule(), m_cdcLegendreQuadTree(0, m_nbinsTheta, m_rMin, m_rMax, 0, nullptr),
  m_cdcLegendreTrackProcessor(),
  m_cdcLegendreFastHough(nullptr), m_cdcLegendreTrackDrawer(nullptr)
{
  setDescription(
    "Performs the pattern recognition in the CDC with the conformal finder: digitized CDCHits are combined to track candidates (genfit::TrackCand)");

  addParam("Threshold", m_param_threshold, "Threshold for peak finder", static_cast<unsigned int>(10));

  addParam("StepScale", m_param_stepScale, "Scale size for Stepped Hough", 0.75);

  addParam("MaxLevel", m_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm", 12);

  addParam("ReconstructCurler", m_reconstructCurler,
           "Flag, whether curlers should be reconstructed", false);

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

  addParam("EnableBetterPerformance", m_increasePerformance,
           "Enable slightly better performance of the module by cost of CPU time", false);
}

void CDCLegendreTrackingModule::initialize()
{
  TrackFinderCDCBaseModule::initialize();

  // initialize track drawer
  m_cdcLegendreTrackDrawer = new TrackDrawer(m_drawCandInfo, m_drawCandidates, m_batchMode);
  m_cdcLegendreTrackDrawer->initialize();

  // initialize fast hough
  m_cdcLegendreFastHough = new FastHough(m_reconstructCurler, m_maxLevel, m_nbinsTheta, m_rMax);

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
  B2INFO("**********   CDCTrackingModule  ************");
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
  for (TrackCandidate* cand : m_cdcLegendreTrackProcessor.getTrackList()) {
    B2DEBUG(100, "R value: " << cand->getR() << "; theta: " << cand->getTheta() << "; radius: " << cand->getRadius() << "; phi: "
            << cand->getPhi() << "; charge: " << cand->getChargeSign() << "; Xc = " << cand->getXc() << "; Yc = " << cand->getYc() <<
            "Hitsize: " << cand->getNHits());
  }
  //create GenFit Track candidates
  m_cdcLegendreTrackProcessor.createCDCTrackCandidates(tracks);
  m_cdcLegendreTrackDrawer->finalizeFile();
}


void CDCLegendreTrackingModule::processNodes(std::vector<QuadTreeLegendre*>& listOfCandidates,
                                             QuadTreeLegendre::CandidateProcessorLambda& lmdFunct, unsigned int limit)
{
  if (listOfCandidates.size() == 0) return;
  QuadTreeProcessor qtProcessor(m_maxLevel);

  do {
    //sort nodes by number of hits
    std::sort(listOfCandidates.begin(), listOfCandidates.end(), [](const QuadTreeLegendre * a, const QuadTreeLegendre * b) {
      return static_cast <bool>(a->getNItems() > b->getNItems());
    });

    lmdFunct(listOfCandidates.front());

    for (QuadTreeLegendre* qt : listOfCandidates) qt->cleanUpItems(qtProcessor);

    listOfCandidates.erase(std::remove_if(listOfCandidates.begin(), listOfCandidates.end(),
    [&](QuadTreeLegendre * qt) { return qt->getNItems() < limit; }),
    listOfCandidates.end());

  } while (listOfCandidates.size() > 0);
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
    for (TrackHit* hit : m_cdcLegendreTrackProcessor.getAxialHitsList()) {
      if (hit->getHitUsage() == TrackHit::bad) hit->setHitUsage(TrackHit::not_used);
    }
  }

  std::set<TrackHit*> hits_set = m_cdcLegendreTrackProcessor.createHitSet();

  QuadTreeProcessor qtProcessor(maxLevel);
  m_cdcLegendreQuadTree.clearTree();
  m_cdcLegendreQuadTree.provideItemsSet(qtProcessor, hits_set);
  int nSteps = 0;

  std::vector<QuadTreeLegendre*> listOfCandidates;


  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations

  QuadTreeLegendre::CandidateProcessorLambda lmdCandidateProcessingFinal = [&](QuadTreeLegendre * qt) -> void {

    std::pair<double, double> track_par;
    std::pair<double, double> ref_point;
    TrackFitter cdcLegendreTrackFitter;
    double chi2 = cdcLegendreTrackFitter.fitTrackCandidateFast(qt->getItemsVector(), track_par, ref_point);
    double D = pow(ref_point.first * ref_point.first + ref_point.second * ref_point.second, 0.5);

    if (not increaseThreshold)
    {
      if (TrackCandidate::convertRhoToPt(fabs(qt->getYMean())) > 0.7 && (D > 0.4)) {
        for (TrackHit* hit : qt->getItemsVector()) {
          hit->setHitUsage(TrackHit::bad);
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

    QuadTreeTemplate<float, float, TrackFindingCDC::TrackHit> qtTemp(static_cast<float>(theta_new - precision_theta), static_cast<float>(theta_new + precision_theta),
        static_cast<float>(track_par.second - precision_r), static_cast<float>(track_par.second + precision_r), 0, nullptr);

    qtProcessor.fillNodeWithRespectToGivenPoint(&qtTemp, m_cdcLegendreTrackProcessor.getAxialHitsList(), ref_point);

    std::vector<QuadTreeLegendre*> nodeList;
    nodeList.push_back(qt);
//    nodeList.push_back(&qtTemp);


    std::vector<TrackHit*> trackHitsTemp;

    for (TrackHit* hit : qt->getItemsVector())
    {
      trackHitsTemp.push_back(hit);
    }
    for (TrackHit* hit : qtTemp.getItemsVector())
    {
      trackHitsTemp.push_back(hit);
    }
    double chi2New = cdcLegendreTrackFitter.fitTrackCandidateFast(trackHitsTemp, track_par, ref_point);

//    if(chi2New != chi2) B2INFO("Chi2 = " << chi2 << "; chi2New = " << chi2New);


    QuadTreeLegendre qtTempLegendre(qt->getXMin(), qt->getXMax(), qt->getYMin(), qt->getYMax(), 0, nullptr);
    if (chi2New * 2. < chi2)
    {
      for (TrackHit* hit : qtTemp.getItemsVector()) {
        qtTempLegendre.insertItem(hit);
      }
      if (qtTempLegendre.getNItems() != 0) nodeList.push_back(&qtTempLegendre);
    }


    TrackCandidate* trackCandidate = m_cdcLegendreTrackProcessor.createLegendreTrackCandidateFromQuadNodeList(nodeList);

    unsigned int numberOfUsedHits = 0;
    for (TrackHit* hit : hits_set)
    {
      if (hit->getHitUsage() == TrackHit::used_in_track)
        numberOfUsedHits++;
    }

    B2DEBUG(90, "Number of used hits: " << numberOfUsedHits)

    // Postprocessing of one track candidate
    m_cdcLegendreTrackProcessor.fitOneTrack(trackCandidate);

    if (m_deleteHitsWhileFinding)
    {
      m_cdcLegendreTrackProcessor.deleteBadHitsOfOneTrack(trackCandidate);
    }

    if (m_mergeTracksWhileFinding)
    {
      m_cdcLegendreTrackProcessor.mergeOneTrack(trackCandidate);
    }

    if (m_appendHitsWhileFinding)
    {
      m_cdcLegendreTrackProcessor.appendHitsOfAllTracks();
    }

    m_cdcLegendreTrackProcessor.deleteTracksWithASmallNumberOfHits();


  };

  QuadTreeLegendre::CandidateProcessorLambda lmdCandidateProcessing = [&](QuadTreeLegendre * qt) -> void {
    if (not m_increasePerformance)
    {
      lmdCandidateProcessingFinal(qt);
    } else {
      if (TrackCandidate::convertRhoToPt(fabs(qt->getYMean())) > 0.4) lmdCandidateProcessingFinal(qt);
      else listOfCandidates.push_back(qt);
    }
  };


  listOfCandidates.clear();
  //find high-pt tracks (not-curlers: diameter of the track higher than radius of CDC -- 2*Rtrk > rCDC => Rtrk < 2./rCDC, r(legendre) = 1/Rtrk =>  r(legendre) < 2./rCDC)
  qtProcessor.fillGivenTree(&m_cdcLegendreQuadTree, lmdCandidateProcessing, 50, 2. / rCDC);
  if (listOfCandidates.size() > 0) processNodes(listOfCandidates, lmdCandidateProcessingFinal, 50);

  listOfCandidates.clear();
  //find curlers with diameter higher than half of radius of CDC (see calculations above)
  qtProcessor.fillGivenTree(&m_cdcLegendreQuadTree, lmdCandidateProcessing, 70, 4. / rCDC);
  if (listOfCandidates.size() > 0) processNodes(listOfCandidates, lmdCandidateProcessingFinal, 70);


  // Start loop, where tracks are searched for
  do {
    listOfCandidates.clear();
    qtProcessor.fillGivenTree(&m_cdcLegendreQuadTree, lmdCandidateProcessing, limit, rThreshold);

    if (listOfCandidates.size() > 0) {
      processNodes(listOfCandidates, lmdCandidateProcessingFinal, limit);
    } else {

      limit = limit * m_param_stepScale;

      if (increaseThreshold) {
        rThreshold *= 2.;
        if (rThreshold > 0.15) rThreshold = 0.15;
      }
    }
    nSteps++;
    //perform search until found track has too few hits or threshold is too small and no tracks are found
  } while (limit >= m_param_threshold && hits_set.size() >= m_param_threshold);

  //sort tracks by value of curvature
  m_cdcLegendreTrackProcessor.getTrackList().sort([](const TrackCandidate * a, const TrackCandidate * b) {
    return static_cast <bool>(a->getRadius() > b->getRadius());
  });

  // ?????
  std::vector<TrackHit*> hits_vector; //temporary array;
  m_cdcLegendreTrackDrawer->finalizeROOTFile(hits_vector);

  B2DEBUG(90, "Number of steps in tree track finding: " << nSteps);
  B2DEBUG(90, "Threshold on number of hits: " << limit);
  B2DEBUG(90, "Threshold on r: " << rThreshold);
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
  delete m_cdcLegendreFastHough;
}

void CDCLegendreTrackingModule::clearVectors()
{
  m_cdcLegendreTrackProcessor.clearVectors();
}
