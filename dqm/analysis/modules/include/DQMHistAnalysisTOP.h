//+
// File : DQMHistAnalysisTOP.h
// Description : An example module for DQM histogram analysis
//
// Author : Boqun Wang
// Date : Feb - 2018
//-

#pragma once

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TH1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisTOPModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisTOPModule();
    virtual ~DQMHistAnalysisTOPModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Data members
  private:
    TCanvas* m_c_particleHitsMean = nullptr;
    TCanvas* m_c_particleHitsRMS = nullptr;
    TCanvas* m_c_otherHitsMean = nullptr;
    TCanvas* m_c_otherHitsRMS = nullptr;

    TH1F* m_h_particleHitsMean = nullptr;
    TH1F* m_h_particleHitsRMS = nullptr;
    TH1F* m_h_otherHitsMean = nullptr;
    TH1F* m_h_otherHitsRMS = nullptr;

  };
} // end namespace Belle2

