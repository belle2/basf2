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

#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
#endif

#include <time.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackFinderCDCAutomatonDev);

TrackFinderCDCAutomatonDevModule::TrackFinderCDCAutomatonDevModule() :
  SegmentFinderCDCFacetAutomatonModule(),
  TrackFinderCDCSegmentPairAutomatonDevModule()
{
  setDescription("For development purposes of the cellular automaton track finding.");
  addParam("runSecondStage",  m_param_runSecondStage, "Switch to deactive the second stage of the tracking algorithm.", true);
}


void TrackFinderCDCAutomatonDevModule::initialize()
{
  SegmentFinderCDCFacetAutomatonModule::initialize();
  if (m_param_runSecondStage) {
    TrackFinderCDCSegmentPairAutomatonDevModule::initialize();
  }
}


void TrackFinderCDCAutomatonDevModule::event()
{
  B2DEBUG(100, "########## TrackFinderCDCAutomatonDevModule begin ##########");

#ifdef HAS_CALLGRIND
  CALLGRIND_START_INSTRUMENTATION;
#endif

  //Start callgrind recording
  //To profile start basf2 with
  //  nohup valgrind --tool=callgrind --instr-atstart=no basf2 [basf2-options] > output.txt &
  // since that takes a while.
  // Do a callgrind_control -b for an intermediate output or callgrind_control -b -e
  // Definitions need callgrind.h

  // Stage one
  SegmentFinderCDCFacetAutomatonModule::event();

  // Stage two
  if (m_param_runSecondStage) {
    TrackFinderCDCSegmentPairAutomatonDevModule::event();
  }

  // End callgrind recording
#ifdef HAS_CALLGRIND
  CALLGRIND_STOP_INSTRUMENTATION;
#endif

  B2DEBUG(100, "########## TrackFinderCDCAutomatonModule end ############");
}


void TrackFinderCDCAutomatonDevModule::terminate()
{
  if (m_param_runSecondStage) {
    TrackFinderCDCSegmentPairAutomatonDevModule::terminate();
  }

  SegmentFinderCDCFacetAutomatonModule::terminate();

#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS;
#endif

}




