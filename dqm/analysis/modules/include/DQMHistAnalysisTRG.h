/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisDeltaEpicsMonObjExample.h
// Description : DQM Analysis Delta+Epics+MonObj example
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TLine.h>

namespace Belle2 {
  /*! DQM Delta Histogram Test code and example */

  class DQMHistAnalysisTRGModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisTRGModule();

  private:

    /**
     * Destructor.
     */
    ~DQMHistAnalysisTRGModule();

    /**
     * Initializer.
     */
    void initialize(void) override final;

    /**
     * Called when entering a new run.
     */
    void beginRun(void) override final;

    /**
     * Called when run ends.
     */
    void endRun(void) override final;

    /**
     * This method is called for each event.
     */
    void event(void) override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate(void) override final;

    /**
     * Do the actual processing
     */
    void doHistAnalysis();

  private:

    // Data members
    //! prefix for EPICS PVs
    std::string m_pvPrefix;

    //! Final Canvas
    TCanvas* m_canvas = nullptr;
    /**
     * @Name of canvas of ECLTRG_timing_mean
     */
    TCanvas* m_canvas_ECLTRG_timing_mean = nullptr;
    /**
     * @Name of canvas of CDCTRG_timing_mean
     */
    TCanvas* m_canvas_CDCTRG_timing_mean = nullptr;
    /**
     * @Name of canvas of TOPTRG_timing_mean
     */
    TCanvas* m_canvas_TOPTRG_timing_mean = nullptr;
    /**
     * @Name of canvas of ECLTRG_peak
     */
    TCanvas* m_canvas_ECLTRG_peak = nullptr;
    /**
     * @Name of canvas of CDCTRG_2D_peak
     */
    TCanvas* m_canvas_CDCTRG_2D_peak = nullptr;
    /**
     * @Name of canvas of NN_peak
     */
    TCanvas* m_canvas_NN_peak = nullptr;
    /**
     * @Name of canvas of CDCTRG_TSF_peak
     */
    TCanvas* m_canvas_CDCTRG_TSF_peak = nullptr;
    /**
     * @Name of canvas of KLMTRG_peak
     */
    TCanvas* m_canvas_KLMTRG_peak = nullptr;
    /**
     * @Name of canvas of TOPTRG_peak
     */
    TCanvas* m_canvas_TOPTRG_peak = nullptr;
    /**
     * @Name of canvas of hadronb2_over_bhabha_all
     */
    TCanvas* m_canvas_hadronb2_over_bhabha_all = nullptr;
    /**
     * @Name of canvas of mumu2trk_over_bhabha_all
     */
    TCanvas* m_canvas_mumu2trk_over_bhabha_all = nullptr;
    /**
     * @Name of canvas of hadronb2_over_mumu2trk
    */
    TCanvas* m_canvas_hadronb2_over_mumu2trk = nullptr;

  };
} // end namespace Belle2

