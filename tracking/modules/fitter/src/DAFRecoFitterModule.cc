/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/fitter/DAFRecoFitterModule.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

using namespace Belle2;

REG_MODULE(DAFRecoFitter);

DAFRecoFitterModule::DAFRecoFitterModule() : BaseRecoFitterModule()
{
  setDescription("DAF Fitter using Genfit. If you have TrackCands, you have to convert them to RecoTracks"
                 "using the RecoTrackCreatorModule first. After that, you probably want to add the measurements from the"
                 "hits using the MeasurementCreatorModule. Default parameters are read from the database.");

  addParam("probCut", m_param_probabilityCut,
           "Probability cut for the DAF. Any value between 0 and 1 is possible. Common values are between 0.01 and 0.001",
           TrackFitter::s_defaultProbCut);
}

/** Create a DAF fitter */
std::shared_ptr<genfit::AbsFitter> DAFRecoFitterModule::createFitter() const
{
  if (!m_DAFparameters.isValid())
    B2FATAL("DAF parameters are not available.");

  if (static_cast<float>(m_param_probabilityCut) != m_DAFparameters->getProbabilityCut()) {
    if (not m_changedParametersMessageWasShown) {
      // Only show this warning the first time a DAF instance is created to avoid spamming logs. Otherwise this warning is shown each event at least once in case of non-default DAF settings / DAF setting not from DB
      B2WARNING("DAF was called with a different probability cut than the database one (new: " << m_param_probabilityCut << " ; DB: " <<
                m_DAFparameters->getProbabilityCut() << " ). This new value will be used, the other parameters are read from the database");
      m_changedParametersMessageWasShown = true;
    }
    std::shared_ptr<genfit::DAF> fitter = std::make_shared<genfit::DAF>(m_DAFparameters->getAnnealingScheme(),
                                          m_DAFparameters->getMinimumIterations(),
                                          m_DAFparameters->getMaximumIterations(),
                                          m_DAFparameters->getMinimumIterationsForPVal(),
                                          true,
                                          m_DAFparameters->getDeltaPValue(),
                                          m_DAFparameters->getDeltaWeight(),
                                          m_param_probabilityCut);
    fitter->setMaxFailedHits(m_DAFparameters->getMaximumFailedHits());

    return fitter;
  } else {
    // The user has not changed any parameters, so this is basically the default fitter. We return nullptr,
    // to not reset the fitter and make refitting unnecessary.
    return std::shared_ptr<genfit::DAF>(nullptr);
  }
}
