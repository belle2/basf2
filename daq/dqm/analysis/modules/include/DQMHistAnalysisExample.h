//+
// File : DQMHistAnalysisExample.h
// Description : An example module for DQM histogram analysis
//
// Author : Tomoyuki Konno, Tokyo Metroplitan Univerisity
// Date : 25  - Dec - 2015 ; first commit
//-

#ifndef _Belle2_DQMHistAnalysisExample_h
#define _Belle2_DQMHistAnalysisExample_h

#include <framework/core/Module.h>

#include <daq/dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisExampleModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisExampleModule();
    virtual ~DQMHistAnalysisExampleModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    TF1* m_f;

  };
} // end namespace Belle2

#endif
