/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <vtx/dataobjects/VTXDigit.h>
#include <vtx/dataobjects/VTXCluster.h>

#include "TH1I.h"
#include "TH1F.h"

namespace Belle2 {

  /** VTX DQM Module */
  class VTXDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    VTXDQMExpressRecoModule();
    /* Destructor */
    virtual ~VTXDQMExpressRecoModule();

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
    float m_CutVTXCharge = 0.0;

    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

    /** VTXDigits StoreArray name */
    std::string m_storeVTXDigitsName;
    /** VTXClusters StoreArray name */
    std::string m_storeVTXClustersName;

    /** Storearray for Digits  */
    StoreArray<VTXDigit> m_storeVTXDigits;
    /** Storearray for Cluster   */
    StoreArray<VTXCluster> m_storeVTXClusters;

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
    // FIXME: Startrow related histos are expert debugging, not for shifter (-> remove this)
    /** Start row distribution */
    //TH1F** m_startRow={};
    /** Cluster seed charge by distance from the start row */
    //TH1F** m_chargStartRow={};
    /** counter for Cluster seed charge by distance from the start row */
    //TH1F** m_startRowCount={};
    /** Charge of clusters */
    TH1F** m_clusterCharge = {};
    /** Charge of pixels */
    TH1F** m_pixelSignal = {};
    /** u cluster size */
    TH1F** m_clusterSizeU = {};
    /** v cluster size */
    TH1F** m_clusterSizeV = {};
    /** Cluster size */
    TH1F** m_clusterSizeUV = {};

  };

}

