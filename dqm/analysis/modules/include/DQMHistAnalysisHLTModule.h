/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TCanvas.h>

#include <map>
#include <string>

#ifdef _BELLE2_EPICS
#include "cadef.h"
#endif

namespace Belle2 {
  /**
   * Class for HLT-related histogram analysis.
   */
  class DQMHistAnalysisHLTModule final : public DQMHistAnalysisModule {
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisHLTModule();

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
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

  private:
    /// prefix for EPICS PVs
    std::string m_pvPrefix = "B2_nsm:get:ECL_LUM_MON:lum_det_run";
    /// name of the bhabha trigger
    std::string m_bhabhaName = "accept_bhabha";
    /// Which columns to use
    std::map<std::string, std::string> m_columnMapping;
    /// Which l1 triggers to show
    std::vector<std::string> m_l1Histograms;
    /// Which HLT filter lines to use for calculation retention rate per unit
    std::vector<std::string> m_retentionPerUnit;
    /// Histogram with final efficiencies to HLT
    std::pair<TCanvas*, TH1F*> m_hEfficiency = {nullptr, nullptr};
    /// Histogram with final efficiencies to all events
    std::pair<TCanvas*, TH1F*> m_hEfficiencyTotal = {nullptr, nullptr};
    /// Histogram with final cross sections
    std::pair<TCanvas*, TH1F*> m_hCrossSection = {nullptr, nullptr};
    /// Histogram with final ratios to bhabha
    std::pair<TCanvas*, TH1F*> m_hRatios = {nullptr, nullptr};
    /// Histogram with hlt&l1 ratios to l1 numbers
    std::map<std::string, std::pair<TCanvas*, TH1F*>> m_hl1Ratios;
    /// Histogram with retention rate per unit of some hlt filter lines
    std::map<std::string, std::pair<TCanvas*, TH1F*>> m_hRetentionPerUnit;
    /// Histogram with mean processing time per process
    std::pair<TCanvas*, TH1F*> m_hMeanTime = {nullptr, nullptr};
    /// Histogram with fraction of events with error flags
    std::pair<TCanvas*, TH1D*> m_hErrorFlagFraction = {nullptr, nullptr};
    /// Histogram with fraction of events filtered per unit
    std::pair<TCanvas*, TH1D*> m_hFilteredFractionPerUnit = {nullptr, nullptr};
    /// Histogram with mean budget time per unit per process
    std::pair<TCanvas*, TH1F*> m_hMeanBudgetTimePerUnit = {nullptr, nullptr};
    /// Histogram with mean processing time per unit per process
    std::pair<TCanvas*, TH1F*> m_hMeanProcessingTimePerUnit = {nullptr, nullptr};
    /// Histogram with mean memory change per process
    std::pair<TCanvas*, TH1F*> m_hMeanMemory = {nullptr, nullptr};

#ifdef _BELLE2_EPICS
    //! one EPICS PV
    chid  m_epicschid;
#endif
  };
}
