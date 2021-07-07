/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle II headers. */
#include <framework/core/Module.h>
#include <generators/utilities/InitialParticleGeneration.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * This module smears the primary vertex (the interaction point) according to the values stored in BeamParameters.
   * The smearing is applied randomly on event by event basis. The module loops over all the MCParticles and smears
   * their production vertices; if an MCParticle has no daughters, the decay vertex is also smeared.
   */
  class SmearPrimaryVertexModule : public Module {

  public:

    /**
     * Constructor.
     */
    SmearPrimaryVertexModule();

    /**
     * Destructor.
     */
    virtual ~SmearPrimaryVertexModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

    /**
     * This method is called for each event.
     */
    virtual void event() override;

  private:

    /**
     * Name of the MCParticles StoreArray.
     */
    std::string m_MCParticlesName;

    /**
     * Initial particle generation.
     */
    InitialParticleGeneration m_Initial;

  };

}
