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
#include <tracking/ckf/general/utilities/Advancer.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <genfit/MeasuredStateOnPlane.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  template <class AState, class AnAdvancer = Advancer>
  class AdvanceFilter : public
    TrackFindingCDC::Filter<std::pair<const std::vector<TrackFindingCDC::WithWeight<const AState*>>, AState*>> {
  public:
    AdvanceFilter()
    {
      this->addProcessingSignalListener(&m_advancer);
    }

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      m_advancer.exposeParameters(moduleParamList, prefix);
    }

    TrackFindingCDC::Weight operator()(const std::pair<const std::vector<TrackFindingCDC::WithWeight<const AState*>>, AState*>& pair)
    override
    {
      const std::vector<TrackFindingCDC::WithWeight<const AState*>>& previousStates = pair.first;
      B2ASSERT("Can not extrapolate with nothing", not previousStates.empty());

      const AState* lastState = previousStates.back();
      AState* currentState = pair.second;

      B2ASSERT("Can not extrapolate with nothing", lastState->mSoPSet());
      genfit::MeasuredStateOnPlane mSoP = lastState->getMeasuredStateOnPlane();

      genfit::SharedPlanePtr plane = currentState->getPlane(mSoP);

      const double returnValue = m_advancer.extrapolateToPlane(mSoP, plane);

      if (not std::isnan(returnValue)) {
        currentState->setMeasuredStateOnPlane(mSoP);
      }

      return returnValue;
    }

  private:
    AnAdvancer m_advancer;
  };
}