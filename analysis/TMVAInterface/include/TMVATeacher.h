/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/utility/VariableManager.h>
#include <framework/logging/Logger.h>

#include <TMVA/Types.h>

#include <map>
#include <vector>
#include <string>

/**
 * Forward declaration
 */
class TFile;
class TTree;


namespace Belle2 {

  class Particle;

  /**
   * Represents a TMVA Method, containing name, type and config of the method
   */
  class TMVAMethod {

  public:
    /**
     * @param name name of the TMVA method
     * @param type string representation of type, see TMVA::Types::EMVA for pissible names (dicarding the leading k)
     * @param config string with the config which is given to the TMVA method when its booked. See TMVA UserGuide for possible options
     */
    TMVAMethod(std::string name, std::string type, std::string config);

    /**
     * Getter for name
     * @return name
     */
    std::string getName() { return m_name; }

    /**
     * Getter for type
     * @return type
     */
    TMVA::Types::EMVA getType() { return m_type; }

    /**
     * Getter for config
     * @return config
     */
    std::string getConfig() { return m_config; }

  private:
    std::string m_name; /**< name of the method */
    TMVA::Types::EMVA m_type; /**< type of the method */
    std::string m_config; /**< config string given to the method */
  };

  /**
   * Interface to ROOT TMVA Factory.
   */
  class TMVATeacher  {

  public:
    /**
     * @param identifier which used to identify the outputted training files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml
     * @param variables the names of the variables (registered in VariableManager), which are used as input for the chosen TMVA method
     * @param target the name of the target variable (registered in VariableManager), which is used as expected output for the chosen TMVA method
     * @param methods vector of TMVAMethod
     */
    TMVATeacher(std::string identifier, std::vector<std::string> variables, std::string target, std::vector<TMVAMethod> methods);

    /**
     * Disallow copy
     */
    TMVATeacher(const TMVATeacher&) = delete;

    /**
     * Disallow assign
     */
    TMVATeacher& operator=(const TMVATeacher&) = delete;

    /**
     * Destructor, closes outputFile, deletes TMVA::Factory
     */
    ~TMVATeacher();

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
    std::vector<TMVAMethod> m_methods; /**< Name, Type and Config of methods */

    TTree* m_signal_tree; /**< holds training and test signal samples */
    TTree* m_bckgrd_tree; /**< holds training and test background samples */

    std::map<const VariableManager::Var*, float> m_input; /**< Pointers to the input variables */
    std::pair<const VariableManager::Var*, float> m_target; /**< Pointer to the target variable */
  };
}

