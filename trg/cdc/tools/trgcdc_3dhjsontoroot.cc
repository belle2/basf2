/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>

#include <nlohmann/json.hpp>

#include "trg/cdc/dataobjects/CDCTriggerHoughMLP.h"

using namespace Belle2;

// Converts a plaintext .json network to a .root network that can be used in basf2
int main(int argc, const char* argv[])
{
  if (argc != 4) {
    std::cout << "Program requires the following 3 arguments:\n"
              << " 1: json weights\n"
              << " 2: configuration file\n"
              << " 3: root output file name\n";
    return -1;
  }
  const std::string jsonWeights = argv[1];
  const std::string configFile = argv[2];
  const std::string outputFile = argv[3];

  NeuroParametersHough neuroParameters3DH = CDCTrigger3DHMLP::loadConfigFromFile(configFile);
  CDCTrigger3DHMLP mlp(neuroParameters3DH);
  std::ifstream netfile(jsonWeights, std::ifstream::binary);
  nlohmann::json network;
  netfile >> network;

  std::vector<float> floatWeights;
  const size_t nLayers = neuroParameters3DH.nHidden.size();
  const size_t layerIndices = (nLayers + 1) * 2;
  for (size_t layerIdx = 0; layerIdx < layerIndices; layerIdx += 2) {
    int nodeIdx = 0;
    const std::string layerWeightName = "model." + std::to_string(layerIdx) + ".weight";
    const std::string layerBiasName = "model." + std::to_string(layerIdx) + ".bias";
    for (const auto& node : network[layerWeightName]) {
      for (float weight : node) {
        floatWeights.push_back(weight);
      }
      float bias = network[layerBiasName][nodeIdx];
      floatWeights.push_back(bias);
      ++nodeIdx;
    }
  }
  mlp.setFloatWeights(floatWeights);

  std::cout << "Writing " << floatWeights.size() << " weights\n";
  mlp.saveMLPToFile(outputFile, "MLP");

  float minFloatWeight = *std::min_element(floatWeights.begin(), floatWeights.end());
  float maxFloatWeight = *std::max_element(floatWeights.begin(), floatWeights.end());
  std::cout << "Max weight: " << maxFloatWeight << ", Min weight: " << minFloatWeight << std::endl;

  return 0;
}
