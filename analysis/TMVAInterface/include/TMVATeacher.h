/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TMVATEACHER_H
#define TMVATEACHER_H

#include <analysis/utility/VariableManager.h>
#include <framework/logging/Logger.h>

#include <map>
#include <vector>
#include <string>

/**
 * Forward declaration
 */
class TFile;
namespace TMVA {

  class Factory;

}

namespace Belle2 {

  class Particle;

  /**
   * Interface to ROOT TMVA Factory.
   */
  class TMVATeacher  {

  public:
    /**
     * @param variables the names of the variables (registered in VariableManager), which are used as input for the chosen TMVA method
     * @param target the name of the target variable (registered in VariableManager), which is used as expected output for the chosen TMVA method
     * @param methods map with the chosen methods and options passed to this methods, the method name has to start with a predefined method name, e.g. BDT is predefined, so BDT, BDTWithGradientBoost,  BDT_1, ... are valid names.
     * @param identifier identifier which used to identify the outputted training files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml
     * @param factory_options options which are passed to the TMVA::Factory constructor, in most cases default options should be fine.
     */
    TMVATeacher(std::vector<std::string> variables, std::string target, std::map<std::string, std::string> methods, std::string identifier = "TMVA",  std::string factory_option = "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification");

    /**
     * Destructor, closes outputFile, deletes TMVA::Factory
     */
    ~TMVATeacher();

    /**
     * Returns Pointer to TMVA::Factory, useful to access the factory directly and set signal and background trees
     * @return Pointer to TMVA::Factory
     */
    TMVA::Factory* getFactory() { return m_factory; }

    /**
     * Adds a training sample. The necessary variables are calculated from the provided particle
     * @param particle Particle which serves as training sample, target variable must be available for this particle.
     */
    void addSample(const Particle* particle);

    /**
     * Train, test and evaluate all methods
     */
    void train();

  private:
    std::map<std::string, std::string> m_methods; /**< Name and Config of methods */
    TMVA::Factory* m_factory; /**< TMVA::Factory steers the booked methods */
    TFile* m_outputFile; /**< Output file which stores the generated histogramms */
    std::vector<const VariableManager::Var*> m_input; /**< Pointers to the input variables */
    const VariableManager::Var* m_target; /**< Pointer to the target variable */
  };
}

#endif
