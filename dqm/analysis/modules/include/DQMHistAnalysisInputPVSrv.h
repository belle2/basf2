/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisInputPVSrv.h
// Description : Input module for DQM Histogram analysis
//-

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <dqm/core/DQMHistAnalysis.h>
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

    /**
     * Constructor.
     */
    DQMHistAnalysisInputPVSrvModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisInputPVSrvModule();

    /**
     * Definition of the histograms.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

    // Data members
  private:
    /** The refresh interval in ms. */
    int m_interval;
    /** Whether to use EPICS callback for changes. */
    bool m_callback;
    /** Whether to start http server on port 8082. */
    bool m_server;
    //THttpServer* m_serv{0};
    //int m_nhist = 0;

    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;

#ifdef _BELLE2_EPICS
    std::vector<MYNODE*> pmynode;
#endif

    /** The metadata for each event. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** Exp number */
    unsigned int m_expno = 0;
    /** Run number */
    unsigned int m_runno = 0;
    /** Event number */
    unsigned int m_count = 0;
  };
} // end namespace Belle2

