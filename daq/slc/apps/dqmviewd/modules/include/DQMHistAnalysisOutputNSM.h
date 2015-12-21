//+
// File : storageoutput.h
// Description : Sequential ROOT I/O output module for DAQ
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//        9  - Dec - 2013 ; update for DAQ
//-

#ifndef _Belle2_DQMHistAnalysisOutputNSM_h
#define _Belle2_DQMHistAnalysisOutputNSM_h

#include <daq/slc/apps/dqmviewd/modules/DQMHistAnalysisOutput.h>
#include <framework/core/Module.h>

#include <string>

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

#endif
