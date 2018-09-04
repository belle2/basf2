//+
// File : DQMHistAnalysisPXDCM.h
// Description : DAQM Analysis for PXD Common Modes
//
// Author : Bjoern Spruck, University Mainz
// Date : 2018
//-

#pragma once

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Common Modes */

  class DQMHistAnalysisPXDCMModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisPXDCMModule();
  private:
    ~DQMHistAnalysisPXDCMModule() override final;

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void endRun(void) override final;
    void terminate(void) override final;

    // Data members
    enum { NUM_MODULES = 40}; // we want that from geometry
    std::string m_histodir;
    std::map <int, int> m_id_to_inx;
    std::map <int, int> m_inx_to_id;

    TH2F* m_hCommonMode;
    TCanvas* m_cCommonMode;
    TLine* m_line1, *m_line2, *m_line3;

    TH1* findHistLocal(TString& a);

  };
} // end namespace Belle2

