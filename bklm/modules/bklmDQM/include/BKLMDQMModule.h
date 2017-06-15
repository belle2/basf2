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

#include <framework/dataobjects/DigitBase.h>   //Missing
#include <bklm/dataobjects/BKLMStatus.h>       //Missing
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
    TH1F* m_LayerHits;

    //! Histogram: Lowest 16 bits of the B2TT CTIME signal
    TH1F* m_CTime;

    //! Histogram: MC simulation event hit time
    TH1F* m_SimTime;

    //! Histogram: Reconstructed hit time relative to trigger
    TH1F* m_Time;

    //! Histogram: MC simulation pulse height
    TH1F* m_SimEDep;

    //! Histogram: Reconstructed pulse height
    TH1F* m_EDep;

    //! Histogram: Simulated number of MPPC pixels
    TH1F* m_SimNPixel;

    //! Histogram: Reconstructed number MPPC pixels
    TH1F* m_NPixel;

    //! name of BKLMDigit store array
    std::string m_OutputDigitsName;

  };

} // Belle2 namespace
