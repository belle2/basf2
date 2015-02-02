/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCAutomatonModule.h>

#include <framework/datastore/StoreArray.h>

//out type
#include "genfit/TrackCand.h"

// Event data objects
#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackFinderCDCAutomaton);

TrackFinderCDCAutomatonModule::TrackFinderCDCAutomatonModule() :
  SegmentFinderCDCFacetAutomatonModule(c_Symmetric),
  m_segmentTripleTrackingWorker(false),
  m_segmentPairTrackingWorker(false)
{
  setDescription("Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.");
}

void TrackFinderCDCAutomatonModule::initialize()
{
  SegmentFinderCDCFacetAutomatonModule::initialize();

  // m_segmentTripleTrackingWorker.initialize();
  m_segmentPairTrackingWorker.initialize();
}

void TrackFinderCDCAutomatonModule::event()
{
  B2DEBUG(100, "########## TrackFinderCDCAutomaton begin ##########");
  // Load the hits from the DataStore into the CDCWireHitTopology.

  // Stage one
  SegmentFinderCDCFacetAutomatonModule::event();

  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > storedRecoSegments(m_param_segmentsStoreObjName);
  std::vector<CDCRecoSegment2D>& recoSegments = *storedRecoSegments;
  B2DEBUG(100, "Received " << recoSegments.size() << " RecoSegments from worker");

  // Stage two
  // Build the gfTracks
  StoreArray < genfit::TrackCand > storedGFTrackCands(m_param_gfTrackCandsStoreArrayName);

  // m_segmentTripleTrackingWorker.apply(m_recoSegments, storedGFTrackCands);
  m_segmentPairTrackingWorker.apply(recoSegments, storedGFTrackCands);

  B2DEBUG(100, "########## TrackFinderCDCAutomaton end ############");
}

void TrackFinderCDCAutomatonModule::terminate()
{
  m_segmentPairTrackingWorker.terminate();
  // m_segmentTripleTrackingWorker.terminate();

  SegmentFinderCDCFacetAutomatonModule::terminate();
}

