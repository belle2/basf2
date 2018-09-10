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
    virtual ~DQMHistAnalysisPXDFitsModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    // std::string m_histoname;
    std::map <int, int> m_id_to_inx;
    std::map <int, int> m_inx_to_id;

    TH1F* m_hSignalAll, *m_hCommonAll, *m_hCountsAll, *m_hOccupancyAll;
    TCanvas* m_cSignalAll, *m_cCommonAll, *m_cCountsAll, *m_cOccupancyAll;

    TH2F* m_hSignal[NUM_MODULES];
    TH2F* m_hCommon[NUM_MODULES];
    TH2F* m_hCounts[NUM_MODULES];
    TCanvas* m_cSignal[NUM_MODULES];
    TCanvas* m_cCommon[NUM_MODULES];
    TCanvas* m_cCounts[NUM_MODULES];
    TF1* m_fLandau;// only one fit function
    TF1* m_fGaus;// only one fit function

    TH1* findHistLocal(TString& a);

  };
} // end namespace Belle2

