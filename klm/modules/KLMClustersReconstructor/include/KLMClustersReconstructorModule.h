/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMHit2d.h>

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/KLMCluster.h>

/* C++ headers. */
#include <string>
#include <vector>

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
      c_FullAverage,        /**< Full average. */
      c_FirstLayer,         /**< Innermost hit layer only. */
      c_FirstTwoLayers,     /**< Two innermost layers with hits (BKLM then EKLM). */
      c_SuccessiveTwoLayers /**< Innermost adjacent layer pair with hits; else FirstLayer. */
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

    /** If true, drop angular outliers after clustering and re-queue them for other clusters. */
    bool m_RemoveOutlierHits;

    /** Floor (minimum) angular threshold in rad; effective cut is max(floor, k*MAD). */
    double m_OutlierTrimAngle;

    /** Maximum number of iterations for the iterative centroid trim. */
    int m_OutlierRemovalMaxIterations;

    /** Multiplier k for k*MAD adaptive angular threshold (scale factor). */
    double m_OutlierMADFactor;

    /** Minimum fraction of original hits that must survive; otherwise cluster is kept untrimmed. */
    double m_OutlierMinInlierFraction;

    /**
     * Optional post-cluster hit filtering. No-op when m_RemoveOutlierHits is false.
     * Outliers are appended to poolHits and poolHits is re-sorted by R.
     */
    void applyOutlierRemoval(std::vector<KLMHit2d*>& clusterHits, std::vector<KLMHit2d*>& poolHits);

    /** KLM clusters. */
    StoreArray<KLMCluster> m_KLMClusters;

    /** Two-dimensional hits. */
    StoreArray<KLMHit2d> m_Hit2ds;

  };

}
