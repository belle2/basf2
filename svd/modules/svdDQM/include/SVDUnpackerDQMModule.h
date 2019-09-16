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


#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDDAQDiagnostic.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <framework/dataobjects/EventMetaData.h>
#include <svd/online/SVDOnlineToOfflineMap.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/PayloadFile.h>
#include <memory>

#include <framework/core/HistoModule.h>
#include <string>
#include <TH2S.h>
#include <TDirectory.h>
#include <TCollection.h>
#include <map>

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
    /**< Required input for EventMetaData */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** SVDShaperDigits StoreArray name */
    std::string m_ShaperDigitName;

    /** SVD diagnostics module name */
    std::string m_SVDDAQDiagnosticsName;

    /** mapping implementation */
    std::unique_ptr<SVDOnlineToOfflineMap> m_map;
    static std::string m_xmlFileName; /**< xml filename*/
    DBObjPtr<PayloadFile> m_mapping; /**< channel map payload*/

    StoreArray<SVDShaperDigit> m_svdShapers; /**<SVDShaperDigits Store Arrra*/
    StoreArray<SVDDAQDiagnostic> m_svdDAQDiagnostics; /**<SVDDAQDiagnostic StoreArray*/
    StoreObjPtr<EventMetaData> m_eventMetaData; /**<EvtMetaData StoreObjectPointer*/

    bool changeFADCaxis = false; /**<change FADC (y) axis*/
    bool shutUpNoData = true; /**<shut up if no data comes*/

    int expNumber = 0; /**< experiment number*/
    int runNumber = 0; /**< run number*/

    bool badEvent = false; /**<indicates if the particular event has any SVD error*/
    unsigned int nEvents = 0;  /**<event counter */
    unsigned int nBadEvents = 0;  /**<counter of events with any kind of error*/
    float errorFraction = 0; /**<fraction of events with any kind of error*/

    uint16_t ftbError = 0; /**<FTB error container*/
    uint16_t ftbFlags = 0; /**<FTB flags container*/
    uint16_t apvError = 0; /**< APV error container*/
    bool apvMatch = true; /**< apv match error*/
    bool fadcMatch = true; /**<fadcc match error*/
    bool upsetAPV = false; /**<upset APV error*/
    bool badMapping = false; /**<bad mapping error*/
    bool badHeader = false; /**<bad header error*/
    bool badTrailer = false; /**<bad trailer error*/
    bool missedHeader = false; /** missed Header error*/
    bool missedTrailer = false; /** missed Trailer error*/

    unsigned short fadcNo = 0; /**<fadc number*/
    //unsigned short apvNo;

    std::unordered_set<unsigned char>* FADCs; /**< FADC boards number*/
    std::unordered_map<unsigned short, unsigned short> fadc_map; /**< FADC board number map*/
    std::vector<unsigned short> vec_fadc; /**<vector of FADC boards*/

    //histogram
    TH2S* DQMUnpackerHisto = NULL; /**< TH2S histogram with Unpacking errors*/
    TH1S* DQMEventFractionHisto = NULL; /**< TH1S histogram showing the fraction of events affected by errors*/

  };

}

#endif // SVDUnpackerDQMModule_H

