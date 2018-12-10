//+
// File : DQMHistAnalysisPXDFits.h
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

  class DQMHistAnalysisPXDFitsModule : public DQMHistAnalysisModule {

    enum { NUM_MODULES = 40}; // we want that from geometry
    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisPXDFitsModule();

  private:

    //! Module functions to be called from main process
    void initialize() override final;

    //! Module functions to be called from event process
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
    void terminate() override final;

    // Data members
    std::string m_histogramDirectoryName;
    std::map <int, int> m_id_to_inx;
    std::map <int, int> m_inx_to_id;

    TH1F* m_hSignalAll = nullptr, *m_hCommonAll = nullptr, *m_hCountsAll = nullptr, *m_hOccupancyAll = nullptr;
    TCanvas* m_cSignalAll = nullptr, *m_cCommonAll = nullptr, *m_cCountsAll = nullptr, *m_cOccupancyAll = nullptr;

    TH2F* m_hSignal[NUM_MODULES];
    TH2F* m_hCommon[NUM_MODULES];
    TH2F* m_hCounts[NUM_MODULES];
    TCanvas* m_cSignal[NUM_MODULES];
    TCanvas* m_cCommon[NUM_MODULES];
    TCanvas* m_cCounts[NUM_MODULES];
    TF1* m_fLandau = nullptr; // only one fit function
    TF1* m_fGaus = nullptr; // only one fit function

  };
} // end namespace Belle2

