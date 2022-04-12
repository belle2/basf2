/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPlotOnly.h
// Description : Module for DQM Histogram analysis
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <string>
#include <TCanvas.h>

namespace Belle2 {

  /**
   * The module to plot a list of histograms into canvases.
   */
  class DQMHistAnalysisPlotOnlyModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPlotOnlyModule();

    /**
     * Destructor.
     */
    virtual ~DQMHistAnalysisPlotOnlyModule();

    /**
     * Initializer.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * This method is called for each event.
     */
    virtual void event() override;

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate() override;

    // Data members
  private:

    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;

    /**
     * Get histogram by its name.
     * @param a The name of the histogram.
     * @return The found histogram, nullptr if not found.
     */
    TH1* GetHisto(TString a);

    /** Parameter list for histograms */
    std::map< std::string, TCanvas*> m_canvasList;

  };
} // end namespace Belle2

