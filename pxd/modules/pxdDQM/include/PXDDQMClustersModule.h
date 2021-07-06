/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <framework/datastore/StoreObjPtr.h>
#include "TH1D.h"
#include "TH2D.h"

namespace Belle2 {

  /** PXD DQM Module */
  class PXDDQMClustersModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDDQMClustersModule();

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

    /** cut for accepting filtered pixel */
    int m_CutMinCharge;
    /** cut for accepting filtered cluster, using cluster charge */
    int m_CutMinClusterCharge;
    /** cut for accepting to filtered hitmap histogram, using cluster seed */
    int m_CutMinSeedCharge;
    /** cut for accepting to filtered hitmap histogram, maximum cluster size */
    int m_CutMaxClusterSize;

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
    TH1D* m_hitMapCounts = {};
    /** Hitmaps of filtered Digits */
    TH1D* m_hitMapFilterCounts = {};
    /** Hitmaps of Clusters*/
    TH1D* m_hitMapClCounts = {};
    /** Hitmaps of filtered Clusters*/
    TH1D* m_hitMapClFilterCounts = {};

    /** Hitmaps of digits on chips */
    TH1D* m_hitMapCountsChip = {};
    /** Hitmaps of clusters on chips */
    TH1D* m_hitMapClCountsChip = {};
    /** Fired pixels per event */
    std::vector<TH1D*> m_fired = {};
    /** Filtered fired pixels per event */
    std::vector<TH1D*> m_goodfired = {};
    /** Clusters per event */
    std::vector<TH1D*> m_clusters = {};
    /** filtered Clusters per event */
    std::vector<TH1D*> m_goodclusters = {};
    /** Start row distribution */
    std::vector<TH1D*> m_startRow = {};
    /** Cluster seed charge by distance from the start row */
    std::vector<TH1D*> m_chargStartRow = {};
    /** counter for Cluster seed charge by distance from the start row */
    std::vector<TH1D*> m_startRowCount = {};
    /** Charge of clusters */
    std::vector<TH1D*> m_clusterCharge = {};
    /** Deposited energy of clusters */
    std::vector<TH1D*> m_clusterEnergy = {};
    /** Charge of pixels */
    std::vector<TH1D*> m_pixelSignal = {};
    /** u cluster size */
    std::vector<TH1D*> m_clusterSizeU = {};
    /** v cluster size */
    std::vector<TH1D*> m_clusterSizeV = {};
    /** Cluster size */
    std::vector<TH1D*> m_clusterSizeUV = {};

    /** Hitmaps pixels for u */
    std::vector<TH1D*> m_hitMapU = {};
    /** Hitmaps pixels for v */
    std::vector<TH1D*> m_hitMapV = {};
    /** Hitmaps pixels */
    std::vector<TH2D*> m_hitMap = {};
    /** Hitmaps clusters for u */
    std::vector<TH1D*> m_hitMapUCl = {};
    /** Hitmaps clusters for v */
    std::vector<TH1D*> m_hitMapVCl = {};
    /** Hitmaps clusters */
    std::vector<TH2D*> m_hitMapCl = {};
    /** Seed */
    std::vector<TH1D*> m_seed = {};

  };

}

