/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>

#include <framework/datastore/StoreObjPtr.h>

#include <vector>
#include <string>
#include <set>

class TFile;
class TTree;


#include <memory>

namespace Belle2 {


  class Particle;
  template <class T> class RootMergeable;

  /**
   * This modules writes out the necessary training data for a MVA training.
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

  class TrainingDataModule : public Module {
  public:

    /**
     * Constructor
     */
    TrainingDataModule();

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

  private:
    /**
     * Returns weight calculated using inverseSamplingRates
     */
    float getInverseSamplingRateWeight(const Particle* particle);

    /**
     * Adds a training sample. The necessary variables are calculated from the provided particle
     * @param particle Particle which serves as training sample, target variable must be available for this particle.
     * @param weight Additional weight for this sample
     */
    void addSample(const Particle* particle, float weight = 1);

  private:

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::string m_fileName; /**< Filename of the created root file */
    std::string m_treeName; /**< Treename of the created root file */

    std::vector<std::string> m_feature_names; /**< Store place for the feature names*/
    std::vector<float> m_feature_input; /**< Store place for the input variables */
    std::vector<const Variable::Manager::Var*> m_feature_variables; /**< Pointers to the feature variables */

    std::map<int, unsigned int> m_inverseSamplingRates; /**< Inverse sampling rates for class id */
    std::string m_sample_name; /**< sample variables used for inverse sampling rates, usually this is the same as the target */
    const Variable::Manager::Var* m_sample_variable; /**< Variable Pointer to target variable */
    std::map<int, unsigned int> m_iSamples; /**< count for classes used by inverse sampling rates */
    std::map<int, unsigned long int> m_class_count; /**< Number of samples with this class id */

    unsigned long int m_maxSamples = 0; /**< Maximum number of samples */
    unsigned long int m_nSamples = 0; /**< Current number of samples */

    TFile* m_file = nullptr; /**< stores TTree and training histograms created by TMVA */
    StoreObjPtr<RootMergeable<TTree>> m_tree; /**< holds training and test signal samples */

    float m_original_weight = 0.0; /**< Original weight of the candidate/event */

    bool m_lowMemoryProfile = false; /**< Flag which indicates if the low memory version of the TMVAInterface::Teacher should be used */
  };

} // Belle2 namespace


