/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHDQMMODULE_H
#define ARICHDQMMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <arich/utility/ARICHChannelHist.h>

namespace Belle2 {

  /**
   * Simple DQM module for occuppancy plots etc.
   */
  class ARICHDQMModule : public Module {

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
    TH1F* m_hHits;  /**< histogram for number of hits / event  */
    TH1F* m_hBits;  /**< histogram for acumulative hit bitmap distribution (4-bits / hit) */
    TH2F* m_hHitsHapd;  /**< accumulated hits per channel */
    TH1F* m_hHitsMerger;  /**< accumulated hits per merger board */
    TH1F* m_hHitsCopper;  /**< accumulated hits per copper board */
    ARICHChannelHist* m_chHist; /**< occupancy histogram for all HAPDs */
    std::string m_fname; /**< output root file name */
  };

} // Belle2 namespace

#endif
