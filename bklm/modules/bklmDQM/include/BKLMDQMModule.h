/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vipin Gaur, Prof. Leo Piilonen                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// Copied 6 lines below from PXDDQMModule.h
#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

#include <framework/dataobjects/DigitBase.h>
#include <bklm/dataobjects/BKLMStatus.h>
#include <framework/core/Module.h>
#include <string>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"

namespace Belle2 {

  /**
   * BKLM DQM histogrammer
   */
  class BKLMDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    BKLMDQMModule();

    /**
     * Destructor
     */
    virtual ~BKLMDQMModule();

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

    // module parameters

    //! Histogram: number of hits per layer
    TH1F* h_layerHits;

    //! Histogram: Lowest 16 bits of the B2TT CTIME signal
    TH1F* h_ctime;

    //! Histogram: MC simulation event hit time
    TH1F* h_simtime;

    //! Histogram: Reconstructed hit time relative to trigger
    TH1F* h_time;

    //! Histogram: MC simulation pulse height
    TH1F* h_simEDep;

    //! Histogram: Reconstructed pulse height
    TH1F* h_eDep;

    //! Histogram: Simulated number of MPPC pixels
    TH1F* h_simNPixel;

    //! Histogram: Reconstructed number MPPC pixels
    TH1F* h_nPixel;

    //! Histogram: Detector-module identifier
    TH1F* h_moduleID;

    //! Histogram: z-measuring strip numbers of the 2D hit
    TH1F* h_zStrips;

    //! Histogram: Phi strip number of muon hit
    TH1F* h_phiStrip;

    //! Histogram: Sector number of muon hit
    TH1F* h_sector;

    //! Histogram: Layer number of muon hit
    TH1F* h_layer;

    //! Histogram: Distance from z axis in transverse plane of muon hit
    TH1F* h_rBKLMHit2ds;

    //! Histogram: Axial position of muon hit
    TH1F* h_zBKLMHit2ds;

    //! Histogram: Position projected into transverse plane of muon hit
    TH2F* h_yvsxBKLMHit2ds;

    //! Histogram: Position projected into x-z plane of muon hit
    TH2F* h_xvszBKLMHit2ds;

    //! Histogram: Position projected into y-z plane of muon hit
    TH2F* h_yvszBKLMHit2ds;

    // other
    //! name of BKLMDigit store array
    std::string m_outputDigitsName;
    std::string m_outputHitsName;
  };

} // Belle2 namespace
