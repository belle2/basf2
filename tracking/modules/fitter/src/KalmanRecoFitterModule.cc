/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/fitter/KalmanRecoFitterModule.h>
using namespace Belle2;

REG_MODULE(KalmanRecoFitter)

/** Module for the Kalman Fitter */
KalmanRecoFitterModule::KalmanRecoFitterModule() : BaseRecoFitterModule()
{
  setDescription("Kalman Fitter using Genfit. If you have TrackCands, you have to convert them to RecoTracks"
                 "using the RecoTrackCreatorModule first. After that, you probably want to add the measurements from the"
                 "hits using the MeasurementCreatorModule.");

  addParam("minimumIterations", m_param_minimumIterations,
           "Minimum number of iterations for the Kalman filter", static_cast<unsigned int>(3));
  addParam("maximumIterations", m_param_maximumIterations,
           "Maximum number of iterations for the Kalman filter", static_cast<unsigned int>(10));

  addParam("numberOfFailedHits", m_param_maxNumberOfFailedHits,
           "Maximum number of failed hits before aborting the fit.", static_cast<unsigned int>(5));
}


/** Create a Kalman fitter */
std::shared_ptr<genfit::AbsFitter> KalmanRecoFitterModule::createFitter() const
{
  std::shared_ptr<genfit::KalmanFitterRefTrack> fitter = std::make_shared<genfit::KalmanFitterRefTrack>();
  fitter->setMinIterations(m_param_minimumIterations);
  fitter->setMaxIterations(m_param_maximumIterations);
  fitter->setMaxFailedHits(m_param_maxNumberOfFailedHits);

  return fitter;
}
