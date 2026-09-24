/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/dataobjects/CDCTriggerHoughMLP.h"

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "framework/logging/Logger.h"

using namespace Belle2;

CDCTriggerHoughMLP::CDCTriggerHoughMLP(const NeuroParametersHough& neuroParametersHough):
  m_neuroParametersHough(neuroParametersHough)
{
  std::vector<size_t> nHidden = m_neuroParametersHough.nHidden;
  m_nodes = {static_cast<size_t>(m_neuroParametersHough.nInput)};
  for (size_t hiddenLayerIdx = 0; hiddenLayerIdx < nHidden.size(); ++hiddenLayerIdx) {
    m_nodes.push_back(nHidden[hiddenLayerIdx]);
  }
  m_nodes.push_back(m_neuroParametersHough.nOutput);
  m_floatWeights.assign(getNumberOfWeights(), 0.0f);
}

// Get the number of weights
size_t CDCTriggerHoughMLP::getNumberOfWeights() const
{
  size_t nWeights = 0;
  size_t nLayers = getNumberOfLayers();
  for (size_t i = 1; i < nLayers; ++i) {
    // +1 for bias node
    nWeights += (m_nodes[i - 1] + 1) * m_nodes[i];
  }
  return nWeights;
}

// Save the current class instance to a root file
void CDCTriggerHoughMLP::saveMLPToFile(const std::string& fileName, const std::string& objName) const
{
  B2INFO(std::string("Saving network to file ") + fileName + ", object " + objName);
  TFile datafile(fileName.c_str(), "UPDATE");
  this->Write(objName.c_str(), TObject::kSingleKey | TObject::kOverwrite);
  datafile.Close();
}

// STATIC: Load config parameters from a plain text config file
NeuroParametersHough CDCTriggerHoughMLP::loadConfigFromFile(const std::string& fileName)
{
  NeuroParametersHough neuroParametersHough;
  std::ifstream configFile(fileName);
  if (!configFile.is_open()) {
    B2ERROR("Could not open configuration file: " + fileName);
    exit(EXIT_FAILURE);
  }
  std::string completeLine;
  while (std::getline(configFile, completeLine)) {
    std::size_t hashtag = completeLine.find('#'); // Remove comments
    std::string line = completeLine.substr(0, hashtag);
    std::string configParameter;
    std::string parameterValue;
    if (line.length() < 3 || line.find('=') == std::string::npos) {
      continue;
    }
    line.erase(std::remove(line.begin(), line.end(), ' '), line.end()); // Remove whitspaces
    size_t equalPosition = line.find('=');
    configParameter = line.substr(0, equalPosition);
    parameterValue = line.substr((equalPosition + 1), line.length() - equalPosition - 1);
    if (configParameter == "nInput") {
      neuroParametersHough.nInput = std::stoull(parameterValue);
    } else if (configParameter == "nOutput") {
      neuroParametersHough.nOutput = std::stoull(parameterValue);
    } else if (configParameter == "nHidden") {
      neuroParametersHough.nHidden = readArray<size_t>(parameterValue);
    } else if (configParameter == "outputScale") {
      neuroParametersHough.outputScale = readArray<float>(parameterValue);
    } else {
      B2WARNING("Unknown config parameter: " + configParameter);
    }
  }
  return neuroParametersHough;
}

// PRIVATE STATIC: Read a one dimensional array from a plain text config file
template <typename T>
std::vector<T> CDCTriggerHoughMLP::readArray(const std::string& rawString)
{
  std::vector<T> configVector;
  std::string strippedString = rawString.substr(1, rawString.size() - 2);  // Strip brackets
  std::stringstream strippedStream(strippedString);
  std::string entry;
  while (std::getline(strippedStream, entry, ',')) {
    configVector.push_back(static_cast<T>(std::stod(entry)));
  }
  return configVector;
}
