//+
// File : DQMHistAnalysisEpicsExample.h
// Description : An example module for DQM histogram analysis
//
// Author : Tomoyuki Konno, Tokyo Metroplitan Univerisity
// Date : 25  - Dec - 2015 ; first commit
//-

#pragma once

// EPICS
//#include "cadef.h"

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
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    std::string m_histoname;
    std::string m_function;
    Int_t m_parameters;
    std::string  m_pvname;

    TF1* m_f1;
    TCanvas* m_c1;
    TLine* m_line, * m_line_lo, * m_line_hi;

//    chid mychid[10];// hard limit max 10 parameters
  };
} // end namespace Belle2

