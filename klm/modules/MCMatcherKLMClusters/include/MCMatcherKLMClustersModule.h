/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/KLMCluster.h>

namespace Belle2 {

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
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:

    /** Add relations for BKLMHit2d and EKLMHit2d. */
    bool m_Hit2dRelations;

    /** KLM clusters. */
    StoreArray<KLMCluster> m_KLMClusters;

  };

}
