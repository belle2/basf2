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

#include <tracking/trackFindingCDC/legendre/quadtreetools/QuadTreePassCounter.h>

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeCandidateFinder.h>

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeNodeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>

#include <tracking/trackFindingCDC/processing/TrackProcessor.h>
#include <tracking/trackFindingCDC/processing/HitProcessor.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(TrackFinderCDCLegendreTracking);

TrackFinderCDCLegendreTrackingModule::TrackFinderCDCLegendreTrackingModule() :
  Module()
{
  setDescription("Performs the pattern recognition in the CDC with the legendre hough finder");

  addParam("maxLevel",
           m_param_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm",
           12);

  addParam("TracksStoreObjName",
           m_param_tracksStoreObjName,
           "Name of the output StoreObjPtr of the tracks generated within this module.",
           m_param_tracksStoreObjName);

  setPropertyFlags(c_ParallelProcessingCertified bitor c_TerminateInAllProcesses);
}

void TrackFinderCDCLegendreTrackingModule::initialize()
{
  StoreWrappedObjPtr<std::vector<CDCWireHit> > storedWireHits("CDCWireHitVector");
  storedWireHits.isRequired();

  StoreWrappedObjPtr<std::vector<CDCTrack>> storedTracks(m_param_tracksStoreObjName);
  storedTracks.registerInDataStore();

  B2ASSERT("Maximal level of QuadTree search is setted to be greater than lookuptable grid level! ",
           m_param_maxLevel <= PrecisionUtil::getLookupGridLevel());

  Super::initialize();
}

void TrackFinderCDCLegendreTrackingModule::event()
{
  // Now aquire the store vector
  StoreWrappedObjPtr<std::vector<CDCTrack>> storedTracks(m_param_tracksStoreObjName);
  storedTracks.create();

  // We now let the generate-method fill or update the outputTracks
  std::vector<CDCTrack>& tracks = *storedTracks;

  startNewEvent();
  findTracks();
  outputObjects(tracks);
  clearVectors();
}

void TrackFinderCDCLegendreTrackingModule::startNewEvent()
{
  B2DEBUG(100, "**********   CDCTrackingModule  ************");

  B2DEBUG(100, "Initializing hits");
  StoreWrappedObjPtr<std::vector<CDCWireHit> > storedWireHits("CDCWireHitVector");
  const std::vector<CDCWireHit>& wireHits = *storedWireHits;

  for (const CDCWireHit& wireHit : wireHits) {
    // Skip taken hits
    if (wireHit.getAutomatonCell().hasTakenFlag()) continue;

    // Only select axial hits for legendre tracking
    if (not wireHit.isAxial()) continue;

    m_allAxialWireHits.push_back(&wireHit);
  }

  B2DEBUG(90, "Number of hits to be used by legendre track finder: " << m_allAxialWireHits.size() << " axial.");
}

void TrackFinderCDCLegendreTrackingModule::findTracks()
{
  // First legendre pass
  applyPass(LegendreFindingPass::NonCurlers);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(m_allAxialWireHits, m_tracks);

  // Second legendre pass
  applyPass(LegendreFindingPass::NonCurlersWithIncreasingThreshold);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(m_allAxialWireHits, m_tracks);

  // Iterate the last finding pass until no track is found anymore

  // Loop counter to guard against infinit loop
  for (int iPass = 0; iPass < 10; ++iPass) {
    int nCandsAdded = m_tracks.size();

    // Second legendre pass
    applyPass(LegendreFindingPass::FullRange);

    // Assign new hits to the tracks
    m_axialTrackHitMigrator.apply(m_allAxialWireHits, m_tracks);

    nCandsAdded = m_tracks.size() - nCandsAdded;

    if (iPass == 9) B2WARNING("Reached maximal number of legendre search passes");
    if (nCandsAdded == 0) break;
  }

  // Merge found tracks
  m_axialTrackMerger.apply(m_tracks, m_allAxialWireHits);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(m_allAxialWireHits, m_tracks);
}

void TrackFinderCDCLegendreTrackingModule::applyPass(LegendreFindingPass pass)
{

  HitProcessor::resetMaskedHits(m_tracks, m_allAxialWireHits);

  // Create object which holds and generates parameters
  QuadTreeParameters quadTreeParameters(m_param_maxLevel, pass);

  //Create quadtree processot
  AxialHitQuadTreeProcessor qtProcessor = quadTreeParameters.constructQTProcessor();

  //Prepare vector of QuadTreeHitWrapper* to provide it to the qt processor
  std::vector<const CDCWireHit*> hitsVector;
  for (const CDCWireHit* wireHit : m_allAxialWireHits) {
    if ((*wireHit)->hasTakenFlag()) continue;
    hitsVector.push_back(wireHit);
  }

  qtProcessor.provideItemsSet(hitsVector);
  //  qtProcessor.seedQuadTree(4, symmetricalKappa);

  // Create object which contains interface between quadtree processor and track processor (module)
  QuadTreeNodeProcessor quadTreeNodeProcessor(qtProcessor, quadTreeParameters.getPrecisionFunction());

  // Object which operates with AxialHitQuadTreeProcessor and QuadTreeNodeProcessor and starts quadtree search
  QuadTreeCandidateFinder quadTreeCandidateFinder;

  // Interface
  AxialHitQuadTreeProcessor::CandidateProcessorLambda lambdaInterface =
    quadTreeNodeProcessor.getLambdaInterface(m_allAxialWireHits, m_tracks);

  // Start candidate finding
  quadTreeCandidateFinder.doTreeTrackFinding(lambdaInterface, quadTreeParameters, qtProcessor);
}


void TrackFinderCDCLegendreTrackingModule::outputObjects(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  TrackProcessor::deleteShortTracks(m_tracks);
  tracks.insert(tracks.end(), m_tracks.begin(), m_tracks.end());
}

void TrackFinderCDCLegendreTrackingModule::clearVectors()
{
  m_allAxialWireHits.clear();
  m_tracks.clear();
}
