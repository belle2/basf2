/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <map>
#include <framework/core/HistoModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <svd/dbobjects/SVDDQMPlotsConfiguration.h>
#include <framework/database/DBObjPtr.h>
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

    /** if true enable 3 samples histograms analysis */
    bool m_3Samples = false;


    /** if additional histograms (Charge, SNR, time) for a given list of sensors */
    bool  m_addSensorPlots = false;

    std::string m_svdShaperDigitsName;   /**< SVDShaperDigits data object  name*/
    std::string m_svdRecoDigitsName;   /**< SVDRecoDigits data object  name*/
    std::string m_svdClustersName;   /**< SVDClusters data object  name*/
    std::string m_svdEventInfoName;   /**< SVDEventInfo data object  name*/
    StoreObjPtr<SVDEventInfo> m_svdEventInfo;  /**< SVDEventInfo data object */
    StoreObjPtr<EventT0> m_eventT0;  /**< EventT0 data object */
    DBObjPtr<SVDDQMPlotsConfiguration> m_svdPlotsConfig; /**< SVD DQM plots configuration */

    /** StoreArray of the Tracks*/
    StoreArray<Track> m_tracks;

    /** Store Object for reading the trigger decision. */
    StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer;

    /** if true skip events rejected by HLT */
    bool m_skipRejectedEvents = false;

    /** if true read back from DB configuration parameters */
    bool m_useParamFromDB = true;

    int m_tb = -1; /**< choose one trigger bin, or none if the value is -1*/

    /** list of cumulative histograms */
    TList* m_histoList = nullptr;

    /** experiment number*/
    int m_expNumber = 0;
    /** run number*/
    int m_runNumber = 0;

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;


    /** u charge of clusters */
    TH1F** m_clstrkChargeU = nullptr;
    /** v charge of clusters */
    TH1F** m_clstrkChargeV = nullptr;

    /** u SNR of clusters per sensor */
    TH1F** m_clstrkSNRU = nullptr;
    /** v SNR of clusters per sensor */
    TH1F** m_clstrkSNRV = nullptr;

    /** u time */
    TH1F** m_clstrkTimeU = nullptr;
    /** v time */
    TH1F** m_clstrkTimeV = nullptr;

    /** charge of clusters related to tracks per ladder */
    TH1F** m_clsTrkCharge = nullptr;

    /** SNR of clusters related to tracks  per ladder */
    TH1F** m_clsTrkSNR = nullptr;


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

    /** u Time of clusters related to tracks for layer 3 sensors for 3 samples*/
    TH1F* m_cls3SampleTrkTimeU3 = nullptr;
    /** v Time of clusters related to tracks for layer 3  sensors for 3 samples*/
    TH1F* m_cls3SampleTrkTimeV3 = nullptr;

    /** u Time of clusters related to tracks for layer 3 sensors for 3 samples*/
    TH1F* m_cls6SampleTrkTimeU3 = nullptr;
    /** v Time of clusters related to tracks for layer 3  sensors for 3 samples*/
    TH1F* m_cls6SampleTrkTimeV3 = nullptr;

    /** u Time of clusters related to tracks for layer 4,5,6 sensors */
    TH1F* m_clsTrkTimeU456 = nullptr;
    /** v Time of clusters related to tracks for layer 4,5,6 sensors */
    TH1F* m_clsTrkTimeV456 = nullptr;

    /** u Time of clusters related to tracks for layer 4,5,6 sensors  for 3 samples*/
    TH1F* m_cls3SampleTrkTimeU456 = nullptr;
    /** v Time of clusters related to tracks for layer 4,5,6 sensors  for 3 samples*/
    TH1F* m_cls3SampleTrkTimeV456 = nullptr;

    /** u Time of clusters related to tracks for layer 4,5,6 sensors  for 6 samples*/
    TH1F* m_cls6SampleTrkTimeU456 = nullptr;
    /** v Time of clusters related to tracks for layer 4,5,6 sensors  for 6 samples*/
    TH1F* m_cls6SampleTrkTimeV456 = nullptr;

    /** map of ladder index*/
    std::map<std::pair<int, int>, int> m_ladderMap;

    /** list of sensor  to monitor (Charge, SNR, time; U/V)  taken from DB (payload)*/
    std::vector<std::string> m_listOfSensorsToMonitor;

    /** additional list of sensor  to monitor (Charge, SNR, time; U/V)  from parameter */
    std::vector<std::string> m_additionalSensorsToMonitor;


  };

}
