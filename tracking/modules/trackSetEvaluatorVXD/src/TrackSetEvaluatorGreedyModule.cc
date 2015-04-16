/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/TrackSetEvaluatorGreedyModule.h"


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

  if (m_PARAMwriteToRoot == false) { setPropertyFlags(c_ParallelProcessingCertified); }
}



void TrackSetEvaluatorGreedyModule::event()
{
//   m_eventCounter++;
//
//   checkMinMaxQI();
//
//   unsigned int nTCsTotal = m_spacePointTrackCands.getEntries();
//   m_nTCsTotal += nTCsTotal;
//
//   if (nTCsTotal < 2) {
//  m_nTCsCompatible += nTCsTotal;
//  m_nFinalTCs += nTCsTotal;
//  return;
//   }
//
//   vector<SpacePointTrackCand*> overlappingTCs = findOverlappingTCs(); // TODO: outsource
//
//   unsigned int nTCsOverlapping = overlappingTCs.size();
//
//   if (nTCsOverlapping == 0) {
//  m_nTCsCompatible += nTCsTotal;
//  m_nFinalTCs += nTCsTotal;
//  return;
//   }
//
//   if (nTCsOverlapping == 1) {
//  B2ERROR("TrackSetEvaluatorGreedyModule - in event " << m_eventCounter << ": only one SpacePointTrackCand was overlapping (should not be possible!)")
//   }
//
//   if (nTCsOverlapping == 2) {
//
//  B2DEBUG(10, "TrackSetEvaluatorGreedyModule - in event " << m_eventCounter << ": 2 overlapping Track Candidates found, tcDuel choses the last TC standing on its own")
//  tcDuell(overlappingTCs);
//
//  m_nTCsOverlapping += 2;
//  m_nFinalTCs += nTCsTotal;
//  return;
//   }


}



void TrackSetEvaluatorGreedyModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("TrackSetEvaluatorGreedyModule-endRun: want to have some summary here... invEvents: " << invEvents) // TODO
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


/** ***** greedy ***** **/
/// search for nonOverlapping trackCandidates using Greedy algorithm (start with TC of highest QI, remove all TCs incompatible with current TC, if there are still TCs there, repeat step until no incompatible TCs are there any more)
// template <typename TCCollectionType, typename TCType>
// void TrackSetEvaluatorGreedyModule::greedy(typename TCCollectionType<TCType*>& tcVector)
// {
//   list< pair< double, TCType*> > overlappingTCs;
//
//   int countTCsAliveAtStart = 0, countSurvivors = 0, countKills = 0;
//   double totalSurvivingQI = 0, totalQI = 0;
//   for (VXDTFTrackCandidate* tc : tcVector) {  // store tcs in list of current overlapping TCs
//  ++countTCsAliveAtStart;
//  if (tc->getCondition() == false) continue;
//
//  double qi = tc->getTrackQuality();
//  totalQI += qi;
//
//  if (int(tc->getBookingRivals().size()) == 0) { // tc is clean and therefore automatically accepted
//    totalSurvivingQI += qi;
//    countSurvivors++;
//    continue;
//  }
//
//  overlappingTCs.push_back({qi, tc});
//   }
//
//   overlappingTCs.sort();
//   overlappingTCs.reverse();
//
//   greedyRecursive(overlappingTCs, totalSurvivingQI, countSurvivors, countKills);
//
//   B2DEBUG(3, "VXDTFModule::greedy: total number of TCs: " << tcVector.size() << " with totalQi " << totalQI <<
//   ", TCs alive at begin of greedy algoritm: " << countTCsAliveAtStart << ", TCs survived: " << countSurvivors << ", TCs killed: " <<
//   countKills)
// }


/** ***** greedyRecursive ***** **/
/// used by VXDTFModule::greedy, recursive function which takes tc with highest QI and kills all its rivals. After that, TC gets removed and process is repeated with shrinking list of TCs until no TCs alive has got rivals alive
// template <typename TCCollectionType, typename TCType>
// void TrackSetEvaluatorGreedyModule::greedyRecursive(TCCollectionType< std::pair<double, TCType*> >& overlappingTCs,
// void TrackSetEvaluatorGreedyModule::greedyRecursive(std::list< std::pair<double, VXDTFTrackCandidate*> >& overlappingTCs,
//                  double& totalSurvivingQI,
//                  int& countSurvivors,
//                  int& countKills)
// {
//   if (overlappingTCs.empty() == true) return;
//
//   list< pair<double, VXDTFTrackCandidate*> >::iterator tcEntry = overlappingTCs.begin();
//
//   while (tcEntry->second->getCondition() == false) {
//  tcEntry = overlappingTCs.erase(tcEntry);
//  if (tcEntry == overlappingTCs.end() or overlappingTCs.empty() == true) return;
//   }
//
//   double qi = tcEntry->first;
//
//   for (VXDTFTrackCandidate* rival : tcEntry->second->getBookingRivals()) {
//  if (rival->getCondition() == false) continue;
//
//  countKills++;
//
//  if (qi >= rival->getTrackQuality()) {
//    rival->setCondition(false);
//
//    // Update Collector TC - hopfield
//    if (m_PARAMdisplayCollector > 0) {
//    m_collector.updateTC(rival->getCollectorID(), CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield, vector<int>(), {FilterID::greedy});
//    }
//
//  } else {
//    tcEntry->second->setCondition(false);
//
//    // Update Collector TC - hopfield
//    if (m_PARAMdisplayCollector > 0) {
//    m_collector.updateTC(tcEntry->second->getCollectorID(),  CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield,
//               vector<int>(), {FilterID::greedy});
//    }
//
//
//    break;
//  }
//   }
//
//   if (tcEntry->second->getCondition() == true) {
//  countSurvivors++;
//  totalSurvivingQI += qi;
//   }
//
//   if (overlappingTCs.empty() != true) { overlappingTCs.pop_front(); }
//
//
//   greedyRecursive(overlappingTCs, totalSurvivingQI, countSurvivors, countKills);
//
//   return;
// }


/** ***** tcDuel ***** **/
/// for that easy situation we dont need the neuronal network or other algorithms for finding the best subset...
template <typename TCCollectionType>
void TrackSetEvaluatorGreedyModule::tcDuel(TCCollectionType& tcVector)
{
  if (tcVector.at(0)->getQualityIndex() > tcVector.at(1)->getQualityIndex()) {
    tcVector.at(1)->setInactive();
  } else {
    tcVector.at(0)->setInactive();
  }
}