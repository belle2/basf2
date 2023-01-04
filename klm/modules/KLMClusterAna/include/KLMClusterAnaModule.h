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

/* KLM headers. */
#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dataobjects/KLMClusterShape.h>
#include <mdst/dataobjects/KLMCluster.h>

namespace Belle2 {

  /**
   * Module for KLM cluster reconstruction efficiency studies.
   */
  class KLMClusterAnaModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMClusterAnaModule();

    /**
     * Destructor.
     */
    ~KLMClusterAnaModule();

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




  private:


    /** KLM clusters. */
    StoreArray<KLMCluster> m_KLMClusters;

    /** Output per cluster. */
    StoreArray<KLMClusterShape> m_KLMClusterShape;

    /** Two-dimensional hits. */
    StoreArray<KLMHit2d> m_klmHit2ds;

  };

}




