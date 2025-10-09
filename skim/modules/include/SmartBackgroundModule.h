/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <mva/methods/ONNX.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Belle2 {

  /**
   * Smart Background Module
   * Predicts whether an event will pass a given skim directly after event generation using a neural network.
   * Used to speed up directly skimmed MC campaigns.
   */
  class SmartBackgroundModule : public Module {

  public:

    /**
     * Constructor.
     */
    SmartBackgroundModule();

    /**
     * Initialization.
     */
    void initialize() override;

    /**
     * Event handling.
     */
    void event() override;

  private:

    /**
     * Activation function to apply to logit output of network.
     */
    float activation(float, float, float);

    /**
     * The ONNX inference session
     */
    std::unique_ptr<MVA::ONNX::Session> m_session;

    /**
     * Mapping of PDG numbers to consecutive integers.
     */
    std::unordered_map<int, int> m_pdgMapping;

    /**
     * Mapping of skimcodes to output indices
     */
    std::unordered_map<int, uint16_t> m_skimcodesMapping;

    /**
     * Mapping of skim codes to skim names
     */
    std::unordered_map<int, std::string> m_skimnamesMapping;

    /**
     * Mapping of skimcodes to activation function parameters optimized for speedup
     */
    std::unordered_map<int, std::vector<float>> m_paramsMapping;

    /**
     * Mapping of event type names to integers
     */
    const std::unordered_map<std::string, int32_t> c_eventtypeMapping = {
      {"ccbar", 0}, {"charged", 1}, {"ddbar", 2}, {"mixed", 3}, {"ssbar", 4}, {"taupair", 5}, {"uubar", 6}
    };

    /**
     * Event type (module parameter)
     */
    std::string m_eventType;

    /**
     * Payload name (weights)
     */
    std::string m_payloadWeights;

    /**
     * Payload name (config)
     */
    std::string m_payloadConfig;

    /**
     * Activation function override parameters (module parameter)
     */
    std::vector<float> m_activationOverrideParams;

    /**
     * Vector to store x inputs of model.
     */
    std::vector<float> m_xValues;

    /**
     * Vector to store pdg inputs of model.
     */
    std::vector<int> m_pdgValues;

    /**
     * Vector to store mother index inputs of model.
     */
    std::vector<int> m_motherValues;

    /**
     * Skim codes (module parameter)
     */
    std::vector<int> m_skimCodes;

    /**
     * Lower clip value for predicted probabilities to limit maximum weight
     */
    float m_minProb;

    /**
     * Lower clip value for predicted probabilities (logarithmic) to limit maximum weight
     */
    float m_minLogProb;

    /**
     * Whether to execute in debug mode (module parameter)
     */
    bool m_debugMode;

    /**
     * Whether to override activation function parameters (module parameter)
     */
    bool m_activationOverride;

    /**
     * Whether to override automatically determined event type (module parameter)
     */
    bool m_overrideEventType;

  };

}
