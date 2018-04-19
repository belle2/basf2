/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/TrackSetEvaluatorHopfieldNNDEVModule.h"

#include "tracking/trackFindingVXD/trackSetEvaluator/HopfieldNetwork.h"

#include <numeric>

#include <TMatrixD.h>


using namespace Belle2;


REG_MODULE(TrackSetEvaluatorHopfieldNNDEV)

TrackSetEvaluatorHopfieldNNDEVModule::TrackSetEvaluatorHopfieldNNDEVModule() : Module()
{
  setDescription("This module expects a container of SpacePointTrackCandidates and an OverlapNetwork\
                  and thenselects a subset of non-overlapping TCs determined using a neural network of Hopfield type.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("NameSpacePointTrackCands", m_PARAMtcArrayName, "Sets the name of expected StoreArray with SpacePointTrackCand in it.",
           std::string(""));
  addParam("NameOverlapNetworks", m_PARAMtcNetworkName, "Sets the name of expected StoreArray<OverlapNetwork>.",
           std::string(""));
  addParam("minActivityState", m_minActivityState, "Sets the minimal value of activity (Neuron Value) for acceptance.",
           float(0.7));
}


void TrackSetEvaluatorHopfieldNNDEVModule::event()
{
  m_eventCounter++;
  //If no SpacePointTrackCands are available, later algorithms may crash.
  if (not m_spacePointTrackCands.getEntries()) {
    return;
  }
  m_nTCsTotal += m_spacePointTrackCands.getEntries();

  //Prepare the information for the actual HNN.
  std::vector<OverlapResolverNodeInfo> overlapResolverNodeInfos;
  overlapResolverNodeInfos.reserve(m_spacePointTrackCands.getEntries());

  for (const SpacePointTrackCand& sPTC : m_spacePointTrackCands) {
    overlapResolverNodeInfos.emplace_back(sPTC.getQualityIndicator(), sPTC.getArrayIndex(),
                                          m_overlapNetworks[0]->getOverlapForTrackIndex(sPTC.getArrayIndex()), 1.0);
  }

  //Performs the actual HNN.
  //As the parameter is taken as reference, the values are changed and can be reused below.
  HopfieldNetwork hopfieldNetwork;
  unsigned maxIterations = 20;
  if (hopfieldNetwork.doHopfield(overlapResolverNodeInfos, maxIterations) == maxIterations) {
    B2INFO("Hopfield Network failed converge.");
    m_nHopfieldFails++;
  }

  //Update tcs and kill those which were rejected by the Hopfield algorithm
  unsigned int nSurvivors = 0;
  for (const auto& overlapResolverNodeInfo : overlapResolverNodeInfos) {
    if (overlapResolverNodeInfo.activityState > m_minActivityState) {
      nSurvivors++;
      continue;
    }
    m_spacePointTrackCands[overlapResolverNodeInfo.trackIndex]->removeRefereeStatus(SpacePointTrackCand::c_isActive);
  }

  //Reporting stuff.
  if (nSurvivors == 0) {
    B2WARNING("Hopfield network - had no survivors!");
  }

  m_nFinalTCs += nSurvivors;
}


void TrackSetEvaluatorHopfieldNNDEVModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("TrackSetEvaluatorHopfieldNNDEVModule-endRun: " <<
         " nTCs per event: " << float(m_nTCsTotal)*invEvents <<
         " nFinalTCs per event: " << float(m_nFinalTCs)*invEvents <<
         "\n nTCs total: " << m_nTCsTotal <<
         ", nFinalTCs total: " << m_nFinalTCs <<
         ", number of times Hopfield did not succeed: " << m_nHopfieldFails);

  //After having evaluated the counters, we reset them to zero.
  m_eventCounter = 0;
  m_nTCsTotal = 0;
  m_nFinalTCs = 0;
  m_nHopfieldFails = 0;
}
