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
//
// Author : Tomoyuki Konno, Tokyo Metroplitan Univerisity
// Date : 25  - Dec - 2015 ; first commit
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisEpicsExampleModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisEpicsExampleModule();
    ~DQMHistAnalysisEpicsExampleModule();

    //! Module functions to be called from main process
    void initialize() override final;

    //! Module functions to be called from event process
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
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

#ifdef _BELLE2_EPICS
    chid mychid[10];// hard limit max 10 parameters
#endif
  };
} // end namespace Belle2

