/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Phase 2 and Belle II geometry                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vector>
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Module */
  class PXDDQMClustersModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDDQMClustersModule();

  private:
    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

  private:

    /** cut for accepting to hitmap histogram, using pixels only, default = 0 */
    int m_CutPXDCharge = 0;

    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

    /** Input array for DAQ Status. */
    StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;

    /** PXDDigits StoreArray name */
    std::string m_storePXDDigitsName;
    /** PXDClusters StoreArray name */
    std::string m_storePXDClustersName;
    /** PXDClustersToPXDDigits RelationArray name */
    std::string m_relPXDClusterDigitName;


    /** Hitmaps of Digits */
    TH1I* m_hitMapCounts;
    /** Hitmaps of Clusters*/
    TH1I* m_hitMapClCounts;
    /** Hitmaps of digits on chips */
    TH1I* m_hitMapCountsChip;
    /** Hitmaps of clusters on chips */
    TH1I* m_hitMapClCountsChip;
    /** Fired pixels per event */
    TH1F** m_fired;
    /** Clusters per event */
    TH1F** m_clusters;
    /** Start row distribution */
    TH1F** m_startRow;
    /** Cluster seed charge by distance from the start row */
    TH1F** m_chargStartRow;
    /** counter for Cluster seed charge by distance from the start row */
    TH1F** m_startRowCount;
    /** Charge of clusters */
    TH1F** m_clusterCharge;
    /** Charge of pixels */
    TH1F** m_pixelSignal;
    /** u cluster size */
    TH1F** m_clusterSizeU;
    /** v cluster size */
    TH1F** m_clusterSizeV;
    /** Cluster size */
    TH1F** m_clusterSizeUV;

    /** Hitmaps pixels for u */
    TH1F** m_hitMapU;
    /** Hitmaps pixels for v */
    TH1F** m_hitMapV;
    /** Hitmaps pixels */
    TH2F** m_hitMap;
    /** Hitmaps clusters for u */
    TH1F** m_hitMapUCl;
    /** Hitmaps clusters for v */
    TH1F** m_hitMapVCl;
    /** Hitmaps clusters */
    TH2F** m_hitMapCl;
    /** Seed */
    TH1F** m_seed;

  };

}

