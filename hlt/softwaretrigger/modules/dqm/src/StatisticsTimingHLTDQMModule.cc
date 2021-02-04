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
#include <hlt/softwaretrigger/modules/dqm/SoftwareTriggerHLTDQMModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/core/ModuleStatistics.h>
#include <framework/gearbox/Unit.h>

#include <TDirectory.h>

#include <TH1F.h>

#include <framework/utilities/Utils.h>

#include <fstream>

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

  addParam("createHLTUnitHistograms", m_param_create_hlt_unit_histograms,
           "Create HLT unit histograms?",
           false);
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

  m_meanTimeHistogram = new TH1F("meanTimeHistogram", "Mean Processing Time [ms]", m_param_overviewModuleList.size(), 0,
                                 m_param_overviewModuleList.size());
  m_meanTimeHistogram->SetStats(false);
  m_meanMemoryHistogram = new TH1F("meanMemoryHistogram", "Mean Memory", m_param_overviewModuleList.size(), 0,
                                   m_param_overviewModuleList.size());
  m_meanMemoryHistogram->SetStats(false);
  m_fullTimeHistogram = new TH1F("fullTimeHistogram", "Budget Time [ms]", 250, 0, 10000);
  m_processingTimeHistogram = new TH1F("processingTimeHistogram", "Processing Time [ms]", 125, 0, 5000);

  for (unsigned int index = 0; index < m_param_overviewModuleList.size(); index++) {
    const std::string& moduleName = m_param_overviewModuleList[index];
    m_meanTimeHistogram->GetXaxis()->SetBinLabel(index + 1, moduleName.c_str());
    m_meanMemoryHistogram->GetXaxis()->SetBinLabel(index + 1, moduleName.c_str());
  }

  if (m_param_create_hlt_unit_histograms) {
    m_fullTimeMeanPerUnitHistogram = new TH1F("fullTimeMeanPerUnitHistogram", "Mean Budget Time Per Unit [ms]",
                                              HLTUnit::m_max_hlt_units + 1, 0,
                                              HLTUnit::m_max_hlt_units + 1);
    m_fullTimeMeanPerUnitHistogram->SetStats(false);
    m_fullTimeMeanPerUnitHistogram->SetXTitle("HLT unit number");
    m_processingTimeMeanPerUnitHistogram = new TH1F("processingTimeMeanPerUnitHistogram", "Mean Processing Time Per Unit [ms]",
                                                    HLTUnit::m_max_hlt_units + 1, 0,
                                                    HLTUnit::m_max_hlt_units + 1);
    m_processingTimeMeanPerUnitHistogram->SetStats(false);
    m_processingTimeMeanPerUnitHistogram->SetXTitle("HLT unit number");

    for (unsigned int index = 1; index <= HLTUnit::m_max_hlt_units; index++) {
      m_fullTimePerUnitHistograms.emplace(index, new TH1F(("fullTimePerUnitHistogram_HLT" + std::to_string(index)).c_str(),
                                                          ("Budget Time Per Unit [ms]: HLT" + std::to_string(index)).c_str(), 250, 0, 10000));
      m_lastFullTimeSumPerUnit.emplace(index, 0);
      m_processingTimePerUnitHistograms.emplace(index, new TH1F(("processingTimePerUnitHistogram_HLT" + std::to_string(index)).c_str(),
                                                                ("Processing Time Per Unit [ms]: HLT" + std::to_string(index)).c_str(), 125, 0, 5000));
      m_lastProcessingTimeSumPerUnit.emplace(index, 0);
    }
  }

  if (oldDirectory) {
    oldDirectory->cd();
  }
}


void StatisticsTimingHLTDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  if (m_param_create_hlt_unit_histograms) {
    std::ifstream file;
    file.open(HLTUnit::m_hlt_unit_file);
    if (file.good()) {
      std::string host;
      getline(file, host);
      m_hlt_unit = atoi(host.substr(3, 2).c_str());
      file.close();
    } else {
      B2WARNING("HLT unit number not found");
    }
  }
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

  double processingTimeSum = 0.0;
  double processingTimeMean = 0.0;
  for (const ModuleStatistics& moduleStatistics : moduleStatisticsList) {
    const std::string& statisticsName = moduleStatistics.getName();
    const auto m_summaryModuleListIterator = std::find(m_summaryModuleList.begin(), m_summaryModuleList.end(),
                                                       statisticsName);
    if (m_summaryModuleListIterator == m_summaryModuleList.end()) {
      continue;
    }
    processingTimeSum += moduleStatistics.getTimeSum(ModuleStatistics::EStatisticCounters::c_Event) / Unit::ms;
    processingTimeMean += moduleStatistics.getTimeMean(ModuleStatistics::EStatisticCounters::c_Event) / Unit::ms;
  }
  m_processingTimeHistogram->Fill(processingTimeSum - m_lastProcessingTimeSum);
  m_lastProcessingTimeSum = processingTimeSum;


  const ModuleStatistics& fullStatistics = stats->getGlobal();
  const double fullTimeSum = fullStatistics.getTimeSum(ModuleStatistics::EStatisticCounters::c_Event) / Unit::ms;
  m_fullTimeHistogram->Fill(fullTimeSum - m_lastFullTimeSum);
  m_lastFullTimeSum = fullTimeSum;

  if (m_param_create_hlt_unit_histograms) {
    if (0 < m_hlt_unit) {
      m_processingTimeMeanPerUnitHistogram->SetBinContent(m_hlt_unit + 1, processingTimeMean);

      m_processingTimePerUnitHistograms[m_hlt_unit]->Fill(processingTimeSum - m_lastProcessingTimeSumPerUnit[m_hlt_unit]);
      m_lastProcessingTimeSumPerUnit[m_hlt_unit] = processingTimeSum;

      const double fullTimeMean = fullStatistics.getTimeMean(ModuleStatistics::EStatisticCounters::c_Event) / Unit::ms;
      m_fullTimeMeanPerUnitHistogram->SetBinContent(m_hlt_unit + 1, fullTimeMean);

      m_fullTimePerUnitHistograms[m_hlt_unit]->Fill(fullTimeSum - m_lastFullTimeSumPerUnit[m_hlt_unit]);
      m_lastFullTimeSumPerUnit[m_hlt_unit] = fullTimeSum;
    }
  }
}

void StatisticsTimingHLTDQMModule::beginRun()
{
  if (!m_meanTimeHistogram || !m_meanMemoryHistogram || !m_fullTimeHistogram || !m_processingTimeHistogram) {
    B2FATAL("Histograms were not created. Did you setup a HistoManager?");
  }

  m_meanTimeHistogram->Reset();
  m_meanMemoryHistogram->Reset();
  m_fullTimeHistogram->Reset();
  m_processingTimeHistogram->Reset();
  if (m_param_create_hlt_unit_histograms) {
    m_fullTimeMeanPerUnitHistogram->Reset();
    m_processingTimeMeanPerUnitHistogram->Reset();
    std::for_each(m_fullTimePerUnitHistograms.begin(), m_fullTimePerUnitHistograms.end(),
    [](auto & it) { it.second->Reset(); });
    std::for_each(m_processingTimePerUnitHistograms.begin(), m_processingTimePerUnitHistograms.end(),
    [](auto & it) { it.second->Reset(); });
  }
}

