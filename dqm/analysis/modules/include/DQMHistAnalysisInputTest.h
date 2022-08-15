/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <dqm/core/DQMHistAnalysis.h>

#include <string>
#include <TH1.h>
#include <TF1.h>

namespace Belle2 {

  /**
   * Class to read histograms from a root file for offline testing of analysis modules.
   */

  class DQMHistAnalysisInputTestModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisInputTestModule();

    /**
     * Initialize the module.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * Event processor.
     */
    void event() override final;

    /**
     * End-of-run action.
     */
    void endRun() override final;

    /**
     * Termination action.
     */
    void terminate() override final;

    /**
     * Plot/print function for debugging
     */
    void PlotDelta(void);

  private:

    /** Global EventMetaData for run number and event number. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** Exp number. */
    unsigned int m_expno;

    /** Run number. */
    unsigned int m_runno;

    /** total number of events for run. */
    unsigned int m_events;

    /** Evt number. */
    unsigned int m_count{};

    /** the test histogram */
    TH1* m_testHisto{};

    /** function for filling histo */
    TF1* m_func{};
  };
} // end namespace Belle2

