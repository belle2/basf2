/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisExample.h
// Description : An example module for DQM histogram analysis
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TF1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisExampleModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisExampleModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisExampleModule();

    /**
     * Initializer.
     * This method is called at the beginning of data processing.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

    //! Parameters accesible from basf2 scripts
  private:
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! name of histogram
    std::string m_histogramName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;

    //! Data members
  private:
    /** The fitting function. */
    TF1* m_function = nullptr;
    /** The drawing canvas for the fitting result. */
    TCanvas* m_canvas = nullptr;

  };
} // end namespace Belle2

