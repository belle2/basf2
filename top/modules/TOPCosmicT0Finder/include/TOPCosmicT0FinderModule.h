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

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * Event T0 finder for global cosmic runs
   */
  class TOPCosmicT0FinderModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCosmicT0FinderModule();

    /**
     * Destructor
     */
    virtual ~TOPCosmicT0FinderModule();

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

    bool m_useIncomingTrack; /**< if true use incoming track, otherwise use outcoming */
    unsigned m_minHits; /**< minimal number of hits on TOP module */
    double m_minSignal; /**< minimal number of expected signal photons */
    bool m_applyT0; /**< if true, subtract T0 in TOPDigits */
    int m_numBins;      /**< number of bins to which time range is divided */
    double m_timeRange; /**< time range in which to search [ns] */
    double m_sigma;     /**< additional time spread added to PDF [ns] */
    bool m_saveHistograms; /**< flag to save histograms */

    int m_num = 0; /**< histogram number */
    int m_acceptedCount = 0;  /**< counter for accepted events */
    int m_successCount = 0; /**< counter for successfully determined T0 */

  };

} // Belle2 namespace

