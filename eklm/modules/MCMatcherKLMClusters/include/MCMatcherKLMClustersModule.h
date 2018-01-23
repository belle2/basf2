/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCMATCHERKLMCLUSTERSMODULE_H
#define MCMATCHERKLMCLUSTERSMODULE_H

/* Belle2 headers. */
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
    void initialize();

    /**
     * Called when entering a new run.
     */
    void beginRun();

    /**
     * This method is called for each event.
     */
    void event();

    /**
     * This method is called if the current run ends.
     */
    void endRun();

    /**
     * This method is called at the end of the event processing.
     */
    void terminate();

  private:

    /** Add relations for BKLMHit2d and EKLMHit2d. */
    bool m_Hit2dRelations;

    /** KLM clusters. */
    StoreArray<KLMCluster> m_KLMClusters;

  };

}

#endif

