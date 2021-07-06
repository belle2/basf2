/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {

  // forward declaration
  class Particle;

  /**
   * Module for creating copies of Particles. Module replaces each
   * Particle in the input ParticleList by its copy. Particle's
   * (grand-)^n-daughters are copied as well.
   * The existing relations of the original Particle (or it's (grand-)^n-daughters)
   * are copied as well. Note that only the relation is copied and that the related
   * object is not. Copied particles are therefore related to the *same* object as
   * the original ones.
   */
  class ParticleCopierModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleCopierModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    std::vector<std::string> m_inputListNames; /**< input ParticleList names */
  };

} // Belle2 namespace

