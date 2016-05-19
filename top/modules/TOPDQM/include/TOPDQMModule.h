/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric,  Dan Santel                                *
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
#include <vector>
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"

namespace Belle2 {

  /**
   * TOP DQM histogrammer
   */
  class TOPDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    TOPDQMModule();

    /**
     * Destructor
     */
    virtual ~TOPDQMModule();

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
    std::string m_histogramDirectoryName; /**< histogram directory in ROOT file */
    double m_momentumCut; /**< momentum cut */
    double m_pValueCut;   /**< pValue cut */
    bool m_usePionID; /**< use pion ID in track selection */

    // histograms
    TH1F* m_barHits = 0;  /**< number of hits per bar */
    std::vector<TH1F*> m_pixelHits; /**< number of hits per pixel for each bar */
    std::vector<TH1F*> m_hitTimes;  /**< time distribution for each bar */
    TH1F* m_recoTime = 0;           /**< time distribution */
    TH1F* m_recoTimeBg = 0;         /**< time distribution background */
    TH1F* m_recoTimeMinT0 = 0;      /**< time distribution in respect to the first peak */
    TH1F* m_recoTimeDiff = 0;       /**< residual distribution */
    TH1F* m_recoPull = 0;           /**< pull distribution */
    TH2F* m_recoTimeDiff_Phic = 0;  /**< residuals vs phiCer */
    TProfile* m_recoPull_Phic = 0;  /**< pulls vs phiCer */

    // other
    int m_numModules = 0; /**< number of TOP modules */

  };

} // Belle2 namespace

