/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentFinderCDCFacetAutomaton);

SegmentFinderCDCFacetAutomatonModule::SegmentFinderCDCFacetAutomatonModule()
  : Super{{{"CDCWireHitVector", "CDCSegment2DVector"}}}
{
  ModuleParamList moduleParamList = this->getParamList();
  moduleParamList.getParameter<std::string>("SegmentOrientation").setDefaultValue("curling");
  this->setParamList(moduleParamList);
}
