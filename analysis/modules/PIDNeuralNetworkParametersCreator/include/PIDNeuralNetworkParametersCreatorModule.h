/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dbobjects/PIDNeuralNetworkParameters.h>


namespace Belle2 {
  /**
   * Module that creates PID neural network parameters and uploads it to the DB
   */
  class PIDNeuralNetworkParametersCreatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PIDNeuralNetworkParametersCreatorModule();

    /** Initialize the module. */
    void initialize() override;


  private:

    std::string m_neuralNetworkParametersName;  /**< Name of the set of parameters */

    // see PIDNeuralNetwork.h for an explanation of these parameters
    std::vector<std::string> m_inputNames;
    std::string m_modelDefinition;
    std::vector<int> m_outputSpeciesPdg;
    std::vector<float> m_meanValues;
    std::vector<float> m_standardDeviations;
    PIDNNMissingInputs m_handleMissingInputs;
    PIDNNInputsToCut m_inputsToCut;

    int m_experimentLow; /**< Lowest valid experiment # */
    int m_experimentHigh; /**< Highest valid experiment # */
    int m_runLow; /**< Lowest valid run # */
    int m_runHigh; /**< Highest valid run # */
  };
}
