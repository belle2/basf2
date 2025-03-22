/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/DistanceCDCStateFilter.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>

using namespace Belle2;

void DistanceCDCStateFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "maximalDistance"),
                                m_maximalDistance, "Maximal distance  between track and hit post kalman update", m_maximalDistance);
}


TrackingUtilities::Weight DistanceCDCStateFilter::operator()(const BaseCDCStateFilter::Object& pair)
{
  const CDCCKFState& state = *(pair.second);

  double dist = std::abs(state.getHitDistance());

  if (dist > m_maximalDistance) {
    return NAN;
  }

  return 1 / dist;
}
