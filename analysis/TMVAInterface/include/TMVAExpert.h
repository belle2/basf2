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
     * @param identifier identifier which was used to train the method. This class expects that the files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml exists.
     * @param method the chosen method
     */
    TMVAExpert(std::string identifier, std::string method);

    /**
     * Disallow copy
     */
    TMVAExpert(const TMVAExpert&) = delete;

    /**
     * Disallow assign
     */
    TMVAExpert& operator=(const TMVAExpert&) = delete;

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
    std::string m_method; /**< The TMVA method */
    TMVA::Reader* m_reader; /**< TMVA::Reader steers the booked TMVA method */
    std::map<const VariableManager::Var*, float> m_input; /**< Pointers to the input variables */
  };

}

#endif
