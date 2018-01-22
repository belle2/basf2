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
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/Exception.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Filter which can be used on a pair of path (vector of states) and states,
   * which will call the extrapolate function of the given advancer class, to extrapolate
   * the last path state to the plane of the new state and store the mSoP in the
   * new state. Will return the result of the advancers extrapolation function as a filter result or NAN
   * of the extrapolation throws a genfit::Exception.
   *
   * @tparam AState: should have the basic functionality of a CKFState.
   * @tparam AnAdvancer: The class to be used during extrapolation. Should have the functions:
   *     * exposeParameters(moduleParamList, prefix)
   *     * setMaterialEffectsToParameterValue()
   *     * extrapolateToPlane(mSoP, plane)
   *     * resetMaterialEffects()
   *     should be a ProcessingSignalListener.
   */
  template <class AState, class AnAdvancer>
  class AdvanceFilter : public
    TrackFindingCDC::Filter<std::pair<const std::vector<TrackFindingCDC::WithWeight<const AState*>>, AState*>> {
    using Super = TrackFindingCDC::Filter<std::pair<const std::vector<TrackFindingCDC::WithWeight<const AState*>>, AState*>>;

  public:
    AdvanceFilter() : Super()
    {
      Super::addProcessingSignalListener(&m_advancer);
    }

    /// Expose the parameters of the advancer.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      m_advancer.exposeParameters(moduleParamList, prefix);
    }

    /// Extrapolate and return the result. Update the mSoP of the new state.
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
    /// The advancer to use.
    AnAdvancer m_advancer;
  };
}