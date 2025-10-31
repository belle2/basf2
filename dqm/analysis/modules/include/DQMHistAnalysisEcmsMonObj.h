/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <unordered_map>
#include <string>
//DQM
#include <dqm/core/DQMHistAnalysis.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <RooDataHist.h>
#include <RooAddPdf.h>
#include <RooArgusBG.h>
#include <RooGaussian.h>
#include <RooRealVar.h>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisEcmsMonObjModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisEcmsMonObjModule();

    /**
     * Destructor
     */
    ~DQMHistAnalysisEcmsMonObjModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override final;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate() override final;


    /**
     * Plot the fit and return TCanvas with the plot
     */
    TCanvas* plotArgusFit(RooDataHist* dataE0, RooAddPdf&  sumB0, RooArgusBG& argus,
                          RooGaussian& gauss, RooRealVar& eNow, TString nTag = "");

    /**
     * Fit the histograms and return the fitted parameters
     */
    std::unordered_map<std::string, double>  fitEcmsBB(TH1D* hB0, TH1D* hBp);

  protected:
    MonitoringObject* m_monObj = nullptr; /**< monitoring object */
    TCanvas* m_canvas = nullptr; /**< Canvas to keep plots of the fit */

  };

} // Belle2 namespace




