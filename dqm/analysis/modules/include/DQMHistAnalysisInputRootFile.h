/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TFile.h>
#include <TCanvas.h>

#include <string>
#include <map>
#include <vector>

namespace Belle2 {

  /**
   * Class to read histograms from a root file for offline testing of analysis modules.
   */

  class DQMHistAnalysisInputRootFileModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisInputRootFileModule();

    /**
     * Initialize the module.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * Event processor.
     */
    void event() override final;

    /**
     * End-of-run action.
     */
    void endRun() override final;

    /**
     * Termination action.
     */
    void terminate() override final;

  private:
    /**
     * Pattern match for histogram name
     * @param pattern Pattern used for matchng the histogram name. Wildcards (* and ?) are supported
     * @param text    Histogram name to be matched
     * @return true, if the pattern matches the text
     */
    bool hname_pattern_match(std::string pattern, std::string text);

    /** The list of names of the input root file. */
    std::vector<std::string> m_file_list;

    /** The TFile object for the input file. */
    TFile* m_file = nullptr;

    /** Global EventMetaData for run number and event number. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** The map between canvas name and canvas object. */
    std::map<std::string, TCanvas*> m_cs;

    /** List of histogram name patterns to process. */
    std::vector<std::string> m_histograms;

    /** Exp number. */
    unsigned int m_expno = 0;

    /** Evt number. */
    unsigned int m_count = 0;

    /** List of total number of events for each run. */
    std::vector<unsigned int> m_events_list;

    /** List of runs. */
    std::vector<unsigned int> m_run_list;

    /** Time between two events in second. */
    unsigned int m_interval = 0;

    /** Index in the list of runs, events and files */
    unsigned int m_run_idx = 0;

    /** Test mode for null histograms */
    bool m_null_histo_mode = false;
    /** Whether to automatically generate canvases for histograms */
    bool m_autocanvas = true;
  };
} // end namespace Belle2

