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
#include <genfit/DAF.h>

namespace Belle2 {

  /** DAF fitter*/
  class DAFRecoFitterModule : public BaseRecoFitterModule {
  public:
    /** Module for the DAF fitter */
    DAFRecoFitterModule();

  protected:
    /** Create a DAF fitter */
    std::shared_ptr<genfit::AbsFitter> createFitter() const override;

  private:
    unsigned int m_param_minimumIterations = 2; /**< Minimum number of iterations for the Kalman filter */
    unsigned int m_param_maximumIterations = 4; /**< Maximum number of iterations for the Kalman filter */
    /** Probability cut for the DAF. Any value between 0 and 1 possible. Common values are between 0.01 and 0.001 */
    double m_param_probabilityCut;
    /** Maximum number of failed hits before aborting the fit */
    int m_param_maxNumberOfFailedHits = -1;
  };
}
