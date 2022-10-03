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

#include <string>
#include <TH1.h>
#include <TF1.h>

namespace Belle2 {

  /**
   * Class to dynamically fill histograms from a config file for offline testing of analysis modules.
   */

  class DQMHistAnalysisInputTestModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisInputTestModule();

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

    /**
     * Plot/print function for debugging
     */
    void PlotDelta(void);

  private:

    /** Global EventMetaData for run number and event number. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** List of config files */
    std::vector<std::string> m_configs{};

    /** Exp number. */
    unsigned int m_expno;

    /** Run number. */
    unsigned int m_runno;

    /** total number of events for run. */
    unsigned int m_events;

    /** Evt number. */
    unsigned int m_count{};

    /** Fill skip modulo. */
    unsigned int m_nonfillmod{0};

    /** list of test histogram */
    std::vector <TH1*> m_testHisto{};

    /** according list of function for filling histo */
    std::vector <TF1*> m_func{};

    /** according parameter list for function definition */
    std::vector <std::vector <double>> m_fpar{};

    /** according list of fills per "event" */
    std::vector <unsigned int> m_fill{};

    /** according list of underflow fills per "event" */
    std::vector <unsigned int> m_underflow{};

    /** keeping a list of histo Names  */
    std::vector <std::string> m_myNames{};

  };
} // end namespace Belle2

