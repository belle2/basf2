/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/MVAFeasibleTrackRelationFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVAFeasibleTrackRelationFilter::MVAFeasibleTrackRelationFilter()
  : Super(std::make_unique<VarSet>(), "tracking/data/trackfindingcdc_FeasibleTrackRelationFilter.xml", 0.03)
{
}
