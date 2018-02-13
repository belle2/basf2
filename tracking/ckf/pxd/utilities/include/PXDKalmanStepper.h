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

#include <tracking/ckf/general/utilities/KalmanStepper.h>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class SpacePoint;
  class CKFToPXDState;

  /// Kalman stepper implementation for the PXD CKF
  class PXDKalmanStepper {
  public:
    /// Do a kalman step of the mSoP to the measurement in the state. Returns the chi2.
    double kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToPXDState& state);

    /// Calculate the residual between the mSoP and the measurement in the state.
    double calculateResidual(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToPXDState& state);

  private:
    /// Implementation using the general kalman stepper
    KalmanStepper<2> m_kalmanStepper;
  };
}