/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/cdc/NeuroTrigger.h>
#include <trg/cdc/NeuroTriggerParameters.h>
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using namespace Belle2;

int main(int argc, const char* argv[])
{

  // get arguments
  if (argc < 4) {
    std::cout << "Program needs at 3 arguments:" << std::endl
              << " 1: json weights" << std::endl
              << " 2: configuration file name" << std::endl
              << " 3: output filename" << std::endl;
    return -1;
  }
  std::string jsonweights = argv[1];
  std::string configfile = argv[2];
  std::string outputfile = argv[3];

  NeuroTriggerParameters p(configfile);
  NeuroTrigger m_nnt;
  m_nnt.initialize(p);
  std::ifstream netfile(jsonweights, std::ifstream::binary);
  nlohmann::json nets;
  netfile >> nets;

  for (unsigned expert = 0; expert < m_nnt.nSectors(); expert++) {

    std::vector<float> weights;
    int numnode = 0;
    for (auto node : nets["expert_" + std::to_string(expert)]["weights"]["model.net.0.weight"]) {
      for (float w : node) {
        weights.push_back(w);
      }
      weights.push_back(nets["expert_" + std::to_string(expert)]["weights"]["model.net.0.bias"][numnode]);
      ++numnode;
    }
    numnode = 0;
    for (auto node : nets["expert_" + std::to_string(expert)]["weights"]["model.net.2.weight"]) {
      for (float w : node) {
        weights.push_back(w);
      }
      weights.push_back(nets["expert_" + std::to_string(expert)]["weights"]["model.net.2.bias"][numnode]);
      ++numnode;
    }
    std::cout << " writing " << weights.size() << " weights for expert " << expert << std::endl;
    m_nnt[expert].setWeights(weights);
  }
  m_nnt.save(outputfile, "MLPs");


  return 0;
}
