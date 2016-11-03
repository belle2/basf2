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
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVARealisticSegmentRelationFilter::MVARealisticSegmentRelationFilter()
  : Super(makeUnique<VarSet>(),
          "trackfindingcdc_RealisticSegmentRelationFilter",
          0.90)
{
}

void MVARealisticSegmentRelationFilter::initialize()
{
  Super::initialize();
  m_feasibleSegmentRelationFilter.initialize();
}

void MVARealisticSegmentRelationFilter::beginRun()
{
  Super::beginRun();
  m_feasibleSegmentRelationFilter.beginRun();
}

Weight MVARealisticSegmentRelationFilter::operator()(const Relation<const CDCRecoSegment2D>& segmentRelation)
{
  double isFeasibleWeight = m_feasibleSegmentRelationFilter(segmentRelation);
  if (std::isnan(isFeasibleWeight)) {
    return NAN;
  } else {
    return Super::operator()(segmentRelation);
  }
}
