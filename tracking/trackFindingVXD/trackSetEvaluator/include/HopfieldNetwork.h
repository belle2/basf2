/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//basf2
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>
#include <framework/logging/Logger.h>

//STL
#include <numeric>

//ROOT
#include <TMatrixD.h>
#include <TRandom.h>


namespace Belle2 {

  /** Hopfield Algorithm with number based inputs.
   *
   *  This class encapsulates the actual algorithm with pure number inputs
   *  (instead of objects of TrackCandidates or whatever).
   *  The reason for this is mainly better testability and potentially as well
   *  reusability.
   *  The development of this class started as a copy from Jakob's implementation for
   *  the VXDTF2.
   *
   *  Relevant resources:
   *  [1] R. Frühwirth, "Selection of optimal subsets of tracks with a feedback neural network",
   *  Comput. Phys. Commun., vol. 78, pp. 23–28, 1993.
   */
  class HopfieldNetwork {
  public:
    HopfieldNetwork(): m_omega(0.5) //InitializerList
    {}

    bool doHopfield(std::vector<OverlapResolverNodeInfo>& tcs)
    {
      unsigned int nTCs = tcs.size();
      B2DEBUG(100, "TrackSetEvaluatorHopfieldNNDEVModule::doHopfield now with " << nTCs << " overlapping TCs");

      float compatibilityValue = (1.0 - m_omega) / double(nTCs - 1); // start value for neurons if they are compatible

      TMatrixD W(nTCs, nTCs);  /// weight matrix, knows compatibility between each possible pair of TCs
      // first: set all elements to compatible:
      for (unsigned int i = 0; i < nTCs; i++) {
        for (unsigned int j = 0; i < nTCs; i++) {
          W(i, j) = compatibilityValue;
        }
      }

      // second: inform weight matrix elements of incompatible neurons:
      for (const auto& aTC : tcs) {
        for (unsigned int overlapIndex : aTC.overlaps) {
          W(aTC.trackIndex, overlapIndex) = -1.0;
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

          act = aTempVal + m_omega * tcs[i].qualityIndex;

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

      // update activityStates:
      for (unsigned int i = 0; i < nTCs; i++) {
        B2DEBUG(50, "tc " << i <<
                " - got final neuron value: " << xMatrix(0, i) <<
                " and quality indicator " << tcs[i].qualityIndex);
        tcs[i].activityState = xMatrix(0, i);
      }

      return true;
    }

  private:
    float m_omega;
    float T = 3.1; // temperature for annealing, original: 3.1
    float Tmin = 0.1;
    float cmax = 0.01;
    float c = 1.0; // biggest difference in neuron values between two iterations
    float act = 0.0;

    //--- Structs to help simplify the process ------------------------------------------------------------------------
    /** Wrap TRandom to be useable as a uniform random number generator with std algorithms like std::shuffle. */
    struct TRandomWrapper {
      /** Define the result type to be a normal unsigned int. */
      typedef unsigned int result_type;

      /** Minimum value returned by the random number generator. */
      static constexpr result_type min() { return 0; }

      /** Maximum value returned by the random number generator. */
      static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

      /** Return a random value in the range [min(), max()]. */
      result_type operator()() { return gRandom->Integer(max()); }
    };
  };
}

