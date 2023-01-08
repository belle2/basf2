/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisRooFitExample.h
// Description : An example module for DQM histogram analysis
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/core/DQMHistAnalysis.h>

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

    /**
     * Constructor.
     */
    DQMHistAnalysisRooFitExampleModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisRooFitExampleModule();

    /**
     * Initializer.
     */
    void initialize(void) override final;

    /**
     * Called when entering a new run.
     */
    void beginRun(void) override final;

    /**
     * This method is called for each event.
     */
    void event(void) override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun(void) override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate(void) override final;

  private:

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

