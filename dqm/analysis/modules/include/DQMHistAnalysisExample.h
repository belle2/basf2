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

#include <TCanvas.h>
#include <TF1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisExampleModule : public DQMHistAnalysisModule {

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
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

    //! Parameters accesible from basf2 scripts
  protected:
    /** The name of the histogram. */
    std::string m_histoname;
    /** The definition of the fitting function. */
    std::string m_function;

    //! Data members
  private:
    /** The fitting function. */
    TF1* m_f = nullptr;
    /** The drawing canvas for the fitting result. */
    TCanvas* m_c = nullptr;

  };
} // end namespace Belle2

