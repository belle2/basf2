/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/MVARealisticTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MVA<BaseTrackRelationFilter>;

MVARealisticTrackRelationFilter::MVARealisticTrackRelationFilter()
  : Super(std::make_unique<VarSet>(), "tracking/data/trackfindingcdc_RealisticTrackRelationFilter.xml", 0.80)
{
  this->addProcessingSignalListener(&m_feasibleTrackRelationFilter);
}

Weight MVARealisticTrackRelationFilter::operator()(const Relation<const CDCTrack>& trackRelation)
{
  double isFeasibleWeight = m_feasibleTrackRelationFilter(trackRelation);
  if (std::isnan(isFeasibleWeight)) {
    return NAN;
  } else {
    return Super::operator()(trackRelation);
  }
}
