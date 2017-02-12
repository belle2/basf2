/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/MVARealisticSegmentPairFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVARealisticSegmentPairFilter::MVARealisticSegmentPairFilter()
  : Super("trackfindingcdc_RealisticSegmentPairFilter", 0.02)
{
  this->addProcessingSignalListener(&m_feasibleSegmentPairFilter);
}

Weight MVARealisticSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  double isFeasibleWeight = m_feasibleSegmentPairFilter(segmentPair);
  if (std::isnan(isFeasibleWeight)) {
    return NAN;
  } else {
    return Super::operator()(segmentPair);
  }
}
