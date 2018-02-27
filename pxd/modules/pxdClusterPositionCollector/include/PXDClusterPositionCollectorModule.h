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
#include <TClonesArray.h>
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

    /** Name of the collection to use for PXDClusters */
    std::string m_storeClustersName;

    /** Array of clusters related to a truehit (always has one entry). */
    TClonesArray* m_rootPxdClusterArray;
    /** Array of digits related to a cluster. */
    TClonesArray* m_rootPxdDigitArray;
    /** Array of truehits related to a cluster. */
    TClonesArray* m_rootPxdTrueHitArray;
  };
}
