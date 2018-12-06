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
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/GeoTools.h>
#include <pxd/unpacking/PXDMappingLookup.h>
#include <vector>
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

    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;
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


    /**< Name of file with list of PXD basic masked channels */
    std::string m_PXDMaskFileBasicName;
    /**< Name of file with list of SVD basic masked channels */
    std::string m_SVDMaskFileBasicName;
    /**< Name of file with list of PXD masked channels */
    std::string m_PXDMaskFileRunName;
    /**< Name of file with list of SVD masked channels */
    std::string m_SVDMaskFileRunName;

    /**< Number of events to process */
    long m_nEventsProcess;
    /**< Real Number of events to process */
    long m_nRealEventsProcess;
    /**< Fraction of events to process to expected No. */
    float m_nEventsProcessFraction;
    /**< How many events in plane */
    long* m_nEventsPlane;
    /**< Set option for append of existing file or recreate new list */
    int m_AppendMaskFile;

    /**< Cut for charge of PXD pixel - in ADU */
    float m_PXDChargeCut = 10;
    /**< Cut for charge of SVD u strip - in ADU */
    float m_SVDuChargeCut = 10;
    /**< Cut for charge of SVD v strip - in ADU */
    float m_SVDvChargeCut = 10;

    /**< Cut for masking of PXD pixel - preset for 1 kEvent */
    float m_PXDCut = 50;
    /**< Cut for masking of SVD u strip - preset for 1 kEvent */
    float m_SVDuCut = 50;
    /**< Cut for masking of SVD v strip - preset for 1 kEvent */
    float m_SVDvCut = 50;

    /**< Hitmaps for pixels by PXD plane */
    TH2F** m_PXDHitMapUV;
    /**< mask for pixels by PXD plane */
    TH2F** m_PXDMaskUV;

    /**< SVD hitmaps for u strips and timestamp by plane */
    TH1F** m_SVDHitMapU;
    /**< SVD hitmaps for v strips and timestamp by plane */
    TH1F** m_SVDHitMapV;
    /**< SVD mask for u strips by plane */
    TH1F** m_SVDMaskU;
    /**< SVD mask for v strips by plane */
    TH1F** m_SVDMaskV;
    /**< SVD hitmaps for u strips and timestamp by plane */
    TH1F** m_SVDHitMapU2;
    /**< SVD hitmaps for v strips and timestamp by plane */
    TH1F** m_SVDHitMapV2;
    /**< SVD mask for u strips by plane */
    TH1F** m_SVDMaskU2;
    /**< SVD mask for v strips by plane */
    TH1F** m_SVDMaskV2;
  };

}
