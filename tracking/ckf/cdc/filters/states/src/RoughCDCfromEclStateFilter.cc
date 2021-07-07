/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/RoughCDCfromEclStateFilter.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;

TrackFindingCDC::Weight RoughCDCfromEclStateFilter::operator()(const BaseCDCStateFilter::Object& pair)
{
  const CDCCKFPath* path = pair.first;
  const CDCCKFState& state = *(pair.second);
  const CDCCKFState& lastState = path->back();

  const double& arcLength = state.getArcLength() - lastState.getArcLength();
  // TODO: magic number
  if (!lastState.isSeed() and (arcLength >= 0 or arcLength < -20)) {
    return NAN;
  }
  if (lastState.isSeed() and (arcLength >= 0 or arcLength < -75)) {
    return NAN;
  }


  const double& hitDistance = state.getHitDistance();
  if (!lastState.isSeed() and std::abs(hitDistance) > m_maximalHitDistance) {
    return NAN;
  }
  if (lastState.isSeed() and std::abs(hitDistance) > m_maximalHitDistanceSeed) {
    return NAN;
  }


  return 1;
}


void RoughCDCfromEclStateFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalHitDistance"),
                                m_maximalHitDistance,
                                "Maximal allowed hit distance",
                                m_maximalHitDistance);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalHitDistanceEclSeed"),
                                m_maximalHitDistanceSeed,
                                "Maximal allowed hit distance",
                                m_maximalHitDistanceSeed);
}