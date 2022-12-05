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

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/core/DQMHistAnalysis.h>

#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisEpicsExampleModule : public DQMHistAnalysisModule {

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

#ifdef _BELLE2_EPICS
    chid mychid[10];// hard limit max 10 parameters
#endif
  };
} // end namespace Belle2

