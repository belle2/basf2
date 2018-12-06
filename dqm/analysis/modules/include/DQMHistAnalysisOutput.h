//+
// File : DQMHistAnalysisOut.h
// Description : Output module for DQM Histogram analysis
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 25 - Dec - 2015 ; first commit
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputModule();
    virtual ~DQMHistAnalysisOutputModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:

  };
} // end namespace Belle2

