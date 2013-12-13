/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TMVAEXPERT_MODULE_H
#define TMVAEXPERT_MODULE_H

#include <framework/core/Module.h>


namespace Belle2 {

  class Particle;
  class TMVAExpert;

  /**
   * This module fills the ParticleInfo with the value calculated by a TMVA method for every Particle in the given ParticleLists
   * The module depends on the ParticleInfoModule
   */

  class TMVAExpertModule : public Module {
  public:

    /**
     * Constructor
     */
    TMVAExpertModule();

    /**
     * Destructor
     */
    virtual ~TMVAExpertModule();

    /**
     * Initialize the module.
     */
    virtual void initialize();

    /**
     * Called when a new run is started.
     */
    virtual void beginRun();

    /**
     * Called for each event.
     */
    virtual void event();

    /**
     * Called when run ended.
     */
    virtual void endRun();

    /**
     * Terminates the module.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::vector<std::string> m_variables; /**< input variables for the TMVA method, the names have to correspond to a registered variable in the VariableManager */
    std::string m_methodName; /**< used multivariate analysis method */
    std::string m_identifierName; /**< identifier name for the files created by the TMVA method */
    std::string m_targetName /**< the name of the target variable which is stored in the ParticleInfo */;
    TMVAExpert* m_method;

  };

} // Belle2 namespace

#endif




