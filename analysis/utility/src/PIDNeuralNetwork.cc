/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/PIDNeuralNetwork.h>

#include <cmath>


using namespace Belle2;


void PIDNeuralNetwork::loadParametersFromDB()
{
  m_pidNeuralNetworkParametersDB = std::make_unique<DBObjPtr<PIDNeuralNetworkParameters>>(m_pidNeuralNetworkParametersName);

  if (!(*m_pidNeuralNetworkParametersDB))
    B2FATAL("The dbobject PIDNeuralNetworkParameters, " << m_pidNeuralNetworkParametersName <<
            ", could not be found! It is necessary for the neural network based PID variables.");

  m_model = std::make_unique<fdeep::model>(fdeep::read_model_from_string((*m_pidNeuralNetworkParametersDB)->getModelDefinition(),
                                           false,
  [](const std::string&) {}  // disable logger
                                                                        ));
}


double PIDNeuralNetwork::predict(const int pdg, std::vector<float> input) const
{
  const int outputIndex = (*m_pidNeuralNetworkParametersDB)->pdg2OutputIndex(pdg);

  // apply cuts, ie. overwrite certain input values with index `inputSetIndex` with the value `setValue`
  // if the input with index `inputCutIndex` is in the range (`rangeStart`, `rangeEnd`)
  for (auto const& inputToCut : (*m_pidNeuralNetworkParametersDB)->getInputsToCut()) {
    const auto [inputSetIndex, inputCutIndex, rangeStart, rangeEnd, setValue] = inputToCut;
    if (!std::isnan(rangeStart) and !std::isnan(rangeEnd)) {
      if (input[inputCutIndex] >= rangeStart and input[inputCutIndex] <= rangeEnd)
        input[inputSetIndex] = setValue;
    } else if (!std::isnan(rangeStart)) {
      if (input[inputCutIndex] >= rangeStart)
        input[inputSetIndex] = setValue;
    } else if (!std::isnan(rangeEnd)) {
      if (input[inputCutIndex] <= rangeEnd)
        input[inputSetIndex] = setValue;
    }
  }

  // Normalize inputs
  // By definition, this has to be done after applying the cuts and before handling missing information
  for (size_t i = 0; i < input.size(); ++i) {
    input[i] = (input[i] - (*m_pidNeuralNetworkParametersDB)->getMeanValues()[i]) /
               (*m_pidNeuralNetworkParametersDB)->getStandardDeviations()[i];
  }

  // handle missing information
  for (auto const& index_value : (*m_pidNeuralNetworkParametersDB)->getHandleMissingInputs()) {
    const auto [index, value] = index_value;
    // const size_t index = index_value[0];
    // const float value = index_value[1];
    if (std::isnan(input[index])) input[index] = value;
  }

  // apply neural network
  const auto inputFdeep = fdeep::tensor(fdeep::tensor_shape(input.size()), input);
  const auto result = m_model->predict({inputFdeep});
  return result.front().get(fdeep::tensor_pos(outputIndex));
}