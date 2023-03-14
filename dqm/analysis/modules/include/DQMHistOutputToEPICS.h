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
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <string>

#ifdef _BELLE2_EPICS
#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"
#endif

namespace Belle2 {
  /*! Write DQM Histogram Content to EPICS Arrays */

  class DQMHistOutputToEPICSModule final : public DQMHistAnalysisModule {

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

    /**
     * Constructor.
     */
    DQMHistOutputToEPICSModule();

    /**
     * Destructor.
     */
    ~DQMHistOutputToEPICSModule();

    /**
     * Initializer.
     */
    void initialize(void) override final;

    /**
     * Called when entering a new run.
     */
    void beginRun(void) override final;

    /**
     * This method is called for each event.
     */
    void endRun(void) override final;

    /**
     * This method is called if the current run ends.
     */
    void event(void) override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate(void) override final;

  private:

    //! copy over to "last" PV
    void copyToLast(void);
    //! set PVs to zero content (at run start)
    void cleanPVs(void);

    // Data members
    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;

    /** Flag to mark that a new runs as started and data not copied to last PV */
    bool m_dirty = false;

#ifdef _BELLE2_EPICS
    std::vector<MYNODE*> pmynode;
#endif

  };
} // end namespace Belle2

