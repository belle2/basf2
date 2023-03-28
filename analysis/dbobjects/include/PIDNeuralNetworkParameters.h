/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TObject.h>

namespace Belle2 {

  /**
   * Stores information on how to handle missing inputs, i.e. inputs that are NaN
   * It is a vector of tuples of two elements, where
   * the first element is the index i of the input variable and the
   * second element is the value to which input i is set if it is NaN.
   */
  typedef std::vector<std::tuple<size_t, float>> PIDNNMissingInputs;

  /**
   * Stores information on whether and how to overwrite certain inputs.
   * It is a vector of tuples of 5 elements:
   *  - Element 0: Index i of input to be overwritten
   *  - Element 1: Index j of input that defines if i is overwritten
   *  - [Element1, Element2] give the range in j in which i is overwritten
   *  - Element 4: Value to which i will be set
   * For example, if input 0 should be overwritten to -1.0 if input 1 is within [1.0, 2.0]:
   *  (0, 1, 1.0, 2.0, -1.0)
   */
  typedef std::vector< std::tuple<size_t, size_t, double, double, double> > PIDNNInputsToCut;

  /**
   * Class for handling the parameters for the neural-network PID.
   */
  class PIDNeuralNetworkParameters: public TObject {

  public:
    /// Constructor
    PIDNeuralNetworkParameters() {}

    /**
     * Construct with individual neural-network parameters
     */
    PIDNeuralNetworkParameters(const std::string& description,
                               const std::vector<std::string>& inputNames,
                               const std::string& modelDefinition,
                               const std::vector<int>& outputSpeciesPdg,
                               const std::vector<float>& meanValues,
                               const std::vector<float>& standardDeviations,
                               const PIDNNMissingInputs& handleMissingInputs,
                               const PIDNNInputsToCut& inputsToCut
                              ):
      m_description(description),
      m_inputNames(inputNames),
      m_modelDefinition(modelDefinition),
      m_outputSpeciesPdg(outputSpeciesPdg),
      m_meanValues(meanValues),
      m_standardDeviations(standardDeviations),
      m_handleMissingInputs(handleMissingInputs),
      m_inputsToCut(inputsToCut)
    {}


    /**
     * Convert pdg code to the index of the neural-network output that represents the corresponding probability
     * @param pdg PDG code for particle-species hypothesis
     * @return int Index of neural-network output
     */
    int pdg2OutputIndex(const int pdg) const;

    /**
     * Get the neural network model-definition string for frugally-deep
     * @return const std::string& model-definition string
     */
    const std::string& getModelDefinition() const {return m_modelDefinition;}
    /**
     * Get the mean values of the inputs for normalization
     * @return const std::vector<float>& input mean values
     */
    const std::vector<float>& getMeanValues() const {return m_meanValues;}
    /**
     * Get the standard deviations of the inputs for normalization
     * @return const std::vector<float>& input standard deviations
     */
    const std::vector<float>& getStandardDeviations() const {return m_standardDeviations;}
    /**
     * Get vector of input indices and corresponding values that are set if the corresponding input is NaN
     * @return const PIDNNMissingInputs&  missing input handnling information
     */
    const PIDNNMissingInputs& getHandleMissingInputs() const {return m_handleMissingInputs;}
    /**
     * Get vector of input indices, whose values are overwritten if other input variables are in a certain range.
     * @return const PIDNNInputsToCut& overwrite inputs information
     */
    const PIDNNInputsToCut& getInputsToCut() const {return m_inputsToCut;}
    /**
     * Get number of inputs
     * @return size_t  number of inputs
     */
    size_t getInputSize() const {return m_inputNames.size();}
    /**
     * Get input names
     * @return const std::vector<std::string>& input names
     */
    const std::vector<std::string>& getInputNames() const {return m_inputNames;}
    /**
     * Get input index for input name
     * @param name Input name that should be matched to a index
     * @return size_t input index, -1 if `name` not in inputs
     */
    size_t getInputIndex(const std::string& name) const;

    /**
     * Get neural network description
     * @return const std::string&  neural network description
     */
    const std::string& getDescription() const {return m_description;}

  private:

    std::string m_description;                 /**< description of neural network parameters*/
    std::vector<std::string> m_inputNames;     /**< list of input names*/
    std::string m_modelDefinition;             /**< neural network string for frugally-deep*/
    std::vector<int> m_outputSpeciesPdg;       /**< PDG codes of hypotheses of neural-network output*/
    std::vector<float> m_meanValues;           /**< mean values of inputs*/
    std::vector<float> m_standardDeviations;   /**< standard deviations of inputs*/
    PIDNNMissingInputs m_handleMissingInputs;  /**< handling of NaN inputs*/
    PIDNNInputsToCut m_inputsToCut;            /**< overwrite certain input variables*/

    ClassDef(PIDNeuralNetworkParameters, 1); /**< ClassDef as this is a TObject */

  };

} // Belle2 namespace


