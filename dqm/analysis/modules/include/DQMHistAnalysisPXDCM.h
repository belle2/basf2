//+
// File : DQMHistAnalysisPXDCM.h
// Description : Analysis of PXD Common Modes
//
// Author : Bjoern Spruck, University Mainz
// Date : 2018
//-

#ifndef _Belle2_DQMHistAnalysisPXDCM_h
#define _Belle2_DQMHistAnalysisPXDCM_h

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisPXDCMModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisPXDCMModule();
    virtual ~DQMHistAnalysisPXDCMModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    enum { NUM_MODULES = 40}; // we want that from geometry
    std::string m_histodir;
    std::map <int, int> m_id_to_inx;
    std::map <int, int> m_inx_to_id;

    TH2F* m_hCommonMode;
    TCanvas* m_cCommonMode;

    TH1* findHistLocal(TString& a);

  };
} // end namespace Belle2

#endif
