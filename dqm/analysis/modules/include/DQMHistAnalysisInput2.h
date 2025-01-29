/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisInput.h
// Description : Input module for DQM Histogram analysis
//-

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <dqm/core/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TKey.h>

#include <string>
#include <map>
#include <vector>
#include <filesystem>

namespace Belle2 {
  /**
   * Class to read histograms from a root file for online analysis modules.
   */

  class DQMHistAnalysisInput2Module : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisInput2Module();

    /**
     * Initialize the module.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

    // Data members
  private:
    /** The name of the shared memory for the histograms. */
    std::string m_mempath;
    /** The name of the memory file (HLT or ExpressReco). */
    /** The refresh interval. */
    int m_interval;
    /** Whether to remove empty histograms. */
    bool m_remove_empty;
    /** Whether to enable the run info to be displayed. */
    bool m_enable_run_info;
    /** The canvas hold the basic DQM info. */
    TCanvas* m_c_info{nullptr};

    /** DAQ number of processed events */
    int m_nevent = 0;

    /** last change date/time of shm input file */
    std::string m_lastChange;

    /** The metadata for each event. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** Exp number */
    unsigned int m_expno = 0;
    /** Run number */
    unsigned int m_runno = 0;
    /** Event number */
    unsigned int m_count = 0;

    /** The file name of the analysis for stats */
    std::string m_statname;
    /** Write stats of analysis */
    void write_state(void);
    /** last time event loop entered */
    time_t m_last_event{};
    /** last time begin run entered */
    time_t m_last_beginrun{};
    /** last time input file update detected */
    time_t m_last_file_update{};
    /** last time input file content has changed */
    time_t m_last_content_update{};
    /** Last time input file changes */
    std::filesystem::file_time_type m_lasttime;
    /** enforce a content change on next event */
    bool m_forceChanged{false};
  };
} // end namespace Belle2

