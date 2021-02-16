/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

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
  class DQMHistAnalysisHLTModule : public DQMHistAnalysisModule {
  public:
    DQMHistAnalysisHLTModule();
    void initialize() final;
    void beginRun()  final;
    void event() final;
    void terminate() final;

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
    // Maximum number of HLT units used during the experiment
    const unsigned int m_max_hlt_units = 10;


#ifdef _BELLE2_EPICS
    //! one EPICS PV
    chid  m_epicschid;
#endif
  };
}
