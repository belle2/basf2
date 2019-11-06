//+
// File : DQMHistAnalysisIP.h
// Description : Median for IP position with delta histogramming
//
// Author : Björn Spruck
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TLine.h>
#include <TH1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisIPModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisIPModule();
    ~DQMHistAnalysisIPModule();

    //! Module functions to be called from main process
    void initialize() override final;

    //! Module functions to be called from event process
    void beginRun() override final;
    void event() override final;
    void terminate() override final;

    // Data members
  private:
    /** The name of the histogram. */
    std::string m_histoname;
    /** The prefix of PV. */
    std::string  m_pvPrefix;
    /** Update entry intervall */
    int m_min_entries = 1000;

    /** The drawing canvas. */
    TCanvas* m_c1 = nullptr;
    /** The line for the fitting result. */
    TLine* m_line = nullptr;

    /** last histogram */
    TH1* m_h_last = nullptr;

    static const int m_parameters = 2;
#ifdef _BELLE2_EPICS
    chid mychid[m_parameters];// hard limit max 2 parameters
#endif
  };
} // end namespace Belle2

