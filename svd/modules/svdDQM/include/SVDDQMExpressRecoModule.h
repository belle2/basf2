/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/dataobjects/VxdID.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <vector>
#include "TList.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** SVD DQM Module for Express Reco */
  class SVDDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    SVDDQMExpressRecoModule();
    /** Copy constructor (disabled) */
    SVDDQMExpressRecoModule(const SVDDQMExpressRecoModule&) = delete;
    /* Destructor */
    virtual ~SVDDQMExpressRecoModule();
    /** Operator = (disabled) */
    SVDDQMExpressRecoModule& operator=(const SVDDQMExpressRecoModule&) = delete;

    /** Module function initialize */
    void initialize() override final;
    /** Module function terminate */
    void terminate() override final;
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

    /** Trigger Summary data object */
    StoreObjPtr<TRGSummary> m_objTrgSummary;

    StoreObjPtr<SVDEventInfo> m_svdEventInfo ;  /**< SVDEventInfo data object */
    /** if TRUE: svdTime back in SVD time reference*/
    bool m_desynchSVDTime = false;

    /** Store Object for reading the trigger decision. */
    StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer;
    /** if true skip events rejected by HLT (default)*/
    bool m_skipRejectedEvents = true;

    /** additional plots flag*/
    bool m_additionalPlots = false;

    /** list of cumulative histograms */
    TList* m_histoList = nullptr;

    /** experiment number*/
    int m_expNumber = 0;
    /** run number*/
    int m_runNumber = 0;

    /** Flag to show all histos in DQM, default = 0 (do not show)*/
    int m_ShowAllHistos = 0;

    /** cut for accepting strips to hitmap histogram default = 0 ADU*/
    float m_CutSVDCharge = 0.0;

    /** cut for accepting clusters to hitmap histogram, default = 0 ke- */
    float m_CutSVDClusterCharge = 0.0;

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    /** not zero-suppressed SVDShaperDigits StoreArray name */
    std::string m_storeNoZSSVDShaperDigitsName;
    /** SVDShaperDigits StoreArray name */
    std::string m_storeSVDShaperDigitsName;
    /** SVDClusters StoreArray name */
    std::string m_storeSVDClustersName;

    /** number of events */
    TH1F* m_nEvents = nullptr;

    /** Hitmaps u of Digits */
    TH1F* m_hitMapCountsU = nullptr;
    /** Hitmaps v of Digits */
    TH1F* m_hitMapCountsV = nullptr;
    /** Hitmaps u of Clusters*/
    TH1F* m_hitMapClCountsU = nullptr;
    /** Hitmaps v of Clusters*/
    TH1F* m_hitMapClCountsV = nullptr;
    /** Hitmaps of digits on chips */
    TH1F* m_hitMapCountsChip = nullptr;
    /** Hitmaps of clusters on chips */
    TH1F* m_hitMapClCountsChip = nullptr;
    /** Fired u strips per event */
    TH1F** m_firedU = nullptr;
    /** Fired v strips per event */
    TH1F** m_firedV = nullptr;
    /** number of u clusters per event */
    TH1F** m_clustersU = nullptr;
    /** number of v clusters per event */
    TH1F** m_clustersV = nullptr;

    /** u charge of clusters */
    TH1F** m_clusterChargeU = nullptr;
    /** v charge of clusters */
    TH1F** m_clusterChargeV = nullptr;
    /** u charge of clusters for all sensors */
    TH1F* m_clusterChargeUAll = nullptr;
    /** v charge of clusters for all sensors */
    TH1F* m_clusterChargeVAll = nullptr;
    /** u charge of clusters for layer 3 sensors */
    TH1F* m_clusterChargeU3 = nullptr;
    /** v charge of clusters for layer 3  sensors */
    TH1F* m_clusterChargeV3 = nullptr;
    /** u charge of clusters for layer 4,5,6 sensors */
    TH1F* m_clusterChargeU456 = nullptr;
    /** v charge of clusters for layer 4,5,6 sensors */
    TH1F* m_clusterChargeV456 = nullptr;

    /** u SNR of clusters per sensor */
    TH1F** m_clusterSNRU = nullptr;
    /** v SNR of clusters per sensor */
    TH1F** m_clusterSNRV = nullptr;
    /** u SNR of clusters for all sensors */
    TH1F* m_clusterSNRUAll = nullptr;
    /** v SNR of clusters for all sensors */
    TH1F* m_clusterSNRVAll = nullptr;
    /** u SNR of clusters for layer 3 sensors */
    TH1F* m_clusterSNRU3 = nullptr;
    /** v SNR of clusters for layer 3  sensors */
    TH1F* m_clusterSNRV3 = nullptr;
    /** u SNR of clusters for layer 4,5,6 sensors */
    TH1F* m_clusterSNRU456 = nullptr;
    /** v SNR of clusters for layer 4,5,6 sensors */
    TH1F* m_clusterSNRV456 = nullptr;

    /** u MaxBin of strips for all sensors (offline Zero Suppression)*/
    TH1F* m_stripMaxBinUAll = nullptr;
    /** v MaxBin of strips for all sensors (offline Zero Suppression)*/
    TH1F* m_stripMaxBinVAll = nullptr;
    /** u MaxBin of strips for layer 3 sensors (offline Zero Suppression)*/
    TH1F* m_stripMaxBinU3 = nullptr;
    /** v MaxBin of strips for layer 3  sensors (offline Zero Suppression)*/
    TH1F* m_stripMaxBinV3 = nullptr;
    /** u MaxBin of strips for layer 6 sensors (offline Zero Suppression)*/
    TH1F* m_stripMaxBinU6 = nullptr;
    /** v MaxBin of strips for layer 6 sensors (offline Zero Suppression)*/
    TH1F* m_stripMaxBinV6 = nullptr;

    /** u charge of strips */
    TH1F** m_stripSignalU = nullptr;
    /** v charge of strips */
    TH1F** m_stripSignalV = nullptr;
    /** u strip count */
    TH1F** m_stripCountU = nullptr;
    /** v strip count */
    TH1F** m_stripCountV = nullptr;
    /** u strip count (online Zero Suppression) */
    TH1F** m_onlineZSstripCountU = nullptr;
    /** v strip count (online Zero Suppression */
    TH1F** m_onlineZSstripCountV = nullptr;

    /** U strip count for cluster time group Id = 0 */
    TH1F** m_stripCountGroupId0U = nullptr;

    /** V strip count for cluster time group Id = 0 */
    TH1F** m_stripCountGroupId0V = nullptr;

    /** u strip count for 3 samples */
    TH1F** m_strip3CountU = nullptr;
    /** v strip count  for 3 samples*/
    TH1F** m_strip3CountV = nullptr;
    /** u strip count (online Zero Suppression)  for 3 samples*/
    TH1F** m_onlineZSstrip3CountU = nullptr;
    /** v strip count (online Zero Suppression  for 3 samples*/
    TH1F** m_onlineZSstrip3CountV = nullptr;


    /** u strip count for 6 samples */
    TH1F** m_strip6CountU = nullptr;
    /** v strip count  for 3 samples*/
    TH1F** m_strip6CountV = nullptr;
    /** u strip count (online Zero Suppression)  for 6 samples*/
    TH1F** m_onlineZSstrip6CountU = nullptr;
    /** v strip count (online Zero Suppression  for 6 samples*/
    TH1F** m_onlineZSstrip6CountV = nullptr;

    /** u size */
    TH1F** m_clusterSizeU = nullptr;
    /** v size */
    TH1F** m_clusterSizeV = nullptr;

    /** time group id for  U side**/
    TH2F* m_clusterTimeGroupIdU = nullptr;
    /** time group id for  V side**/
    TH2F* m_clusterTimeGroupIdV = nullptr;


    /** time group id for  U side for fine trigger**/
    TH2F* m_clusterTime3GroupIdU = nullptr;
    /** time group id for  V side for fine trigger**/
    TH2F* m_clusterTime3GroupIdV = nullptr;

    /** time group id for  U side for coarse trigger**/
    TH2F* m_clusterTime6GroupIdU = nullptr;
    /** time group id for  V side for coarse trigger**/
    TH2F* m_clusterTime6GroupIdV = nullptr;

    /** u time */
    TH1F** m_clusterTimeU = nullptr;
    /** v time */
    TH1F** m_clusterTimeV = nullptr;
    /** u time of clusters for all sensors */
    TH1F* m_clusterTimeUAll = nullptr;
    /** v time of clusters for all sensors */
    TH1F* m_clusterTimeVAll = nullptr;
    /** u Time of clusters for layer 3 sensors */
    TH1F* m_clusterTimeU3 = nullptr;
    /** v Time of clusters for layer 3  sensors */
    TH1F* m_clusterTimeV3 = nullptr;
    /** u Time of clusters for layer 4,5,6 sensors */
    TH1F* m_clusterTimeU456 = nullptr;
    /** v Time of clusters for layer 4,5,6 sensors */
    TH1F* m_clusterTimeV456 = nullptr;

    /** u Time of clusters for layer 3 sensors  for 3 samples*/
    TH1F* m_cluster3TimeU3 = nullptr;
    /** v Time of clusters for layer 3  sensors   for 3 samples*/
    TH1F* m_cluster3TimeV3 = nullptr;
    /** u Time of clusters for layer 4,5,6 sensors  for 3 samples*/
    TH1F* m_cluster3TimeU456 = nullptr;
    /** v Time of clusters for layer 4,5,6 sensors  for 3 samples*/
    TH1F* m_cluster3TimeV456 = nullptr;

    /** u Time of clusters for layer 3 sensors   for 6 samples*/
    TH1F* m_cluster6TimeU3 = nullptr;
    /** v Time of clusters for layer 3  sensors  for 6 samples*/
    TH1F* m_cluster6TimeV3 = nullptr;
    /** u Time of clusters for layer 4,5,6 sensors  for 6 samples */
    TH1F* m_cluster6TimeU456 = nullptr;
    /** v Time of clusters for layer 4,5,6 sensors  for 6 samples */
    TH1F* m_cluster6TimeV456 = nullptr;

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

  };

}
