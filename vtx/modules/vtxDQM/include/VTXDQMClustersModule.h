/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
// #include <vtx/dataobjects/VTXDAQStatus.h>
#include <framework/datastore/StoreObjPtr.h>
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** VTX DQM Module */
  class VTXDQMClustersModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    VTXDQMClustersModule();

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
    int m_CutVTXCharge = 0;

    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

    /** VTXDigits StoreArray name */
    std::string m_storeVTXDigitsName;
    /** VTXClusters StoreArray name */
    std::string m_storeVTXClustersName;
    /** VTXClustersToVTXDigits RelationArray name */
    std::string m_relVTXClusterDigitName;


    /** Hitmaps of Digits */
    TH1I* m_hitMapCounts = {};
    /** Hitmaps of Clusters*/
    TH1I* m_hitMapClCounts = {};
    /** Hitmaps of digits on chips */
    TH1I* m_hitMapCountsChip = {};
    /** Hitmaps of clusters on chips */
    TH1I* m_hitMapClCountsChip = {};
    /** Fired pixels per event */
    TH1F** m_fired = {};
    /** Clusters per event */
    TH1F** m_clusters = {};
    /** Start row distribution */
    TH1F** m_startRow = {};
    /** Cluster seed charge by distance from the start row */
    TH1F** m_chargStartRow = {};
    /** counter for Cluster seed charge by distance from the start row */
    TH1F** m_startRowCount = {};
    /** Charge of clusters */
    TH1F** m_clusterCharge = {};
    /** Deposited energy of clusters */
    TH1F** m_clusterEnergy = {};
    /** Charge of pixels */
    TH1F** m_pixelSignal = {};
    /** u cluster size */
    TH1F** m_clusterSizeU = {};
    /** v cluster size */
    TH1F** m_clusterSizeV = {};
    /** Cluster size */
    TH1F** m_clusterSizeUV = {};

    /** Hitmaps pixels for u */
    TH1F** m_hitMapU = {};
    /** Hitmaps pixels for v */
    TH1F** m_hitMapV = {};
    /** Hitmaps pixels */
    TH2F** m_hitMap = {};
    /** Hitmaps clusters for u */
    TH1F** m_hitMapUCl = {};
    /** Hitmaps clusters for v */
    TH1F** m_hitMapVCl = {};
    /** Hitmaps clusters */
    TH2F** m_hitMapCl = {};
    /** Seed */
    TH1F** m_seed = {};

  };

}

