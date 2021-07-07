/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {
  class Particle;

  /**
   * Functions that create copies of Particles.
   *
   * Functions take existing Particle and create its copy ((grand)^n-daughter particles are copied as well).
   */
  namespace ParticleCopy {
    /**
     * Function takes argument Particle and creates a copy of it and copies of all its (grand-)^n-daughters.
     * The existing relations of the original Particle are copied as well. Note that only the relation is
     * copied and that the related object is not. Copy is related to the *same* object as the original one.
     *
     * Created copies are stored in the same StoreArray as the original Particle.
     *
     * @param original pointer to the original Particle to be copied
     *
     * @return returns pointer to the copied Particle
     */
    Particle* copyParticle(const Particle* original);

    /**
     * Function copies all (grand-)^n-daughter particles of the argument mother Particle. The copied particles are
     * stored in the same StoreArray as the mother Particle and its original daughters. The indices of daughter
     * particles in the mother Particle are replaced with the indices of copied daughter particles. The existing
     * relations of the original daughter Particles are copied as well. Note that only the relation is
     * copied and that the related object is not. Copy is related to the *same* object as the original one.
     *
     * @param mother pointer to the mother Particle whose daughters are to be copied
     */
    void copyDaughters(Particle* mother);

  }
}
