/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/MVARealisticSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MVA<BaseSegmentRelationFilter>;

MVARealisticSegmentRelationFilter::MVARealisticSegmentRelationFilter()
  : Super(std::make_unique<VarSet>(),
          "tracking/data/trackfindingcdc_RealisticSegmentRelationFilter.xml",
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
