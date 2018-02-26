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
#include <pxd/dataobjects/PXDTrueHit.h>
#include <calibration/CalibrationCollectorModule.h>
#include <string>

namespace Belle2 {
  /**
   * Calibration collector module for PXD cluster position estimation
   *
   *
   */
  class PXDClusterPositionCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDClusterPositionCollectorModule();
    void prepare() override final;
    void collect() override final;

  private:
    /**< Required input for PXDCluster  */
    StoreArray<PXDCluster> m_pxdCluster;

    /**< Required input for PXDTrueHit  */
    StoreArray<PXDTrueHit> m_pxdTrueHit;

    /** Name of the collection to use for PXDClusters */
    std::string m_storeClustersName;

    /** Name of the collection to use for PXDTrueHits */
    std::string m_storeTrueHitsName;
  };
}
