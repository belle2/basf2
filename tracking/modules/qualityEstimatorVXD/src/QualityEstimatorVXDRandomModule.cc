/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorVXDRandomModule.h>
#include <framework/logging/Logger.h>
#include <genfit/TrackCand.h>

#include <TRandom.h>


using namespace std;
using namespace Belle2;


REG_MODULE(QualityEstimatorVXDRandom)

QualityEstimatorVXDRandomModule::QualityEstimatorVXDRandomModule() : Module()
{
  InitializeCounters();

  //Set module properties
  setDescription("The quality estimator module for SpacePointTrackCandidates using random values.");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it", string(""));

  addParam("useTimeSeedAsQI", m_PARAMuseTimeSeedAsQI,
           "JKL - WARNING evil hack: uses an ugly workaround to be able to use realistically determined quality indicators (TFRedesignModule fills the TimeSeed with its determined QI) ",
           false);
}



void QualityEstimatorVXDRandomModule::event()
{
  m_eventCounter++;
  B2DEBUG(1, "\n" << "QualityEstimatorVXDRandomModule:event: event " << m_eventCounter << "\n")
  m_nTCsTotal += m_spacePointTrackCands.getEntries();


  // assign a random QI for each given SpacePointTrackCand
  if (m_PARAMuseTimeSeedAsQI) {
    for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {
      double qi = aTC.getRelatedTo<genfit::TrackCand>("ALL")->getTimeSeed();
      B2DEBUG(1, "in event " << m_eventCounter << ", for aTC " << aTC.getArrayIndex() << ": retrieved QI using getTimeSeed is " << qi);
      aTC.setQualityIndex(qi);
    }
    return;
  }

  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {
    aTC.setQualityIndex(gRandom->Uniform(1.0));
  }
}



void QualityEstimatorVXDRandomModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("QualityEstimatorVXDRandomModule:endRun: events: " << m_eventCounter
         << ", nSPTCsPerEvent: " << invEvents * float(m_nTCsTotal)
         << ", random number for check of deterministic behavior: " << gRandom->Uniform(1.0)
        )
}



void QualityEstimatorVXDRandomModule::InitializeCounters()
{
  m_eventCounter = 0;
  m_nTCsTotal = 0;
}
