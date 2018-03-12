//+
// File : DQMHistAnalysisInput.h
// Description : Input module for DQM Histogram analysis
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 25 - Dec - 2015 ; first commit
//-

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <daq/dqm/DqmMemFile.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>

#include <string>
#include <map>
#include <vector>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisInputModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisInputModule();
    virtual ~DQMHistAnalysisInputModule();

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
    bool m_autocanvas;
    bool m_remove_empty;
    std::vector<std::string> m_acfolders;

    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
    std::map<std::string, TCanvas*> m_cs;

    //! Exp number, Run number
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned int m_count;
  };
} // end namespace Belle2

