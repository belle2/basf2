/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//THIS MODULE
#include <dqm/analysis/modules/DQMHistAnalysisECLClusters.h>

//ROOT
#include <TProfile.h>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLClusters);

DQMHistAnalysisECLClustersModule::DQMHistAnalysisECLClustersModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECLClusters: Constructor done.");

  addParam("pvPrefix", m_pvPrefix, "Prefix to use for PVs registered by this module",
           std::string("DQM:ECL:"));
}

void DQMHistAnalysisECLClustersModule::initialize()
{
  for (auto& pv_name : {"avg_crystals_in_cluster", "bad_clusters_num"}) {
    registerEpicsPV(m_pvPrefix + pv_name, pv_name);
  }
  updateEpicsPVs(5.0);

  m_monObj = getMonitoringObject("ecl");

  B2DEBUG(20, "DQMHistAnalysisECLClusters: initialized.");
}

void DQMHistAnalysisECLClustersModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLClusters: beginRun called.");
}

void DQMHistAnalysisECLClustersModule::event()
{
  //== Get DQM info
  auto avgCrystalsHist = (TProfile*)findHist("ECL/avg_crystals_in_cluster");
  if (avgCrystalsHist) {
    m_avgClusterCrystalsNum = avgCrystalsHist->GetBinContent(1);
  }
  auto badClustersHist = (TProfile*)findHist("ECL/bad_clusters");
  if (badClustersHist) {
    m_badClustersNumber = badClustersHist->GetBinContent(1);
  }

  //== Set EPICS PVs
  setEpicsPV("avg_crystals_in_cluster", m_avgClusterCrystalsNum);
  setEpicsPV("bad_clusters", m_badClustersNumber);
  updateEpicsPVs(5.0);
}

void DQMHistAnalysisECLClustersModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLClusters: endRun called");
  m_monObj->setVariable("avg_crystals_in_cluster", m_avgClusterCrystalsNum);
  m_monObj->setVariable("bad_clusters", m_badClustersNumber);
}


void DQMHistAnalysisECLClustersModule::terminate()
{
  B2DEBUG(20, "terminate called");
}

