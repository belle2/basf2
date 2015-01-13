/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCCosmicsModule.h>

#include <framework/datastore/StoreArray.h>

//out type
#include "genfit/TrackCand.h"

// Event data objects
#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackFinderCDCCosmics);

TrackFinderCDCCosmicsModule::TrackFinderCDCCosmicsModule() :
  TrackFinderCDCBaseModule(),
  m_segmentWorker(false),
  m_segmentTripleTrackingWorker(false),
  m_segmentPairTrackingWorker(false)
{
  setDescription("Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.");
}

TrackFinderCDCCosmicsModule::~TrackFinderCDCCosmicsModule()
{
}

void TrackFinderCDCCosmicsModule::initialize()
{
  TrackFinderCDCBaseModule::initialize();

  m_segmentWorker.initialize();

  // m_segmentTripleTrackingWorker.initialize();
  m_segmentPairTrackingWorker.initialize();

}

void TrackFinderCDCCosmicsModule::beginRun()
{
  TrackFinderCDCBaseModule::beginRun();
}

void TrackFinderCDCCosmicsModule::event()
{
  B2DEBUG(100, "########## TrackFinderCDCCosmics begin ##########");

  // Load the hits from the DataStore into the CDCWireHitTopology.
  TrackFinderCDCBaseModule::event();

  // Stage one
  // Build the segments
  m_recoSegments.clear();
  m_segmentWorker.generate(m_recoSegments);
  B2DEBUG(100, "Received " << m_recoSegments.size() << " RecoSegments from worker");

  // Stage two
  // Build the gfTracks
  StoreArray < genfit::TrackCand > storedGFTrackCands(m_param_gfTrackCandsStoreArrayName);

  // m_segmentTripleTrackingWorker.apply(m_recoSegments, storedGFTrackCands);
  m_segmentPairTrackingWorker.apply(m_recoSegments, storedGFTrackCands);

  B2DEBUG(100, "########## TrackFinderCDCCosmics end ############");
}

void TrackFinderCDCCosmicsModule::endRun()
{
  TrackFinderCDCBaseModule::endRun();
}

void TrackFinderCDCCosmicsModule::terminate()
{
  m_segmentPairTrackingWorker.terminate();
  // m_segmentTripleTrackingWorker.terminate();

  m_segmentWorker.terminate();

  TrackFinderCDCBaseModule::terminate();
}

