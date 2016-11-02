/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// I copied 6 lines below from PXDDQMModule.h - is it realy needed?
#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

#include <framework/core/Module.h>
#include <string>
#include <TH1F.h>
#include <TH2F.h>
#include <TH2Poly.h>
#include <TCanvas.h>
#include <arich/utility/ARICHChannelHist.h>

namespace Belle2 {

  /**
   * Simple DQM module for occuppancy plots etc.
   */
  class ARICHDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    ARICHDQMModule();

    /**
     * Destructor
     */
    virtual ~ARICHDQMModule();

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
     */
    virtual void defineHisto();


    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:
    std::string m_histogramDirectoryName; /**< histogram directory in ROOT file */
    TH1F* m_hHits = 0; /**< histogram for number of hits / event  */
    TH1F* m_hBits = 0; /**< histogram for acumulative hit bitmap distribution (4-bits / hit) */
    TH2F* m_hHitsHapd = 0; /**< accumulated hits per channel */
    TH1F* m_hHitsMerger = 0; /**< accumulated hits per merger board */
    TH1F* m_hHitsCopper = 0; /**< accumulated hits per copper board */
  };

} // Belle2 namespace

