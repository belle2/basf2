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

REG_MODULE(TrackFinderCDCLegendreTracking);

TrackFinderCDCLegendreTrackingModule::TrackFinderCDCLegendreTrackingModule()
  : Module()
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
  std::vector<CDCTrack>& tracks = *storedTracks;


  StoreWrappedObjPtr<std::vector<CDCWireHit> > storedWireHits("CDCWireHitVector");
  const std::vector<CDCWireHit>& wireHits = *storedWireHits;

  // We now let the generate-method fill or update the outputTracks
  apply(wireHits, tracks);
}


void TrackFinderCDCLegendreTrackingModule::apply(const std::vector<CDCWireHit>& wireHits,
                                                 std::vector<CDCTrack>& tracks)
{
  B2DEBUG(100, "**********   CDCTrackingModule  ************");

  // Acquire the axial hits
  std::vector<const CDCWireHit*> axialWireHits;
  axialWireHits.reserve(wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    wireHit->unsetTemporaryFlags();
    wireHit->unsetMaskedFlag();
    if (not wireHit.isAxial()) continue;
    if (wireHit->hasBackgroundFlag()) continue;
    axialWireHits.emplace_back(&wireHit);
  }

  // First legendre pass
  applyPass(LegendreFindingPass::NonCurlers, axialWireHits, tracks);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(axialWireHits, tracks);

  // Second legendre pass
  applyPass(LegendreFindingPass::NonCurlersWithIncreasingThreshold, axialWireHits, tracks);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(axialWireHits, tracks);

  // Iterate the last finding pass until no track is found anymore

  // Loop counter to guard against infinit loop
  for (int iPass = 0; iPass < 20; ++iPass) {
    int nCandsAdded = tracks.size();

    // Second legendre pass
    applyPass(LegendreFindingPass::FullRange, axialWireHits, tracks);

    // Assign new hits to the tracks
    m_axialTrackHitMigrator.apply(axialWireHits, tracks);

    nCandsAdded = tracks.size() - nCandsAdded;

    if (iPass == 19) B2WARNING("Reached maximal number of legendre search passes");
    if (nCandsAdded == 0) break;
  }

  // Merge found tracks
  m_axialTrackMerger.apply(tracks, axialWireHits);

  // Assign new hits to the tracks
  m_axialTrackHitMigrator.apply(axialWireHits, tracks);

  TrackProcessor::deleteShortTracks(tracks);
}

void TrackFinderCDCLegendreTrackingModule::applyPass(LegendreFindingPass pass,
                                                     const std::vector<const CDCWireHit*>& axialWireHits,
                                                     std::vector<CDCTrack>& tracks)
{

  HitProcessor::resetMaskedHits(tracks, axialWireHits);

  // Create object which holds and generates parameters
  QuadTreeParameters quadTreeParameters(m_param_maxLevel, pass);

  //Create quadtree processot
  AxialHitQuadTreeProcessor qtProcessor = quadTreeParameters.constructQTProcessor();

  //Prepare vector of QuadTreeHitWrapper* to provide it to the qt processor
  std::vector<const CDCWireHit*> hitsVector;
  for (const CDCWireHit* wireHit : axialWireHits) {
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
    quadTreeNodeProcessor.getLambdaInterface(axialWireHits, tracks);

  // Start candidate finding
  quadTreeCandidateFinder.doTreeTrackFinding(lambdaInterface, quadTreeParameters, qtProcessor);
}
