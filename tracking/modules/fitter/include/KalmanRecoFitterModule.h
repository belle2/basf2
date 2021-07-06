/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/modules/fitter/BaseRecoFitterModule.h>
#include <genfit/KalmanFitterRefTrack.h>

namespace Belle2 {

  /** Kalman fitter. */
  class KalmanRecoFitterModule : public BaseRecoFitterModule {
  public:
    /** Module for the Kalman Fitter. */
    KalmanRecoFitterModule();

  protected:
    /** Create a Kalman fitter. */
    std::shared_ptr<genfit::AbsFitter> createFitter() const override;

  private:
    unsigned int m_param_minimumIterations; /**< Minimum number of iterations for the Kalman filter. */
    unsigned int m_param_maximumIterations; /**< Maximum number of iterations for the Kalman filter. */
    /** Maximum number of failed hits before aborting the fit. */
    unsigned int m_param_maxNumberOfFailedHits = 5;
  };
}
