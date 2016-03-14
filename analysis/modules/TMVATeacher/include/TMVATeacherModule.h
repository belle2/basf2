/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/TMVAInterface/Teacher.h>

#include <memory>

namespace Belle2 {

  class TMVATeacher;

  /**
   * This module trains a TMVA method with the given particle lists as training samples.  The target variable has to be a
   * discrete integer valued variable (although the Variable::Manager returns the value as a float) which defines the clusters
   * in the sample. All the clusters are trained against each other.
   * You can apply a trained TMVAMethod with the TMVAExpertModule and calculate the SignalProbability for the Particles in a ParticleList.
   */

  class TMVATeacherModule : public Module {
  public:


    /**
     * Constructor
     */
    TMVATeacherModule();

    /**
     * Destructor
     */
    virtual ~TMVATeacherModule();

    /**
     * Initialize the module.
     */
    virtual void initialize();

    /**
     * Called for each event.
     */
    virtual void event();

    /**
     * Terminates the module.
     */
    virtual void terminate();

  private:

    /**
     * Checks if given sample should be used in training
     */
    bool checkSampling(int target);

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::vector<std::string> m_variables; /**< input variables for the TMVA method */
    std::vector<std::string> m_spectators; /**< input spectators for the TMVA method */
    std::string m_target; /**< target used by multivariate analysis method has to be integer valued variable which defines clusters in the sample. */
    std::vector<std::tuple<std::string, std::string, std::string>> m_methods; /**< tuple(name, type, config) for every method */
    std::string m_methodPrefix; /**< common prefix for the methods trained by TMVATeacher */
    std::string m_workingDirectory; /**< Working directory in which the config file and the weight file directory is created */
    std::string m_factoryOption; /**< Options which are passed to the TMVA Factory */
    std::string m_prepareOption; /**< Options which are passed to the TMVA Factory::PrepareTrainingAndTestTree */
    bool m_createMVAPDFs; /**< Creates the MVA PDFs for signal and background. This is needed to transform the output of the trained method to a probability */

    bool m_useExistingData; /**< Use existing data if available */
    bool m_doNotTrain; /**< do not train the method, just create the sample file. Useful for external training with externTeacher */
    unsigned int m_maxEventsPerClass; /**< Maximum nuber of events per class passed to TMVA */

    std::shared_ptr<TMVAInterface::Teacher> m_teacher; /**< Used TMVA method */
    const Variable::Manager::Var* m_target_var; /**< Variable Pointer to target variable */
    std::map<int, unsigned int> m_inverseSamplingRates; /**< Inverse sampling rates for class id */
    std::map<int, unsigned int> m_iSamples; /**< Number of samples with this class id */

  };

} // Belle2 namespace


