/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *

 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <tracking/spacePointCreation/sptcNetwork/TrackSetEvaluatorBase.h>

#include <framework/logging/Logger.h>

#include <TMatrixT.h>
#include <TMatrixD.h>
#include <TMath.h>

#include <vector>
#include <list>
#include <functional>


namespace Belle2 {


  /** TrackSetEvaluator-Class for greedy algorithm.
   *
   * with given trackSet and manager for competitors, this class takes care that there are no competing tracks in the end.
   */
  template<class TCType, class CompetitorManagingType>
  class TrackSetEvaluatorHopfieldNN : public TrackSetEvaluatorBase<TCType, CompetitorManagingType> {
  protected:
    /** simple typedef to increase readability. BaseClass simply means the base class of this one */
    typedef TrackSetEvaluatorBase<TCType, CompetitorManagingType> BaseClass;

    /** parameter shall be in range [0;1], behavior described as 'omega' parameter in paper mentioned below (doHopfield)*/
    double m_tuneHopfield;

    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */


    /** DEBUG: mini helper function for printing. */
    std::string miniPrinter(std::vector<TCType*> vec)
    {
      std::string out;
      for (auto tc : vec) { out += "tc (alive:" + std::to_string(tc->isAlive()) + ") " + std::to_string(tc->getID()) + " got QI: " + std::to_string(tc->getTrackQuality()) + "\n" ; }
      return out;
    }


    /** main Hopfield neural network function, returns number of TCs died.
     *
     * Algorithm:
     * Neural network of Hopfield type as described in:
    * R. Frühwirth, “Selection of optimal subsets of tracks with a feedback neural network,”
    * C omput.Phys.Commun., vol. 78, pp. 23–28, 1993.*
     */
    unsigned int doHopfield(std::vector<TCType*>& overlappingTCs)
    {
      unsigned int countTCsAliveAtStart = overlappingTCs.size(), countSurvivors = 0, countKills = 0;
      double totalQI = 0, totalSurvivingQI = 0;
      using namespace std;

      B2DEBUG(25, "doHopfield - given TCs:\n" << miniPrinter(overlappingTCs))

//    TMatrixD W(countTCsAliveAtStart, countTCsAliveAtStart);  /// weight matrix, knows compatibility between each possible pair of TCs
//    TMatrixD xMatrix(1, countTCsAliveAtStart); /// Neuron values
//    TMatrixD xMatrixCopy(1, countTCsAliveAtStart); // copy of initial values
//    TMatrixD xMatrixOld(1, countTCsAliveAtStart);
//    TMatrixD actMatrix(1, countTCsAliveAtStart);
//    TMatrixD tempMatrix(1, countTCsAliveAtStart);
//    TMatrixD tempXMatrix(1, countTCsAliveAtStart);

      /** TODO
      *
      * commit stuff sofar (take care of non-working tests
      * continue with Hopfield stuff.
      * */


      B2DEBUG(50, "doHopfield: at end of greedy algoritm: total number of TCs alive: " << countTCsAliveAtStart <<
              " with totalQi: " << totalQI <<
              ", TCs survived: " << countSurvivors <<
              ", TCs killed: " << countKills <<
              ", survivingQI: " << totalSurvivingQI <<
              "\n Result:\n" << miniPrinter(overlappingTCs))
      return countKills;
    }


  public:
    /** constructor */
    TrackSetEvaluatorHopfieldNN(std::vector<TCType>& trackSet, CompetitorManagingType& manager,
                                double parameter = 0.5) : BaseClass(trackSet, manager), m_tuneHopfield(parameter) {}


    /** destructor */
    virtual ~TrackSetEvaluatorHopfieldNN() {}


    /** main function. returns number of final TCs */
    virtual unsigned int cleanOverlaps()
    {
      unsigned int nTCsDied = 0, nTCsAlive = 0;
      std::vector<TCType*> overlaps = BaseClass::getOverlappingTCs();

      // deal with simple cases first (contains many safety checks, which can therefore be ommitted by the actual algorithms):
      bool wasSimpleCase = BaseClass::dealWithSimpleCases(overlaps);

      if (wasSimpleCase) { return BaseClass::getNTCs() - overlaps.size() + 1; }

      // executing actual algorithm:
      nTCsAlive = doHopfield(overlaps);
      nTCsDied = BaseClass::getNTCs() - nTCsAlive;
      B2DEBUG(25, "TrackSetEvaluatorHopfieldNN::cleanOverlaps: nTCs alive/dead at end: " << nTCsAlive << "/" << nTCsDied)
      return nTCsAlive;
    }

  };

} // end namespace Belle2





/** ***** hopfield ***** **/
/// Neuronal network filtering overlapping Track Candidates by searching best subset of TC's
// void VXDTFModule::hopfield(TCsOfEvent& tcVector, double omega)
// {
/** REDESIGNCOMMENT HOPFIELD 1:
 * * short:
 *
 ** long (+personal comments):
 * that function has a lot of debug only output
 *
 ** dependency of module parameters (global):
 * m_PARAMdisplayCollector, m_PARAMDebugMode, m_PARAMnameOfInstance,
 * m_PARAMqiSmear
 *
 ** dependency of global in-module variables:
 * m_eventCounter, m_collector, m_TESTERbadHopfieldCtr,
 *
 *
 ** dependency of global stuff just because of B2XX-output or debugging only:
 * m_eventCounter, m_PARAMdisplayCollector, m_collector,
 * m_PARAMDebugMode, m_TESTERbadHopfieldCtr, m_PARAMnameOfInstance,
 * m_PARAMqiSmear
 *
 ** in-module-function-calls:
 * greedy(tcVector)
 */
//   int nTCs = tcVector.size();
//   if (nTCs < 2) { B2FATAL("Hopfield got only " << nTCs << " overlapping TCs! This should not be possible!"); return; }
//
//   // TMatrixD = MatrixT <double>
//   TMatrixD W(nTCs, nTCs);  /// weight matrix, knows compatibility between each possible pair of TCs
//   TMatrixD xMatrix(1, nTCs); /// Neuron values
//   TMatrixD xMatrixCopy(1, nTCs); // copy of initial values
//   TMatrixD xMatrixOld(1, nTCs);
//   TMatrixD actMatrix(1, nTCs);
//   TMatrixD tempMatrix(1, nTCs);
//   TMatrixD tempXMatrix(1, nTCs);
//
/// QI: wird in SPTCAvatar gespeichert:
//   vector<double> QIsOfTCs;
//   QIsOfTCs.reserve(nTCs);
//   for (VXDTFTrackCandidate* tc : tcVector) {
//  B2DEBUG(10, "hopfield, QI: " << tc->getTrackQuality())
//  QIsOfTCs.push_back(tc->getTrackQuality());
//   }
// //
//   int countCasesWhen2NeuronsAreCompatible = 0;
//   double compatibleValue = (1.0 - omega) / double(nTCs - 1);
//   list<int> hitsBoth, hitsItrk, hitsJtrk;
//   for (int itrk = 0; itrk < nTCs; itrk++) {
//
//  hitsItrk = tcVector[itrk]->getHopfieldHitIndices();
//  hitsItrk.sort();
//  int nHitsItrk = hitsItrk.size();
//
//  for (int jtrk = itrk + 1; jtrk < nTCs; jtrk++) {
//
//    hitsBoth = hitsItrk;
//    hitsJtrk = tcVector[jtrk]->getHopfieldHitIndices();
//    hitsJtrk.sort();
//    int nHitsJtrk = hitsJtrk.size();
//
//    hitsBoth.merge(hitsJtrk);
//
//    hitsBoth.unique();
//
//    int nHitsBoth = hitsBoth.size();
//
//    if (nHitsBoth < (nHitsItrk + nHitsJtrk)) { // means they share hits if true
//    W(itrk, jtrk) = -1.0;
//    W(jtrk, itrk) = -1.0;
//    } else {
//    W(itrk, jtrk) = compatibleValue;
//    W(jtrk, itrk) = compatibleValue;
//    countCasesWhen2NeuronsAreCompatible++;
//    }
//  }
//   } // checking compatibility of TCs (compatible if they dont share hits, not compatible if they share ones)
//
//   if (countCasesWhen2NeuronsAreCompatible == 0) {
//  B2DEBUG(2, "VXDTF event " << m_eventCounter << ": hopfield: no compatible neurons found, chosing TC by best QI...");
//  int tempIndex = 0;
//  double tempQI = tcVector[0]->getTrackQuality();
//  for (int i = 1; i < nTCs; i++) {
//    double tempQI2 = tcVector[i]->getTrackQuality();
//    if (tempQI < tempQI2) { tempIndex = i; }
//  }
//  for (int i = 0; i < nTCs; i++) {
//    if (i != tempIndex) {
//    tcVector[i]->setCondition(false);
//
//    // Update Collector TC - hopfield
//    if (m_PARAMdisplayCollector > 0) {
//      m_collector.updateTC(tcVector[i]->getCollectorID(), CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield, vector<int>(), {FilterID::hopfield});
//    }
//
//
//    }
//  }
//  return; // leaving hopfield after chosing the last man standing
//   }
//
//   if ((m_PARAMDebugMode == true) && (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 100, PACKAGENAME()) == true)) {
//  stringstream printOut;
//  printMyMatrix(W, printOut);
//  B2DEBUG(100, " weight matrix W: " << endl << printOut << endl);
//   }
//
//   vector<int> sequenceVector(nTCs, 0);
//
//   for (int i = 0; i < nTCs; i++) {
//  double rNum = gRandom->Uniform(1.0);
//  xMatrix(0, i) = rNum;
//  sequenceVector[i] = i;
//   }
//   xMatrixCopy = xMatrix;
//
//   double T = 3.1; // original: 3.1
//   double Tmin = 0.1;
//   double cmax = 0.01;
//   double c = 1.0;
//   double act = 0.0;
//
//   int nNcounter = 0;
//   std::array<double, 100> cValues; // protocolling all values of c
//   cValues.fill(0);
//
//   while (c > cmax) {
//
//  random_shuffle(sequenceVector.begin(), sequenceVector.end(), rngWrapper);
//
//  xMatrixOld = xMatrix;
//
//  for (int i : sequenceVector) {
//    double aTempVal = 0.0;
//    for (int a = 0; a < nTCs; a++) { aTempVal = aTempVal + W(i, a) * xMatrix(0, a); } // doing it by hand...
//
//    act = aTempVal + omega * QIsOfTCs[i];
//
//    xMatrix(0, i) = 0.5 * (1. + tanh(act / T));
//
//    B2DEBUG(100, "tc, random number " << i << " -  old value: " << xMatrix(0, i))
//  }
//
//  T = 0.5 * (T + Tmin);
//
//  tempMatrix = (xMatrix - xMatrixOld);
//  tempMatrix.Abs();
//  c = tempMatrix.Max();
//  B2DEBUG(10, " c value is " << c << " at iteration " << nNcounter)
//  cValues.at(nNcounter) = c;
//
//  xMatrixOld = xMatrix;
//
//  if (nNcounter == 99 || std::isnan(c) == true) {
//    stringstream cOutPut;
//    for (auto entry : cValues) { if (entry != 0) { cOutPut << entry << " ";} }
//    B2ERROR("Hopfield took " << nNcounter << " iterations or is nan, current c/cmax: " << c << "/" << cmax << " c-history: " <<
//    cOutPut.str()); break;
//  }
//  nNcounter++;
//   }
//
//   B2DEBUG(3, "Hopfield network - found subset of TCs within " << nNcounter << " iterations... with c=" << c);
//   list<VXDTFHit*> allHits;
//   int survivorCtr = 0;
//
//   for (int i = 0; i < nTCs; i++) { if (xMatrix(0, i) > 0.7) { survivorCtr++; } }
//
//   if (survivorCtr == 0) {
//  m_TESTERbadHopfieldCtr++;
//  B2DEBUG(3, "VXDTF event " << m_eventCounter << ": hopfield had no survivors! now using greedy... ")
//  greedy(tcVector); /// greedy
//  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 4, PACKAGENAME()) == true) {
//    for (int i = 0; i < nTCs; i++) {
//    B2DEBUG(4, "tc " << i << " - got final neuron value: " << xMatrix(0,
//                                      i) << " while having " << int((tcVector.at(i)->getHits()).size()) << " hits and quality indicator " << tcVector.at(
//                                      i)->getTrackQuality())
//    }
//  }
//
//  survivorCtr = 0;
//  for (VXDTFTrackCandidate* tc : tcVector) {
//    if (tc->getCondition() == true)  { survivorCtr++; }
//  } // should now have got some survivors
//   } else {
//  for (int i = 0; i < nTCs; i++) {
//    B2DEBUG(50, "tc " << i << " - got final neuron value: " << xMatrix(0,
//                                     i) << " while having " << int((tcVector.at(i)->getHits()).size()) << " hits and quality indicator " << tcVector.at(
//                                       i)->getTrackQuality())
//    if (xMatrix(0, i) > 0.7) { /// do we want to let this threshold hardcoded?
//    tcVector[i]->setCondition(true);
//
//    // Update Collector TC - hopfield
//    if (m_PARAMdisplayCollector > 0) {
//      m_collector.updateTC(tcVector[i]->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::hopfield}, vector<int>());
//    }
//
//
//    } else {
//    tcVector[i]->setCondition(false);
//
//    // Update Collector TC - hopfield
//    if (m_PARAMdisplayCollector > 0) {
//      m_collector.updateTC(tcVector[i]->getCollectorID(), CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield, vector<int>(), {FilterID::hopfield});
//    }
//
//    }
//    tcVector[i]->setNeuronValue(xMatrix(0, i));
//
//    bool condi = tcVector[i]->getCondition();
//    if (condi == true) {
//    const vector<VXDTFHit*>& currentHits = tcVector[i]->getHits();
//    for (int j = 0; j < int(currentHits.size()); ++j) { allHits.push_back(currentHits[j]); }
//    }
//  }
//   }
//
//   if (survivorCtr == 0) { // in this case the NN AND greedy could not find any compatible entries
//  B2WARNING(m_PARAMnameOfInstance << " smear:" << m_PARAMqiSmear << " event " << m_eventCounter <<
//  ": hopfield and greedy had no survivors! ")
//
//  /** file output: */
//  if (m_PARAMDebugMode == true) {
//    stringstream noSurvivors, qiVec, xMatrixBegin, xMatrixEnd, weightMatrix, fileName;
//    noSurvivors << "event " << m_eventCounter << endl;
//    qiVec << "qiVector " << endl;
//    xMatrixBegin << "neurons at start: " << endl;
//    xMatrixEnd << "neurons at end: " << endl;
//
//    printMyMatrix(W, weightMatrix);
//
//    for (int i = 0; i < nTCs; i++) {
//    qiVec << tcVector[i]->getTrackQuality() << " ";
//    xMatrixBegin << xMatrixCopy(0, i) << " ";
//    xMatrixEnd << xMatrix(0, i) << " ";
//
//    B2WARNING(m_PARAMnameOfInstance << " tc " << i << " - got final neuron value: " << xMatrix(0,
//                                                   i) << " while having " << int((tcVector.at(i)->getHits()).size()) << " hits and quality indicator " << tcVector.at(
//                                                   i)->getTrackQuality())
//    }
//
//    noSurvivors << xMatrixBegin.str() << endl << xMatrixEnd.str() << endl << qiVec.str() << endl << weightMatrix.str() << endl;
//    ofstream myfile;
//    fileName << "noSurvivors" << m_PARAMnameOfInstance << ".txt";
//    myfile.open((fileName.str()).c_str(), ios::out | ios::app);
//    myfile << noSurvivors.str();
//    myfile.close();
//  }
//  /** file output-end */
//   }
//   int sizeOld = allHits.size();
//   list<VXDTFHit*> tempHits = allHits;
//   allHits.sort(); allHits.unique();
//   int sizeNew = allHits.size();
//
//   if (sizeOld != sizeNew) { /// checks for strange behavior of NN
//  B2ERROR(m_PARAMnameOfInstance << ", NN smear" << m_PARAMqiSmear << " event " << m_eventCounter <<
//  ": illegal result! Overlapping TCs (with " << sizeOld - sizeNew << " overlapping hits) accepted!")
//  tempHits.sort();
//  int hctr = 0;
//  for (VXDTFHit* hit : tempHits) {
//    if (hit->getDetectorType() == Const::IR) {
//    B2ERROR("Even more illegal result: hit " << hctr << " is attached to interaction point! Perp/secID " <<
//    hit->getHitCoordinates()->Perp() << "/" << hit->getSectorString())
//    } else if (hit->getDetectorType() == Const::PXD) {
//    B2WARNING("hit " << hctr << " is a PXD-hit with clusterIndexUV: " << hit->getClusterIndexUV())
//    } else if (hit->getDetectorType() == Const::TEST) {
//    B2WARNING("hit " << hctr << " is a TEL-hit with clusterIndexUV: " << hit->getClusterIndexUV())
//    } else if (hit->getDetectorType() == Const::SVD) {
//    B2WARNING("hit " << hctr << " is a SVD-hit with clusterIndexU/V: " << hit->getClusterIndexU() << "/" << hit->getClusterIndexV())
//    } else {
//    B2ERROR("Most illegal result: hit " << hctr << " could not be attached to any detector!")
//    }
//    ++hctr;
//  }
//
//  stringstream overlappingTCsAccepted, qiVec, xMatrixBegin, xMatrixEnd, weightMatrix, fileName;
//  overlappingTCsAccepted << "event " << m_eventCounter << endl;
//  qiVec << "qiVector " << endl;
//  xMatrixBegin << "neurons at start: " << endl;
//  xMatrixEnd << "neurons at end: " << endl;
//
//  printMyMatrix(W, weightMatrix);
//
//  for (int i = 0; i < nTCs; i++) {
//    qiVec << tcVector[i]->getTrackQuality() << " ";
//    xMatrixBegin << xMatrixCopy(0, i) << " ";
//    xMatrixEnd << xMatrix(0, i) << " ";
//  }
//
//  overlappingTCsAccepted << xMatrixBegin.str() << endl << xMatrixEnd.str() << endl << qiVec.str() << endl << weightMatrix.str() <<
//  endl;
//  ofstream myfile;
//  fileName << "overlappingTCsAccepted" << m_PARAMnameOfInstance << ".txt";
//  myfile.open((fileName.str()).c_str(), ios::out | ios::app);
//  myfile << overlappingTCsAccepted.str();
//  myfile.close();
//   }
// }

