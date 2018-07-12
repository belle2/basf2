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
#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <genfit/MeasuredStateOnPlane.h>

#include <vector>
#include <framework/logging/Logger.h>

namespace Belle2 {
  /**
   * Filter which can be used on a pair of path (vector of states) and states,
   * which will call the kalmanStep function of the given stepper class, to update the mSoP
   * of the new state with a klaman update. Will return the result of the steppers kalmanStep function as a filter
   * result (which should be the chi2) and will also set the chi2 of the new state.
   * @tparam AState: should have the basic functionality of a CKFState.
   * @tparam AKalmanStepper: should have a kalmanStep function.
   */
  template <class AState, class AKalmanStepper>
  class KalmanFilter : public
    TrackFindingCDC::Filter<std::pair<const std::vector<TrackFindingCDC::WithWeight<const AState*>>, AState*>> {
  public:
    /// Call the kalmanStep and update the new state and its chi2.
    TrackFindingCDC::Weight operator()(const std::pair<const std::vector<TrackFindingCDC::WithWeight<const AState*>>, AState*>& pair)
    override
    {
      AState* currentState = pair.second;

      B2ASSERT("Can not update with nothing", currentState->mSoPSet());
      genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState->getMeasuredStateOnPlane();

      const double chi2 = m_kalmanStepper.kalmanStep(measuredStateOnPlane, *currentState);

      currentState->setChi2(chi2);
      currentState->setMeasuredStateOnPlane(measuredStateOnPlane);

      return chi2;
    }

  private:
    /// The used stepper algorithm
    AKalmanStepper m_kalmanStepper;
  };
}