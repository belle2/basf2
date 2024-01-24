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

#include <TFile.h>
#include <TCanvas.h>

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

  private:
    /**
     * Pattern match for histogram name
     * @param pattern Used for matchng the histogram name. Wildcards (* and ?) are supported
     * @param text    Histogram name to be matched
     * @return true, if the pattern matches the text
     */
    bool hnamePatternMatch(std::string pattern, std::string text);

    /** The list of names of the input root file. */
    std::vector<std::string> m_fileList;

    /** The TFile object for the input file. */
    TFile* m_file = nullptr;

    /** Global EventMetaData for run number and event number. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** List of histogram name patterns to process. */
    std::vector<std::string> m_histograms;

    /** Exp number. */
    unsigned int m_expno = 0;

    /** Evt number. */
    unsigned int m_count = 0;

    /** Run Type Override*/
    std::string m_runType;

    /** Filled event number. */
    int m_fillEvent = 0;

    /** List of total number of events for each run.
     * This is the number of issued update events for testing,
     * not the number of events from which histograms were filled
     */
    std::vector<unsigned int> m_eventsList;

    /** List of runs. */
    std::vector<unsigned int> m_runList;

    /** Time between two events in second. */
    unsigned int m_interval = 0;

    /** Index in the list of runs, events and files */
    unsigned int m_run_idx = 0;

    /** Test mode for null histograms */
    bool m_nullHistoMode = false;
  };
} // end namespace Belle2

