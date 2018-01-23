/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/MVARealisticAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVARealisticAxialSegmentPairFilter::MVARealisticAxialSegmentPairFilter()
  : Super(std::make_unique<VarSet>(),
          "tracking/data/trackfindingcdc_RealisticAxialSegmentPairFilter.xml",
          0.49)
{
  this->addProcessingSignalListener(&m_feasibleAxialSegmentPairFilter);
}

Weight MVARealisticAxialSegmentPairFilter::operator()(const CDCAxialSegmentPair& axialSegmentPair)
{
  double isFeasibleWeight = m_feasibleAxialSegmentPairFilter(axialSegmentPair);
  if (std::isnan(isFeasibleWeight)) {
    return NAN;
  } else {
    return Super::operator()(axialSegmentPair);
  }
}
