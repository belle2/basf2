/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/utilities/KalmanStepper.h>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class CKFToSVDState;

  /// Kalman stepper implementation for the SVD CKF
  class SVDKalmanStepper {
  public:
    /// Do a kalman step of the mSoP to the measurement in the state. Returns the chi2.
    double kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToSVDState& state);

    /// Calculate the residual between the mSoP and the measurement in the state.
    double calculateResidual(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToSVDState& state);

  private:
    /// Implementation using the general kalman stepper
    KalmanStepper<1> m_kalmanStepper;
  };
}
