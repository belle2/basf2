/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisEpicsExample.h
// Description : An example module for DQM histogram analysis
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TF1.h>
#include <TLine.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisEpicsExampleModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisEpicsExampleModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisEpicsExampleModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

    // Data members
  private:
    /** The name of the histogram. */
    std::string m_histoname;
    /** The definition of the fit function. */
    std::string m_function;
    /** The fit function parameters for EPICS. */
    Int_t m_parameters;
    /** The prefix of PV. */
    std::string  m_pvPrefix;

    /** The fit function. */
    TF1* m_f1 = nullptr;
    /** The drawing canvas. */
    TCanvas* m_c1 = nullptr;
    /** The line for the fitting result. */
    TLine* m_line = nullptr;
    /** The line for the lower bound. */
    TLine* m_line_lo = nullptr;
    /** The line for the higher bound. */
    TLine* m_line_hi = nullptr;

    /** list of pv names */
    std::vector<std::string> mypv;
  };
} // end namespace Belle2

