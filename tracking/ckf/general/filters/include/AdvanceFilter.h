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
#include <genfit/Exception.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  template <class AState, class AnAdvancer>
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
      m_advancer.setMaterialEffectsToParameterValue();

      const std::vector<TrackFindingCDC::WithWeight<const AState*>>& previousStates = pair.first;
      B2ASSERT("Can not extrapolate with nothing", not previousStates.empty());

      const AState* lastState = previousStates.back();
      AState* currentState = pair.second;

      B2ASSERT("Can not extrapolate with nothing", lastState->mSoPSet());
      genfit::MeasuredStateOnPlane mSoP = lastState->getMeasuredStateOnPlane();

      double returnValue = NAN;
      try {
        genfit::SharedPlanePtr plane = currentState->getPlane(mSoP);
        returnValue = m_advancer.extrapolateToPlane(mSoP, plane);
      } catch (genfit::Exception& e) {
        B2DEBUG(50, "Plane extraction failed: " << e.what());
      }

      if (not std::isnan(returnValue)) {
        currentState->setMeasuredStateOnPlane(mSoP);
      }

      m_advancer.resetMaterialEffects();

      return returnValue;
    }

  private:
    AnAdvancer m_advancer;
  };
}