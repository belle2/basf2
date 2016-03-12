/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KLMCLUSTEREFFICIENCYMODULE_H
#define KLMCLUSTEREFFICIENCYMODULE_H

/* Belle2 headers. */
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module for KLM cluster reconstruction efficiency studies.
   */
  class KLMClusterEfficiencyModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMClusterEfficiencyModule();

    /**
     * Destructor.
     */
    ~KLMClusterEfficiencyModule();

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

    /** Number of clusters from a K_L0. */
    int m_KL0Clusters;

    /** Number of clusters from a K_L0 + other particles. */
    int m_PartlyKL0Clusters;

    /** Number of clusters from other particles. */
    int m_OtherClusters;

    /** Number of K_L0 reconstructed as 0, 1, >= 2 clusters. */
    int m_ReconstructedKL0Clusters[3];

  };

}

#endif

