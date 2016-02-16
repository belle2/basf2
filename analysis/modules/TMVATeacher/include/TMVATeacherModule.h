/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
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

  /**
   * This modules writes out the necessary training data for a TMVA training.
   * TMVA methods can be trained using the externTeacher tool using this data.
   *
   * The samples are constructed using by calculating given variables for each candidate
   * in the given particleLists. If no particleLists are given an event-based training is assumed.
   * On sample per event is calculated in this case.
   *
   * It is possible to provide a sample variable and write out only every n-th sample of a specific class, the weight of each sample is increased accordingly.
   * Furthermore it is possible to restrict the amount of samples to an upper boundary, the weight of each sample is NOT changed in this case.
   *
   * If you run several basf2 instances on a cluster, you can just merge the resulting
   * *.root files using hadd or the fei_merge_files tool.
   *
   * If you run only one instance of basf2, you can also train "on-the-fly" at the end of data taking,
   * without invoking the externTeacher tool, using the TMVAOnTheFlyTeacher.
   *
   * You can apply a trained TMVAMethod with the TMVAExpertModule and calculate the SignalProbability for the Particles in a ParticleList.
   */

  class TMVATeacherModule : public Module {
  public:

    /**
     * Constructor
     */
    TMVATeacherModule();

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

  protected:

    /**
     * Returns weight calculated using inverseSamplingRates
     */
    float getInverseSamplingRateWeight(const Particle* particle);

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::vector<std::string> m_variables; /**< input variables for the TMVA method */
    std::vector<std::string>
    m_spectators; /**< input spectators for the TMVA method (e.g. possible target, weight or mcStatus variables) */
    std::string m_prefix; /**< Filename of the created root file and common prefix for the trained TMVA method weight-files */
    std::string m_treeName; /**< Treename of the created root file*/
    std::string m_workingDirectory; /**< Working directory in which the config file and the weight file directory is created */

    std::string m_sample; /**< sample variables used for inverse sampling rates, usually this is the same as the target */
    std::map<int, unsigned int> m_inverseSamplingRates; /**< Inverse sampling rates for class id */
    const Variable::Manager::Var* m_sample_var; /**< Variable Pointer to target variable */
    std::map<int, unsigned int> m_iSamples; /**< count for classes used by inverse sampling rates */
    std::map<int, unsigned long int> m_class_count; /**< Number of samples with this class id */

    std::vector<std::tuple<std::string, std::string, std::string>> m_methods; /**< tuple(name, type, config) for every method */
    std::unique_ptr<TMVAInterface::Teacher> m_teacher; /**< Used TMVA method */

    unsigned long int m_maxSamples; /**< Maximum number of samples */
    unsigned long int m_nSamples; /**< Current number of samples */

    bool m_lowMemoryProfile; /**< Flag which indicates if the low memory version of the TMVAInterface::Teacher should be used */
  };

} // Belle2 namespace


