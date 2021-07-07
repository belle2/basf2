/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/MVARealisticTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MVA<BaseTrackRelationFilter>;

MVARealisticTrackRelationFilter::MVARealisticTrackRelationFilter()
  : Super(std::make_unique<VarSet>(), "trackfindingcdc_RealisticTrackRelationFilter", 0.80)
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
