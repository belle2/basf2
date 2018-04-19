/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li, Thomas Hauth, Nils Braun                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/modules/dqm/StatisticsTimingHLTDQMModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/core/ModuleStatistics.h>
#include <framework/gearbox/Unit.h>

#include <TDirectory.h>

#include <TH1F.h>

using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(StatisticsTimingHLTDQM)

StatisticsTimingHLTDQMModule::StatisticsTimingHLTDQMModule() : HistoModule()
{
  setDescription("Monitor reconstruction runtime on HLT");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("histogramDirectoryName", m_param_histogramDirectoryName,
           "Runtime DQM histograms on HLT will be put into this directory", m_param_histogramDirectoryName);

  addParam("m_param_overviewModuleList", m_param_overviewModuleList,
           "Which modules should be shown in the overview mean list", m_param_overviewModuleList);
}

void StatisticsTimingHLTDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDirectory = nullptr;

  if (!m_param_histogramDirectoryName.empty()) {
    oldDirectory = gDirectory;
    TDirectory* histDir = oldDirectory->mkdir(m_param_histogramDirectoryName.c_str());
    histDir->cd();
  }

  m_meanTimeHistogram = new TH1F("meanTimeHistogram", "Mean Processing Time", m_param_overviewModuleList.size(), 0,
                                 m_param_overviewModuleList.size());
  m_meanTimeHistogram->SetStats(false);
  m_meanMemoryHistogram = new TH1F("meanMemoryHistogram", "Mean Memory", m_param_overviewModuleList.size(), 0,
                                   m_param_overviewModuleList.size());
  m_meanMemoryHistogram->SetStats(false);
  m_fullTimeHistogram = new TH1F("fullTimeHistogram", "Full Processing Time", 100, 0, 4000);

  for (unsigned int index = 0; index < m_param_overviewModuleList.size(); index++) {
    const std::string& moduleName = m_param_overviewModuleList[index];
    m_meanTimeHistogram->GetXaxis()->SetBinLabel(index + 1, moduleName.c_str());
    m_meanMemoryHistogram->GetXaxis()->SetBinLabel(index + 1, moduleName.c_str());
  }

  if (oldDirectory) {
    oldDirectory->cd();
  }
}


void StatisticsTimingHLTDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void StatisticsTimingHLTDQMModule::event()
{
  StoreObjPtr<ProcessStatistics> stats("", DataStore::c_Persistent);

  if (not stats.isValid()) {
    return;
  }

  const std::vector<ModuleStatistics>& moduleStatisticsList = stats->getAll();

  std::vector<double> meanTimes(m_param_overviewModuleList.size(), 0);
  std::vector<double> meanMemories(m_param_overviewModuleList.size(), 0);

  for (const ModuleStatistics& moduleStatistics : moduleStatisticsList) {
    const std::string& statisticsName = moduleStatistics.getName();
    const auto m_param_overviewModuleListIterator = std::find(m_param_overviewModuleList.begin(), m_param_overviewModuleList.end(),
                                                              statisticsName);
    if (m_param_overviewModuleListIterator == m_param_overviewModuleList.end()) {
      continue;
    }

    const double statisticsTime = moduleStatistics.getTimeMean(ModuleStatistics::EStatisticCounters::c_Event) / Unit::ms;
    const double statisticsMemory = moduleStatistics.getMemoryMean(ModuleStatistics::EStatisticCounters::c_Event);

    const int m_param_overviewModuleListIndex = std::distance(m_param_overviewModuleList.begin(), m_param_overviewModuleListIterator);
    meanTimes[m_param_overviewModuleListIndex] += statisticsTime;
    meanMemories[m_param_overviewModuleListIndex] += statisticsMemory;
  }

  for (unsigned int index = 0; index < m_param_overviewModuleList.size(); index++) {
    m_meanTimeHistogram->SetBinContent(index + 1, meanTimes[index]);
    m_meanMemoryHistogram->SetBinContent(index + 1, meanMemories[index]);
  }

  const ModuleStatistics& fullStatistics = stats->getGlobal();
  const double fullTimeSum = fullStatistics.getTimeSum(ModuleStatistics::EStatisticCounters::c_Event) / Unit::ms;
  m_fullTimeHistogram->Fill(fullTimeSum - m_lastFullTimeSum);
  m_lastFullTimeSum = fullTimeSum;
}
