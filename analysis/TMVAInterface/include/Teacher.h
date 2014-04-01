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
#include <analysis/utility/VariableManager.h>

#include <vector>
#include <string>

/**
 * Forward declaration
 */
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
       * @param identifier which used to identify the outputted training files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml
       * @param workingDirectory where the config file and weight file directory is stored
       * @param target the name of the target variable (registered in VariableManager), which is used as expected output for the chosen TMVA method
       * @param methods vector of Method
       */
      Teacher(std::string identifier, std::string workingDirectory, std::string target, std::vector<Method> methods);

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
       * Train, test and evaluate all methods
       * @param factoryOption options which are passed to the TMVA::Factory constructor, in most cases default options should be fine.
       * @param prepareOption options which are passed to the TMVA::Factory::PrepareTrainingAndTestTree, in most cases default options should be fine.
       */
      void train(std::string factoryOption = "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification", std::string prepareOption = "SplitMode=random:!V");


    private:
      std::string m_identifier; /**< used to identify the outputted training files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml */
      std::string m_workingDirectory; /**< workingDirectory where the config file and weight file directory is stored */
      std::vector<Method> m_methods; /**< Name, Type and Config of methods */

      TTree* m_tree; /**< holds training and test signal samples */
      const VariableManager::Var* m_target_var; /**< Variable Pointer to target variable */
      int m_target; /**< Storage for the target variable */
      std::vector<float> m_input; /**< Storage for input variables */
      std::map<int, unsigned int> m_cluster_count; /**< Number of events foreach identified cluster */
    };

  }
}

