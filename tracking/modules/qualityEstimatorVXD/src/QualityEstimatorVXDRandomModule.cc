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
}



void QualityEstimatorVXDRandomModule::event()
{
  m_eventCounter++;
  m_nTCsTotal += m_spacePointTrackCands.getEntries();


  // assign a random QI for each given SpacePointTrackCand
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
