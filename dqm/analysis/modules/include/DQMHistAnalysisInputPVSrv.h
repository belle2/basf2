//+
// File : DQMHistAnalysisInputPVSrv.h
// Description : Input module for DQM Histogram analysis
//
// Author : B. Spruck
// Date : 25 - Mar - 2017
// based on wrok from Tomoyuki Konno, Tokyo Metropolitan Univerisity
//-

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>
//#include <THttpServer.h>

#include <string>
#ifdef _BELLE2_EPICS
#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"
#endif

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

#ifdef _BELLE2_EPICS
#define MAX_PV_NAME_LEN 40

  typedef struct {
    char    name[MAX_PV_NAME_LEN];
    chid    mychid;
    evid    myevid;
    bool    changed;
    TH1*    histo;
    unsigned int binx; // not needed until now
    unsigned int biny; // not needed until now
    unsigned int binmax; // store bincount to prevent overflow
  } MYNODE;
#endif

  class DQMHistAnalysisInputPVSrvModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisInputPVSrvModule();
    virtual ~DQMHistAnalysisInputPVSrvModule();

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
    int m_interval;
    bool m_callback;
    bool m_server;
    //THttpServer* m_serv{0};
    int m_nhist = 0;

    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;

#ifdef _BELLE2_EPICS
    std::vector<MYNODE*> pmynode;
#endif

    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! Exp number, Run number
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned int m_count;
  };
} // end namespace Belle2

