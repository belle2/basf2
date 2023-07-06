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

  addParam("useEpics", m_useEpics, "Whether to update EPICS PVs.", false);
}


DQMHistAnalysisECLClustersModule::~DQMHistAnalysisECLClustersModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisECLClustersModule::initialize()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    // Register EPICS PVs

    SEVCHK(ca_create_channel("ECL:avg_crystals_in_cluster", NULL, NULL, 10, &avgCrystalsInCluster),
           "ca_create_channel failure");
    SEVCHK(ca_create_channel("ECL:bad_clusters_num", NULL, NULL, 10, &badClustersNum),
           "ca_create_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif

  m_monObj = getMonitoringObject("ecl");

  m_c_main = new TCanvas("ecl_main");
  m_monObj->addCanvas(m_c_main);

  B2DEBUG(20, "DQMHistAnalysisECLClusters: initialized.");
}

void DQMHistAnalysisECLClustersModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLClusters: beginRun called.");
}

void DQMHistAnalysisECLClustersModule::event()
{
  //== Get DQM info
  TProfile* avgCrystalsHist = (TProfile*)findHist("ECL/avg_crystals_in_cluster");
  if (avgCrystalsHist) {
    m_avgClusterCrystalsNum = avgCrystalsHist->GetBinContent(1);
  }
  TProfile* badClustersHist = (TProfile*)findHist("ECL/bad_clusters");
  if (badClustersHist) {
    m_badClustersNumber = badClustersHist->GetBinContent(1);
  }

  //== Set EPICS PVs

#ifdef _BELLE2_EPICS
  if (m_useEpics) {

    SEVCHK(ca_put(DBR_DOUBLE, avgCrystalsInCluster, (void*)&m_avgClusterCrystalsNum), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, badClustersNum, (void*)&m_badClustersNumber), "ca_set failure");

    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
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

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    SEVCHK(ca_clear_channel(avgCrystalsInCluster), "ca_clear_channel failure");
    SEVCHK(ca_clear_channel(badClustersNum), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

