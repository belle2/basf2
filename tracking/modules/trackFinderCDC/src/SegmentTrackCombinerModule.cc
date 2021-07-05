/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/SegmentTrackCombinerModule.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_SegmentTrackCombiner);

TFCDC_SegmentTrackCombinerModule::TFCDC_SegmentTrackCombinerModule()
  : Super( {"CDCSegment2DVector", "CDCTrackVector"})
{
}
