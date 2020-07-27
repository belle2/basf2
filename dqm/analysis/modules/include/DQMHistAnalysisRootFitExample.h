//+
// File : DQMHistAnalysisRooFitExample.h
// Description : An example module for DQM histogram analysis
//
// Author : B. Spruck
// Date : 25 - Mar - 2017
// based on wrok from Tomoyuki Konno, Tokyo Metropolitan Univerisity
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <RooWorkspace.h>
#include <RooDataHist.h>
#include <RooPlot.h>
#include <RooFitResult.h>
#include <RooAbsPdf.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisRooFitExampleModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisRooFitExampleModule();
    //! Destructor
    ~DQMHistAnalysisRooFitExampleModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void endRun(void) override final;
    void terminate(void) override final;

    // Data member
    /** The RooFit work space. */
    RooWorkspace* w = nullptr;
    /** The fitting variable. */
    RooRealVar* x = nullptr;
    /** The data of histogram for fitting. */
    RooDataHist* data = nullptr;
    /** The plot of the fitting result. */
    RooPlot* plot = nullptr;
    /** The fitting result. */
    RooFitResult* r = nullptr;
    /** The PDF for fitting. */
    RooAbsPdf* model = nullptr;


    /** The drawing canvas for plotting the fitting result. */
    TCanvas* m_c0 = nullptr;

#ifdef _BELLE2_EPICS
    chid  mychid;
#endif
  };
} // end namespace Belle2

