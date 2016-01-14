/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/TrackFinderCDCSegmentPairAutomatonDevModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackFinderCDCSegmentPairAutomatonDev);

TrackFinderCDCSegmentPairAutomatonDevModule::TrackFinderCDCSegmentPairAutomatonDevModule()
{
  this->setDescription("This is a depricated alias for TrackFinderCDCSegmentPairAutomatonModule");
}

void TrackFinderCDCSegmentPairAutomatonDevModule::initialize()
{
  B2WARNING("TrackFinderCDCSegmentPairAutomatonDevModule is a depricated alias for TrackFinderCDCSegmentPairAutomatonModule");
  TrackFinderCDCSegmentPairAutomatonModule::initialize();
}
