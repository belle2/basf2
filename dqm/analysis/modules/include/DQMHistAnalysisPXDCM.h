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
    /** Update entry interval */
    int m_minEntries = 10000;

    /** warn level for mean */
    double m_warnMean{};
    /** error level for mean */
    double m_errorMean{};
    /** warn level for outside fraction */
    double m_warnOutside{};
    /** error level for outside fraction */
    double m_errorOutside{};
    /** threshold level/line for outside fraction */
    int m_upperLine{};
    /** Indizes of excluded PXD Modules */
    std::vector<int> m_excluded;

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
    //! Line in the Canvas to guide the eye, target CM
    TLine* m_line10 = nullptr;
    //! Line in the Canvas to guide the eye, outside boundary
    TLine* m_lineOutside = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};
  };
} // end namespace Belle2

