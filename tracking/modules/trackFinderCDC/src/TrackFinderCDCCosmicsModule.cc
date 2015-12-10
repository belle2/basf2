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

TrackFinderCDCCosmicsModule::TrackFinderCDCCosmicsModule()
{
  setDescription("Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.");

  // Set the default segment to symmetric
  ModuleParamList moduleParamList = this->getParamList();
  moduleParamList.getParameter<std::string>("SegmentOrientation").setDefaultValue("symmetric");
  this->setParamList(moduleParamList);
  this->setTrackOrientation(ETrackOrientation::c_Downwards);
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



