/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDCM.h
// Description : DQM Analysis for PXD Common Modes
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <vxd/dataobjects/VxdID.h>

#include <TH2.h>
#include <TLine.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Common Modes */

  class DQMHistAnalysisPXDCMModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPXDCMModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisPXDCMModule();

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
    /** Update entry intervall */
    int m_minEntries = 10000;

    /** warn level for mean adhoc plot */
    double m_warnMeanAdhoc{};
    /** error level for mean adhoc plot */
    double m_errorMeanAdhoc{};
    /** warn level for outside fraction */
    double m_warnOutsideAdhoc{};
    /** error level for outside fraction */
    double m_errorOutsideAdhoc{};
    /** threshold level/line for outside fraction */
    int m_upperLineAdhoc{};

    //! Module list for masking
    std::vector <std::string> m_parModuleList;
    //! Gate list for masking
    std::vector <std::vector<int>> m_parGateList;

    //! Module wise gate masking in CM plot and alarm
    std::map <VxdID, std::vector<int>> m_maskedGates;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! histogram covering all modules
    TH2D* m_hCommonModeDelta = nullptr;
    //! Final Canvas
    TCanvas* m_cCommonModeDelta = nullptr;
    //! Line in the Canvas to guide the eye
    TLine* m_line1 = nullptr;
    //! Line in the Canvas to guide the eye
    TLine* m_lineA = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

#ifdef _BELLE2_EPICS
    //! epics PVs
    std::vector <chid> mychid;
    //! epics PVs
    std::map <VxdID, chid> mychid_mean;
#endif
  };
} // end namespace Belle2

