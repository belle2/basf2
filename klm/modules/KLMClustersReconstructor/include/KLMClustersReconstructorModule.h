/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <klm/bklm/dataobjects/BKLMHit2d.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/KLMCluster.h>

namespace Belle2 {

  /**
   * Module KLMClustersReconstructorModule.
   * @details
   * Module for KLMClusters reconstruction.
   */
  class KLMClustersReconstructorModule : public Module {

    /**
     * Vertex position calculation mode.
     */
    enum PositionMode {
      c_FullAverage, /**< Full average. */
      c_FirstLayer,  /**< First layer only. */
    };

    /**
     * Clusterization mode.
     */
    enum ClusterMode {
      c_AnyHit,   /**< Angle from any hit. */
      c_FirstHit, /**< Angle from first hit. */
    };

  public:

    /**
     * Constructor.
     */
    KLMClustersReconstructorModule();

    /**
     * Destructor.
     */
    ~KLMClustersReconstructorModule();

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

    /** Clustering angle. */
    double m_ClusteringAngle;

    /** Vertex position calculation mode. */
    std::string m_PositionModeString;

    /** Vertex position calculation mode. */
    enum PositionMode m_PositionMode;

    /** Clusterization mode. */
    std::string m_ClusterModeString;

    /** Clusterization mode. */
    enum ClusterMode m_ClusterMode;

    /** KLM clusters. */
    StoreArray<KLMCluster> m_KLMClusters;

    /** BKLM 2d hits. */
    StoreArray<BKLMHit2d> m_BKLMHit2ds;

    /** EKLM 2d hits. */
    StoreArray<EKLMHit2d> m_EKLMHit2ds;

  };

}
