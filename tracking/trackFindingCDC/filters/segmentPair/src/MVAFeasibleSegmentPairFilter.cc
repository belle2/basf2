/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/MVAFeasibleSegmentPairFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVAFeasibleSegmentPairFilter::MVAFeasibleSegmentPairFilter()
  : Super("trackfindingcdc_FeasibleSegmentPairFilter", 0.004)
{
}
