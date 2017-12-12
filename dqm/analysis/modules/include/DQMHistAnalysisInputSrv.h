//+
// File : DQMHistAnalysisInputSrv.h
// Description : Input module for DQM Histogram analysis
//
// Author : B. Spruck
// Date : 25 - Mar - 2017
// based on wrok from Tomoyuki Konno, Tokyo Metropolitan Univerisity
//-

#ifndef _Belle2_DQMHistAnalysisInputSrv_h
#define _Belle2_DQMHistAnalysisInputSrv_h

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <daq/dqm/DqmMemFile.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>
//#include <THttpServer.h>

#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisInputSrvModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisInputSrvModule();
    virtual ~DQMHistAnalysisInputSrvModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    //! Hist memory
    DqmMemFile* m_memory;
    std::string m_mempath;
    int m_memsize;
    int m_interval;
    //  THttpServer* m_serv;

    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! Exp number, Run number
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned int m_count;
  };
} // end namespace Belle2

#endif
