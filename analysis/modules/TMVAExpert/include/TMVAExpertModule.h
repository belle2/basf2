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
#include <analysis/TMVAInterface/Expert.h>

namespace Belle2 {

  class Particle;

  /**
   * This module fills the ExtraInfo of the Particle object with the value calculated by a TMVA method for every Particle in the given ParticleLists
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
    std::string m_methodName; /**< used multivariate analysis method */
    std::string m_identifier; /**< identifier name for the files created by the TMVA method */
    std::string m_workingDirectory; /**< Working directory in which the expert finds the config file and the weight file directory */
    std::string m_signalProbabilityName /**< Name under which the signal probability is stored in the ExtraInfo of the Particle object. */;
    TMVAInterface::Expert* m_method; /**< Method used to calculate the target variable */
    int m_signalCluster; /**< Number of the cluster which is considered signal */

  };

} // Belle2 namespace

