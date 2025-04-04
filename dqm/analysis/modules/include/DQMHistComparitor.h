/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistComparitor.h
// Description : Compare a list of histograms with its reference
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

namespace Belle2 {
  /** Class definition for the reference histogram display. */

  class DQMHistComparitorModule final : public DQMHistAnalysisModule {

    /**
     * The struct for reference histogram comparison.
     */
    typedef struct {
      /** The name of the fit result PV, empty if none */
      std::string pvfit;
      /** The name of the status PV, empty if none */
      std::string pvstatus;
      /** Whether to use delta. */
      bool deltaflag;
      /** Whether to Colorize. */
      bool colorflag;
      /** The name of the histogram to be compared. */
      std::string histName;
      /** The name of the reference histogram. */
      std::string refName;
      /** The canvas to display both original and reference histograms. */
      TCanvas* canvas;
      /** The algo to use: 0=Chi2Test (default), 1=KolmogorovTest (2.. not used yet) */
      int algo;
      /** The warning level for the histogram difference. */
      float warning;
      /** The error level for the histogram difference. */
      float error;
      /** The minimum entries for histogram comparison. */
      int min_entries;
    } CMPNODE;

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistComparitorModule();

  private:
    /**
     * Destructor.
     */
    ~DQMHistComparitorModule();

    /**
     * Initializer.
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
    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;
    /** Struct for extracted parameters + EPICS PV */
    std::vector<CMPNODE*> m_pnode;

  };
} // end namespace Belle2

