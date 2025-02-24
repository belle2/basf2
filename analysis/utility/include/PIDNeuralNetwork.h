/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <analysis/dbobjects/PIDNeuralNetworkParameters.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

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
      check();
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
      check();
    }

    /**
     * Move constructor
     */
    PIDNeuralNetwork(PIDNeuralNetwork&& other):
      m_pidNeuralNetworkParametersName(std::move(other.m_pidNeuralNetworkParametersName)),
      m_pidNeuralNetworkParametersDB(std::move(other.m_pidNeuralNetworkParametersDB)),
      m_model(std::move(other.m_model)),
      m_inputBasf2Names(std::move(other.m_inputBasf2Names)),
      m_extraInfoNames(std::move(other.m_extraInfoNames))
    {
    }

    /**
     * Predict neural-network output for all implemented hypotheses using the given inputs
     * @param inputs  set of input variables
     * @return double probability of the asked particle-species hypothesis
     * @return std::map<int, double> map of pdg codes to the corresponding probabilities
     */
    std::map<int, double> predict(std::vector<float> inputs) const;

    /**
     * Get number of inputs
     * @return size_t  number of inputs
     */
    size_t getInputSize() const { return (*m_pidNeuralNetworkParametersDB)->getInputSize(); }

    /**
     * Get names of input variables in the basf2 naming scheme, which may be different from the one in the payload
     * @return const std::vector<const std::string>& input variables
     */
    const std::vector<std::string>& getInputBasf2Names() const { return m_inputBasf2Names; }

    /**
     * Get the list of pdg codes of species hypotheses, for which the network predicts
     * the probability in the order defined by the network output layer
     * @return const std::vector<int>&  list of pdg codes of hypotheses predicted by the network
     */
    const std::vector<int>& getOutputSpeciesPdg() const {return (*m_pidNeuralNetworkParametersDB)->getOutputSpeciesPdg();}

    /**
     * Get the name of the used neural network
     * @return const std::string& name of the used neural network
     */
    const std::string& getPIDNeuralNetworkParametersName() const {return m_pidNeuralNetworkParametersName;}

    /**
     * @param pdg pdg code of hypothesis
     * @param throwException throw exception if pdg code is not predicted
     * @return true  if neural network predicts probability for the given hypothesis
     * @return false if neural network does not predict probability for the given hypothesis
     */
    bool hasPdgCode(const int pdg, const bool throwException = false) const {return (*m_pidNeuralNetworkParametersDB)->hasPdgCode(pdg, throwException);}

    /**
     * @param pdg pdg code of hypothesis
     * @return name of the extra info that stores the probability of the given pdg code
     */
    const std::string& getExtraInfoName(const int pdg) const {return m_extraInfoNames.at(pdg);}


  private:

    /**
     * Load neural-network parameters with name `m_pidNeuralNetworkParametersName` from the conditions data base.
     */
    void loadParametersFromDB();
    /**
     * Check that Neural Network can be evaluated, e.g. that all required inputs exist
     */
    void check();

    std::string m_pidNeuralNetworkParametersName = "PIDNeuralNetworkParameters"; /**< name of the parameter set */
    std::unique_ptr<DBObjPtr<PIDNeuralNetworkParameters>> m_pidNeuralNetworkParametersDB; /**< db object for the parameter set */
    std::unique_ptr<const fdeep::model> m_model; /**< frugally-deep neural network */
    std::vector<std::string> m_inputBasf2Names; /**< list of input names of input variables in the basf2 naming scheme*/
    std::map<int, std::string> m_extraInfoNames; /**< map from PDG code to extraInfo name that stores the output of this network */

  };

} // Belle2 namespace


inline void Belle2::PIDNeuralNetwork::check()
{
  for (const auto& name : getInputBasf2Names()) {
    if (!Variable::Manager::Instance().getVariable(name))
      B2FATAL("PID neural network needs input '" + name + "', but this input is not available!");
  }

}
