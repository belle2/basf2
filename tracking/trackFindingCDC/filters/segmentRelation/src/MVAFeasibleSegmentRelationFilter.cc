/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/MVAFeasibleSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVAFeasibleSegmentRelationFilter::MVAFeasibleSegmentRelationFilter()
  : Super(makeUnique<VarSet>(),
          "tracking/data/FeasibleSegmentRelationFilter.weights.xml",
          0.02)
{
}
