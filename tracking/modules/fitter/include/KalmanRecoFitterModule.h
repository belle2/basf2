/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/fitter/BaseRecoFitterModule.h>
#include <genfit/KalmanFitterRefTrack.h>

namespace Belle2 {

  class KalmanRecoFitterModule : public BaseRecoFitterModule {
  public:
    /** Module for the Kalman Fitter */
    KalmanRecoFitterModule() : BaseRecoFitterModule()
    {
      addParam("MaximumFailedHits", m_param_maximumFailedHits,
               "Maximum number of of failed hits before aborting the fit", static_cast<unsigned int>(5));
      addParam("MinimumIterations", m_param_minimumIterations,
               "Minimum number of iterations for the Kalman filter", static_cast<unsigned int>(3));
      addParam("MaximumIterations", m_param_maximumIterations,
               "Maximum number of iterations for the Kalman filter", static_cast<unsigned int>(10));
    }

  protected:
    /** Create a Kalman fitter */
    std::shared_ptr<genfit::AbsFitter> createFitter() const override
    {
      std::shared_ptr<genfit::KalmanFitterRefTrack> fitter = std::make_shared<genfit::KalmanFitterRefTrack>();
      fitter->setMaxFailedHits(m_param_maximumFailedHits);
      fitter->setMinIterations(m_param_minimumIterations);
      fitter->setMaxIterations(m_param_maximumIterations);

      return fitter;
    }

  private:
    unsigned int m_param_maximumFailedHits; /**< Maximum number of of failed hits before aborting the fit */
    unsigned int m_param_minimumIterations; /**< Minimum number of iterations for the Kalman filter */
    unsigned int m_param_maximumIterations; /**< Maximum number of iterations for the Kalman filter */
  };
}
