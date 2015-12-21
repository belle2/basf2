//+
// File : storageoutput.h
// Description : Sequential ROOT I/O output module for DAQ
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//        9  - Dec - 2013 ; update for DAQ
//-

#ifndef _Belle2_DQMHistAnalysisExample_h
#define _Belle2_DQMHistAnalysisExample_h

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <daq/slc/system/Time.h>

#include <daq/slc/apps/dqmviewd/HistMemory.h>
#include <daq/slc/apps/dqmviewd/modules/DQMHistAnalysis.h>

#include <string>

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
    //! Hist memory
    HistMemory m_memory;
    std::string m_mempath;
    int m_memsize;

    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! Exp number, Run number
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned int m_count;

  };
} // end namespace Belle2

#endif
