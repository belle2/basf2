//+
// File : DQMHistAnalysisOut.h
// Description : Output module to NSM vars for DQM Histogram analysis
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 25 - Dec - 2015 ; first commit
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysisOutput.h>

namespace Belle2 {

  /*! Class definition for the output module of Sequential ROOT I/O */
  class NSMCallback;

  class DQMHistAnalysisOutputNSMModule : public DQMHistAnalysisOutputModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputNSMModule();
    virtual ~DQMHistAnalysisOutputNSMModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    std::string m_nodename;
    std::string m_rcnodename;
    NSMCallback* m_callback;

  };
} // end namespace Belle2

