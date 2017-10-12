/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>
#include <tracking/ckf/general/utilities/Advance.h>

namespace Belle2 {
  template <class AState, class AnAdvancer = Advancer>
  class AdvanceFilter : public TrackFindingCDC::Filter<std::pair<const std::vector<const AState*>, AState*>> {
  public:
    AdvanceFilter()
    {
      this->addProcessingSignalListener(&m_advancer);
    }

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      m_advancer.exposeParameters(moduleParamList, prefix);
    }

    TrackFindingCDC::Weight operator()(const std::pair<const std::vector<const AState*>, AState*>& pair) override
    {
      const std::vector<const AState*>& previousStates = pair.first;
      AState* currentState = pair.second;

      B2ASSERT("Can not extrapolate with nothing", not previousStates.empty());
      genfit::MeasuredStateOnPlane mSoP = previousStates.back()->getMeasuredStateOnPlane();
      genfit::SharedPlanePtr plane = currentState->getPlane(mSoP);

      const double returnValue = m_advancer.extrapolateToPlane(mSoP, plane);

      currentState->setMeasuredStateOnPlane(mSoP);

      return returnValue;
    }

  private:
    AnAdvancer m_advancer;
  };
}