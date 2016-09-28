/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonDevModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentFinderCDCFacetAutomatonDev);

SegmentFinderCDCFacetAutomatonDevModule::SegmentFinderCDCFacetAutomatonDevModule()
{
  this->setDescription("This is a depricated alias for SegmentFinderCDCFacetAutomatonModule");
}


void SegmentFinderCDCFacetAutomatonDevModule::initialize()
{
  B2WARNING("SegmentFinderCDCFacetAutomatonDevModule is a depricated alias for SegmentFinderCDCFacetAutomatonModule");
  SegmentFinderCDCFacetAutomatonModule::initialize();
}
