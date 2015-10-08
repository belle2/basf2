/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreSegmentModule.h>

using namespace Belle2;

REG_MODULE(CDCLegendreTracking);

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
//  TrackFinderCDCBaseModule(c_Outwards), // Virtual base class must be instantiated from the most derived class.
  SegmentFinderCDCFacetAutomatonModule(c_Symmetric),
  TrackFinderCDCLegendreModule()
{
//  setDescription("Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.");
}

void CDCLegendreTrackingModule::initialize()
{
//  SegmentFinderCDCFacetAutomatonModule::initialize();
  TrackFinderCDCLegendreModule::initialize();
}

void CDCLegendreTrackingModule::event()
{
  B2DEBUG(100, "########## TrackFinderCDCAutomaton begin ##########");

  // Stage one
//  SegmentFinderCDCFacetAutomatonModule::event();

  // Stage two
  TrackFinderCDCLegendreModule::event();

  B2DEBUG(100, "########## TrackFinderCDCAutomaton end ############");
}

void CDCLegendreTrackingModule::terminate()
{
//  SegmentFinderCDCFacetAutomatonModule::terminate();
  TrackFinderCDCLegendreModule::terminate();
}

