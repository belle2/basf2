/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisExampleFlags.h
// Description : An example module for DQM histogram analysis
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TH2F.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisExampleFlagsModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisExampleFlagsModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:

    /** The histogram for the test. */
    TH2F* m_hFlagtest = nullptr;
    /** The drawing canvas for the test. */
    TCanvas* m_cFlagtest = nullptr;

  };
} // end namespace Belle2

