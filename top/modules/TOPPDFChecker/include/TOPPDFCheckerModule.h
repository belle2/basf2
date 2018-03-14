/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <top/dataobjects/TOPDigit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <string>
#include <vector>
#include <set>
#include <TH2F.h>
#include <TVector3.h>


namespace Belle2 {

  /**
   * Module for checking analytic PDF used in likelihood calculation
   */
  class TOPPDFCheckerModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    TOPPDFCheckerModule();

    /**
     * Destructor
     */
    virtual ~TOPPDFCheckerModule();

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

    /**
     * Checks if digit comes from given MC particle
     */
    bool isFromThisParticle(const TOPDigit& digit, const MCParticle* particle);

    double m_minTime; /**< histogram lower bound in time [ns] */
    double m_maxTime; /**< histogram upper bound in time [ns] */
    int m_numBins;    /**< number of bins in time */

    TH2F* m_hits = 0; /**< histogram of photon hits */
    TH2F* m_pdf = 0;  /**< histogram of PDF */
    TH2F* m_hitsCol = 0; /**< histogram of photon hits projected to pixel columns */
    TH2F* m_pdfCol = 0;  /**< histogram of PDF projected to pixel columns */

    TVector3 m_avrgMomentum; /**< average particle momentum at bar entrance (bar frame) */
    TVector3 m_avrgPosition; /**< average particle position at bar entrance (bar frame) */
    int m_numTracks = 0;     /**< number of tracks */
    std::set<int> m_slotIDs;      /**< slot ID's that are hit by particle */
    std::set<int> m_PDGCodes;     /**< particle PDG codes */

  };

} // Belle2 namespace

