//+
// File : DQMHistAnalysisPXDFits.h
// Description : An example module for DQM histogram analysis
//
// Author : Tomoyuki Konno, Tokyo Metroplitan Univerisity
// Date : 25  - Dec - 2015 ; first commit
//-

#ifndef _Belle2_DQMHistAnalysisPXDFits_h
#define _Belle2_DQMHistAnalysisPXDFits_h

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisPXDFitsModule : public DQMHistAnalysisModule {

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
    std::string m_histoname;

    TH2F* m_hSignal[64];
    TH2F* m_hCommon[64];
    TH2F* m_hCounts[64];
    TCanvas* m_cSignal[64];
    TCanvas* m_cCommon[64];
    TCanvas* m_cCounts[64];
    TF1* m_fLandau;// only one fit function
    TF1* m_fGaus;// only one fit function

    TH1* findHistLocal(TString& a);

  };
} // end namespace Belle2

#endif
