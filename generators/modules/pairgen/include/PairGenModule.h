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
#include <generators/utilities/InitialParticleGeneration.h>

namespace Belle2 {

  /** The PairGen module.
   * Simplistic generator of track pairs back to back in CMS.
   * Stores them into the MCParticle class.
   */
  class PairGenModule : public Module {
  public:
    /**
     * Constructor.
     * Sets the module parameters.
     */
    PairGenModule();

    /** Destructor. */
    virtual ~PairGenModule() {}

    /** Initializes the module. */
    void initialize() override;

    /** Method is called for each event. */
    void event() override;

  protected:
    int m_PDG; /**< Particles PDG code */

    /** Store both particles.
     * If set to false, only one of the two generated particles will be
     * used later by the simulation.
     */
    bool m_saveBoth;
    //TODO other parameters to make the module more useful

    InitialParticleGeneration m_initialParticleGeneration{MCInitialParticles::c_smearALL}; /**< Particle generator to generate initial beam energy, vertex and time */
    MCParticleGraph m_particleGraph; /**< Particle graph to generate MCParticle list */
  };

} // end namespace Belle2
