/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/MVARealisticAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVARealisticAxialSegmentPairFilter::MVARealisticAxialSegmentPairFilter()
  : Super(std::make_unique<VarSet>(),
          "trackfindingcdc_RealisticAxialSegmentPairFilter",
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
