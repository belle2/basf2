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

#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>
#include <TCollection.h>
#include <map>

namespace Belle2 {

  /** SVD DQM Module .......*/
  class SVDUnpackerDQMModule : public Module {

  public:

    /** Constructor */
    SVDUnpackerDQMModule();

    /* Destructor */
    virtual ~SVDUnpackerDQMModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /* ROOT File */
    std::string m_rootFileName;
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */
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
    static std::string m_xmlFileName;
    DBObjPtr<PayloadFile> m_mapping;

    StoreArray<SVDShaperDigit> m_svdShapers;
    StoreArray<SVDDAQDiagnostic> m_svdDAQDiagnostics;
    StoreObjPtr<EventMetaData> m_eventMetaData;

    bool changeFADCaxis;

    uint16_t ftbError;
    uint16_t ftbFlags;
    uint16_t apvError;
    bool apvMatch;
    bool fadcMatch;
    bool upsetAPV;
    bool badMapping;
    unsigned short fadcNo;
    //unsigned short apvNo;

    std::unordered_set<unsigned char>* FADCs;
    std::unordered_map<unsigned short, unsigned short> fadc_map;
    std::vector<unsigned short> vec_fadc;

    //histogram
    TH2S* DQMUnpackerHisto;

  };

}

#endif // SVDUnpackerDQMModule_H

