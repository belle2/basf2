/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDReduction.h
// Description : DAQM Analysis for PXD Data Reduction
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/core/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>

#include <TLine.h>

//#include <vector>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Reduction */

  class DQMHistAnalysisPXDReductionModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPXDReductionModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisPXDReductionModule();

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
    void event(void) override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate(void) override final;

  private:

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! Histogram covering all modules
    TH1F* m_hReduction = nullptr;
    //! Final Canvas
    TCanvas* m_cReduction = nullptr;

    //! Line in the Canvas to guide the eye
    TLine* m_line1 = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

    /** flag if to export to EPICS */
    bool m_useEpics;

#ifdef _BELLE2_EPICS
    //! EPICS PVs
    std::vector <chid>  mychid;
#endif
  };
} // end namespace Belle2

