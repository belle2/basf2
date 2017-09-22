/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/state/SimpleCKFSpacePointFilter.h>

#include <tracking/ckf/utilities/StateAlgorithms.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void SimpleCKFSpacePointFilter::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitJumpingUpTo"), m_param_hitJumpingUpTo,
                                "", m_param_hitJumpingUpTo);
}

void SimpleCKFSpacePointFilter::beginRun()
{
  m_cachedBField = TrackFindingCDC::CDCBFieldUtil::getBFieldZ();
}

bool SimpleCKFSpacePointFilter::checkOverlapAndHoles(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState)
{
  if (currentState.getHit()) {
    return true;
  }

  int numberOfHoles = 0;

  const auto countNumberOfHoles = [&numberOfHoles](const BaseCKFCDCToSpacePointStateObjectFilter::Object * walkObject) {
    if (not walkObject->getHit() and not isOnOverlapLayer(*walkObject)) {
      numberOfHoles++;
    }
  };
  currentState.walk(countNumberOfHoles);

  return isOnOverlapLayer(currentState) or numberOfHoles <= m_param_hitJumpingUpTo;
}