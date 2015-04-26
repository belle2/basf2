/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/TrackSetEvaluatorGreedyModule.h"

#include <tracking/spacePointCreation/sptcNetwork/TrackSetEvaluatorGreedy.h>


using namespace std;
using namespace Belle2;


REG_MODULE(TrackSetEvaluatorGreedy)

TrackSetEvaluatorGreedyModule::TrackSetEvaluatorGreedyModule() : Module()
{
  InitializeCounters();

  //Set module properties
  setDescription("This module expects a container of SpacePointTrackCandidates and selects a subset of non-overlapping TCs determined using the Greedy algorithm.");

  addParam("writeToRoot", m_PARAMwriteToRoot, " if true, analysis data is written into a root file - standard is false", bool(false));
  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it", string(""));
  addParam("tcNetworkName", m_PARAMtcNetworkName, " sets the name of tcNetwork to loaded as a StoreObjPtr", string(""));

  if (m_PARAMwriteToRoot == false) { setPropertyFlags(c_ParallelProcessingCertified); }
}



void TrackSetEvaluatorGreedyModule::event()
{
  m_eventCounter++;
  m_nTCsTotal += m_spacePointTrackCands.getEntries();
  checkMinMaxQI();
  m_nTCsOverlapping += m_tcNetwork->getNCompetitors();

  B2INFO("TrackSetEvaluatorGreedyModule - in event " << m_eventCounter << ": got " << m_spacePointTrackCands.getEntries() <<
         " TC of which " << m_tcNetwork->getNCompetitors() << " are overlapping")

  m_tcNetwork->replaceTrackSetEvaluator(new TrackSetEvaluatorGreedy<SPTCAvatar<TCCompetitorGuard>, TCCompetitorGuard>
                                        (m_tcNetwork->getNodes(), m_tcNetwork->getObserver()));

  unsigned int nTCsAlive = m_tcNetwork->cleanOverlaps();

  m_nFinalTCs += nTCsAlive;
  m_nRejectedTCs += m_spacePointTrackCands.getEntries() - nTCsAlive;


  // TODO inform SpacePointTrackCands about the decissions made here!
}



void TrackSetEvaluatorGreedyModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("TrackSetEvaluatorGreedyModule-endRun: " <<
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


void TrackSetEvaluatorGreedyModule::InitializeCounters()
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


void TrackSetEvaluatorGreedyModule::checkMinMaxQI()
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
