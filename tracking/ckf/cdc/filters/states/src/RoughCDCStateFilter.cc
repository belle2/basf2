/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/RoughCDCStateFilter.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;

TrackingUtilities::Weight RoughCDCStateFilter::operator()(const BaseCDCStateFilter::Object& pair)
{
  const CDCCKFPath* path = pair.first;
  const CDCCKFState& state = *(pair.second);
  const CDCCKFState& lastState = path->back();

  const double& arcLength = state.getArcLength() - lastState.getArcLength();
  // TODO: magic number
  std::cout << "m_maximalArcLengthDistance " << m_maximalArcLengthDistance << std::endl;
  std::cout << "m_maximalHitDistance " << m_maximalHitDistance << std::endl;

  if (arcLength <= 0 or arcLength > m_maximalArcLengthDistance) {
    return NAN;
  }


  const double& hitDistance = state.getHitDistance();
  if (std::abs(hitDistance) > m_maximalHitDistance) {
    return NAN;
  }

  return 1;
}

void RoughCDCStateFilter::setMaximalArcLengthDistance(double arclength)
{
  m_maximalArcLengthDistance = arclength;
}

void RoughCDCStateFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{

  std::cout << "prefix " << prefix << std::endl;

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "maximalHitDistance"),
                                m_maximalHitDistance,
                                "Maximal allowed hit distance",
                                m_maximalHitDistance);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "maximalArcLengthDistance"),
                                m_maximalArcLengthDistance,
                                "Maximal allowed arclength distance",
                                m_maximalArcLengthDistance);
}
