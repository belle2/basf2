/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDQMMODULE_H
#define CDCDQMMODULE_H

#include <framework/core/HistoModule.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class CDCDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    CDCDQMModule();

    /**
     * Destructor
     */
    virtual ~CDCDQMModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override;

    /**
     * Histogram definitions.
     *
     */
    void defineHisto() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    void beginRun() override;

    /**
     * Event processor.
     */
    void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate() override;


  protected:
    Long64_t m_nEvents = 0;     /**< Number of events processed */
    TH1F* m_hNEvents = nullptr; /**< Histogram of num. events */
    TH1F* m_hADC[300] = {};     /**< Histogram of TDC for all boards (0-299) */
    TH1F* m_hTDC[300] = {};     /**< Histogram of TDC for all boards (0-299) */
    TH1F* m_hTDCbig[300] = {};  /**< Histogram of TDC with coarse bin for all boards (0-299) */
    TH2F* m_hADCTDC[300] = {};  /**< Histogram of ADC vs. TDC for all boards (0-299) */
    TH2F* m_hADCTOT[300] = {};  /**< Histogram of ADC vs. TOT for all boards (0-299) */
    TH1F* m_hHit[56]  = {};     /**< Histogram of hits for all layers (0-55) */
    TH1F* m_hOcc = nullptr;     /**< Histogram of occupancy */

    StoreArray<CDCHit> m_cdcHits;         /**< CDCHit data object */
    StoreArray<CDCRawHit> m_cdcRawHits;   /**< CDCRawHit data object */
    StoreObjPtr<TRGSummary> m_trgSummary; /**< TRG summary data object */

    /**
     * Number of sense wires for each layer.
     */
    int m_nSenseWires[56] = {160, 160, 160, 160, 160, 160, 160, 160,
                             160, 160, 160, 160, 160, 160,
                             192, 192, 192, 192, 192, 192,
                             224, 224, 224, 224, 224, 224,
                             256, 256, 256, 256, 256, 256,
                             288, 288, 288, 288, 288, 288,
                             320, 320, 320, 320, 320, 320,
                             352, 352, 352, 352, 352, 352,
                             384, 384, 384, 384, 384, 384
                            };

    std::string m_cdcHitName;     /**< Name of CDCHit data object */
    std::string m_cdcRawHitName;  /**< Name of CDCRawHit data object */

  };

} // Belle2 namespace

#endif
