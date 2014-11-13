/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/utility/ParticleSubset.h>
#include <analysis/dataobjects/ParticleList.h>


#include <string>
#include <vector>
#include <set>

namespace Belle2 {

  /**
  Removes all Particles that are not in a given list of ParticleLists (or daughters of those). All relations from/to Particles, daughter indices, and other ParticleLists are fixed.
  */
  class DiscardParticlesModule : public Module {

  public:

    /**
     * Constructor
     */
    DiscardParticlesModule();

    /**
     * Destructor
     */
    virtual ~DiscardParticlesModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();


    virtual void event();

    virtual void terminate();


  private:
    std::vector<std::string>  m_particleLists; /**< keep Particles and daughters in these lists. */
    ParticleSubset m_subset; /**< reduces the Particles array. */

  };

} // Belle2 namespace
