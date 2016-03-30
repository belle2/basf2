/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/utility/WorkingDirectoryManager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/FANNMLP.h>
#include <analysis/dataobjects/FANNMLPData.h>
#include <analysis/VariableManager/Utility.h>

#include <memory>
#include <fstream>
#include <cmath>

namespace Belle2 {

  class FANNTeacher;

  /**
   * This module trains a FANN Method.
   *
   */

  class FANNTeacherModule : public Module {
  public:

    /**
     * Constructor
     */
    FANNTeacherModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

    /**
     * Called for each event.
     */
    virtual void event() override;

    /**
     * Terminates the module.
     */
    virtual void terminate() override;

    /** Train a single MLP. */
    void train(unsigned isector);

    /** Save all training samples.
     * @param filename name of the TFile to write to
     * @param arrayname name of the TObjArray holding the training samples in the file
     */
    void saveTraindata(const std::string& filename);

    /** Load saved training samples.
     * @param filename name of the TFile to read from
     * @param arrayname name of the TObjArray holding the training samples in the file
     * @return true if the training sets were loaded correctly
     */
    bool loadTraindata(const std::string& filename);

    std::vector<float> getInputVector(unsigned isector);
    /**
    * Load feature variables from VariableManager
    */
    std::vector<const Variable::Manager::Var*> getVariablesFromManager() const;

    /**
     * Adds a training sample. The necessary variables are calculated from the provided particle
     * @param particle Particle which serves as training sample, target variable must be available for this particle.
     */
    void addSampleToSet(const Particle* particle);

    /** Save MLPs to file.
     * @param filename name of the TFile to write to
     * @param arrayname name of the TObjArray holding the MLPs in the file
     */
    void save(const std::string& filename, const std::string& arrayname);

    /** Load MLPs from file.
     * @param filename name of the TFile to read from
     * @param arrayname name of the TObjArray holding the MLPs in the file
     * @return true if the MLPs were loaded correctly
     */
    bool load(const std::string& filename, const std::string& arrayname);

  protected:

    std::vector<std::string> m_listNames; /**< Input particle list names */
    std::vector<const Variable::Manager::Var*> m_variables; /**< Input variables for the FANN MLP */
    std::vector<std::string> m_variableNames; /**< Input variables for the FANN MLP */
    std::vector<std::vector<float>> m_variableLimits; /**< Input variables for the FANN MLP */
    std::string m_prefix; /**< Filename of the created root file and common prefix for the trained FANN MLP weight-files */
    std::string m_logFilename; /**< Log Filename */
    std::string m_trainFilename;/**< Filename to save the training Sample.*/
    std::string m_filename; /**< Filename to save the weights of the trained MLPs.*/
    std::string m_trainArrayname; /**< Filename to save the training Sample.*/

    std::string m_treeName; /**< Treename of the created root file with the training Sample.*/
    std::string m_workingDirectory; /**< Working directory in which the config file and the weight file directory is created */

    float m_nTestAndValid;/**< Number of events in validation and training samples*/

    std::vector<std::tuple<std::string, std::string, std::string>> m_methods; /**< Tuple(name, type, config) for every method. */

    std::string
    m_targetName; /**< Name of the target variable used by multivariate analysis method. It has to be integer valued variable which defines clusters in the sample. */
    const Variable::Manager::Var* m_target; /**< Target used by multivariate analysis.*/

    /** List of networks */
    std::vector<FANNMLP> m_MLPs = {};
    /** Sets of training data for all methods. */
    FANNMLPData m_trainSet;

    unsigned long int m_maxSamples; /**< Maximum number of samples. */
    unsigned long int m_nSamples; /**< Current number of samples. */

    std::map<std::string, enum fann_activationfunc_enum>
    m_neuronTypes; /**< Map containing all possible neuron types, i.e. activation functions supported by the FANN library.*/
    std::map<std::string, enum  fann_train_enum>
    m_trainingMethods; /**< Map containing all possible training algorithms supported by the FANN library.*/

    typedef float (*FnPtr)(struct fann* ann, struct fann_train_data* data, const unsigned int threadnumb);
    std::map<std::string, FnPtr>
    m_trainingMethodsOPENMP; /**< Map containing all possible training algorithms supported by the FANN library which need OPENMP to be run in parallel.*/




  };

} // Belle2 namespace


