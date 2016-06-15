/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackSetEvaluatorVXD/SPTCNetworkProducerModule.h>
#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;


REG_MODULE(SPTCNetworkProducer)

SPTCNetworkProducerModule::SPTCNetworkProducerModule() : Module()
{
  InitializeCounters();

  //Set module properties
  setDescription("This module builds a network of SpacePointTrackCands and keeps track of their connections (overlaps) during evaluation phase.");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it", string(""));
  addParam("tcNetworkName", m_PARAMtcNetworkName, " sets the name of tcNetwork to be created as a StoreObjPtr", string(""));
  addParam("checkSPsInsteadOfClusters", m_PARAMcheckSPsInsteadOfClusters,
           "if true, overlaps are checked via SpacePoints. If false, overlaps are checked via clusters", bool(false));
}



void SPTCNetworkProducerModule::event()
{
  m_eventCounter++;
  m_nTCsTotal += m_spacePointTrackCands.getEntries();

  // prepare the tcNetwork
  m_tcNetwork.construct(m_PARAMcheckSPsInsteadOfClusters);

  B2DEBUG(10, "SPTCNetworkProducer:event " << m_eventCounter
          << ": tcArray with name " << m_PARAMtcArrayName
          << " got " << m_spacePointTrackCands.getEntries()
          << " SPTCs!\n"
          << ", mode to compare TCs is set to " << m_tcNetwork->getCompareTCsMode());

  // fill the tcNetwork
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {
    m_tcNetwork->add(aTC);
  }

  B2DEBUG(10, "SPTCNetworkProducer:event " << m_eventCounter
          << ": tcNetwork " << m_tcNetwork.getName()
          << " has " << m_tcNetwork->size()
          << " nodes (TCs), of which " << m_tcNetwork->getNTCsAlive()
          << "are still alive and " << m_tcNetwork->getNCompetitors()
          << "are linked to each other (overlapping)..."
         );

  B2DEBUG(10, "in event " << m_eventCounter << " the network looks like this:");
//   m_tcNetwork->print();

  m_nNodesNetwork += m_tcNetwork->size();
  m_nLinksNetwork += m_tcNetwork->getNCompetitors();
}



void SPTCNetworkProducerModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("SPTCNetworkProducerModule:endRun: events: " << m_eventCounter
         << ", nSPTCsPerEvent: " << invEvents * float(m_nTCsTotal)
         << ", nNodesPerEvent: " << invEvents * float(m_nNodesNetwork)
         << ", nLinksPerEvent: " << invEvents * float(m_nLinksNetwork)
        );
}



void SPTCNetworkProducerModule::InitializeCounters()
{
  m_eventCounter = 0;
  m_nTCsTotal = 0;
  m_nNodesNetwork = 0;
  m_nLinksNetwork = 0;
}
