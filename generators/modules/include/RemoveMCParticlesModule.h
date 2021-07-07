/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <string>

namespace Belle2 {


  /** The RemoveMCParticles module.
   *
   * Writes the MCParticle collection to a HepEvt file.
   */
  class RemoveMCParticlesModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    RemoveMCParticlesModule();

    /** Destructor. */
    virtual ~RemoveMCParticlesModule() {}

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Terminates the module. */
    virtual void terminate() override;

    /** Apply cuts on a Particle and call for all daugthers recursively.
     * @param particle Particle to check
     * @param cut Indicates wether the mother has been removed
     */
    void applyCuts(const MCParticle& particle, bool cut = false);

  protected:
    //Parameters
    std::string m_particleList; /**< Name of the MCParticle collection to work on */
    double m_minZ; /**< Minimum Z value of particles to keep */
    double m_maxZ; /**< Maximum Z value of particles to keep */
    double m_minR; /**< Minimum Rphi value of particles to keep */
    double m_maxR; /**< Maximum Rphi value of particles to keep */
    double m_minTheta; /**< Minimum Theta value of particles to keep */
    double m_maxTheta; /**< Maximum Theta value of particles to keep */
    double m_minPt; /**< Minimum Pt value of particles to keep */
    double m_maxPt; /**< Maximum Pt value of particles to keep */
    bool   m_alsoChildren; /**< If true, also remove all children of a particle if it fails any cut */
    std::vector<int> m_pdgCodes; /**< List of pdgCodes wo apply cuts on. If empty, apply cuts to all particles */

    MCParticleGraph m_mpg; /**<ParticleGraph used for reformatting MCParticle collection */
  };


} // end namespace Belle2


