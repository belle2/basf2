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
#include <tracking/spacePointCreation/sptcNetwork/TrackSetEvaluatorGreedy.h>


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
  unsigned int nTCs = m_spacePointTrackCands.getEntries();
  unsigned int nCompetitors = m_tcNetwork->getNCompetitors();
  unsigned int nCleanTCsAtStart = nTCs - nCompetitors;
  m_nTCsTotal += nTCs;
  checkMinMaxQI();
  m_nTCsOverlapping += nCompetitors;
  m_nTCsCleanAtStart += nCleanTCsAtStart;

  B2DEBUG(10, "TrackSetEvaluatorHopfieldNNModule - in event " << m_eventCounter << ": got " << nTCs <<
          " TC of which " << nCompetitors << " are overlapping")

  m_tcNetwork->replaceTrackSetEvaluator(new TrackSetEvaluatorHopfieldNN<SPTCAvatar<TCCompetitorGuard>, TCCompetitorGuard>
                                        (m_tcNetwork->getNodes(), m_tcNetwork->getObserver()));

  bool wasSuccessful = m_tcNetwork->cleanOverlaps();

  if (!wasSuccessful) {
    B2WARNING("TrackSetEvaluatorHopfieldNNModule - in event " << m_eventCounter <<
              ": Hopfield did not succeed! Now trying fallback solution: Greedy!!")
    m_nHopfieldFails++;

    m_tcNetwork->replaceTrackSetEvaluator(new TrackSetEvaluatorGreedy<SPTCAvatar<TCCompetitorGuard>, TCCompetitorGuard>
                                          (m_tcNetwork->getNodes(), m_tcNetwork->getObserver()));

    wasSuccessful = m_tcNetwork->cleanOverlaps();

    if (!wasSuccessful) { // not even greedy did work!
      B2ERROR("TrackSetEvaluatorGreedyModule - in event " << m_eventCounter <<
              ": greedy did not succeed! tracks were not successfully cleaned of overlaps!")
      m_completeFails++;
      return;
    }
  }
  m_totalQI += m_tcNetwork->accessEvaluator()->getTotalQI();
  m_totalSurvivingQI += m_tcNetwork->accessEvaluator()->getTotalSurvivingQI();

//   B2INFO("TrackSetEvaluatorHopfieldNNModule - in event " << m_eventCounter << ": after cleanOverlaps: network now looks like this:")
//   m_tcNetwork->print();

  m_nFinalTCs += m_tcNetwork->accessEvaluator()->getNSurvivors();
  m_nRejectedTCs += nTCs - m_tcNetwork->accessEvaluator()->getNSurvivors();
  // TODO inform SpacePointTrackCands about the decissions made here!
}



void TrackSetEvaluatorHopfieldNNModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("TrackSetEvaluatorHopfieldNNModule-endRun: " <<
         " nTCs per event: " << float(m_nTCsTotal)*invEvents <<
         ", nTCs clean at start per event: " << float(m_nTCsCleanAtStart)*invEvents <<
         ", nTCs overlapping per event: " << float(m_nTCsOverlapping)*invEvents <<
         " nFinalTCs per event: " << float(m_nFinalTCs)*invEvents <<
         ", nTCsRejected per event: " << float(m_nRejectedTCs)*invEvents <<
         "\n nTCs total: " << m_nTCsTotal <<
         ", nTCs clean at start total: " << m_nTCsCleanAtStart <<
         ", nTCs overlapping total: " << m_nTCsOverlapping <<
         ", nFinalTCs total: " << m_nFinalTCs <<
         ", nTCsRejected total: " << m_nRejectedTCs <<
         ", sum of QI total: " << m_totalQI <<
         ", sum of QI after cleanOverlaps: " << m_totalSurvivingQI <<
         ", highest/lowest QI found: " << m_maxQI << "/" << m_minQI <<
         ", number of times Hopfield (and Greedy) did not succeed: " << m_nHopfieldFails << "(" << m_completeFails << ")")
}


void TrackSetEvaluatorHopfieldNNModule::InitializeCounters()
{
  m_eventCounter = 0;
  m_nTCsTotal = 0;
  m_nTCsCleanAtStart = 0;
  m_nTCsOverlapping = 0;
  m_nFinalTCs = 0;
  m_nRejectedTCs = 0;
  m_totalQI = 0;
  m_totalSurvivingQI = 0;
  m_minQI = std::numeric_limits<double>::max();
  m_maxQI = std::numeric_limits<double>::min();
  m_nHopfieldFails = 0;
  m_completeFails = 0;
}


void TrackSetEvaluatorHopfieldNNModule::checkMinMaxQI()
{
  for (const auto& aTC : m_spacePointTrackCands) {
    if (aTC.hasRefereeStatus(SpacePointTrackCand::c_isActive) == false) continue;
    double tempQI = aTC.getQualityIndex();
    if (tempQI > m_maxQI) {
      m_maxQI = tempQI;
    }
    if (tempQI < m_minQI) {
      m_minQI = tempQI;
    }
  }
}
