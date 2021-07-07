/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/MVARealisticSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MVAFilter<MVARealisticSegmentPairVarSet>;

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
