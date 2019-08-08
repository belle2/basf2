//+
// File : DQMHistAnalysisExampleFlags.h
// Description : An example module for DQM histogram analysis
//
// Author : Bjoern Spruck, University Mainz
// Date : 2017
//-

#pragma once

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisExampleFlagsModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisExampleFlagsModule();
  private:

    //! Module functions to be called from main process
    void initialize() override final;

    //! Module functions to be called from event process
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
    void terminate() override final;

    // Data members

    /** The histogram for the test. */
    TH2F* m_hFlagtest = nullptr;
    /** The drawing canvas for the test. */
    TCanvas* m_cFlagtest = nullptr;

  };
} // end namespace Belle2

