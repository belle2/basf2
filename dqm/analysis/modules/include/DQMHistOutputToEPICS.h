/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistOutputToEPICS.h
// Description : Write Histogram Content to EPICS Arrays
//
// Author : Bjoern Spruck, University Mainz
// Date : 2019
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>
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
    typedef struct {
      chid    mychid;
      chid    mychid_last;
      std::string  histoname;
      std::vector <double> data;
    } MYNODE;
#endif

    // Public functions
  public:

    //! Constructor
    DQMHistOutputToEPICSModule();
    //! Destructor
    ~DQMHistOutputToEPICSModule();

  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void endRun(void) override final;
    void event(void) override final;
    void terminate(void) override final;

    //! copy over to "last" PV
    void copyToLast(void);
    //! set PVs to zero content (at run start)
    void cleanPVs(void);

    // Data members
    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;

    /** Flag to mark that a new runs as started anddata not copied to last PV */
    bool m_dirty = false;

#ifdef _BELLE2_EPICS
    std::vector<MYNODE*> pmynode;
#endif

  };
} // end namespace Belle2

