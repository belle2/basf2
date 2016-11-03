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
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVARealisticTrackRelationFilter::MVARealisticTrackRelationFilter()
  : Super(makeUnique<VarSet>(),
          "trackfindingcdc_RealisticTrackRelationFilter",
          0.80)
{
}

void MVARealisticTrackRelationFilter::initialize()
{
  Super::initialize();
  m_feasibleTrackRelationFilter.initialize();
}

void MVARealisticTrackRelationFilter::beginRun()
{
  Super::beginRun();
  m_feasibleTrackRelationFilter.beginRun();
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
