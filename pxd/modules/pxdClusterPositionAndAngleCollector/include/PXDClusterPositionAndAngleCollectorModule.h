/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <calibration/CalibrationCollectorModule.h>
#include <string>

namespace Belle2 {
  /**
   * Calibration collector module for combined cluster position and angle estimation on PXD
   *
   */
  class PXDClusterPositionAndAngleCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDClusterPositionAndAngleCollectorModule();
    void prepare() override final;
    void collect() override final;

  private:
    /**< Required input for PXDCluster  */
    StoreArray<PXDCluster> m_pxdCluster;

    /** Name of the collection to use for PXDClusters */
    std::string m_storeClustersName;

    /** Collect data for Clusterkind  */
    int m_clusterKind;
    /** Number of bins for thetaU  */
    int m_binsU;
    /** Number of bins for thetaV  */
    int m_binsV;

    /** Name of cluster shape */
    std::string m_shapeName;
    /** Name of mirrored cluster shape */
    std::string m_mirroredShapeName;
    /** Eta value of cluster */
    float m_clusterEta;
    /** Charge value of cluster */
    float m_clusterCharge;
    /** Position offset u of truehit related to cluster */
    float m_positionOffsetU;
    /** Position offset v of truehit related to cluster */
    float m_positionOffsetV;
    /** Incidence angle thetaU of truehit related to cluster */
    float m_thetaU;
    /** Incidence angle thetaV of truehit related to cluster */
    float m_thetaV;
    /** Magnitude of momentum of truehit related to cluster */
    float m_momentumMag;
    /** Size in V */
    int m_sizeV;
    /** Pitch in V */
    float m_pitchV;
  };
}
