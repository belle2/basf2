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
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <framework/dataobjects/EventT0.h>
#include "TList.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** SVD DQM Module for Clusters related to Tracks */
  class SVDDQMClustersOnTrackModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    SVDDQMClustersOnTrackModule();
    /** Copy constructor (disabled) */
    SVDDQMClustersOnTrackModule(const SVDDQMClustersOnTrackModule&) = delete;
    /* Destructor */
    virtual ~SVDDQMClustersOnTrackModule();
    /** Operator = (disabled) */
    SVDDQMClustersOnTrackModule& operator=(const SVDDQMClustersOnTrackModule&) = delete;

    /** Module function initialize */
    void initialize() override final;
    /** Module function terminate */
    void terminate() override final;
    /** Module function beginRun */
    void beginRun() override final;
    /** Module function event */
    void event() override final;

    /** Contains the Histogram definitions    */
    void defineHisto() override final;

  private:

    /** if TRUE: svdTime back in SVD time reference*/
    bool m_desynchSVDTime = false;

    std::string m_svdShaperDigitsName;   /**< SVDShaperDigits data object  name*/
    std::string m_svdRecoDigitsName;   /**< SVDRecoDigits data object  name*/
    std::string m_svdClustersName;   /**< SVDClusters data object  name*/
    std::string m_svdEventInfoName;   /**< SVDEventInfo data object  name*/
    StoreObjPtr<SVDEventInfo> m_svdEventInfo;  /**< SVDEventInfo data object */
    StoreObjPtr<EventT0> m_eventT0;  /**< EventT0 data object */

    /** StoreArray of the Tracks*/
    StoreArray<Track> m_tracks;

    /** Store Object for reading the trigger decision. */
    StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer;

    /** if true skip events rejected by HLT (default)*/
    bool m_skipRejectedEvents = true;

    int m_tb = -1; /**< choose one trigger bin, or none if the value is -1*/

    /** list of cumulative histograms */
    TList* m_histoList = nullptr;

    /** experiment number*/
    int m_expNumber = 0;
    /** run number*/
    int m_runNumber = 0;

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    /** u charge of clusters related to tracks for layer 3 sensors */
    TH1F* m_clsTrkChargeU3 = nullptr;
    /** v charge of clusters related to tracks for layer 3  sensors */
    TH1F* m_clsTrkChargeV3 = nullptr;
    /** u charge of clusters related to tracks for layer 4,5,6 sensors */
    TH1F* m_clsTrkChargeU456 = nullptr;
    /** v charge of clusters related to tracks for layer 4,5,6 sensors */
    TH1F* m_clsTrkChargeV456 = nullptr;

    /** u SNR of clusters related to tracks for layer 3 sensors */
    TH1F* m_clsTrkSNRU3 = nullptr;
    /** v SNR of clusters related to tracks for layer 3  sensors */
    TH1F* m_clsTrkSNRV3 = nullptr;
    /** u SNR of clusters related to tracks for layer 4,5,6 sensors */
    TH1F* m_clsTrkSNRU456 = nullptr;
    /** v SNR of clusters related to tracks for layer 4,5,6 sensors */
    TH1F* m_clsTrkSNRV456 = nullptr;

    /** u MaxBin of strips related to tracks for all sensors */
    TH1F* m_stripMaxBinUAll = nullptr;
    /** v MaxBin of strips related to tracks for all sensors*/
    TH1F* m_stripMaxBinVAll = nullptr;

    /** u Time of clusters related to tracks vs EventT0 */
    TH2F* m_clsTrkTimeUEvtT0 = nullptr;
    /** v Time of clusters related to tracks vs EventT0 */
    TH2F* m_clsTrkTimeVEvtT0 = nullptr;
    /** u Time of clusters related to tracks for layer 3 sensors */
    TH1F* m_clsTrkTimeU3 = nullptr;
    /** v Time of clusters related to tracks for layer 3  sensors */
    TH1F* m_clsTrkTimeV3 = nullptr;
    /** u Time of clusters related to tracks for layer 4,5,6 sensors */
    TH1F* m_clsTrkTimeU456 = nullptr;
    /** v Time of clusters related to tracks for layer 4,5,6 sensors */
    TH1F* m_clsTrkTimeV456 = nullptr;

  };

}
