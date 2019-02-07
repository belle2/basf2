/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Boqun Wang                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <daq/dqm/DqmMemFile.h>
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

    /** The name of the input root file. */
    std::string m_input_name;

    /** The TFile object for the input file. */
    TFile* m_file = nullptr;

    /** Global EventMetaData for run number and event number. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** The map between canvas name and canvas object. */
    std::map<std::string, TCanvas*> m_cs;

    /** List of folders to process. */
    std::vector<std::string> m_folders;

    /** List of histogram name patterns to process. */
    std::vector<std::string> m_histograms;

    /** Exp number. */
    unsigned int m_expno = 0;

    /** Run number. */
    unsigned int m_runno = 0;

    /** Evt number. */
    unsigned int m_count = 0;
  };
} // end namespace Belle2

