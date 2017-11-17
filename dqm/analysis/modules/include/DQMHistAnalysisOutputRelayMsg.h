//+
// File : DQMHistAnalysisOutputRelayMsg.h
// Description : Output module for DQM Histogram analysis
//
// Author : B. Spruck
// Date : 25 - Mar - 2017
// based on wrok from Tomoyuki Konno, Tokyo Metropolitan Univerisity
//-

#ifndef _Belle2_DQMHistAnalysisOutputRelayMsg_h
#define _Belle2_DQMHistAnalysisOutputRelayMsg_h

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <TSocket.h>
#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputRelayMsgModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputRelayMsgModule();
    virtual ~DQMHistAnalysisOutputRelayMsgModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    TSocket* m_sock;
    int m_port;
    std::string m_hostname;

  };
} // end namespace Belle2

#endif
