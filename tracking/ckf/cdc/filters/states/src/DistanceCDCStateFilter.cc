/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/DistanceCDCStateFilter.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace Belle2;

void DistanceCDCStateFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalDistance"),
                                m_maximalDistance, "Maximal distance  between track and hit post kalman update", m_maximalDistance);
}


TrackFindingCDC::Weight DistanceCDCStateFilter::operator()(const BaseCDCStateFilter::Object& pair)
{
  const CDCCKFState& state = *(pair.second);

  double dist = std::abs(state.getHitDistance());

  if (dist > m_maximalDistance) {
    return NAN;
  }

  return 1 / dist;
}
