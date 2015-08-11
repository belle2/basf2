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
    }

  protected:
    /** Create a DAF fitter */
    std::shared_ptr<genfit::AbsKalmanFitter> createFitter() const override
    {
      std::shared_ptr<genfit::DAF> fitter = std::make_shared<genfit::DAF>();
      fitter->setProbCut(m_param_probabilityCut);

      return fitter;
    }

  private:
    double m_param_probabilityCut; /**< Probability cut for the DAF. Any value between 0 and 1 possible. Common values are between 0.01 and 0.001 */
  };
}
