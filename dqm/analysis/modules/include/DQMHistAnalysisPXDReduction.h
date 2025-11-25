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

#include <dqm/core/DQMHistAnalysis.h>

#include <vxd/dataobjects/VxdID.h>
#include <TLine.h>


namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Reduction */

  class DQMHistAnalysisPXDReductionModule final : public DQMHistAnalysisModule {

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
    //! low warn limit for alarm
    double m_meanLowerWarn{NAN};
    //! low error limit for alarm
    double m_meanLowerAlarm{NAN};
    //! high warn limit for alarm
    double m_meanUpperWarn{NAN};
    //! high error limit for alarm
    double m_meanUpperAlarm{NAN};
    /** Update entry interval */
    int m_minEntries = 1000;
    /** Indizes of excluded PXD Modules */
    std::vector<int> m_excluded;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! Histogram covering all modules
    TH1F* m_hReduction = nullptr;
    //! Final Canvas
    TCanvas* m_cReduction = nullptr;

    //! Line in the Canvas to guide the eye
    TLine* m_meanLine = nullptr;
    //! Line in the Canvas to indicate limits
    TLine* m_meanLowerWarnLine = nullptr;
    //! Line in the Canvas to indicate limits
    TLine* m_meanUpperWarnLine = nullptr;
    //! Line in the Canvas to indicate limits
    TLine* m_meanLowerAlarmLine = nullptr;
    //! Line in the Canvas to indicate limits
    TLine* m_meanUpperAlarmLine = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};
  };
} // end namespace Belle2

