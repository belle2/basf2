/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstdlib>

#include <TFile.h>
#include <TObjArray.h>

#include "trg/cdc/dataobjects/CDCTriggerHoughMLP.h"
#include "trg/cdc/NeuroTrigger3DH.h"

using namespace Belle2;

// Converts a .root network to a plaintext integerized .dat file for FPGA implementation
int main(int argc, const char* argv[])
{
  if (argc != 3) {
    std::cout << "Program requires the following 2 arguments:\n"
              << " 1: mlp rootfile\n"
              << " 2: output filename\n";
    return -1;
  }
  const std::string fileNameMLP = argv[1];
  const std::string fileNameDAT = argv[2];

  const CDCTrigger3DHMLP& mlp = CDCTrigger3DHMLP::loadMLPFromFile<CDCTrigger3DHMLP>(fileNameMLP, "MLP");
  const NeuroParametersHough neuroParameters3DH = mlp.getNeuroParameters();

  NeuroTrigger3DH neuroTrigger3DH;
  neuroTrigger3DH.initialize();
  neuroTrigger3DH.setMLP(mlp);
  neuroTrigger3DH.createIntWeights();

  std::ofstream weightStream(fileNameDAT);

  // Write architecture to file
  weightStream << neuroParameters3DH.nInput << " ";
  for (const size_t hiddenNodes : neuroParameters3DH.nHidden) {
    weightStream << hiddenNodes << " ";
  }
  weightStream << neuroParameters3DH.nOutput << "\n";
  // Write int weights to file
  const std::vector<int32_t>& weights = neuroTrigger3DH.getIntWeights();
  int32_t minIntWeight = 0, maxIntWeight = 0;
  for (int32_t weight : weights) {
    if (weight < minIntWeight) minIntWeight = weight;
    if (weight > maxIntWeight) maxIntWeight = weight;
    weightStream << weight << " ";
  }
  weightStream << "\n";

  int fractionalWeightBits = neuroTrigger3DH.getFractionalWeightBits();
  int weightBits = neuroTrigger3DH.getWeightBits();
  std::cout << "Writing " << weights.size() << " weights.\n"
            << "With a precision of " << weightBits << " bits, corresponding to " << fractionalWeightBits << " fractional bits.\n"
            << "Max weight: " << maxIntWeight << ", Min weight: " << minIntWeight << "\n";
  weightStream.close();

  return 0;
}
