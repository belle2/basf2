/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCAutomatonDevModule.h>

#include <framework/datastore/StoreArray.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#ifdef TRACKFINDINGCDC_USE_MC_INFORMATION
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#endif

//out type
#include "genfit/TrackCand.h"

#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
#endif

#include <time.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


REG_MODULE(TrackFinderCDCAutomatonDev);

TrackFinderCDCAutomatonDevModule::TrackFinderCDCAutomatonDevModule() :
  TrackFinderCDCBaseModule(),
  m_segmentWorker(true),
  m_segmentTripleTrackingWorker(true),
  m_segmentPairTrackingWorker(true)
{
  setDescription("For development purposes of the cellular automaton track finding.");
  addParam("runSecondStage",  m_param_runSecondStage, "Switch to deactive the second stage of the tracking algorithm.", true);

}

TrackFinderCDCAutomatonDevModule::~TrackFinderCDCAutomatonDevModule()
{
}

void TrackFinderCDCAutomatonDevModule::initialize()
{
  TrackFinderCDCBaseModule::initialize();

  m_segmentWorker.initialize();

  if (m_param_runSecondStage) {
    //m_segmentTripleTrackingWorker.initialize();
    m_segmentPairTrackingWorker.initialize();
  }

#ifdef TRACKFINDINGCDC_USE_MC_INFORMATION
  StoreArray <CDCSimHit>::required();
  StoreArray <MCParticle>::required();
#endif
}

void  TrackFinderCDCAutomatonDevModule::beginRun()
{
  TrackFinderCDCBaseModule::beginRun();
}

void  TrackFinderCDCAutomatonDevModule::event()
{
  B2DEBUG(100, "########## CDCLocalTracking begin ##########");

#ifdef TRACKFINDINGCDC_USE_MC_INFORMATION
  CDCMCManager::getInstance().clear();
  CDCMCManager::getInstance().fill();
#endif

#ifdef HAS_CALLGRIND
  CALLGRIND_START_INSTRUMENTATION;
#endif

  // Load the hits from the DataStore into the CDCWireHitTopology.
  TrackFinderCDCBaseModule::event();

  //Start callgrind recording
  //To profile start basf2 with
  //  nohup valgrind --tool=callgrind --instr-atstart=no basf2 [basf2-options] > output.txt &
  // since that takes a while.
  // Do a callgrind_control -b for an intermediate output or callgrind_control -b -e
  // Definitions need callgrind.h

  // Stage one
  // Build the segments
  m_recoSegments.clear();
  m_segmentWorker.generate(m_recoSegments);
  B2DEBUG(100, "Received " << m_recoSegments.size() << " RecoSegments from worker");

  if (m_param_runSecondStage) {
    // Stage two
    // Build the gfTracks
    StoreArray < genfit::TrackCand > storedGFTrackCands(m_param_gfTrackCandsStoreArrayName);
    //m_segmentTripleTrackingWorker.apply(m_recoSegments, storedGFTrackCands);
    m_segmentPairTrackingWorker.apply(m_recoSegments, storedGFTrackCands);
  }

  // End callgrind recording
#ifdef HAS_CALLGRIND
  CALLGRIND_STOP_INSTRUMENTATION;
#endif

  B2DEBUG(100, "########## CDCLocalTracking end ############");

}

void  TrackFinderCDCAutomatonDevModule::endRun()
{
  TrackFinderCDCBaseModule::endRun();
}

void  TrackFinderCDCAutomatonDevModule::terminate()
{
  if (m_param_runSecondStage) {
    //m_segmentTripleTrackingWorker.terminate();
    m_segmentPairTrackingWorker.terminate();
  }

  m_segmentWorker.terminate();

#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS;
#endif
  TrackFinderCDCBaseModule::terminate();
}

