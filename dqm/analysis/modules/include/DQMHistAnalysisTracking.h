//+
// File : DQMHistAnalysisTracking.h
// Description : DQM Tracking Analysis
//
// Author : Bjoern Spruck, University Mainz
// Date : 2019
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for Tracking */

  class DQMHistAnalysisTrackingModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisTrackingModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;

    //! Final Canvas
    TCanvas* m_cTrackingError = nullptr;

  };
} // end namespace Belle2

