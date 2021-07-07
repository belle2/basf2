/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/MVARealisticSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MVA<BaseSegmentRelationFilter>;

MVARealisticSegmentRelationFilter::MVARealisticSegmentRelationFilter()
  : Super(std::make_unique<VarSet>(),
          "trackfindingcdc_RealisticSegmentRelationFilter",
          0.805)
{
  this->addProcessingSignalListener(&m_feasibleSegmentRelationFilter);
}

Weight MVARealisticSegmentRelationFilter::operator()(const Relation<const CDCSegment2D>& segmentRelation)
{
  double isFeasibleWeight = m_feasibleSegmentRelationFilter(segmentRelation);
  if (std::isnan(isFeasibleWeight)) {
    return NAN;
  } else {
    return Super::operator()(segmentRelation);
  }
}
