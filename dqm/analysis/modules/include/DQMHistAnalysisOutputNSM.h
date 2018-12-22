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

  /** Class definition for the output module of Sequential ROOT I/O */
  class NSMCallback;

  /**
   * The module to output values to NSM network.
   */
  class DQMHistAnalysisOutputNSMModule : public DQMHistAnalysisOutputModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputNSMModule();
    virtual ~DQMHistAnalysisOutputNSMModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:
    /** The NSM node name. */
    std::string m_nodename;
    /** The RC node name. */
    std::string m_rcnodename;
    /** The NSM callback object. */
    NSMCallback* m_callback = nullptr;

  };
} // end namespace Belle2

