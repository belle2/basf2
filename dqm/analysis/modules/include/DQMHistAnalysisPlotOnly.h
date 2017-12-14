//+
// File : DQMHistAnalysisPlotOnly.h
// Description : Module for DQM Histogram analysis
//
// Author : B. Spruck
// Date : 13 - Oct - 2017
//-

#ifndef _Belle2_DQMHistAnalysisPlotOnly_h
#define _Belle2_DQMHistAnalysisPlotOnly_h

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <string>
#include <TCanvas.h>

namespace Belle2 {

  class DQMHistAnalysisPlotOnlyModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisPlotOnlyModule();
    virtual ~DQMHistAnalysisPlotOnlyModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;

    //TH1* findHistLocal(TString a);
    TH1* GetHisto(TString a);

    /** Parameter list for histograms */
    std::map< std::string, TCanvas*> m_canvasList;

  };
} // end namespace Belle2

#endif
