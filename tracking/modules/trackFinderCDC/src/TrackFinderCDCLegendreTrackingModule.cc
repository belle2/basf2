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
#include <tracking/trackFindingCDC/legendre/TrackMerger.h>
#include <tracking/trackFindingCDC/legendre/ConformalExtension.h>

#include <tracking/trackFindingCDC/legendre/TrackFitter.h>
#include <tracking/trackFindingCDC/legendre/TrackCreator.h>
#include <tracking/trackFindingCDC/legendre/QuadTreeCandidateFinder.h>

#include <tracking/trackFindingCDC/legendre/TrackQuality.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessorWithNewReferencePoint.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <tracking/trackFindingCDC/legendre/QuadTreeNodeProcessor.h>

#include <cdc/dataobjects/CDCHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(CDCLegendreTracking);

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  TrackFinderCDCBaseModule(), m_QuadTreePassCounter(), m_hitFactory(), m_trackHolder(), m_trackProcessor(m_hitFactory, m_trackHolder)
{
  setDescription(
    "Performs the pattern recognition in the CDC with the conformal finder: digitized CDCHits are combined to track candidates (genfit::TrackCand)");

  addParam("MaxLevel", m_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm", 12);

  addParam("DoEarlyMerging", m_doEarlyMerging,
           "Set whether merging of track should be performed after each pass candidate finding; has impact on CPU time", false);

}

void CDCLegendreTrackingModule::initialize()
{
  TrackFinderCDCBaseModule::initialize();

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

  B2DEBUG(100, "Initializing hits");
  m_hitFactory.initializeQuadTreeHitWrappers();


}

void CDCLegendreTrackingModule::findTracks()
{

  TrackMerger trackMerger(m_trackProcessor);

  m_QuadTreePassCounter.reset();

  // Here starts iteration over finding passes -- in each pass slightly different conditions of track finding applied
  do {
    m_trackProcessor.resetMaskedHits();

    // Create object which holds and generates parameters
    QuadTreeParameters quadTreeParameters(m_maxLevel, m_QuadTreePassCounter.getPass());

    //Create quadtree processot
    AxialHitQuadTreeProcessor qtProcessor = quadTreeParameters.constructQTProcessor();

    //Prepare vector of QuadTreeHitWrapper* to provide it to the qt processor
    std::vector<QuadTreeHitWrapper*> hitsVector = m_hitFactory.createQuadTreeHitWrappersForQT();
    qtProcessor.provideItemsSet(hitsVector);
    //  qtProcessor.seedQuadTree(4, symmetricalKappa);

    // Create object which contains interface between quadtree processor and track processor (module)
    QuadTreeNodeProcessor quadTreeNodeProcessor(m_hitFactory, m_trackProcessor, qtProcessor, quadTreeParameters.getPrecisionFunction());

    // Object which operates with AxialHitQuadTreeProcessor and QuadTreeNodeProcessor and starts quadtree search
    QuadTreeCandidateFinder quadTreeCandidateFinder;

    int nCandsAdded = m_trackHolder.getCDCTracks().size();

    // Interface
    AxialHitQuadTreeProcessor::CandidateProcessorLambda lambdaInterface = quadTreeNodeProcessor.getLambdaInterface();

    // Start candidate finding
    quadTreeCandidateFinder.doTreeTrackFinding(lambdaInterface, quadTreeParameters, qtProcessor);

    // Assign new hits to the tracks
    m_trackProcessor.assignNewHits();

    // Check p-value of the tracks
    m_trackProcessor.checkTrackProb();

    // Try to merge tracks
    if (m_doEarlyMerging)trackMerger.doTracksMerging(m_trackHolder.getCDCTracks());

    nCandsAdded = m_trackHolder.getCDCTracks().size() - nCandsAdded;

    // Change to the next pass
    if (m_QuadTreePassCounter.getPass() != LegendreFindingPass::FullRange) {
      m_QuadTreePassCounter.nextPass();
    }
    if ((m_QuadTreePassCounter.getPass() == LegendreFindingPass::FullRange) && (nCandsAdded == 0)) {
      m_QuadTreePassCounter.nextPass();
    }

  } while (m_QuadTreePassCounter.getPass() != LegendreFindingPass::End);


  // Check quality of the track basing on holes on the trajectory;
  // if holes exsist then track is splitted
  m_trackHolder.doForAllTracks([&](CDCTrack & track) {
//   return;
    if (track.size() > 3) {
      TrackQuality trackQuality(track);
      HitProcessor::splitBack2BackTrack(track);
      m_trackProcessor.updateTrack(track);
      std::vector<const CDCWireHit*> hitsToSplit;

      for (CDCRecoHit3D& hit : track) {
        if (hit.getWireHit().getAutomatonCell().hasMaskedFlag()) {
          hitsToSplit.push_back(&(hit.getWireHit()));
        }
      }

      HitProcessor::deleteAllMarkedHits(track);

      for (const CDCWireHit* hit : hitsToSplit) {
        hit->getAutomatonCell().setMaskedFlag(false);
        hit->getAutomatonCell().setTakenFlag(false);
      }

      m_trackProcessor.createCandidate(hitsToSplit);

    }
//    TrackMergerNew::deleteAllMarkedHits(track);
  });

  // Update tracks before storing to DataStore
  for (CDCTrack& track : m_trackHolder.getCDCTracks()) {
    m_trackProcessor.updateTrack(track);
  }

  // Remove bad tracks
  m_trackProcessor.checkTrackProb();

  // Perform tracks merging
  trackMerger.doTracksMerging(m_trackHolder.getCDCTracks());

  // Assign new hits
  m_trackProcessor.assignNewHits();

}

void CDCLegendreTrackingModule::outputObjects(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  //create GenFit Track candidates
  TrackCreator trackCreator;
  trackCreator.createCDCTracks(tracks, m_trackHolder.getCDCTracks());
}

void CDCLegendreTrackingModule::terminate()
{

}

void CDCLegendreTrackingModule::clearVectors()
{
  // Clear tracks and hits vectors
  m_trackProcessor.clearVectors();
}
