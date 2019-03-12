//+
// File : DQMHistOutputToEPICS.h
// Description : Write Histogram Content to EPICS Arrays
//
// Author : Bjoern Spruck, University Mainz
// Date : 2019
//-

#pragma once

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <TF1.h>
#include <string>

#ifdef _BELLE2_EPICS
#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"
#endif

namespace Belle2 {
  /*! Write DQM Histogram Content to EPICS Arrays */

  class DQMHistOutputToEPICSModule : public DQMHistAnalysisModule {

#ifdef _BELLE2_EPICS
// #define MAX_PV_NAME_LEN 40

    typedef struct {
//     char    name[MAX_PV_NAME_LEN];
      chid    mychid;
      std::string  histoname;
    } MYNODE;
#endif

    // Public functions
  public:

    //! Constructor
    DQMHistOutputToEPICSModule();

  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void terminate(void) override final;

    // Data members
    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;

#ifdef _BELLE2_EPICS
    std::vector<MYNODE*> pmynode;
#endif

  };
} // end namespace Belle2

