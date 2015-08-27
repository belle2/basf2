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
  TrackFinderCDCBaseModule(), m_cdcLegendreTrackProcessor(), m_cdcLegendreTrackDrawer(nullptr)
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

  doTreeTrackFinding(50, 0.07, true);
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


  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  std::function< double(double) > lmdFunctionResQTD0 = [&](double d0) -> double {
    double res;
    res = 0.000127297 + 0.0000281511 * d0 + 0.000288808 * d0 * d0 - 0.0000541841 * d0 * d0 * d0 + 0.00000316108  * d0 * d0 * d0 * d0;

    B2DEBUG(100, "origin: res = " << res << "; d0 = " << d0);
    return res;
  };



  std::function< double(double) > lmdFunctionResQTOrigin = [&](double r_qt) -> double {
    double res;
    //TODO: bug is here!
    if ((TrackCandidate::convertRhoToPt(fabs(r_qt)) > 3.) && (r_qt != 0))
      r_qt = fabs(TrackCandidate::convertPtToRho(3.)) * r_qt / fabs(r_qt);

    if (r_qt != 0)
      res = exp(-16.1987 * TrackCandidate::convertRhoToPt(fabs(r_qt)) - 5.96206)
      + 0.000190872 - 0.0000739319 * TrackCandidate::convertRhoToPt(fabs(r_qt));

    else
      res = 0.00005;

    B2DEBUG(100, "origin: res = " << res << "; r = " << r_qt);
    return res;
  };


  std::function< double(double) > lmdFunctionResQTNonOrigin = [&](double r_qt) -> double {
    double res;
    if ((TrackCandidate::convertRhoToPt(fabs(r_qt)) > 3.) && (r_qt != 0))
      r_qt = fabs(TrackCandidate::convertPtToRho(3.)) * r_qt / fabs(r_qt);

    if (r_qt != 0)
      if (TrackCandidate::convertRhoToPt(fabs(r_qt)) < 0.36)
        res = exp(-0.356965 - 0.00186066 * TrackCandidate::convertRhoToPt(fabs(r_qt))) - 0.697526;
      else
        res = exp(-0.357335 + 0.000438872 * TrackCandidate::convertRhoToPt(fabs(r_qt))) - 0.697786;
    else
      res = 0.00005;
    B2DEBUG(100, "non origin: res = " << res << "; r = " << r_qt);
    return res;
  };
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>.

  std::function< double(double) > currentFunct;

  if (not increaseThreshold) currentFunct = lmdFunctionResQTOrigin;
  else currentFunct = lmdFunctionResQTNonOrigin;



  AxialHitQuadTreeProcessor::ChildRanges ranges(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2, 15)),
                                                AxialHitQuadTreeProcessor::rangeY(m_rMin, m_rMax));
  std::vector<AxialHitQuadTreeProcessor::ReturnList> candidates;


  AxialHitQuadTreeProcessor qtProcessor(maxLevel, ranges, currentFunct, (increaseThreshold and (rThreshold < 0.15)));
  qtProcessor.provideItemsSet(hitsVector);


  AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdAdvancedProcessing = [&](const AxialHitQuadTreeProcessor::ReturnList &
  hits, AxialHitQuadTreeProcessor::QuadTree * qt) -> void {
    double rRes = currentFunct(qt->getYMean());
    int thetaRes = abs(m_nbinsTheta* rRes / 0.3);

    int meanTheta = qt->getXMean();
    double meanR = qt->getYMean();


    std::vector< AxialHitQuadTreeProcessor::QuadTree*> nodesWithPossibleCands;

    for (int ii = -1; ii <= 1; ii = +2)
    {
      for (int jj = -1; jj <= 1; jj = +2) {
        AxialHitQuadTreeProcessor::ChildRanges rangesTmp(AxialHitQuadTreeProcessor::rangeX(meanTheta + static_cast<int>((
          ii - 1)*thetaRes / 2),
        meanTheta + static_cast<int>((ii + 1)*thetaRes / 2)),
        AxialHitQuadTreeProcessor::rangeY(meanR + (jj - 1)*rRes / 2.,
        meanR + (jj + 1)*rRes / 2.));

        nodesWithPossibleCands.push_back(qtProcessor.createSingleNode(rangesTmp));
      }
    }

    for (int ii = -1; ii <= 1; ii++)
    {
      for (int jj = -1; jj <= 1; jj++) {
        AxialHitQuadTreeProcessor::ChildRanges rangesTmp(AxialHitQuadTreeProcessor::rangeX(meanTheta + static_cast<int>((
                                                           2 * ii - 1)*thetaRes / 2),
                                                         meanTheta + static_cast<int>((2 * ii + 1)*thetaRes / 2)),
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

//    std::sort( qt->getItemsVector().begin(), qt->getItemsVector().end() );
//    qt->getItemsVector().erase( std::unique( qt->getItemsVector().begin(), qt->getItemsVector().end() ), qt->getItemsVector().end() );


//    std::set<AxialHitQuadTreeProcessor::ItemType*> s;
//    unsigned size = qt->getItemsVector().size();
//    for( unsigned i = 0; i < size; ++i ) s.insert( qt->getItemsVector()[i] );
//    qt->getItemsVector().assign( s.begin(), s.end() );


    for (AxialHitQuadTreeProcessor::QuadTree* node : nodesWithPossibleCands)
    {
      delete node;
    }
  };



  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations

  AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdCandidateProcessingFinal = [&](const AxialHitQuadTreeProcessor::ReturnList&
  /*hits*/, AxialHitQuadTreeProcessor::QuadTree * qt) -> void {

    for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItemsVector())
    {
      hit->setUsedFlag(TrackHit::c_notUsed);
    }
    lmdAdvancedProcessing(hits, qt);

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
      if (rThreshold > m_rMax) rThreshold = m_rMax;
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

  double levelPrecision = 10;//.5 - 0.24 * exp(-4.13118 * TrackCandidate::convertRhoToPt(fabs(track_par.second)) + 2.74);

  double precision_r, precision_theta;
  precision_theta = 3.1415 / (pow(2., levelPrecision + 1));
  precision_r = 0.15 / (pow(2., levelPrecision));

  double B = -1.*track_par.second * (ref_point.first) - (1 - track_par.second * D) * cos(track_par.first);
  double C = track_par.second * (ref_point.second) + (1 - track_par.second * D) * sin(track_par.first);
  double theta_new = atan2(B, C) + boost::math::constants::pi<double>() / 2.;
  if (theta_new < 0) theta_new += boost::math::constants::pi<double>();


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

//  if(newAssignedHits.size()>1)B2INFO("hits added: " << newAssignedHits.size());

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


  m_cdcLegendreTrackProcessor.deleteBadHitsOfOneTrack(trackCandidate);


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
  delete m_cdcLegendreTrackDrawer;
}

void CDCLegendreTrackingModule::clearVectors()
{
  m_cdcLegendreTrackProcessor.clearVectors();
}
