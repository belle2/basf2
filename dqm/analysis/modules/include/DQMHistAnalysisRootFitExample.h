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

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>
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

    //! Constructor / Destructor
    DQMHistAnalysisRooFitExampleModule();
    virtual ~DQMHistAnalysisRooFitExampleModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    RooWorkspace* w;
    RooRealVar* x;
    RooDataHist* data;
    RooPlot* plot;
    RooFitResult* r;
    RooAbsPdf* model;


    TCanvas* m_c0;

#ifdef _BELLE2_EPICS
    chid  mychid;
#endif
  };
} // end namespace Belle2

