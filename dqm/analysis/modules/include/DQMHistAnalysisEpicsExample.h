//+
// File : DQMHistAnalysisEpicsExample.h
// Description : An example module for DQM histogram analysis
//
// Author : Tomoyuki Konno, Tokyo Metroplitan Univerisity
// Date : 25  - Dec - 2015 ; first commit
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisEpicsExampleModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisEpicsExampleModule();
    virtual ~DQMHistAnalysisEpicsExampleModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:
    std::string m_histoname;
    std::string m_function;
    Int_t m_parameters;
    std::string  m_pvPrefix;

    TF1* m_f1;
    TCanvas* m_c1;
    TLine* m_line, * m_line_lo, * m_line_hi;

#ifdef _BELLE2_EPICS
    chid mychid[10];// hard limit max 10 parameters
#endif
  };
} // end namespace Belle2

