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

#include <numeric>

#include <TMatrixD.h>


using namespace std;
using namespace Belle2;


REG_MODULE(TrackSetEvaluatorHopfieldNNDEV)

TrackSetEvaluatorHopfieldNNDEVModule::TrackSetEvaluatorHopfieldNNDEVModule() : Module()
{
  InitializeCounters();

  //Set module properties
  setDescription("This module expects a container of SpacePointTrackCandidates and an OverlapNetwork\
 and thenselects a subset of non-overlapping TCs determined using a neural network of Hopfield type.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it",
           string(""));
  addParam("tcNetworkName", m_PARAMtcNetworkName, " sets the name of expected StoreArray<OverlapNetwork>", string(""));

  addParam("reduceTCSetBeforeHNN", m_PARAMreduceTCSetBeforeHNN,
           "If true, only overlapping TCs are considered by the HNN, if false all TCs are considered,\
 including non-overlapping ones. WARNING at the moment only option(false) is supported!",
           bool(false));
}


void TrackSetEvaluatorHopfieldNNDEVModule::event()
{
  m_eventCounter++;
  unsigned int nTCs = m_spacePointTrackCands.getEntries();
  m_nTCsTotal += nTCs;

  std::vector<TcInfo4Hopfield> overlapTCinfo;
  if (m_PARAMreduceTCSetBeforeHNN) {
    B2FATAL(" TrackSetEvaluatorHopfieldNNDEVModule: option reduceTCSetBeforeHNN\
 is not yet supported - would you like to implement it?");
    overlapTCinfo = reduceOverlappingNetwork();
  } else {
    overlapTCinfo = convertOverlappingNetwork();
  }
  unsigned int nCompetitors = overlapTCinfo.size();
  m_nTCsOverlapping += nCompetitors;
  m_nTCsCleanAtStart += nTCs - nCompetitors;

  B2DEBUG(10, "TrackSetEvaluatorHopfieldNNDEVModule - in event " << m_eventCounter << ": got " << nTCs <<
          "TC of which " << nCompetitors << " are overlapping");

  // case selection: 0 there are no overlaps, do nothing, if there are two, do tcDuel being faster than HNN, else use HNN:
  if (nCompetitors == 0) {
    B2DEBUG(50, "TrackSetEvaluatorHopfieldNNDEVModule: of" << nTCs << " TCs no one is overlapping, skipping event");
    m_nFinalTCs += nTCs;
    return;
  } else if (nCompetitors == 2) {
    tcDuel(overlapTCinfo);
  } else {
    bool wasSuccsessful = doHopfield(overlapTCinfo);
    if (!wasSuccsessful) {
      B2WARNING("TrackSetEvaluatorHopfieldNNDEVModule: failed in cleaning overlaps!"); m_nHopfieldFails++; return;
    }
  }

  // update tcs and kill those which were rejected by the Hopfield algorithm
  unsigned int nSurvivors = 0;
  for (const auto& tcInfo : overlapTCinfo) {
    if (tcInfo.neuronValue > 0.7) { nSurvivors++; continue; }   // WARNING hardcoded value, taken from paper!

    m_spacePointTrackCands[tcInfo.realID]->removeRefereeStatus(SpacePointTrackCand::c_isActive);
  }

  if (nSurvivors == 0) {
    B2WARNING("Hopfield network - had no survivors of  " << nCompetitors << " overlaps!");
  }

  B2DEBUG(50, "executeAlgorithm Hopfield: at end of algoritm:\
 total number of formaly overlaping TCs alive: " << nSurvivors);

  m_nFinalTCs += nSurvivors;
  m_nRejectedTCs += nTCs - nSurvivors;
}



void TrackSetEvaluatorHopfieldNNDEVModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("TrackSetEvaluatorHopfieldNNDEVModule-endRun: " <<
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
         ", number of times Hopfield did not succeed: " << m_nHopfieldFails);
}


void TrackSetEvaluatorHopfieldNNDEVModule::InitializeCounters()
{
  m_eventCounter = 0;
  m_nTCsTotal = 0;
  m_nTCsCleanAtStart = 0;
  m_nTCsOverlapping = 0;
  m_nFinalTCs = 0;
  m_nRejectedTCs = 0;
  m_nHopfieldFails = 0;
}



std::vector<TrackSetEvaluatorHopfieldNNDEVModule::TcInfo4Hopfield> TrackSetEvaluatorHopfieldNNDEVModule::convertOverlappingNetwork()
{
  B2DEBUG(100, "TrackSetEvaluatorHopfieldNNDEVModule::convertOverlappingNetwork now");
  std::vector<TcInfo4Hopfield> overlappingTCs;

  for (const SpacePointTrackCand& aTC : m_spacePointTrackCands) {
    const vector<unsigned short>& currentOverlaps = m_overlapNetworks[0]->getOverlapForTrackIndex(
                                                      aTC.getArrayIndex()); // Jakob: WARNING only first
    //overlapNetwork is currently used! Could lead to problems in the long run! WARNING part 2: unsigned short is imho too dangerous! please increase to unsigned int!

    //  if (currentOverlaps.empty() or aTC.hasRefereeStatus(SpacePointTrackCand::c_isActive) == false) { continue; }

    overlappingTCs.push_back(TcInfo4Hopfield());
    TcInfo4Hopfield& tcInfo = overlappingTCs.back();
    tcInfo.reducedID = overlappingTCs.size() - 1;
    tcInfo.realID = aTC.getArrayIndex();
    tcInfo.competitorsIDs = currentOverlaps; // warning: does hard copy! TODO optimize!
    tcInfo.qi = aTC.getQualityIndex();
  }

  return overlappingTCs;
}



std::vector<TrackSetEvaluatorHopfieldNNDEVModule::TcInfo4Hopfield>
TrackSetEvaluatorHopfieldNNDEVModule::reduceOverlappingNetwork()
{
  B2DEBUG(100, "TrackSetEvaluatorHopfieldNNDEVModule::reduceToOverlappingNetwork now");
  std::vector<TcInfo4Hopfield> overlappingTCs;

  /* find out which TCs are actually overlapping and then collect only the overlapping ones.
   * They get a reduced index number only indexing overlapping TCs.
   * First: collect reduced entries, but realCompetitors (will be updated later)
   */
  for (const SpacePointTrackCand& aTC : m_spacePointTrackCands) {
    const vector<unsigned short>& currentOverlaps = m_overlapNetworks[0]->getOverlapForTrackIndex(
                                                      aTC.getArrayIndex());
    // WARNING only first overlapNetwork is currently used! Could lead to problems in the long run!
    // WARNING part 2: unsigned short is imho too dangerous! please increase to unsigned int!
    if (currentOverlaps.empty() or aTC.hasRefereeStatus(SpacePointTrackCand::c_isActive) == false) { continue; }

    overlappingTCs.push_back(TcInfo4Hopfield());
    TcInfo4Hopfield& tcInfo = overlappingTCs.back();
    tcInfo.reducedID = overlappingTCs.size() - 1;
    tcInfo.realID = aTC.getArrayIndex();
//  tcInfo.competitorsIDs = currentOverlaps; // warning: does hard copy! TODO optimize!
    //    tcInfo.neuronValue = aTC.getNeuronValue();
    tcInfo.qi = aTC.getQualityIndex();
  }

  // update realCompetitors
  unsigned short nOverlaps = overlappingTCs.size();
  unsigned nUpdated = 0, nOuterPassed = 0, nInnerPassed = 0;
  for (auto& currentInfo : overlappingTCs) {
//  const vector<unsigned short>& currentOverlaps = m_overlapNetworks[0]->getOverlapForTrackIndex(currentInfo.realID);
    unsigned short realID = currentInfo.realID;

    for (auto& otherInfo : overlappingTCs) {
      if (currentInfo.reducedID == otherInfo.reducedID) { continue; }
      nOuterPassed++;

      for (auto& iD : otherInfo.competitorsIDs) {
        nInnerPassed++;
        if (iD != realID) { continue; }
        iD = currentInfo.reducedID;
        nUpdated++;
        break;
      }
    }
  }
//   unsigned short nOverlaps = overlappingTCs.size();
//   unsigned nUpdated = 0, nOuterPassed = 0, nInnerPassed = 0;
//   for (auto& currentInfo : overlappingTCs) {
//  unsigned short realID = currentInfo.realID;
//
//  for (auto& otherInfo : overlappingTCs) {
//    if (currentInfo.reducedID == otherInfo.reducedID) { continue; }
//    nOuterPassed++;
//
//    for (auto& iD : otherInfo.competitorsIDs) {
//    nInnerPassed++;
//    if (iD != realID) { continue; }
//    iD = currentInfo.reducedID;
//    nUpdated++;
//    break;
//    }
//  }
//   }
  B2DEBUG(100, "reduceToOverlappingNetwork: nOverlaps " << nOverlaps << ", nUpdated: " << nUpdated << ", nOuterPassed: " <<
          nOuterPassed << ", nInnerPassed: " << nInnerPassed);
//   for (unsigned short newIndex = 0; newIndex < nOverlaps ; newIndex++) { // newIndex == overlappingTCs[newIndex].reducedID
//  unsigned short realID = overlappingTCs[newIndex].realID;
//  for (unsigned short i = 0; i < nOverlaps ; i++) { // checking now all other entries for competitors to newIndex-entry
//    if (i == newIndex) { continue; }
//
//    for (unsigned short compID = 0; compID < overlappingTCs[i].competitorsIDs.size() ; compID++) { // replace old (real) iD with reduced one
//    if (overlappingTCs[i].competitorsIDs[compID] != realID) { continue; }
//    overlappingTCs[i].competitorsIDs[compID] = newIndex;
//    break;
//    }
//  }
//   }

  for (auto& info : overlappingTCs) {
    std::string overlappers = "";
    for (auto& id : info.competitorsIDs) { overlappers += std::to_string(id) + " "; }
    B2DEBUG(1, "info with realID/reducedID " << info.realID << "/" << info.reducedID << " has " << info.competitorsIDs.size() <<
            " overlappers with ids: " << overlappers);
  }

  return overlappingTCs;
}



/** returns infos of overlapping TCs only (leaving out all the non-overlapping ones) and updated competitor-iDs TODO write a test for that, otherwise not trustorthy! */
// // // // // std::vector<TcInfo4Hopfield> getOverlappedNetwork(std::vector<TCType*>& overlappingTCs)
// // // // // {
// // // // //   unsigned nOverlaps = overlappingTCs.size();
// // // // //   std::vector<TcInfo4Hopfield> tcInfo(nOverlaps);
// // // // //
// // // // //   // collect reduced entries, but realCompetitors (will be updated later)
// // // // //   for (unsigned int newIndex = 0; newIndex < nOverlaps ; newIndex++) {
// // // // //  tcInfo[newIndex].reducedID = newIndex;
// // // // //  tcInfo[newIndex].realID = overlappingTCs[newIndex]->getID();
// // // // //  tcInfo[newIndex].competitorsIDs = BaseClass::m_manager.getCompetitors(tcInfo[newIndex].realID);
// // // // //  tcInfo[newIndex].neuronValue = overlappingTCs[newIndex]->getNeuronValue();
// // // // //  tcInfo[newIndex].qi = overlappingTCs[newIndex]->getTrackQuality();
// // // // //   }
// // // // //
// // // // //   // update realCompetitors
// // // // //   for (unsigned int newIndex = 0; newIndex < nOverlaps ; newIndex++) { // newIndex == tcInfo[newIndex].reducedID
// // // // //  unsigned int realID = tcInfo[newIndex].realID;
// // // // //  for (unsigned int i = 0; i < nOverlaps ; i++) { // checking now all other entries for competitors to newIndex-entry
// // // // //    if (i == newIndex) { continue; }
// // // // //
// // // // //    for (unsigned int compID = 0; compID < tcInfo[i].competitorsIDs.size() ; compID++) { // replace old (real) iD with reduced one
// // // // //    if (tcInfo[i].competitorsIDs[compID] != realID) { continue; }
// // // // //    tcInfo[i].competitorsIDs[compID] = newIndex;
// // // // //    break;
// // // // //    }
// // // // //  }
// // // // //   }
// // // // //
// // // // //   return tcInfo;
// // // // // }


bool TrackSetEvaluatorHopfieldNNDEVModule::doHopfield(std::vector<TrackSetEvaluatorHopfieldNNDEVModule::TcInfo4Hopfield>& tcs,
                                                      double omega)
{
  unsigned int nTCs = tcs.size();
  B2DEBUG(100, "TrackSetEvaluatorHopfieldNNDEVModule::doHopfield now with " << nTCs << " overlapping TCs");
  // WARNING hardcoded!
  double T = 3.1; // temperature for annealing, original: 3.1
  double Tmin = 0.1;
  double cmax = 0.01;
  double c = 1.0; // biggest difference in neuron values between two iterations
  double act = 0.0;
  // TODO: where to store/ get these values?
  double compatibilityValue = (1.0 - omega) / double(nTCs - 1); // start value for neurons if they are compatible

  TMatrixD W(nTCs, nTCs);  /// weight matrix, knows compatibility between each possible pair of TCs
  // first: set all elements to compatible:
  for (unsigned int i = 0; i < nTCs; i++) {
    for (unsigned int j = 0; i < nTCs; i++) {
      W(i, j) = compatibilityValue;
    }
  }
  // second: inform weight matrix elements of incompatible neurons:
  for (const auto& aTC : tcs) {
    for (unsigned int competitorID : aTC.competitorsIDs) {
      W(aTC.reducedID, competitorID) = -1.0;
    }
  }

  TMatrixD xMatrix(1, nTCs); /// Neuron values
  // randomize neuron values for first iteration:
  for (unsigned int i = 0; i < nTCs; i++) {
    xMatrix(0, i) = gRandom->Uniform(1.0); // WARNING: original does Un(0;0.1) not Un(0;1)!
  }

  TMatrixD xMatrixOld(1, nTCs); /// stores results from last round
  TMatrixD tempMatrix(1, nTCs); /// just to determine the new value for 'c'

  // order of execution for neuron values:
  std::vector<unsigned int> sequenceVector(nTCs);
  std::iota(sequenceVector.begin(), sequenceVector.end(), 0);
  std::string cOutPut;
  for (unsigned entry : sequenceVector) { cOutPut += std::to_string(entry) + " "; }
  B2DEBUG(100, "sequenceVector with length " << sequenceVector.size() <<
          " created, entries are from begin to end: " << cOutPut);

  int nIterations = 0;
  std::array<double, 100> cValues; // protocolling all values of c
  cValues.fill(0);

  while (c > cmax) {

    std::shuffle(sequenceVector.begin(), sequenceVector.end(), TRandomWrapper());

    xMatrixOld = xMatrix;

    for (unsigned int i : sequenceVector) {
      double aTempVal = 0.0;
      for (unsigned int a = 0; a < nTCs; a++) { aTempVal += W(i, a) * xMatrix(0, a); } // doing it by hand...

      act = aTempVal + omega * tcs[i].qi;

      xMatrix(0, i) = 0.5 * (1. + tanh(act / T));

      B2DEBUG(100, "tc, random number " << i << " -  old neuron value: " << xMatrix(0, i));
    }

    T = 0.5 * (T + Tmin);

    tempMatrix = (xMatrix - xMatrixOld);
    tempMatrix.Abs();
    c = tempMatrix.Max();
    B2DEBUG(10, " c value is " << c << " at iteration " << nIterations);
    cValues.at(nIterations) = c;

    xMatrixOld = xMatrix;

    if (nIterations == 99 || std::isnan(c) == true) {
      std::string cOutPut;
      for (double entry : cValues) { if (entry != 0) { cOutPut += std::to_string(entry) + " ";} }
      B2ERROR("Hopfield took " << nIterations <<
              " iterations or is nan, current c/cmax: " << c <<
              "/" << cmax <<
              " and c-history: " << cOutPut);
      return false;
    }
    nIterations++;
  }

  B2DEBUG(3, "Hopfield network - found subset of TCs within " << nIterations << " iterations... with c=" << c);

  // update neuronValues:
  for (unsigned int i = 0; i < nTCs; i++) {
    B2DEBUG(50, "tc " << i <<
            " - got final neuron value: " << xMatrix(0, i) <<
            " and quality indicator " << tcs[i].qi);
    tcs[i].neuronValue = xMatrix(0, i);
  }

  return true;
}



/** to be called if only two TCs are overlapping */
void TrackSetEvaluatorHopfieldNNDEVModule::tcDuel(std::vector<TrackSetEvaluatorHopfieldNNDEVModule::TcInfo4Hopfield>&
                                                  overlappingTCs)
{
  B2DEBUG(25, "TrackSetEvaluator::tcDuel: first competitor got QI: " << overlappingTCs.front().qi <<
          ", and second one got: " << overlappingTCs.back().qi);
  if (overlappingTCs.front().qi > overlappingTCs.back().qi) {
    overlappingTCs.front().neuronValue = 1.;
    overlappingTCs.back().neuronValue = 0.;
  } else {
    overlappingTCs.front().neuronValue = 0.;
    overlappingTCs.back().neuronValue = 1.;
  }
}
