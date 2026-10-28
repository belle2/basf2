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
   * DQM framework core module to read histograms from a root file.
   * The stored histograms are then available for the DQM online analysis
   * modules by the base class interfaces.
   * This module must be the first in the analysis chain as it provides
   * the (faked) EventMetaData for the basf2 processing ("InputModule").
   */

  class DQMHistAnalysisInput2Module : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisInput2Module();

  private:
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

    /**
     * Read histogram from key and add to list vector
     */
    static void addToHistList(std::vector<TH1*>& inputHistList, const std::string& dirname, TKey* key);

    /**
     * Write state of analysis to a file
     */
    void write_state(void);

    // Data members
    /** The input root-file name for the histograms. */
    std::string m_inputFileName;
    /** The refresh interval. */
    int m_interval;
    /** Whether to the run info canvas should be created. */
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
    int m_expno = 0;
    /** Run number */
    int m_runno = 0;
    /** Event number */
    unsigned int m_count = 0;

    /** The file name of the analysis for stats */
    std::string m_statname;
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

    /** last run */
    int m_lastRun{-1};
    /** last exp */
    int m_lastExp{-1};
  };
} // end namespace Belle2

