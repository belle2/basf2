/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/pathFilters/FiveHitFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

void FiveHitFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "helixFitPocaDCut"), m_param_helixFitPocaDCut,
                                "Cut on the POCA difference in xy with the POCA obtained from a .",
                                m_param_helixFitPocaDCut);
}

void FiveHitFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
}

TrackFindingCDC::Weight
FiveHitFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const VXDHoughState*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 4) {
    return NAN;
  }

  return 1.0;
}
