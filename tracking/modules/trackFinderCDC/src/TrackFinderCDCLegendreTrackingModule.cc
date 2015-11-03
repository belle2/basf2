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

#include <tracking/trackFindingCDC/legendre/QuadTreeCandidateFinder.h>

#include <tracking/trackFindingCDC/legendre/TrackQuality.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessorWithNewReferencePoint.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <tracking/trackFindingCDC/legendre/QuadTreeNodeProcessor.h>


#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtreetools/QuadTreePassCounter.h>

#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>

#include <cdc/dataobjects/CDCHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(TrackFinderCDCLegendreTracking);

TrackFinderCDCLegendreTrackingModule::TrackFinderCDCLegendreTrackingModule() :
  TrackFinderCDCBaseModule()
{
  setDescription("Performs the pattern recognition in the CDC with the conformal finder:"
                 "digitized CDCHits are combined to track candidates (genfit::TrackCand)");

  addParam("maxLevel",
           m_param_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm",
           12);

  addParam("doEarlyMerging",
           m_param_doEarlyMerging,
           "Set whether merging of track should be performed after each pass candidate finding; has impact on CPU time",
           false);
}

void TrackFinderCDCLegendreTrackingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  startNewEvent();
  findTracks();
  outputObjects(tracks);
  clearVectors();
}

void TrackFinderCDCLegendreTrackingModule::startNewEvent()
{
  B2DEBUG(100, "**********   CDCTrackingModule  ************");

  B2DEBUG(100, "Initializing hits");
  ConformalCDCWireHitCreator::copyHitsFromTopology(m_conformalCDCWireHitList);
}

void TrackFinderCDCLegendreTrackingModule::findTracks()
{
  TrackMerger trackMerger;
  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  QuadTreePassCounter quadTreePassCounter;

  // Here starts iteration over finding passes -- in each pass slightly different conditions of track finding applied
  do {
    HitProcessor::resetMaskedHits(m_trackProcessor.getCDCTrackListTmp(), m_conformalCDCWireHitList);

    // Create object which holds and generates parameters
    QuadTreeParameters quadTreeParameters(m_param_maxLevel, quadTreePassCounter.getPass());

    //Create quadtree processot
    AxialHitQuadTreeProcessor qtProcessor = quadTreeParameters.constructQTProcessor();

    //Prepare vector of QuadTreeHitWrapper* to provide it to the qt processor
    std::vector<ConformalCDCWireHit*> hitsVector = ConformalCDCWireHitCreator::createConformalCDCWireHitListForQT(
                                                     m_conformalCDCWireHitList);
    qtProcessor.provideItemsSet(hitsVector);
    //  qtProcessor.seedQuadTree(4, symmetricalKappa);

    // Create object which contains interface between quadtree processor and track processor (module)
    QuadTreeNodeProcessor quadTreeNodeProcessor(m_trackProcessor, qtProcessor, quadTreeParameters.getPrecisionFunction());

    // Object which operates with AxialHitQuadTreeProcessor and QuadTreeNodeProcessor and starts quadtree search
    QuadTreeCandidateFinder quadTreeCandidateFinder;

    int nCandsAdded = m_trackProcessor.getCDCTrackListTmp().getCDCTracks().size();

    // Interface
    AxialHitQuadTreeProcessor::CandidateProcessorLambda lambdaInterface = quadTreeNodeProcessor.getLambdaInterface(
          m_conformalCDCWireHitList, m_trackProcessor.getCDCTrackListTmp());

    // Start candidate finding
    quadTreeCandidateFinder.doTreeTrackFinding(lambdaInterface, quadTreeParameters, qtProcessor);

    // Assign new hits to the tracks
    m_trackProcessor.assignNewHits(m_conformalCDCWireHitList, m_trackProcessor.getCDCTrackListTmp());

    // Check p-value of the tracks
    m_trackProcessor.checkTrackProb(m_trackProcessor.getCDCTrackListTmp());

    // Try to merge tracks
    if (m_param_doEarlyMerging)trackMerger.doTracksMerging(m_trackProcessor.getCDCTrackListTmp(), m_conformalCDCWireHitList);

    nCandsAdded = m_trackProcessor.getCDCTrackListTmp().getCDCTracks().size() - nCandsAdded;

    // Change to the next pass
    if (quadTreePassCounter.getPass() != LegendreFindingPass::FullRange) {
      quadTreePassCounter.nextPass();
    }
    if ((quadTreePassCounter.getPass() == LegendreFindingPass::FullRange) && (nCandsAdded == 0)) {
      quadTreePassCounter.nextPass();
    }

  } while (quadTreePassCounter.getPass() != LegendreFindingPass::End);


  // Check quality of the track basing on holes on the trajectory;
  // if holes exsist then track is splitted
  m_trackProcessor.getCDCTrackListTmp().doForAllTracks([&](CDCTrack & track) {
    if (track.size() > 3) {
      TrackQuality trackQuality(track);
      HitProcessor::splitBack2BackTrack(track);

      trackQualityTools.normalizeTrack(track);
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

      m_trackProcessor.addCandidateWithHits(hitsToSplit, m_conformalCDCWireHitList, m_trackProcessor.getCDCTrackListTmp());

    }
//    TrackMergerNew::deleteAllMarkedHits(track);
  });

  // Update tracks before storing to DataStore
  m_trackProcessor.getCDCTrackListTmp().doForAllTracks([&](CDCTrack & track) {
    trackQualityTools.normalizeTrack(track);
  });

  // Remove bad tracks
  m_trackProcessor.checkTrackProb(m_trackProcessor.getCDCTrackListTmp());

  // Perform tracks merging
  trackMerger.doTracksMerging(m_trackProcessor.getCDCTrackListTmp(), m_conformalCDCWireHitList);

  // Assign new hits
  m_trackProcessor.assignNewHits(m_conformalCDCWireHitList, m_trackProcessor.getCDCTrackListTmp());

}

void TrackFinderCDCLegendreTrackingModule::outputObjects(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  tracks.reserve(tracks.size() + m_trackProcessor.getCDCTrackListTmp().getCDCTracks().size());

  m_trackProcessor.getCDCTrackListTmp().doForAllTracks([&](CDCTrack & track) {
    if (track.size() > 5) tracks.push_back(std::move(track));
  });
}

void TrackFinderCDCLegendreTrackingModule::clearVectors()
{
  m_conformalCDCWireHitList.clear();
  m_trackProcessor.getCDCTrackListTmp().clear();
}
