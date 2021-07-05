/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Module */
  class vxdDigitMaskingModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    vxdDigitMaskingModule();
    /* Destructor */
    virtual ~vxdDigitMaskingModule();

    /** Initialize */
    void initialize() override final;
    /** Begin run */
    void beginRun() override final;
    /** Event */
    void event() override final;
    /** End run */
    void endRun() override final;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

  private:

    /** directory fo histograms name */
    std::string m_histogramDirectoryName;
    /** PXDDigits StoreArray name */
    std::string m_storePXDDigitsName;
    /** SVDShaperDigits StoreArray name */
    std::string m_storeSVDShaperDigitsName;


    /** Name of file with list of PXD basic masked channels */
    std::string m_PXDMaskFileBasicName;
    /** Name of file with list of SVD basic masked channels */
    std::string m_SVDMaskFileBasicName;
    /** Name of file with list of PXD masked channels */
    std::string m_PXDMaskFileRunName;
    /** Name of file with list of SVD masked channels */
    std::string m_SVDMaskFileRunName;

    /** Number of events to process */
    long m_nEventsProcess;
    /** Real Number of events to process */
    long m_nRealEventsProcess{ -1};
    /** Fraction of events to process to expected No. */
    float m_nEventsProcessFraction{ -1.0};
    /** How many events in plane */
    long* m_nEventsPlane{nullptr};
    /** Set option for append of existing file or recreate new list */
    int m_AppendMaskFile;

    /** Cut for charge of PXD pixel - in ADU */
    float m_PXDChargeCut = 10;
    /** Cut for charge of SVD u strip - in ADU */
    float m_SVDuChargeCut = 10;
    /** Cut for charge of SVD v strip - in ADU */
    float m_SVDvChargeCut = 10;

    /** Cut for masking of PXD pixel - preset for 1 kEvent */
    float m_PXDCut = 50;
    /** Cut for masking of SVD u strip - preset for 1 kEvent */
    float m_SVDuCut = 50;
    /** Cut for masking of SVD v strip - preset for 1 kEvent */
    float m_SVDvCut = 50;

    /** Hitmaps for pixels by PXD plane */
    TH2F** m_PXDHitMapUV{nullptr};
    /** mask for pixels by PXD plane */
    TH2F** m_PXDMaskUV{nullptr};

    /** SVD hitmaps for u strips and timestamp by plane */
    TH1F** m_SVDHitMapU{nullptr};
    /** SVD hitmaps for v strips and timestamp by plane */
    TH1F** m_SVDHitMapV{nullptr};
    /** SVD mask for u strips by plane */
    TH1F** m_SVDMaskU{nullptr};
    /** SVD mask for v strips by plane */
    TH1F** m_SVDMaskV{nullptr};
    /** SVD hitmaps for u strips and timestamp by plane */
    TH1F** m_SVDHitMapU2{nullptr};
    /** SVD hitmaps for v strips and timestamp by plane */
    TH1F** m_SVDHitMapV2{nullptr};
    /** SVD mask for u strips by plane */
    TH1F** m_SVDMaskU2{nullptr};
    /** SVD mask for v strips by plane */
    TH1F** m_SVDMaskV2{nullptr};
  };

}
