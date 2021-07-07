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

#include <analysis/dataobjects/Particle.h>

#include <analysis/utility/ParticleSubset.h>

#include <string>
#include <vector>

namespace Belle2 {

  /**
  Removes all Particles that are not in a given list of ParticleLists (or daughters of those). All relations from/to Particles, daughter indices, and other ParticleLists are fixed.
  */
  class RemoveParticlesNotInListsModule : public Module {

  public:

    /**
     * Constructor
     */
    RemoveParticlesNotInListsModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /** process event */
    virtual void event() override;

    /**
     * Terminate the Module.
     * This method is called at the end of data processing.
     */
    virtual void terminate() override;


  private:
    StoreArray<Particle> m_particles; /**< StoreArray of Particles */
    std::vector<std::string>  m_particleLists; /**< keep Particles and daughters in these lists. */
    ParticleSubset m_subset; /**< reduces the Particles array. */
    unsigned long m_nRemoved; /**< number of particles removed. */
    unsigned long m_nTotal; /**< number of particles before removal. */

  };

} // Belle2 namespace
