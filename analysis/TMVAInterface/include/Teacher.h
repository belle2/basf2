/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/TMVAInterface/Method.h>
#include <analysis/VariableManager/Manager.h>

#include <vector>
#include <string>

/**
 * Forward declaration
 */
class TFile;
class TTree;


namespace Belle2 {

  class Particle;

  namespace TMVAInterface {

    /**
     * Interface to ROOT TMVA Factory.
     */
    class Teacher  {

    public:
      /**
       * @param prefix which used to identify the outputted training files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml
       * @param workingDirectory where the config file and weight file directory is stored
       * @param target the name of the target variable (registered in Variable::Manager), which is used as expected output for the chosen TMVA method
       * @param methods vector of Method
       * @param useExistingData if correct TFile and TTree exists already exists, use the stored samples for training
       */
      Teacher(std::string prefix, std::string workingDirectory, std::string target, std::vector<Method> methods, bool useExistingData = false);

      /**
       * Disallow copy
       */
      Teacher(const Teacher&) = delete;

      /**
       * Disallow assign
       */
      Teacher& operator=(const Teacher&) = delete;

      /**
       * Destructor, closes outputFile, deletes TMVA::Factory
       */
      ~Teacher();

      /**
       * Adds a training sample. The necessary variables are calculated from the provided particle
       * @param particle Particle which serves as training sample, target variable must be available for this particle.
       */
      void addSample(const Particle* particle);

      /**
       * TODO Implement read and write of current sample tree to implement external training via a new tool TMVATeacher tree-var-file + opts
      void readTree();
      void writeTree();
      */

      /**
       * Train, test and evaluate all methods
       * @param factoryOption options which are passed to the TMVA::Factory constructor, in most cases default options should be fine.
       * @param prepareOption options which are passed to the TMVA::Factory::PrepareTrainingAndTestTree, in most cases default options should be fine.
       * @param maxEventsPerClass maximum number of events given to TMVA per class. TMVA internally uses a vector instead of a tree and therefore looses out-of-core ability.
       */
      void train(std::string factoryOption = "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification", std::string prepareOption = "SplitMode=random:!V", unsigned int maxEventsPerClass = 0);

    private:
      std::string m_prefix; /**< used to identify the outputted training files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml */
      std::string m_workingDirectory; /**< workingDirectory where the config file and weight file directory is stored */
      std::vector<Method> m_methods; /**< Name, Type and Config of methods */

      TFile* m_file; /**< stores TTree and training histograms created by TMVA */
      TTree* m_tree; /**< holds training and test signal samples */

      const Variable::Manager::Var* m_target_var; /**< Variable Pointer to target variable */
      int m_target; /**< Storage for the target variable */
      std::vector<float> m_input; /**< Storage for input variables */
      std::map<int, unsigned int> m_cluster_count; /**< Number of events foreach identified cluster */
    };

  }
}

