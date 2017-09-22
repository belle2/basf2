/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/MVARealisticSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MVA<BaseSegmentPairRelationFilter>;

MVARealisticSegmentPairRelationFilter::MVARealisticSegmentPairRelationFilter()
  : Super(std::make_unique<VarSet>(), "tracking/data/trackfindingcdc_RealisticSegmentPairRelationFilter.xml", 0.22)
{
}
