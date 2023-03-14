/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/KLMCluster.h>

namespace Belle2 {
  class MCParticle;

  /**
   * Module for MC matching for KLM clusters.
   */
  class MCMatcherKLMClustersModule : public Module {

  public:

    /**
     * Constructor.
     */
    MCMatcherKLMClustersModule();

    /**
     * Destructor.
     */
    ~MCMatcherKLMClustersModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * This method is called for each event.
     */
    void event() override;

  private:

    /** Add relations for KLMHit2d and KLMHit2d. */
    bool m_Hit2dRelations;

    /** KLM clusters. */
    StoreArray<KLMCluster> m_KLMClusters;

    /** MCParticles StoreArray */
    StoreArray<MCParticle> m_MCParticles;

  };

}
