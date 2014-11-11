/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PRINTMCPARTICLESMODULE_H
#define PRINTMCPARTICLESMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>
#include <vector>


namespace Belle2 {

  class MCParticle;

  /** The PrintMCParticles module.
   * Prints the content of the MCParticle collection
   * as tree using the B2INFO message to the logging system.
   */
  class PrintMCParticlesModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    PrintMCParticlesModule();

    /** Destructor. */
    virtual ~PrintMCParticlesModule() {}

    /** Method is called for each event. */
    virtual void event();

  protected:

    /** Loops recursively over the MCParticle list and prints information about each particle.
     * @param mc Reference to the MCParticle whose information should be printed and whose daughters should be visited.
     * @param level The current level of the recursive call of the method.
     */
    void printTree(const MCParticle& mc, int level = 0);

    std::string m_particleList; /**< The name of the MCParticle collection. */
    std::vector<bool> m_seen;   /**< Tag the particles which were already visited using their index. */
    bool m_onlyPrimaries;       /**< Print only primary particles. */
    int m_maxLevel;             /**< Show only up to specified depth level. */
    StoreArray<MCParticle> m_mcparticles; /**< store array for the MCParticles */
  };

} // end namespace Belle2

#endif // PRINTMCPARTICLESMODULE_H
