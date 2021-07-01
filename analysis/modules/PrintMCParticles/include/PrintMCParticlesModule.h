/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

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

    /** init. */
    virtual void initialize() override;
    /** Method is called for each event. */
    virtual void event() override;

  protected:

    /** Loops recursively over the MCParticle list and prints information about each particle.
     * @param particles Reference to the MCParticle whose information should be printed and whose daughters should be visited.
     * @param level The current level of the recursive call of the method.
     * @param indent String to be used for indentation.
     */
    void printTree(const std::vector<MCParticle*>& particles, int level = 1, const std::string& indent = "");

    /** if m_onlyPrimary is set remove all non primary particles from the given vector inplace */
    void filterPrimaryOnly(std::vector<MCParticle*>& particles) const;

    std::stringstream m_output; /**< Buffer to keep all the output while building the tree */
    std::string m_particleList; /**< The name of the MCParticle collection. */
    bool m_onlyPrimaries;       /**< Print only primary particles. */
    int m_maxLevel;             /**< Show only up to specified depth level. */
    bool m_showVertices;        /**< Show particle production vertices */
    bool m_showMomenta;         /**< Show particle momenta */
    bool m_showProperties;      /**< Show remaining properties */
    bool m_showStatus;          /**< Show extended status information */
    StoreArray<MCParticle> m_mcparticles; /**< store array for the MCParticles */
  };

} // end namespace Belle2
