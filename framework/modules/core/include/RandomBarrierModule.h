/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * Sets gRandom to an independent generator for the following modules.
   *
   * E.g. a module chain of the sort [ParticleGun -> RandomBarrier -> FullSim]
   * would use one RNG instance for the ParticleGun, and another for FullSim and all following modules. You may find this
   * useful if you want to change the simulation, but don't want differences to affect the particle generation.
   * The output is equivalent to saving the output of ParticleGun in a file, and reading it again to do the simulation.
   *
   * Correct separation is not provided for terminate(), don't use random numbers there.
   */
  class RandomBarrierModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    RandomBarrierModule();

    ~RandomBarrierModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
  };
}
