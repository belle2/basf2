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

#include <TRandom.h>

#include <array>
#include <vector>
#include <limits>
#include <list>
#include <functional>
#include <algorithm>    // std::shuffle
#include <numeric>      // std::iota


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

    /** ************************* INTERNAL CLASS DEFINITIONS ************************* */


    /** small container to carry all the info needed for Hopfield algorithm*/
    struct TcInfo4Hopfield {
      /** new ID the TC used for the reduced network only. Reduced network only contains overlapping tcs for faster algorithm execution */
      unsigned int reducedID;

      /** the corresponding iD of the TC in the real network, allows access to TC and competitor */
      unsigned int realID;

      /** carries iDs for competitors, after creation the reduced iDs can be found here */
      std::vector<unsigned int>competitorsIDs;

      /** stores the neuron value */
      double neuronValue;

      /** quality indicator (higher is better) */
      double qi;
    };


    /** Wrap TRandom to be useable as a uniform random number generator with std
     * algorithms like std::shuffle. */
    struct TRandomWrapper {
      /** define the result type to be a normal unsigned int */
      typedef unsigned int result_type;

      /** minimum value returned by the random number generator */
      static constexpr result_type min() { return 0; }

      /** maximum value returned by the random number generator */
      static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

      /** return a random value in the range [min(), max()]. */
      result_type operator()() { return gRandom->Integer(max()); }
    };


    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */


    /** prepares everything for Hopfield and does some simple validation checks on the resul.
    * returns true if everything was fine, false if not
     */
    bool executeAlgorithm(std::vector<TCType*>& overlappingTCs)
    {
      B2DEBUG(25, "doHopfield - given TCs:\n" << BaseClass::miniPrinter(overlappingTCs));

      /** TODO
      * commit stuff sofar (take care of non-working tests
      * continue with Hopfield stuff.
      * */
      std::vector<TcInfo4Hopfield> reducedInfo = getOverlappedNetwork(overlappingTCs);

      bool wasSuccsessful = doHopfield(reducedInfo);

      if (wasSuccsessful == false) { return false; }

      /** TODO: verify output and stuff */

      // update tcs and kill those which were rejected by the Hopfield algorithm
      unsigned int nSurvivors = 0;
      for (const auto& tcInfo : reducedInfo) {
        BaseClass::m_trackSet[tcInfo.realID].setNeuronValue(tcInfo.neuronValue);

        if (tcInfo.neuronValue > 0.7) { nSurvivors++; continue; }   // WARNING hardcoded value!

        BaseClass::m_trackSet[tcInfo.realID].setAliveState(false);
      }

      if (nSurvivors == 0) {
        B2DEBUG(3, "Hopfield network - had no survivors! " << BaseClass::checkAtEnd() << "\n Overview:\n" << BaseClass::miniPrinter(
                  overlappingTCs));
        return false;
      }

      B2DEBUG(50, "executeAlgorithm Hopfield: at end of algoritm: total number of TCs alive: " << BaseClass::checkAtEnd() <<
              "\n Overview:\n" << BaseClass::miniPrinter(overlappingTCs));
      return true;
    }


    /** returns infos of overlapping TCs only (leaving out all the non-overlapping ones) and updated competitor-iDs TODO write a test for that, otherwise not trustorthy! */
    std::vector<TcInfo4Hopfield> getOverlappedNetwork(std::vector<TCType*>& overlappingTCs)
    {
      unsigned nOverlaps = overlappingTCs.size();
      std::vector<TcInfo4Hopfield> tcInfo(nOverlaps);

      // collect reduced entries, but realCompetitors (will be updated later)
      for (unsigned int newIndex = 0; newIndex < nOverlaps ; newIndex++) {
        tcInfo[newIndex].reducedID = newIndex;
        tcInfo[newIndex].realID = overlappingTCs[newIndex]->getID();
        tcInfo[newIndex].competitorsIDs = BaseClass::m_manager.getCompetitors(tcInfo[newIndex].realID);
        tcInfo[newIndex].neuronValue = overlappingTCs[newIndex]->getNeuronValue();
        tcInfo[newIndex].qi = overlappingTCs[newIndex]->getTrackQuality();
      }

      // update realCompetitors
      for (unsigned int newIndex = 0; newIndex < nOverlaps ; newIndex++) { // newIndex == tcInfo[newIndex].reducedID
        unsigned int realID = tcInfo[newIndex].realID;
        for (unsigned int i = 0; i < nOverlaps ; i++) { // checking now all other entries for competitors to newIndex-entry
          if (i == newIndex) { continue; }

          for (unsigned int compID = 0; compID < tcInfo[i].competitorsIDs.size() ; compID++) { // replace old (real) iD with reduced one
            if (tcInfo[i].competitorsIDs[compID] != realID) { continue; }
            tcInfo[i].competitorsIDs[compID] = newIndex;
            break;
          }
        }
      }

      return tcInfo;
    }


    /** Hopfield neural network function, returns true if it was successful.
     *
     * Algorithm:
     * Neural network of Hopfield type as described in:
     * R. Frühwirth, “Selection of optimal subsets of tracks with a feedback neural network,”
     * C omput.Phys.Commun., vol. 78, pp. 23–28, 1993.*
     */
    bool doHopfield(std::vector<TcInfo4Hopfield>& tcs, double omega = 0.5)
    {
      unsigned int nTCs = tcs.size();
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
        xMatrix(0, i) = gRandom->Uniform(1.0);
      }

      TMatrixD xMatrixOld(1, nTCs); /// stores results from last round
      TMatrixD tempMatrix(1, nTCs); /// just to determine the new value for 'c'

      // order of execution for neuron values:
      std::vector<unsigned int> sequenceVector(nTCs);
      std::iota(sequenceVector.begin(), sequenceVector.end(), 0);
      std::string cOutPut;
      for (unsigned entry : sequenceVector) { cOutPut += std::to_string(entry) + " "; }
      B2DEBUG(100, "sequenceVector with length " << sequenceVector.size() << " created, entries are from begin to end: " << cOutPut);

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


  public:
    /** constructor */
    TrackSetEvaluatorHopfieldNN(std::vector<TCType>& trackSet, CompetitorManagingType& manager,
                                double parameter = 0.5) : BaseClass(trackSet, manager), m_tuneHopfield(parameter) {}


    /** destructor */
    virtual ~TrackSetEvaluatorHopfieldNN() {}


    /** main function. returns true, if clean overlaps was successfull and returns false if not.
    *
    * after executing cleanOverlaps, the counters are updated.
    * These counters can give you a clue why there was a problem.
    */
    virtual bool cleanOverlaps()
    {
      std::string result = BaseClass::checkAtStart();
      B2DEBUG(25, "TrackSetEvaluatorHopfieldNN::cleanOverlaps: " << result);

      std::vector<TCType*> overlaps = BaseClass::getOverlappingTCs();
      unsigned int nOverlaps = overlaps.size();

      if (nOverlaps == 0) return true; // nothing to be done, no checks needed

      // deal with simple cases first (contains many safety checks, which can therefore be ommitted by the actual algorithms):
      bool wasSimpleCase = BaseClass::dealWithSimpleCases(overlaps);

      if (wasSimpleCase) {
        return BaseClass::doSanityChecks(true, overlaps.size(), "was easy case");
      }

      // executing actual algorithm:
      bool wasSuccsessfull = executeAlgorithm(overlaps);

      return BaseClass::doSanityChecks(wasSuccsessfull, nOverlaps, "Hopfield neural network");
    }
  };

} // end namespace Belle2
