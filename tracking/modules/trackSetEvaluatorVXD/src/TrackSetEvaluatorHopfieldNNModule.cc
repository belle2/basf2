/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/TrackSetEvaluatorHopfieldNNModule.h"

#include <tracking/spacePointCreation/sptcNetwork/TrackSetEvaluatorHopfieldNN.h>


using namespace std;
using namespace Belle2;


REG_MODULE(TrackSetEvaluatorHopfieldNN)

TrackSetEvaluatorHopfieldNNModule::TrackSetEvaluatorHopfieldNNModule() : Module()
{
  InitializeCounters();

  //Set module properties
  setDescription("This module expects a container of SpacePointTrackCandidates and selects a subset of non-overlapping TCs determined using a neural network of Hopfield type.");

  addParam("writeToRoot", m_PARAMwriteToRoot, " if true, analysis data is written into a root file - standard is false", bool(false));
  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it", string(""));
  addParam("tcNetworkName", m_PARAMtcNetworkName, " sets the name of tcNetwork to loaded as a StoreObjPtr", string(""));

  if (m_PARAMwriteToRoot == false) { setPropertyFlags(c_ParallelProcessingCertified); }
}



void TrackSetEvaluatorHopfieldNNModule::event()
{
  m_eventCounter++;
  m_nTCsTotal += m_spacePointTrackCands.getEntries();
  checkMinMaxQI();
  m_nTCsOverlapping += m_tcNetwork->getNCompetitors();

  B2INFO("TrackSetEvaluatorHopfieldNNModule - in event " << m_eventCounter << ": got " << m_spacePointTrackCands.getEntries() <<
         " TC of which " << m_tcNetwork->getNCompetitors() << " are overlapping")

  m_tcNetwork->replaceTrackSetEvaluator(new TrackSetEvaluatorHopfieldNN<SPTCAvatar<TCCompetitorGuard>, TCCompetitorGuard>
                                        (m_tcNetwork->getNodes(), m_tcNetwork->getObserver()));

  unsigned int nTCsAlive = m_tcNetwork->cleanOverlaps();

  B2INFO("TrackSetEvaluatorHopfieldNNModule - in event " << m_eventCounter << ": after cleanOverlaps: network now looks like this:")
  m_tcNetwork->print();

  m_nFinalTCs += nTCsAlive;
  m_nRejectedTCs += m_spacePointTrackCands.getEntries() - nTCsAlive;


  // TODO inform SpacePointTrackCands about the decissions made here!
}



void TrackSetEvaluatorHopfieldNNModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("TrackSetEvaluatorHopfieldNNModule-endRun: " <<
         " nTCs per event: " << float(m_nTCsTotal)*invEvents <<
         ", nTCs overlapping per event: " << float(m_nTCsOverlapping)*invEvents <<
         " nFinalTCs per event: " << float(m_nFinalTCs)*invEvents <<
         ", nTCsRejected per event: " << float(m_nRejectedTCs)*invEvents <<
         "\n nTCs total: " << m_nTCsTotal <<
         ", nTCs overlapping total: " << m_nTCsOverlapping <<
         " nFinalTCs total: " << m_nFinalTCs <<
         ", nTCsRejected total: " << m_nRejectedTCs <<
         " highest/lowest QI found: " << m_maxQI << "/" << m_minQI)
}


void TrackSetEvaluatorHopfieldNNModule::InitializeCounters()
{
  m_eventCounter = 0;
  m_nTCsTotal = 0;
  m_nTCsCompatible = 0;
  m_nTCsOverlapping = 0;
  m_nFinalTCs = 0;
  m_nRejectedTCs = 0;
  m_minQI = std::numeric_limits<double>::max();
  m_maxQI = std::numeric_limits<double>::min();
}


void TrackSetEvaluatorHopfieldNNModule::checkMinMaxQI()
{
  for (const auto& aTC : m_spacePointTrackCands) {
    double tempQI = aTC.getQualityIndex();
    if (tempQI > m_maxQI) {
      m_maxQI = tempQI;
    }
    if (tempQI < m_minQI) {
      m_minQI = tempQI;
    }
  }
}
