/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <array>
#include <cstdint>

#include <TObject.h>
#include <TFile.h>
#include "framework/logging/Logger.h"

namespace Belle2 {

  // Full set of parameters to describe a network
  struct NeuroParametersHough : public TObject {
    // Number of input nodes
    size_t nInput;
    // Number of output nodes
    size_t nOutput;
    // Number of nodes for each hidden layer
    std::vector<size_t> nHidden;
    // Output scale/range of each output node
    std::vector<float> outputScale;

    ClassDef(NeuroParametersHough, 1)
  };

  // Class to represent a network for the 3DNeuro Trigger (3DFinder input)
  class CDCTriggerHoughMLP : public TObject {

  public:
    // Default constructor
    CDCTriggerHoughMLP() = default;
    // Constructor to set the network architecture
    CDCTriggerHoughMLP(const NeuroParametersHough& neuroParametersHough);
    // Default destructor
    ~CDCTriggerHoughMLP() = default;

    // Setter methods for the network
    void setFloatWeights(const std::vector<float>& weights) { m_floatWeights = weights; }

    // Getter methods for the network
    size_t getNumberOfLayers() const { return m_nodes.size(); }
    size_t getNumberOfNodes(const size_t layerIdx) const { return m_nodes[layerIdx]; }
    size_t getNumberOfWeights() const;
    const std::vector<float>& getFloatWeights() const { return m_floatWeights; }
    const NeuroParametersHough& getNeuroParameters() const { return m_neuroParametersHough; }

    // Save the current class instance to a root file
    void saveMLPToFile(const std::string& fileName, const std::string& objName) const;
    // Load MLP from file
    template <typename T>
    static T loadMLPFromFile(const std::string& fileName, const std::string& key)
    {
      TFile datafile(fileName.c_str(), "READ");
      if (!datafile.IsOpen()) {
        B2ERROR("Could not open file " << fileName);
        throw std::runtime_error("Could not open file " + fileName);
      }
      T* network = dynamic_cast<T*>(datafile.Get(key.c_str()));
      if (!network) {
        throw std::runtime_error("File " + fileName + " does not contain key " + key + " of requested type");
      }
      T result = *network;
      datafile.Close();
      return result;
    }

    // Load config parameters from a plain text config file
    static NeuroParametersHough loadConfigFromFile(const std::string& fileName);

  private:
    // Read a one dimensional array from a plain text config file
    template <typename T>
    static std::vector<T> readArray(const std::string& rawString);

    // Network configuration
    NeuroParametersHough m_neuroParametersHough;
    // Number of nodes in each layer, not including bias nodes.
    std::vector<size_t> m_nodes;
    // Weights of the network.
    std::vector<float> m_floatWeights;

    ClassDef(CDCTriggerHoughMLP, 1);
  };

  class CDCTrigger3DHMLP : public CDCTriggerHoughMLP {
  public:
    using CDCTriggerHoughMLP::CDCTriggerHoughMLP;
    static CDCTrigger3DHMLP loadFromFile(const std::string& fileName, const std::string& key)
    {
      return loadMLPFromFile<CDCTrigger3DHMLP>(fileName, key);
    }
    ClassDefOverride(CDCTrigger3DHMLP, 1);
  };

  class CDCTriggerDVTMLP : public CDCTriggerHoughMLP {
  public:
    using CDCTriggerHoughMLP::CDCTriggerHoughMLP;
    static CDCTriggerDVTMLP loadFromFile(const std::string& fileName, const std::string& key)
    {
      return loadMLPFromFile<CDCTriggerDVTMLP>(fileName, key);
    }
    ClassDefOverride(CDCTriggerDVTMLP, 1);
  };
}
