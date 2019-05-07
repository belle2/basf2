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

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>
#include <TCanvas.h>

#include <map>
#include <string>

#ifdef _BELLE2_EPICS
#include "cadef.h"
#endif

namespace Belle2 {
  class DQMHistAnalysisHLTModule : public DQMHistAnalysisModule {
  public:
    DQMHistAnalysisHLTModule();
    void initialize() final;
    void beginRun()  final;
    void event() final;

  private:
    /// prefix for EPICS PVs
    std::string m_pvPrefix = "B2_nsm:get:ECL_LUM_MON:lum_det_run";
    /// Which columns to use
    std::map<std::string, std::string> m_columnMapping;
    /// Histogram with final efficiencies
    std::pair<TCanvas*, TH1F*> m_hEfficiency = {nullptr, nullptr};
    /// Histogram with final cross sections
    std::pair<TCanvas*, TH1F*> m_hCrossSection = {nullptr, nullptr};
    /// Histogram with final ratios to bhabha
    std::pair<TCanvas*, TH1F*> m_hRatios = {nullptr, nullptr};

#ifdef _BELLE2_EPICS
    //! one EPICS PV
    chid  m_epicschid;
#endif
  };
}