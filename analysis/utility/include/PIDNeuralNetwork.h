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
      initializeNetwork();
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
      initializeNetwork();
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
     * Get input variables
     * @return const std::vector<const Variable::Manager::Var*>& input variables
     */
    const std::vector<const Variable::Manager::Var*>& getInputVariables() const { return m_inputVariables; }

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
     * Initialize the neural network from the loaded parameters
    */
    void initializeNetwork();

    std::string m_pidNeuralNetworkParametersName = "PIDNeuralNetworkParameters"; /**< name of the parameter set */
    std::unique_ptr<DBObjPtr<PIDNeuralNetworkParameters>> m_pidNeuralNetworkParametersDB; /**< db object for the parameter set */
    std::unique_ptr<const fdeep::model> m_model; /**< frugally-deep neural network */
    std::vector<const Variable::Manager::Var*> m_inputVariables; /**< list of input variables */
    std::map<int, std::string> m_extraInfoNames; /**< map from PDG code to extraInfo name that stores the output of this network */

  };

} // Belle2 namespace


inline void Belle2::PIDNeuralNetwork::initializeNetwork()
{
  m_model = std::make_unique<fdeep::model>(fdeep::read_model_from_string((*m_pidNeuralNetworkParametersDB)->getModelDefinition(),
                                           false,
  [](const std::string&) {}  // disable logger
                                                                        ));

  // Rename same of the input variables such that they match the basf2 internal names
  // This is needed for backwards compatibility with older payloads where the input variable names are different from the basf2 internal names
  std::map<std::string, std::string> inputNameMap; // maps old -> new names
  for (const Const::EDetector& detector : Const::PIDDetectorSet::set()) {
    for (const auto& hypeType : Const::chargedStableSet) {
      const auto oldName = "pidLogLikelihood_Of_" + std::to_string(abs(hypeType.getPDGCode())) + "_From_" + Const::parseDetectors(
                             detector);
      const auto newName = "pidLogLikelihoodValueExpert(" + std::to_string(abs(hypeType.getPDGCode())) + ',' + Const::parseDetectors(
                             detector) + ")";
      inputNameMap[oldName] = newName;
    }
  }
  inputNameMap["momentum"] = "p";

  // initialize input variable functions
  m_inputVariables.reserve(getInputSize());
  for (std::string name : (*m_pidNeuralNetworkParametersDB)->getInputNames()) {
    const auto itr = inputNameMap.find(name);
    if (itr != inputNameMap.end()) name = itr->second;

    const auto variable = Variable::Manager::Instance().getVariable(name);
    if (variable) {
      m_inputVariables.push_back(variable);
    } else {
      B2FATAL("PID neural network needs input '" + name + "', but this input is not available!");
    }
  }

  for (const auto outputPdgCode : getOutputSpeciesPdg()) {
    m_extraInfoNames[outputPdgCode] = "pidNeuralNetworkValueExpert(" + std::to_string(outputPdgCode) \
                                      + "," + m_pidNeuralNetworkParametersName + ")";
  }

}


