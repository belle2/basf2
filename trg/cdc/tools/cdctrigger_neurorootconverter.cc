/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/cdc/NeuroTrigger.h>
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
//#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
//#include <framework/database/DBObjPtr.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

//std::vector<float> loadweights (std::string filename) {
//    std::ifstream netfile(filename, std::ifstream::binary);
//    nlohmann::json nets;
//    netfile >> nets;
//    std::cout << nets["model.net.0.weight"][0] << std::endl;
//    std::vector<float> ret;
//    ret.push_back(4.);
//    return ret;
//
//
//
//}
using namespace Belle2;

int main(int argc, char* argv[])
{

  // get arguments
  if (argc < 4) {
    std::cout << "Program needs at 3 arguments:" << std::endl
              << " 1: parameter rootfile" << std::endl
              << " 2: MLP weights from external training" << std::endl
              << " 3: output filename" << std::endl;
    return -1;
  }
  NeuroTrigger::Parameters p;
  p.nMLP = 5;
  p.nHidden = {{81.}};
  p.targetZ = true;
  p.targetTheta = true;
  p.multiplyHidden = false;
  p.outputScale = {{ -100, 100, 10, 170}};
  p.phiRange = {{0, 360}};
  p.thetaRange = {{10, 170}};
  p.invptRange = {{ -5, 5}};
  p.phiRangeTrain = {{0, 360}};
  p.thetaRangeTrain = {{10, 170}};
  p.invptRangeTrain = {{ -5, 5}};
  p.maxHitsPerSL = {1};
  p.SLpattern = {511, 383, 479, 503, 509};
  p.SLpatternMask = {170};
  p.tMax = 256;
  p.et_option = "fastestpriority";
  p.T0fromHits = false;


  NeuroTrigger m_nnt;
  m_nnt.initialize(p);
  m_nnt.loadIDHist("IDHist.gz");
  //if (!m_nnt.load(argv[1], "MLPs")) {
  //    std::cout << "Error loading file: " << argv[1] << std::endl;
  //}
  std::ifstream netfile(argv[2], std::ifstream::binary);
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
  m_nnt.save(argv[3], "MLPs");


  return 0;
}
