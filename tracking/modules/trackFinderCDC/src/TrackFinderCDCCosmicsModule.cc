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

using namespace Belle2;

REG_MODULE(TrackFinderCDCCosmics);

TrackFinderCDCCosmicsModule::TrackFinderCDCCosmicsModule() :
  TrackFinderCDCBaseModule(c_Downwards), // Virtual base class must be instantiated from the most derived class.
  SegmentFinderCDCFacetAutomatonModule(c_Symmetric)
{
  setDescription("Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.");
}

void TrackFinderCDCCosmicsModule::initialize()
{
  SegmentFinderCDCFacetAutomatonModule::initialize();
  TrackFinderCDCSegmentPairAutomatonModule::initialize();
}

void TrackFinderCDCCosmicsModule::event()
{
  B2DEBUG(100, "########## TrackFinderCDCCosmics begin ##########");

  // Stage one
  SegmentFinderCDCFacetAutomatonModule::event();

  // Stage two
  TrackFinderCDCSegmentPairAutomatonModule::event();

  B2DEBUG(100, "########## TrackFinderCDCCosmics end ############");
}

void TrackFinderCDCCosmicsModule::terminate()
{
  TrackFinderCDCSegmentPairAutomatonModule::terminate();
  SegmentFinderCDCFacetAutomatonModule::terminate();
}



