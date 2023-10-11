/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisDeltaEpicsMonObjExample.h
// Description : DQM Analysis Delta+Epics+MonObj example
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TLine.h>

namespace Belle2 {
  /*! DQM Delta Histogram Test code and example */

  class DQMHistAnalysisTRGModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisTRGModule();

  private:

    /**
     * Destructor.
     */
    ~DQMHistAnalysisTRGModule();

    /**
     * Initializer.
     */
    void initialize(void) override final;

    /**
     * Called when entering a new run.
     */
    void beginRun(void) override final;

    /**
     * Called when run ends.
     */
    void endRun(void) override final;

    /**
     * This method is called for each event.
     */
    void event(void) override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate(void) override final;

    /**
     * Do the actual processing
     */
    void doHistAnalysis();

  private:

    // Data members
    //! prefix for EPICS PVs
    std::string m_pvPrefix;

    //! Final Canvas
    TCanvas* m_canvas = nullptr;

  };
} // end namespace Belle2

