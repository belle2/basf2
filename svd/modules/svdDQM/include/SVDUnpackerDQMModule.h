/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jarek Wiechczynski                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDUnpackerDQMModule_H
#define SVDUnpackerDQMModule_H

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDDAQDiagnostic.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <framework/dataobjects/EventMetaData.h>
#include <svd/online/SVDOnlineToOfflineMap.h>
#include <framework/database/PayloadFile.h>
#include <memory>

#include <framework/core/HistoModule.h>
#include <string>
#include <TH2S.h>

namespace Belle2 {

  /** SVD DQM Module for the Unpacker*/
  class SVDUnpackerDQMModule : public HistoModule {

  public:

    /** Constructor */
    SVDUnpackerDQMModule();

    /* Destructor */
    virtual ~SVDUnpackerDQMModule();

    /** Module function initialize */
    void initialize() override final;
    /** Module function beginRun */
    void beginRun() override final;
    /** Module function event */
    void event() override final;
    /** Module function endRun */
    void endRun() override final;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

  private:
    /** Required input for EventMetaData */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** SVDEventInfo StoreObjectPointer */
    StoreObjPtr<SVDEventInfo> m_svdEventInfo;

    /** Trigger Summary data object */
    StoreObjPtr<TRGSummary> m_objTrgSummary;

    /** SVD diagnostics module name */
    std::string m_SVDDAQDiagnosticsName;

    /** SVDEventInfo name */
    std::string m_SVDEventInfoName;

    /** mapping implementation */
    std::unique_ptr<SVDOnlineToOfflineMap> m_map;
    static std::string m_xmlFileName; /**< xml filename*/
    DBObjPtr<PayloadFile> m_mapping; /**< channel map payload*/

    StoreArray<SVDDAQDiagnostic> m_svdDAQDiagnostics; /**<SVDDAQDiagnostic StoreArray*/
    StoreObjPtr<EventMetaData> m_eventMetaData; /**<EvtMetaData StoreObjectPointer*/

    bool m_changeFADCaxis = false; /**<change FADC (y) axis*/
    bool m_shutUpNoData = true; /**<shut up if no data comes*/

    int m_expNumber = 0; /**< experiment number*/
    int m_runNumber = 0; /**< run number*/

    bool m_badEvent = false; /**<indicates if the particular event has any SVD error*/
    unsigned int m_nEvents = 0;  /**<event counter */
    unsigned int m_nBadEvents = 0;  /**<counter of events with any kind of error*/
    float m_errorFraction = 0; /**<fraction of events with any kind of error*/

    uint16_t m_ftbError = 0; /**<FTB error container*/
    uint16_t m_ftbFlags = 0; /**<FTB flags container*/
    uint16_t m_apvError = 0; /**<APV error container*/
    bool m_apvMatch = true; /**<apv match error*/
    bool m_fadcMatch = true; /**<fadcc match error*/
    bool m_upsetAPV = false; /**<upset APV error*/
    bool m_badMapping = false; /**<bad mapping error*/
    bool m_badHeader = false; /**<bad header error*/
    bool m_badTrailer = false; /**<bad trailer error*/
    bool m_missedHeader = false; /**<missed Header error*/
    bool m_missedTrailer = false; /**<missed Trailer error*/

    unsigned short m_fadcNo = 0; /**<fadc number*/
    //unsigned short apvNo;

    std::unordered_set<unsigned char>* m_FADCs; /**< FADC boards number*/
    std::unordered_map<unsigned short, unsigned short> m_fadc_map; /**< FADC board number map*/
    std::vector<unsigned short> m_vec_fadc; /**<vector of FADC boards*/

    //histogram
    TH2F* m_DQMUnpackerHisto = nullptr; /**< TH2F histogram with Unpacking errors*/
    TH1F* m_DQMEventFractionHisto = nullptr; /**< TH1F histogram showing the fraction of events affected by errors*/
    TH2F* m_DQMnSamplesHisto = nullptr; /**< TH2F histogram showing number of samples in data VS daqMode */
    TH2F* m_DQMnSamplesHisto2 = nullptr; /**< TH2F histogram showing number of samples in data VS daqMode (2bins only) */
    TH2F* m_DQMtrgQuality = nullptr; /**< TH2F histogram showing number of samples in data VS Trigger Quality */

  };

}

#endif // SVDUnpackerDQMModule_H

