/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <vtx/dataobjects/VTXCluster.h>
#include <calibration/CalibrationCollectorModule.h>
#include <string>

namespace Belle2 {
  /**
   * Calibration collector module for VTX cluster position estimation
   *
   */
  class VTXClusterPositionCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    VTXClusterPositionCollectorModule();
    void prepare() override final;
    void collect() override final;

  private:
    /**< Required input for VTXCluster  */
    StoreArray<VTXCluster> m_vtxCluster;

    /** Name of the collection to use for VTXClusters */
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
    /** Position offset u of cluster */
    float m_positionOffsetU;
    /** Position offset v of cluster */
    float m_positionOffsetV;
    /** Size in V */
    int m_sizeV;
    /** Pitch in V */
    float m_pitchV;
  };
}
