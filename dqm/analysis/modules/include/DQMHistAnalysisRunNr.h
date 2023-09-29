/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisRunNr.h
// Description : DQM Analysis for RunNr Check
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Common Modes */

  class DQMHistAnalysisRunNrModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisRunNrModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisRunNrModule();

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
    //! HLT/ERECO prefix for EPICS PVs
    std::string m_prefix;

    //! Final Canvas
    TCanvas* m_cRunNr = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

  };
} // end namespace Belle2

