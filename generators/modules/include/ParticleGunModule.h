/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/particlegun/ParticleGun.h>

#include <string>

namespace Belle2 {

  /** The ParticleGun module.
   * Generate tracks with the particle gun and store them
   * into the MCParticle class.
   */
  class ParticleGunModule : public Module {
  public:
    /**
     * Constructor.
     * Sets the module parameters.
     */
    ParticleGunModule();

    /** Destructor. */
    virtual ~ParticleGunModule() {}

    /** Initializes the module. */
    void initialize() override;

    /** Method is called for each event. */
    void event() override;

  protected:
    /** function to convert the strings the uses sets in the parameter list to the internal encoding */
    ParticleGun::EDistribution convertDistribution(std::string name);

    /** Instance of the particle gun */
    ParticleGun m_particleGun;
    /** Parameters of the particle gun */
    ParticleGun::Parameters m_parameters;
    /** String representation of the momentum distribution */
    std::string m_momentumDist;
    /** String representation of the azimuth angle distribution */
    std::string m_phiDist;
    /** String representation of the polar angle distribution */
    std::string m_thetaDist;
    /** String representation of the time distribution */
    std::string m_timeDist;
    /** String representation of the vertex distribution */
    std::string m_vertexDist;
    /** String representation of the x vertex distribution */
    std::string m_xVertexDist;
    /** String representation of the y vertex distribution */
    std::string m_yVertexDist;
    /** String representation of the z vertex distribution */
    std::string m_zVertexDist;
    /** Particle graph to generate MCParticle list */
    MCParticleGraph m_particleGraph;
  };

} // end namespace Belle2

