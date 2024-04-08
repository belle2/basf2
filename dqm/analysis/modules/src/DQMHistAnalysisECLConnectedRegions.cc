/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//THIS MODULE
#include <dqm/analysis/modules/DQMHistAnalysisECLConnectedRegions.h>

//ROOT
#include <TProfile.h>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLConnectedRegions);

DQMHistAnalysisECLConnectedRegionsModule::DQMHistAnalysisECLConnectedRegionsModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECLConnectedRegions: Constructor done.");

  addParam("pvPrefix", m_pvPrefix, "Prefix to use for PVs registered by this module",
           std::string("ECL:"));
}

void DQMHistAnalysisECLConnectedRegionsModule::initialize()
{
  for (auto& pv_name : {"largest_cr_avg_crystals_num", "largest_cr_localmax_num"}) {
    registerEpicsPV(m_pvPrefix + pv_name, pv_name);
  }

  m_monObj = getMonitoringObject("ecl");

  B2DEBUG(20, "DQMHistAnalysisECLConnectedRegions: initialized.");
}

void DQMHistAnalysisECLConnectedRegionsModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLConnectedRegions: beginRun called.");
}

void DQMHistAnalysisECLConnectedRegionsModule::event()
{
  //== Get DQM info
  auto crystalsDistributionHist = (TH1F*)findHist("ECL/Crystals_distribution_in_largest_cr");
  if (crystalsDistributionHist) {
    m_largestCRAvgCrystalsNum = crystalsDistributionHist->GetMean();
  }
  auto localMaxDistributionHist = (TH1F*)findHist("ECL/LocalMaximum_distribution_in_largest_cr");
  if (localMaxDistributionHist) {
    m_largestCRLocalMaxNum = localMaxDistributionHist->GetMean();
  }

  //== Set EPICS PVs
  setEpicsPV("largest_cr_avg_crystals_num", m_largestCRAvgCrystalsNum);
  setEpicsPV("largest_cr_localmax_num", m_largestCRLocalMaxNum);
}

void DQMHistAnalysisECLConnectedRegionsModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLConnectedRegions: endRun called");
  m_monObj->setVariable("largest_cr_avg_crystals_num", m_largestCRAvgCrystalsNum);
  m_monObj->setVariable("largest_cr_localmax_num", m_largestCRLocalMaxNum);
}


void DQMHistAnalysisECLConnectedRegionsModule::terminate()
{
  B2DEBUG(20, "terminate called");
}

