/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/SegmentTrackCombinerModule.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParam.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_SegmentTrackCombiner);

TFCDC_SegmentTrackCombinerModule::TFCDC_SegmentTrackCombinerModule()
  : Super( {"CDCSegment2DVector", "CDCTrackVector"})
{
}
