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

  class DAFRecoFitterModule : public BaseRecoFitterModule {
  public:
    /** Module for the DAF fitter */
    DAFRecoFitterModule() : BaseRecoFitterModule()
    {
      setDescription("DAF Fitter using Genfit. If you have TrackCands, you have to convert them to RecoTracks"
                     "using the RecoTrackCreatorModule first. After that, you probably want to add the measurements from the"
                     "hits using the MeasurementCreatorModule.");

      addParam("ProbCut", m_param_probabilityCut,
               "Probability cut for the DAF. Any value between 0 and 1 is possible. Common values are between 0.01 and 0.001",
               double(0.001));

      addParam("MinimumIterations", m_param_minimumIterations,
               "Minimum number of iterations for the Kalman filter", static_cast<unsigned int>(2));
      addParam("MaximumIterations", m_param_maximumIterations,
               "Maximum number of iterations for the Kalman filter", static_cast<unsigned int>(4));

      addParam("numberOfFailedHits", m_param_maxNumberOfFailedHits,
               "Maximum number of failed hits before aborting the fit.", static_cast<int>(-1));
    }

  protected:
    /** Create a DAF fitter */
    std::shared_ptr<genfit::AbsFitter> createFitter() const override
    {
      std::shared_ptr<genfit::DAF> fitter = std::make_shared<genfit::DAF>();
      fitter->setMaxFailedHits(m_param_maxNumberOfFailedHits);
      fitter->setMinIterations(m_param_minimumIterations);
      fitter->setMaxIterations(m_param_maximumIterations);

      fitter->setProbCut(m_param_probabilityCut);

      return fitter;
    }

  private:
    unsigned int m_param_minimumIterations = 2; /**< Minimum number of iterations for the Kalman filter */
    unsigned int m_param_maximumIterations = 4; /**< Maximum number of iterations for the Kalman filter */
    /** Probability cut for the DAF. Any value between 0 and 1 possible. Common values are between 0.01 and 0.001 */
    double m_param_probabilityCut;
    /** Maximum number of failed hits before aborting the fit */
    int m_param_maxNumberOfFailedHits = -1;
  };
}
