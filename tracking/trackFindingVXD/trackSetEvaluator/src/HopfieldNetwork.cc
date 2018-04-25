/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2017  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingVXD/trackSetEvaluator/HopfieldNetwork.h>
#include <Eigen/Dense>

using namespace Belle2;

short HopfieldNetwork::doHopfield(
  std::vector<OverlapResolverNodeInfo>& overlapResolverNodeInfos, unsigned short nIterations)
{
  //Start value for neurons if they are compatible.
  //Each compatible connection activates a node with this value.
  //As the sum of all the activations shall be less than one, we divide the
  //activiation by the total number of Nodes.
  //Incompatible Nodes get later minus one, which counteracts all activation,
  //if the incompatible Node is active.
  if (overlapResolverNodeInfos.size() < 2) {
    B2DEBUG(20, "No reason to doHopfield with less than 2 nodes!");
    return -1;
  }

  const float compatibilityValue = (1.0 - m_omega) / static_cast<float>(overlapResolverNodeInfos.size() - 1);

  const size_t overlapSize = overlapResolverNodeInfos.size();

  //Weight matrix; knows compatibility between each possible pair of Nodes
  Eigen::MatrixXd W(overlapSize, overlapSize);
  //A): Set all elements to compatible:
  W.fill(compatibilityValue);

  //B): Inform weight matrix elements of incompatible neurons:
  for (const auto& aTC : overlapResolverNodeInfos) {
    for (unsigned int overlapIndex : aTC.overlaps) {
      W(aTC.trackIndex, overlapIndex) = -1.0;
    }
  }


  // Neuron values
  Eigen::VectorXd x(overlapSize);
  // randomize neuron values for first iteration:
  for (unsigned int i = 0; i < overlapSize; i++) {
    x(i) = gRandom->Uniform(1.0); // WARNING: original does Un(0;0.1) not Un(0;1)!
  }

  //Store for results from last round:
  Eigen::VectorXd xOld(overlapSize);

  //Order of execution for neuron values:
  std::vector<unsigned short> sequenceVector(overlapSize);
  //iota fills the vector with 0, 1, 2, ... , (size-1)
  std::iota(sequenceVector.begin(), sequenceVector.end(), 0);

  //The following block will be evaluated to empty, if LOG_NO_B2DEBUG is defined:
  B2DEBUG(100, "sequenceVector with length " << sequenceVector.size());
  B2DEBUG(100, "Entries are from begin to end:");
  for (auto && entry : sequenceVector) {
    B2DEBUG(100, std::to_string(entry) + ", ");
  }

  //Store all values of c for protocolling:
  std::vector<float> cValues(nIterations);
  //Store for maximum change of weights between iterations.
  float c = 1.0;

  //Iterate until change in weights is small:
  unsigned iIterations = 0;

  float T = m_T;


  while (c > m_cmax) {
    std::shuffle(sequenceVector.begin(), sequenceVector.end(), TRandomWrapper());

    xOld = x;

    for (unsigned int i : sequenceVector) {
      float aTempVal = W.row(i).dot(x);
      float act = aTempVal + m_omega * overlapResolverNodeInfos[i].qualityIndicator;
      x(i) = 0.5 * (1. + tanh(act / T));
    }

    T = 0.5 * (T + m_Tmin);

    //Determine maximum change in weights:
    c = (x - xOld).cwiseAbs().maxCoeff();
    B2DEBUG(10, "c value is " << c << " at iteration " << iIterations);
    cValues.at(iIterations) = c;

    if (iIterations + 1 == nIterations) {
      B2INFO("Hopfield reached maximum number of iterations without convergence. cValues are:");
      for (auto && entry : cValues) {
        B2INFO(std::to_string(entry));
      }
      break;
    }
    iIterations++;
  }

  //Copy Node values into the activity state of the OverlapResolverNodeInfo objects:
  for (unsigned int i = 0; i < overlapSize; i++) {
    overlapResolverNodeInfos[i].activityState = x(i);
  }

  return iIterations;
}
