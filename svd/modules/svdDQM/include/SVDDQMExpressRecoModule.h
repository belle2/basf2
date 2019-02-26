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
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** SVD DQM Module for Express Reco */
  class SVDDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    SVDDQMExpressRecoModule();
    /* Destructor */
    virtual ~SVDDQMExpressRecoModule();

    /** Module function initialize */
    void initialize() override final;
    /** Module function beginRun */
    void beginRun() override final;
    /** Module function event */
    void event() override final;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

  private:

    /** Flag to show all histos in DQM, default = 0 */
    int m_ShowAllHistos = 0;

    /** cut for accepting to hitmap histogram, using strips only, default = 0 ADU (was 22 ADU) */
    float m_CutSVDCharge = 0.0;
    /** cut for accepting clusters to hitmap histogram, default = 5 ke- */
    float m_CutSVDClusterCharge = 0.0;

    /** No of FADCs, for Phase2: 5,
     *  TODO add to VXD::GeoCache& geo = VXD::Ge... geo.getFADCs() for
     *  keep universal code for Phase 2 and 3
    */
    // int c_nFADC = 5;

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    /** SVDShaperDigits StoreArray name */
    std::string m_storeSVDShaperDigitsName;
    /** SVDClusters StoreArray name */
    std::string m_storeSVDClustersName;
    /** SVD diagnostics module name */
    std::string m_svdDAQDiagnosticsListName;

    /** number of events */
    TH1I* m_nEvents = nullptr;

    /** Hitmaps u of Digits */
    TH1I* m_hitMapCountsU = nullptr;
    /** Hitmaps v of Digits */
    TH1I* m_hitMapCountsV = nullptr;
    /** Hitmaps u of Clusters*/
    TH1I* m_hitMapClCountsU = nullptr;
    /** Hitmaps v of Clusters*/
    TH1I* m_hitMapClCountsV = nullptr;
    /** Hitmaps of digits on chips */
    TH1I* m_hitMapCountsChip = nullptr;
    /** Hitmaps of clusters on chips */
    TH1I* m_hitMapClCountsChip = nullptr;
    /** Fired u strips per event */
    TH1F** m_firedU = nullptr;
    /** Fired v strips per event */
    TH1F** m_firedV = nullptr;
    /** u clusters per event */
    TH1F** m_clustersU = nullptr;
    /** v clusters per event */
    TH1F** m_clustersV = nullptr;
    /** u charge of clusters */
    TH1F** m_clusterChargeU = nullptr;
    /** v charge of clusters */
    TH1F** m_clusterChargeV = nullptr;
    /** u charge of clusters for all sensors */
    TH1F* m_clusterChargeUAll = nullptr;
    /** v charge of clusters for all sensors */
    TH1F* m_clusterChargeVAll = nullptr;
    /** u SNR of clusters per sensor */
    TH1F** m_clusterSNRU = nullptr;
    /** v SNR of clusters per sensor */
    TH1F** m_clusterSNRV = nullptr;
    /** u SNR of clusters for all sensors */
    TH1F* m_clusterSNRUAll = nullptr;
    /** v SNR of clusters for all sensors */
    TH1F* m_clusterSNRVAll = nullptr;
    /** u charge of strips */
    TH1F** m_stripSignalU = nullptr;
    /** v charge of strips */
    TH1F** m_stripSignalV = nullptr;
    /** u strip count */
    TH1F** m_stripCountU = nullptr;
    /** v strip count */
    TH1F** m_stripCountV = nullptr;
    /** u size */
    TH1F** m_clusterSizeU = nullptr;
    /** v size */
    TH1F** m_clusterSizeV = nullptr;
    /** u time */
    TH1F** m_clusterTimeU = nullptr;
    /** v time */
    TH1F** m_clusterTimeV = nullptr;
    /** u time of clusters for all sensors */
    TH1F* m_clusterTimeUAll = nullptr;
    /** v time of clusters for all sensors */
    TH1F* m_clusterTimeVAll = nullptr;

    /** Counter of APV errors (16) */
    // --for future-- TH1I** m_CounterAPVErrors;
    /** Counter of FTB errors (256) */
    // --for future-- TH1I** m_CounterFTBErrors;
    /** Counter of apvErrorOR (16) */
    // --for future-- TH1I** m_CounterApvErrorORErrors;
    /** Counter of FTB Flags (32) */
    // --for future-- TH1I** m_CounterFTBFlags;

    //----------------------------------------------------------------
    // Additional histograms for out of ExpressReco
    //----------------------------------------------------------------

    /** Hitmaps pixels for u */
    TH2F** m_hitMapU = nullptr;
    /** Hitmaps pixels for v */
    TH2F** m_hitMapV = nullptr;
    /** Hitmaps clusters for u */
    TH1F** m_hitMapUCl = nullptr;
    /** Hitmaps clusters for v */
    TH1F** m_hitMapVCl = nullptr;

    /** u charge of clusters for layer 3 sensors */
    TH1F* m_clusterChargeU3 = nullptr;
    /** v charge of clusters for layer 3  sensors */
    TH1F* m_clusterChargeV3 = nullptr;
    /** u charge of clusters for layer 4,5,6 sensors */
    TH1F* m_clusterChargeU456 = nullptr;
    /** v charge of clusters for layer 4,5,6 sensors */
    TH1F* m_clusterChargeV456 = nullptr;


  };

}
