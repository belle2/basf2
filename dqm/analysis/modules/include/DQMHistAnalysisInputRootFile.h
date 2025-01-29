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

#include <dqm/core/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TFile.h>

#include <string>
#include <map>
#include <vector>

namespace Belle2 {
  /**
   * Class to read histograms from a root file for offline testing of analysis modules.
   */

  class DQMHistAnalysisInputRootFileModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisInputRootFileModule();

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
     * Event processor.
     */
    void event() override final;

    /**
     * Read histogram from key and add to list vector
     */
    void addToHistList(std::vector<TH1*>& hs, std::string dirname, TKey* key);

    /**
     * Pattern match for histogram name
     * @param pattern Used for matchng the histogram name. Wildcards (* and ?) are supported
     * @param text    Histogram name to be matched
     * @return true, if the pattern matches the text
     */
    bool hnamePatternMatch(std::string pattern, std::string text);

    // Data members
    /** The list of names of the input root file. */
    std::vector<std::string> m_fileList;

    /** The TFile object for the input file. */
    TFile* m_file = nullptr;

    /** Whether to enable the run info to be displayed. */
    bool m_enable_run_info;
    /** The canvas hold the basic DQM info. */
    TCanvas* m_c_info{nullptr};

    /** Global EventMetaData for run number and event number. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** List of histogram name patterns to process. */
    std::vector<std::string> m_histograms;

    /** Exp number */
    int m_expno = 0;
    /** Event number */
    int m_count = 0;

    /** Run Type Override*/
    std::string m_runType;

    /** Filled event number. */
    int m_fillNEvent = 0;

    /** List of total number of events for each run.
     * This is the number of issued update events for testing,
     * not the number of events from which histograms were filled
     */
    std::vector<int> m_eventsList;

    /** List of runs. */
    std::vector<int> m_runList;

    /** Time between two events in second. */
    unsigned int m_interval = 0;

    /** Index in the list of runs, events and files */
    unsigned int m_run_idx = 0;

    /** Test mode for null histograms */
    bool m_nullHistoMode = false;

    /** Whether to add the run control histograms. */
    bool m_add_runcontrol_hist;
    /** emulated histogram from runcontrol, expno */
    TH1F* m_h_expno{nullptr};
    /** emulated histogram from runcontrol, runno */
    TH1F* m_h_runno{nullptr};
    /** emulated histogram from runcontrol, runtype */
    TH1F* m_h_rtype{nullptr};
    /** emulated histogram from daq, nevent */
    TH1F* m_h_fillNEvent{nullptr};

    /** last run */
    int m_lastRun{-1};
    /** last exp */
    int m_lastExp{-1};
  };
} // end namespace Belle2

