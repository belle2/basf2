/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/CDCLocalTrackingModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(CDCLocalTracking);

CDCLocalTrackingModule::CDCLocalTrackingModule():
  TrackFinderCDCAutomatonModule()
{
  setDescription("Depricated alias for TrackFinderCDCAutomaton");
}

void CDCLocalTrackingModule::initialize()
{
  B2WARNING("CDCLocalTrackingModule is a depricated alias TrackFinderCDCAutomatonModule");
  TrackFinderCDCAutomatonModule::initialize();
}

