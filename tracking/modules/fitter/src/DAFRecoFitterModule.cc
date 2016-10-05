/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/fitter/DAFRecoFitterModule.h>
using namespace Belle2;

REG_MODULE(DAFRecoFitter)

DAFRecoFitterModule::DAFRecoFitterModule() : BaseRecoFitterModule()
{
  setDescription("DAF Fitter using Genfit. If you have TrackCands, you have to convert them to RecoTracks"
                 "using the RecoTrackCreatorModule first. After that, you probably want to add the measurements from the"
                 "hits using the MeasurementCreatorModule.");

  addParam("probCut", m_param_probabilityCut,
           "Probability cut for the DAF. Any value between 0 and 1 is possible. Common values are between 0.01 and 0.001",
           double(0.001));

  addParam("numberOfFailedHits", m_param_maxNumberOfFailedHits,
           "Maximum number of failed hits before aborting the fit.", static_cast<int>(5));

  addParam("deltaPvalue", m_param_deltaPValue,
           "If the difference in p-value between two DAF iterations is smaller than this value, the iterative procedure will be terminated early.",
           /// This is the difference on pvalue between two fit iterations of the DAF procedure which
           /// is used as a early termination criteria of the DAF procedure. This is large on purpose
           /// See https://agira.desy.de/browse/BII-1725 for details
           double(1.0f));
}

/** Create a DAF fitter */
std::shared_ptr<genfit::AbsFitter> DAFRecoFitterModule::createFitter() const
{
  std::shared_ptr<genfit::DAF> fitter = std::make_shared<genfit::DAF>(true, m_param_deltaPValue);
  fitter->setMaxFailedHits(m_param_maxNumberOfFailedHits);

  fitter->setProbCut(m_param_probabilityCut);

  return fitter;
}
