/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TMVAEXPERT_H
#define TMVAEXPERT_H

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
  class Reader;
}

namespace Belle2 {

  class Particle;

  /**
   * Interface to ROOT TMVA Reader.
   * Requires the output of TMVATeacher.
   */
  class TMVAExpert {

  public:

    /**
     * @param variables the names of the variables (registered in VariableManager), which are used as input for the chosen TMVA method
     * @param method the chosen method, the string has to start with a predefined method name, e.g. BDT is predefined, so BDT, BDTWithGradientBoost,  BDT_1, ... are valid names
     * @param identifier identifier which was used to train the method. This class expects that the files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml exists.
     */
    TMVAExpert(std::vector<std::string> variables, std::string method, std::string identifier = "TMVA");

    /**
     * Destructor, closes outputFile, deletes TMVA::Reader
     */
    ~TMVAExpert();

    /**
     * Analyse a Particle with the given method and calculates signal probability
     *
     * @param particle the particle which should be analysed
     * @return signal probability of the particle
     */
    float analyse(const Particle*);

  private:
    float* m_inputProxy;
    std::string m_method;
    TMVA::Reader* m_reader;
    std::vector<const VariableManager::Var*> m_input;
  };

}

#endif
