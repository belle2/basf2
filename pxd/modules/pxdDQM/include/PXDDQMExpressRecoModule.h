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
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>

#include "TH1D.h"

namespace Belle2 {

  /** PXD DQM Module */
  class PXDDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDDQMExpressRecoModule();

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

    /** PXDDigits StoreArray name */
    std::string m_storePXDDigitsName;
    /** PXDClusters StoreArray name */
    std::string m_storePXDClustersName;

    /** Storearray for Digits  */
    StoreArray<PXDDigit> m_storePXDDigits;
    /** Storearray for Cluster   */
    StoreArray<PXDCluster> m_storePXDClusters;

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
    std::vector <TH1D*> m_fired = {};
    /** Filtered fired pixels per event */
    std::vector<TH1D*> m_goodfired = {};
    /** Clusters per event */
    std::vector <TH1D*> m_clusters = {};
    /** filtered Clusters per event */
    std::vector<TH1D*> m_goodclusters = {};
    // FIXME: Startrow related histos are expert debugging, not for shifter (-> remove this)
    /** Start row distribution */
    //std::vector <TH1D*> m_startRow={};
    /** Cluster seed charge by distance from the start row */
    //std::vector <TH1D*> m_chargStartRow={};
    /** counter for Cluster seed charge by distance from the start row */
    //std::vector <TH1D*> m_startRowCount={};
    /** Charge of clusters */
    std::vector <TH1D*> m_clusterCharge = {};
    /** Charge of pixels */
    std::vector <TH1D*> m_pixelSignal = {};
    /** u cluster size */
    std::vector <TH1D*> m_clusterSizeU = {};
    /** v cluster size */
    std::vector <TH1D*> m_clusterSizeV = {};
    /** Cluster size */
    std::vector <TH1D*> m_clusterSizeUV = {};

  };

}

