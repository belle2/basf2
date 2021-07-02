/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Corr Module */
  class PXDDQMCorrModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDDQMCorrModule();

  private:
    /** Initialize */
    void initialize() override final;
    /** Begin run */
    void beginRun() override final;
    /** Event */
    void event() override final;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

  private:
    /** PXDClusters StoreArray name */
    std::string m_storeClustersName;
    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    /** Storearray for clusters   */
    StoreArray<PXDCluster> m_storeClusters;

    // +1 in dimensions to protect against noisy VXDID values.
    /** Correlation Sensor 1 vs 2 */
    TH2F* m_CorrelationU{};
    /** Correlation Sensor 1 vs 2 */
    TH2F* m_CorrelationV{};
    /** Correlation Sensor 1 vs 2 */
    TH1F* m_DeltaU{};
    /** Correlation Sensor 1 vs 2 */
    TH1F* m_DeltaV{};
  };

}

