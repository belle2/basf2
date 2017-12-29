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

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <genfit/MeasuredStateOnPlane.h>

namespace Belle2 {
  template <class AState, class AKalmanStepper>
  class KalmanFilter : public
    TrackFindingCDC::Filter<std::pair<const std::vector<TrackFindingCDC::WithWeight<const AState*>>, AState*>> {
  public:
    TrackFindingCDC::Weight operator()(const std::pair<const std::vector<TrackFindingCDC::WithWeight<const AState*>>, AState*>& pair)
    override
    {
      AState* currentState = pair.second;

      genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState->getMeasuredStateOnPlane();

      const double chi2 = m_kalmanStepper.kalmanStep(measuredStateOnPlane, *currentState);

      currentState->setChi2(chi2);
      currentState->setMeasuredStateOnPlane(measuredStateOnPlane);

      return chi2;
    }

  private:
    AKalmanStepper m_kalmanStepper;
  };
}