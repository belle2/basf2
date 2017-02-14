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

    bool doHopfield(std::vector<OverlapResolverNodeInfo>& overlapResolverNodeInfos)
    {
      //Start value for neurons if they are compatible.
      //Each compatible connection activates a node with this value.
      //As the sum of all the activations shall be less than one, we divide the
      //activiation by the total number of Nodes.
      //Incompatible Nodes get later minus one, which counteracts all activation,
      //if the incompatible Node is active.
      float compatibilityValue = (1.0 - m_omega) / static_cast<float>(overlapResolverNodeInfos.size() - 1);

      //Weight matrix; knows compatibility between each possible pair of Nodes
      TMatrixD W(overlapResolverNodeInfos.size(), overlapResolverNodeInfos.size());
      //A): Set all elements to compatible:
      for (unsigned int i = 0; i < overlapResolverNodeInfos.size(); i++) {
        for (unsigned int j = 0; i < overlapResolverNodeInfos.size(); i++) {
          W(i, j) = compatibilityValue;
        }
      }
      //B): Inform weight matrix elements of incompatible neurons:
      for (const auto& aTC : overlapResolverNodeInfos) {
        for (unsigned int overlapIndex : aTC.overlaps) {
          W(aTC.trackIndex, overlapIndex) = -1.0;
        }
      }

      // Neuron values
      TMatrixD xMatrix(1, overlapResolverNodeInfos.size());
      // randomize neuron values for first iteration:
      for (unsigned int i = 0; i < overlapResolverNodeInfos.size(); i++) {
        xMatrix(0, i) = gRandom->Uniform(1.0); // WARNING: original does Un(0;0.1) not Un(0;1)!
      }

      //Store for results from last round:
      TMatrixD xMatrixOld(1, overlapResolverNodeInfos.size());
      //Just to determine the new value for 'c'....
      TMatrixD tempMatrix(1, overlapResolverNodeInfos.size());

      //Order of execution for neuron values:
      std::vector<unsigned short> sequenceVector(overlapResolverNodeInfos.size());
      //iota fills the vector with 0, 1, 2, ... , (size-1)
      std::iota(sequenceVector.begin(), sequenceVector.end(), 0);

      //The following block will be evaluated to empty, if LOG_NO_B2DEBUG is defined:
      B2DEBUG(100, "sequenceVector with length " << sequenceVector.size());
      B2DEBUG(100, "Entries are from begin to end:");
      for (auto && entry : sequenceVector) {
        B2DEBUG(100, std::to_string(entry) + ", ");
      }

      //Store all values of c for protocolling:
      std::array<float, 100> cValues = {};
      unsigned nIterations = 0;

      //Iterate until change in weights is small:
      while (c > cmax) {

        std::shuffle(sequenceVector.begin(), sequenceVector.end(), TRandomWrapper());

        xMatrixOld = xMatrix;

        for (unsigned int i : sequenceVector) {
          float aTempVal = 0.0;
          for (unsigned int a = 0; a < overlapResolverNodeInfos.size(); a++) { aTempVal += W(i, a) * xMatrix(0, a); }

          float act = aTempVal + m_omega * overlapResolverNodeInfos[i].qualityIndex;

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
      for (unsigned int i = 0; i < overlapResolverNodeInfos.size(); i++) {
        B2DEBUG(50, "tc " << i <<
                " - got final neuron value: " << xMatrix(0, i) <<
                " and quality indicator " << overlapResolverNodeInfos[i].qualityIndex);
        overlapResolverNodeInfos[i].activityState = xMatrix(0, i);
      }

      return true;
    }

  private:
    float m_omega;
    float T = 3.1; // temperature for annealing, original: 3.1
    float Tmin = 0.1;
    float cmax = 0.01;
    float c = 1.0; // biggest difference in neuron values between two iterations

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

