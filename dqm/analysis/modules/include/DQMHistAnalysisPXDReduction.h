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
    double m_lowarnlimit{NAN};
    //! low error limit for alarm
    double m_loerrorlimit{NAN};
    //! high warn limit for alarm
    double m_hiwarnlimit{NAN};
    //! high error limit for alarm
    double m_hierrorlimit{NAN};
    /** Update entry intervall */
    int m_minEntries = 1000;

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
  };
} // end namespace Belle2

