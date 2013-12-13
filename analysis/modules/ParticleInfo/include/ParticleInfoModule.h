/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLE_INFO_MODULE_H
#define PARTICLE_INFO_MODULE_H

#include <framework/core/Module.h>


namespace Belle2 {

  /**
   * This module creates a ParticleInfo and relates it to a particle.
   * One has to provide a list of ParticleLists.
   * Once the ParticleInfo is created, other Modules can use this as a standard way to store simple float values
   */

  class ParticleInfoModule : public Module {
  public:

    /**
     * Constructor
     */
    ParticleInfoModule();

    /**
     * Destructor
     */
    virtual ~ParticleInfoModule();

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

    std::vector<std::string> m_inputListNames; /**< input particle list names */

  };

} // Belle2 namespace

#endif




