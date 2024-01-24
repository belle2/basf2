/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <analysis/modules/PIDNeuralNetworkParametersCreator/PIDNeuralNetworkParametersCreatorModule.h>


#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/logging/Logger.h>

#define FDEEP_FLOAT_TYPE float
#include <fdeep/fdeep.hpp>

using namespace Belle2;

REG_MODULE(PIDNeuralNetworkParametersCreator);

PIDNeuralNetworkParametersCreatorModule::PIDNeuralNetworkParametersCreatorModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Module that creates PID neural network parameters and uploads them to the DB)DOC");

  // Parameter definitions
  addParam("neuralNetworkParametersName", m_neuralNetworkParametersName, "Name of the set of parameters");
  addParam("description", m_description, "Description of the neural network");

  addParam("inputNames", m_inputNames, "List of name of input variables in the required order");
  addParam("modelDefinition", m_modelDefinition, "Keras string encoding the neural-network model and parameters");
  addParam("outputSpeciesPdg", m_outputSpeciesPdg,
           "List of PDG codes of the hypotheses that correspond to the neural network output probabilities");
  addParam("meanValues", m_meanValues, "List of mean values of input variables for normalization");
  addParam("standardDeviations", m_standardDeviations, "List of standard deviations of input variables for normalization");
  addParam("handleMissingInputs", m_handleMissingInputs,
           "List of indices and values set if the variable defined by the index is NaN");
  addParam("inputsToCut", m_inputsToCut, "List of input values that are cut if another input value is in a given range");

  addParam("experimentLow", m_experimentLow, "Interval of validity, exp low");
  addParam("experimentHigh", m_experimentHigh, "Interval of validity, exp high");
  addParam("runLow", m_runLow, "Interval of validity, run low");
  addParam("runHigh", m_runHigh, "Interval of validity, run high");
}

void PIDNeuralNetworkParametersCreatorModule::initialize()
{
  bool isValid = true;
  const size_t nInputs = m_inputNames.size();
  const size_t nOutputs = m_outputSpeciesPdg.size();

  // this performs some tests and raises an exception if a test failes
  const auto model = fdeep::read_model_from_string(m_modelDefinition);

  const auto inputShapes = model.get_input_shapes();
  size_t nModelInputs = 0;
  for (const auto& shape : inputShapes) {
    if (shape.rank() != 1) {
      std::cout << "Can handle only rank=1 inputs, but input has rank " << shape.rank() << std::endl;
      isValid = false;
      break;
    }
    nModelInputs += shape.minimal_volume();
  }
  if (nModelInputs != nInputs) {
    std::cout << "Model requires " << nModelInputs << " inputs, but parameters have only " << nInputs << " inputs!" << std::endl;
    isValid = false;
  }

  const auto outputShapes = model.get_output_shapes();
  size_t nModelOutputs = 0;
  for (const auto& shape : outputShapes) {
    if (shape.rank() != 1) {
      std::cout << "Can handle only rank=1 outputs, but output has rank " << shape.rank() << std::endl;
      isValid = false;
      break;
    }
    nModelOutputs += shape.minimal_volume();
  }
  if (nModelOutputs != nOutputs) {
    std::cout << "Model has " << nModelOutputs << " outputs, but parameters have only " << nOutputs << " outputs!" << std::endl;
    isValid = false;
  }

  if (nInputs != m_meanValues.size()) {
    std::cout << "Parameters have " << m_meanValues.size() << " mean values, but " << nInputs << " inputs!";
    isValid = false;
  }

  if (nInputs != m_standardDeviations.size()) {
    std::cout << "Parameters have " << m_standardDeviations.size() << " standard deviations, but " << nInputs << " inputs!";
    isValid = false;
  }

  for (auto const& index_value : m_handleMissingInputs) {
    const auto [index, _] = index_value;
    if (index >= nInputs) {
      std::cout << "Index " << index << " of handleMissingInputs out of range!" << std::endl;
      isValid = false;
    }
  }

  for (auto const& inputToCut : m_inputsToCut) {
    const size_t inputSetIndex = std::get<0>(inputToCut);
    const size_t inputCutIndex = std::get<1>(inputToCut);
    if (inputSetIndex >= nInputs) {
      std::cout << "inputSetIndex " << inputSetIndex << " of handleMissingInputs out of range!" << std::endl;
      isValid = false;
    }
    if (inputCutIndex >= nInputs) {
      std::cout << "inputCutIndex " << inputCutIndex << " of handleMissingInputs out of range!" << std::endl;
      isValid = false;
    }
  }

  if (!isValid)
    B2ERROR("The given neural-network parametes are invalid!");

  Belle2::DBImportObjPtr<Belle2::PIDNeuralNetworkParameters> importer{m_neuralNetworkParametersName};
  importer.construct(
    m_description,
    m_inputNames,
    m_modelDefinition,
    m_outputSpeciesPdg,
    m_meanValues,
    m_standardDeviations,
    m_handleMissingInputs,
    m_inputsToCut
  );
  importer.import(Belle2::IntervalOfValidity(m_experimentLow, m_runLow, m_experimentHigh, m_runHigh));
}


