/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisTRGECL.h
// Description : DQM analysis for ECL trigger
//
#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <TGraphErrors.h>

namespace Belle2 {
  //! Module for DQM histogram of ECL trigger event timing
  class DQMHistAnalysisTRGECLModule final : public DQMHistAnalysisModule {

  public:

    //! Constructor
    DQMHistAnalysisTRGECLModule();
    //! Destructor
    ~DQMHistAnalysisTRGECLModule();
    //! initialization
    void initialize() override final;
    //! begin run
    void beginRun() override final;
    //! event function
    void event() override final;
    //! end run
    void endRun() override final;
    //! delete pointers
    void terminate() override final;

  private:

    //! minimum entry in EventT0 histogram to fit
    int m_MinEntryForFit = 200;
    //! get EventT0 mean and width
    void getEventT0(std::vector<std::string>, TGraphErrors*, TGraphErrors*);
    //! single Gaussian function
    static double fGaus(double* x, double* par);
    //! fit on EventT0 histogram
    void fitEventT0(TH1* hist, std::vector<double>&);
    //! name of EventT0 histograms
    std::vector<std::string> s_histNameEventT0;
    //! fraction of event timing with different max TC selection
    TGraph* h_EventTimingEnergyFraction = nullptr;
    //! graph of EventT0 mean
    TGraphErrors* h_EventT0Mean  = nullptr;
    //! graph of EventT0 width
    TGraphErrors* h_EventT0Width = nullptr;
    //! canvas for fraction of event timing with different max TC selection
    TCanvas* c_TCEFraction  = nullptr;
    //! canvas for EventT0 mean
    TCanvas* c_EventT0Mean  = nullptr;
    //! canvas for EventT0 width
    TCanvas* c_EventT0Width = nullptr;

  };

} // end namespace Belle2
