/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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

    /** CDC hits. */
    StoreArray<CDCHit> m_cdcHits;

    /** CDC raw hits. */
    StoreArray<CDCRawHit> m_cdcRawHits;

    /** Trigger summary. */
    StoreObjPtr<TRGSummary> m_trgSummary;

    Long64_t m_nEvents = 0;          /**< Number of events processed */
    TH1F* m_hNEvents = nullptr;      /**< Histogram of num. events */
    TH2F* m_hADC  = nullptr;              /**< Histogram of ADC for all boards (0-299) */
    TH2F* m_hADCTOTCut = nullptr;    /**< Histogram of ADC with tot cut for all boards (0-299) */
    TH2F* m_hTDC = nullptr;          /**< Histogram of TDC for all boards (0-299) */
    TH2F* m_hHit  = nullptr;          /**< Histogram of hits for all layers (0-55) */
    TH1F* m_hOcc = nullptr;          /**< Histogram of occupancy */
    int m_minHits;         /**< Minimum hits for processing. */
  };

} // Belle2 namespace
