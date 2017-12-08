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
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

using namespace Belle2;

REG_MODULE(DAFRecoFitter)

DAFRecoFitterModule::DAFRecoFitterModule() : BaseRecoFitterModule()
{
  setDescription("DAF Fitter using Genfit. If you have TrackCands, you have to convert them to RecoTracks"
                 "using the RecoTrackCreatorModule first. After that, you probably want to add the measurements from the"
                 "hits using the MeasurementCreatorModule.");

  addParam("probCut", m_param_probabilityCut,
           "Probability cut for the DAF. Any value between 0 and 1 is possible. Common values are between 0.01 and 0.001",
           TrackFitter::s_defaultProbCut);

  addParam("numberOfFailedHits", m_param_maxNumberOfFailedHits,
           "Maximum number of failed hits before aborting the fit.", static_cast<int>(TrackFitter::s_defaultMaxFailedHits));

  addParam("deltaPvalue", m_param_deltaPValue,
           "If the difference in p-value between two DAF iterations is smaller than this value, the iterative procedure will be terminated early.",
           /// This is the difference on pvalue between two fit iterations of the DAF procedure which
           /// is used as a early termination criteria of the DAF procedure. This is large on purpose
           /// See https://agira.desy.de/browse/BII-1725 for details
           TrackFitter::s_defaultDeltaPValue);
}

/** Create a DAF fitter */
std::shared_ptr<genfit::AbsFitter> DAFRecoFitterModule::createFitter() const
{
  if (m_param_deltaPValue != TrackFitter::s_defaultDeltaPValue or
      m_param_maxNumberOfFailedHits != TrackFitter::s_defaultMaxFailedHits or
      m_param_probabilityCut != TrackFitter::s_defaultProbCut) {
    std::shared_ptr<genfit::DAF> fitter = std::make_shared<genfit::DAF>(true, m_param_deltaPValue);
    fitter->setMaxFailedHits(m_param_maxNumberOfFailedHits);

    fitter->setProbCut(m_param_probabilityCut);

    return fitter;
  } else {
    // The user has not changed any parameters, so this is basically the default fitter. We return nullptr,
    // to not reset the fitter and make refitting unnecessary.
    return std::shared_ptr<genfit::DAF>(nullptr);
  }
}
