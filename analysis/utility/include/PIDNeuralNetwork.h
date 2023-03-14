/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/dbobjects/PIDNeuralNetworkParameters.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

#include <framework/gearbox/Const.h>

#define FDEEP_FLOAT_TYPE float
#include <fdeep/fdeep.hpp>



namespace Belle2 {

  /**
   * Class to call PID neural network
   */
  class PIDNeuralNetwork {

  public:

    /**
     * Constructor with default paramenter-set name
     */
    PIDNeuralNetwork():
      m_pidNeuralNetworkParametersDB(nullptr),
      m_model(nullptr)
    {
      loadParametersFromDB();
    };


    /**
     * Constructor with given paramenter-set name
     */
    PIDNeuralNetwork(const std::string& parameterName):
      m_pidNeuralNetworkParametersName(parameterName),
      m_pidNeuralNetworkParametersDB(nullptr),
      m_model(nullptr)
    {
      loadParametersFromDB();
    }

    /**
     * Move constructor
     */
    PIDNeuralNetwork(PIDNeuralNetwork&& other):
      m_pidNeuralNetworkParametersName(std::move(other.m_pidNeuralNetworkParametersName)),
      m_pidNeuralNetworkParametersDB(std::move(other.m_pidNeuralNetworkParametersDB)),
      m_model(std::move(other.m_model))
    {
    }

    /**
     * Predict neural-network output for hypotheses `pdg` using the given inputs
     * @param pdg PDG code of particle-species hypothisis for wich the probability is calculated
     * @param inputs  set of input variables
     * @return double probability of the asked particle-species hypothesis
     */
    double predict(int pdg, std::vector<float> inputs) const;

    /**
     * Get number of inputs
     * @return size_t  number of inputs
     */
    size_t getInputSize() const { return (*m_pidNeuralNetworkParametersDB)->getInputSize(); }

    /**
     * Get input names
     * @return const std::vector<std::string>& input names
     */
    size_t getInputIndex(const std::string& name) const { return (*m_pidNeuralNetworkParametersDB)->getInputIndex(name); }

    /**
     * Get input names
     * @return const std::vector<std::string>& input names
     */
    const std::vector<std::string>& getInputNames() const { return (*m_pidNeuralNetworkParametersDB)->getInputNames(); }


  private:

    /**
     * Load neural-network parameters with name `m_pidNeuralNetworkParametersName` from the conditions data base.
     */
    void loadParametersFromDB();

    std::string m_pidNeuralNetworkParametersName = "PIDNeuralNetworkParameters"; /**< name of the parameter set */
    std::unique_ptr<DBObjPtr<PIDNeuralNetworkParameters>>
                                                       m_pidNeuralNetworkParametersDB; /**< db object for the parameter set */

    std::unique_ptr<const fdeep::model> m_model; /**< frugaly-deep neural network */

  };

} // Belle2 namespace




