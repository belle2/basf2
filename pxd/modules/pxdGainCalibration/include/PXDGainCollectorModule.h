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
   * Calibration collector module for PXD gain calibration
   *
   * Firstly, the collector module selects signal like clusters based on cluster size
   * and cluster charge. Secondly, the collector fills trees with cluster charge and
   * current gains for all selected clusters.
   */
  class PXDGainCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDGainCollectorModule();
    void prepare() override final;
    void collect() override final;

  private:
    /**< Required input for PXDCluster  */
    StoreArray<PXDCluster> m_pxdCluster;

    /** Name of the collection to use for PXDClusters */
    std::string m_storeClustersName;

    /** Minimum cluster charge cut  */
    int m_minClusterCharge;
    /**  Minimum cluster size cut */
    int m_minClusterSize;
    /**  Maximum cluster size cut */
    int m_maxClusterSize;
    /**  Flag for MC data */
    bool m_simulatedDataFlag;
    /** Number of corrections per sensor along u side */
    int m_nBinsU;
    /** Number of corrections per sensor along v side */
    int m_nBinsV;


    /** Cluster charge in ADU */
    int m_signal;
    /** Current gain for cluster */
    float m_gain;
    /** Flag for MC data  */
    bool m_isMC;
  };
}
