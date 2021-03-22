/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/FiveHitFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
FiveHitFilter::operator()(const BasePathFilter::Object& pair)
{
//   const std::vector<TrackFindingCDC::WithWeight<const HitData*>>& previousHits = pair.first;
//   HitData* currentHit = pair.second;

  return 1.0;
//   return 0.0;
//   return NAN;
}

void FiveHitFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
}
