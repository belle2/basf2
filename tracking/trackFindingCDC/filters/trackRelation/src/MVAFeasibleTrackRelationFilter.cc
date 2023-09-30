/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/MVAFeasibleTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MVA<BaseTrackRelationFilter>;

MVAFeasibleTrackRelationFilter::MVAFeasibleTrackRelationFilter()
  : Super(std::make_unique<VarSet>(), "trackfindingcdc_FeasibleTrackRelationFilter", 0.03,
          "trackfindingcdc_FeasibleTrackRelationFilterParameters")
{
}
