/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/softwaretrigger/modules/dqm/StatisticsTimingHLTDQMModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/core/ModuleStatistics.h>
#include <framework/gearbox/Unit.h>
#include <hlt/utilities/Units.h>

#include <TDirectory.h>

#include <TH1F.h>
#include <TH2F.h>

#include <fstream>

using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(StatisticsTimingHLTDQM);

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
  m_meanMemoryHistogram = new TH1F("meanMemoryHistogram", "Mean Memory Change [MB]", m_param_overviewModuleList.size(), 0,
                                   m_param_overviewModuleList.size());
  m_meanMemoryHistogram->SetStats(false);
  m_fullTimeHistogram = new TH1F("fullTimeHistogram", "Budget Time [ms]", m_fullTimeNBins, 0, m_fullTimeMax);
  m_fullTimeHistogram->StatOverflows(true);
  m_processingTimeHistogram = new TH1F("processingTimeHistogram", "Processing Time [ms]", m_processingTimeNBins, 0,
                                       m_processingTimeMax);
  m_processingTimeHistogram->StatOverflows(true);
  m_fullMemoryHistogram = new TH1F("fullMemoryHistogram", "Total memory used [MB]", m_fullMemoryNBins, 0,
                                   m_fullMemoryMax);
  m_fullMemoryHistogram->StatOverflows(true);

  for (unsigned int index = 0; index < m_param_overviewModuleList.size(); index++) {
    const std::string& moduleName = m_param_overviewModuleList[index];
    m_meanTimeHistogram->GetXaxis()->SetBinLabel(index + 1, moduleName.c_str());
    m_meanMemoryHistogram->GetXaxis()->SetBinLabel(index + 1, moduleName.c_str());
    m_moduleTimeHistograms.emplace(moduleName, new TH1F((moduleName + "_time").c_str(),
                                                        ("Time spent in: " + moduleName + " [ms]").c_str(), m_processingTimeNBins, 0, m_processingTimeMax));
    m_moduleTimeHistograms[moduleName]->StatOverflows(true);
    m_lastModuleTimeSum.emplace(moduleName, 0);
    m_moduleMemoryHistograms.emplace(moduleName, new TH1F((moduleName + "_memory").c_str(),
                                                          ("Memory used in: " + moduleName + " [MB]").c_str(), m_fullMemoryNBins, 0, m_fullMemoryMax));
    m_moduleMemoryHistograms[moduleName]->StatOverflows(true);
  }

  if (m_param_create_hlt_unit_histograms) {
    m_fullTimeMeanPerUnitHistogram = new TH1F("fullTimeMeanPerUnitHistogram", "Mean Budget Time Per Unit [ms]",
                                              HLTUnits::max_hlt_units + 1, 0,
                                              HLTUnits::max_hlt_units + 1);
    m_fullTimeMeanPerUnitHistogram->SetStats(false);
    m_fullTimeMeanPerUnitHistogram->SetXTitle("HLT unit number");
    m_processingTimeMeanPerUnitHistogram = new TH1F("processingTimeMeanPerUnitHistogram", "Mean Processing Time Per Unit [ms]",
                                                    HLTUnits::max_hlt_units + 1, 0,
                                                    HLTUnits::max_hlt_units + 1);
    m_processingTimeMeanPerUnitHistogram->SetStats(false);
    m_processingTimeMeanPerUnitHistogram->SetXTitle("HLT unit number");

    for (unsigned int index = 1; index <= HLTUnits::max_hlt_units; index++) {
      m_fullTimePerUnitHistograms.emplace(index, new TH1F(("fullTimePerUnitHistogram_HLT" + std::to_string(index)).c_str(),
                                                          ("Budget Time Per Unit: HLT" + std::to_string(index) + " [ms]").c_str(), m_fullTimeNBins, 0, m_fullTimeMax));
      m_fullTimePerUnitHistograms[index]->StatOverflows(true);
      m_lastFullTimeSumPerUnit.emplace(index, 0);
      m_processingTimePerUnitHistograms.emplace(index, new TH1F(("processingTimePerUnitHistogram_HLT" + std::to_string(index)).c_str(),
                                                                ("Processing Time Per Unit: HLT" + std::to_string(index) + " [ms]").c_str(), m_processingTimeNBins, 0, m_processingTimeMax));
      m_processingTimePerUnitHistograms[index]->StatOverflows(true);
      m_lastProcessingTimeSumPerUnit.emplace(index, 0);
      m_fullMemoryPerUnitHistograms.emplace(index, new TH1F(("fullMemoryPerUnitHistogram_HLT" + std::to_string(index)).c_str(),
                                                            ("Total Memory Used Per Unit: HLT" + std::to_string(index) + " [MB]").c_str(), m_fullMemoryNBins, 0, m_fullMemoryMax));
      m_fullMemoryPerUnitHistograms[index]->StatOverflows(true);
    }

    m_processesPerUnitHistogram = new TH1F("processesPerUnitHistogram", "Number of Processes Per Unit",
                                           HLTUnits::max_hlt_units + 1, 0,
                                           HLTUnits::max_hlt_units + 1);
    m_processesPerUnitHistogram->SetXTitle("HLT unit number");
  }
  m_processingTimePassiveVeto = new TH1F("processingTimePassiveVeto", "Processing Time of events passing passive veto [ms]",
                                         m_processingTimeNBins, 0,
                                         m_processingTimeMax);
  m_processingTimePassiveVeto->StatOverflows(true);
  m_processingTimeNotPassiveVeto = new TH1F("processingTimeNotPassiveVeto", "Processing Time of events not passing passive veto [ms]",
                                            m_processingTimeNBins, 0,
                                            m_processingTimeMax);
  m_processingTimeNotPassiveVeto->StatOverflows(true);

  m_procTimeVsnSVDShaperDigitsPassiveVeto = new TH2F("procTimeVsnSVDShaperDigitsPassiveVeto",
                                                     "Processing time [ms] vs nSVDShaperDigits of events passing passive veto",
                                                     m_nSVDShaperDigitsNBins, 0, m_nSVDShaperDigitsMax,
                                                     m_processingTimeNBins, 0, m_processingTimeMax);
  m_procTimeVsnSVDShaperDigitsPassiveVeto->StatOverflows(true);
  m_procTimeVsnSVDShaperDigitsPassiveVeto->SetXTitle("nSVDShaperDigits");
  m_procTimeVsnSVDShaperDigitsPassiveVeto->SetYTitle("Processing time [ms]");
  m_procTimeVsnSVDShaperDigitsNotPassiveVeto = new TH2F("procTimeVsnSVDShaperDigitsNotPassiveVeto",
                                                        "Processing time [ms] vs nSVDShaperDigits of events not passing passive veto",
                                                        m_nSVDShaperDigitsNBins, 0, m_nSVDShaperDigitsMax,
                                                        m_processingTimeNBins, 0, m_processingTimeMax);
  m_procTimeVsnSVDShaperDigitsNotPassiveVeto->StatOverflows(true);
  m_procTimeVsnSVDShaperDigitsNotPassiveVeto->SetXTitle("nSVDShaperDigits");
  m_procTimeVsnSVDShaperDigitsNotPassiveVeto->SetYTitle("Processing time [ms]");
  m_procTimeVsnCDCHitsPassiveVeto = new TH2F("procTimeVsnCDCHitsPassiveVeto",
                                             "Processing time [ms] vs nCDCHits of events passing passive veto",
                                             m_nCDCHitsNBins, 0, m_nCDCHitsMax,
                                             m_processingTimeNBins, 0, m_processingTimeMax);
  m_procTimeVsnCDCHitsPassiveVeto->StatOverflows(true);
  m_procTimeVsnCDCHitsPassiveVeto->SetXTitle("nCDCHits");
  m_procTimeVsnCDCHitsPassiveVeto->SetYTitle("Processing time [ms]");
  m_procTimeVsnCDCHitsNotPassiveVeto = new TH2F("procTimeVsnCDCHitsNotPassiveVeto",
                                                "Processing time [ms] vs nCDCHits of events not passing passive veto",
                                                m_nCDCHitsNBins, 0, m_nCDCHitsMax,
                                                m_processingTimeNBins, 0, m_processingTimeMax);
  m_procTimeVsnCDCHitsNotPassiveVeto->StatOverflows(true);
  m_procTimeVsnCDCHitsNotPassiveVeto->SetXTitle("nCDCHits");
  m_procTimeVsnCDCHitsNotPassiveVeto->SetYTitle("Processing time [ms]");
  m_procTimeVsnECLDigitsPassiveVeto = new TH2F("procTimeVsnECLDigitsPassiveVeto",
                                               "Processing time [ms] vs nECLDigits of events passing passive veto",
                                               m_nECLDigitsNBins, 0, m_nECLDigitsMax,
                                               m_processingTimeNBins, 0, m_processingTimeMax);
  m_procTimeVsnECLDigitsPassiveVeto->StatOverflows(true);
  m_procTimeVsnECLDigitsPassiveVeto->SetXTitle("nECLDigits");
  m_procTimeVsnECLDigitsPassiveVeto->SetYTitle("Processing time [ms]");
  m_procTimeVsnECLDigitsNotPassiveVeto = new TH2F("procTimeVsnECLDigitsNotPassiveVeto",
                                                  "Processing time [ms] vs nECLDigits of events not passing passive veto",
                                                  m_nECLDigitsNBins, 0, m_nECLDigitsMax,
                                                  m_processingTimeNBins, 0, m_processingTimeMax);
  m_procTimeVsnECLDigitsNotPassiveVeto->StatOverflows(true);
  m_procTimeVsnECLDigitsNotPassiveVeto->SetXTitle("nECLDigits");
  m_procTimeVsnECLDigitsNotPassiveVeto->SetYTitle("Processing time [ms]");

  if (oldDirectory) {
    oldDirectory->cd();
  }
}


void StatisticsTimingHLTDQMModule::initialize()
{
  m_trgSummary.isOptional();
  m_svdShaperDigits.isOptional();
  m_cdcHits.isOptional();
  m_eclDigits.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  if (m_param_create_hlt_unit_histograms) {
    // Read the HLT unit's hostname straight from the HLT worker
    FILE* pipe = popen("hostname -d", "r");
    if (pipe) {
      char buffer[128];
      std::string host = "";

      while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        host += buffer;
      }

      pclose(pipe);

      // Trim space and new line
      host.erase(std::remove_if(host.begin(), host.end(), ::isspace), host.end());

      if (host.rfind("hlt", 0) == 0 && host.length() == 5) {
        m_hlt_unit = std::atoi(host.substr(3, 2).c_str());
      } else {
        B2WARNING("HLT unit number not found");
      }
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
    const double statisticsMemory = moduleStatistics.getMemoryMean(ModuleStatistics::EStatisticCounters::c_Total) / 1024;
    const double statisticsTime_sum = moduleStatistics.getTimeSum(ModuleStatistics::EStatisticCounters::c_Event) / Unit::ms;
    const double statisticsMemory_sum = moduleStatistics.getMemorySum(ModuleStatistics::EStatisticCounters::c_Total) / 1024;

    const int m_param_overviewModuleListIndex = std::distance(m_param_overviewModuleList.begin(), m_param_overviewModuleListIterator);
    meanTimes[m_param_overviewModuleListIndex] += statisticsTime;
    meanMemories[m_param_overviewModuleListIndex] += statisticsMemory;

    m_moduleTimeHistograms[statisticsName]->Fill(statisticsTime_sum - m_lastModuleTimeSum[statisticsName]);
    m_lastModuleTimeSum[statisticsName] = statisticsTime_sum;
    m_moduleMemoryHistograms[statisticsName]->Fill(statisticsMemory_sum);
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

  const ModuleStatistics& fullStatistics = stats->getGlobal();
  const double fullTimeSum = fullStatistics.getTimeSum(ModuleStatistics::EStatisticCounters::c_Event) / Unit::ms;
  m_fullTimeHistogram->Fill(fullTimeSum - m_lastFullTimeSum);
  m_lastFullTimeSum = fullTimeSum;
  const double fullMemorySum = fullStatistics.getMemorySum(ModuleStatistics::EStatisticCounters::c_Total) / 1024;
  m_fullMemoryHistogram->Fill(fullMemorySum);

  if (m_param_create_hlt_unit_histograms) {
    if (0 < m_hlt_unit) {
      m_processingTimeMeanPerUnitHistogram->SetBinContent(m_hlt_unit + 1, processingTimeMean);

      m_processingTimePerUnitHistograms[m_hlt_unit]->Fill(processingTimeSum - m_lastProcessingTimeSumPerUnit[m_hlt_unit]);
      m_lastProcessingTimeSumPerUnit[m_hlt_unit] = processingTimeSum;

      const double fullTimeMean = fullStatistics.getTimeMean(ModuleStatistics::EStatisticCounters::c_Event) / Unit::ms;
      m_fullTimeMeanPerUnitHistogram->SetBinContent(m_hlt_unit + 1, fullTimeMean);

      m_fullTimePerUnitHistograms[m_hlt_unit]->Fill(fullTimeSum - m_lastFullTimeSumPerUnit[m_hlt_unit]);
      m_lastFullTimeSumPerUnit[m_hlt_unit] = fullTimeSum;

      m_fullMemoryPerUnitHistograms[m_hlt_unit]->Fill(fullMemorySum);
    }
  }

  const uint32_t nCDCHits = m_cdcHits.isOptional() ? m_cdcHits.getEntries() : 0;
  const uint32_t nSVDShaperDigits = m_svdShaperDigits.isOptional() ? m_svdShaperDigits.getEntries() : 0;
  const uint32_t nECLDigits = m_eclDigits.isOptional() ? m_eclDigits.getEntries() : 0;
  if (!m_trgSummary.isValid()) {
    return;
  }
  try {
    if (m_trgSummary->testInput("passive_veto") == 0) { // These events would stay even with just passive veto
      m_processingTimePassiveVeto->Fill(processingTimeSum - m_lastProcessingTimeSum);

      m_procTimeVsnSVDShaperDigitsPassiveVeto->Fill(nSVDShaperDigits, processingTimeSum - m_lastProcessingTimeSum);
      m_procTimeVsnCDCHitsPassiveVeto->Fill(nCDCHits, processingTimeSum - m_lastProcessingTimeSum);
      m_procTimeVsnECLDigitsPassiveVeto->Fill(nECLDigits, processingTimeSum - m_lastProcessingTimeSum);
    } else {
      m_processingTimeNotPassiveVeto->Fill(processingTimeSum - m_lastProcessingTimeSum);

      m_procTimeVsnSVDShaperDigitsNotPassiveVeto->Fill(nSVDShaperDigits, processingTimeSum - m_lastProcessingTimeSum);
      m_procTimeVsnCDCHitsNotPassiveVeto->Fill(nCDCHits, processingTimeSum - m_lastProcessingTimeSum);
      m_procTimeVsnECLDigitsNotPassiveVeto->Fill(nECLDigits, processingTimeSum - m_lastProcessingTimeSum);
    }
  } catch (const std::exception&) {
    return;
  }

  m_lastProcessingTimeSum = processingTimeSum;
}

void StatisticsTimingHLTDQMModule::beginRun()
{
  if (!m_meanTimeHistogram || !m_meanMemoryHistogram || !m_fullTimeHistogram || !m_processingTimeHistogram) {
    B2FATAL("Histograms were not created. Did you setup a HistoManager?");
  }

  m_meanTimeHistogram->Reset();
  m_meanMemoryHistogram->Reset();
  std::for_each(m_moduleTimeHistograms.begin(), m_moduleTimeHistograms.end(),
  [](auto & it) { it.second->Reset(); });
  std::for_each(m_moduleMemoryHistograms.begin(), m_moduleMemoryHistograms.end(),
  [](auto & it) { it.second->Reset(); });
  m_fullTimeHistogram->Reset();
  m_processingTimeHistogram->Reset();
  m_fullMemoryHistogram->Reset();
  if (m_param_create_hlt_unit_histograms) {
    m_fullTimeMeanPerUnitHistogram->Reset();
    m_processingTimeMeanPerUnitHistogram->Reset();
    std::for_each(m_fullTimePerUnitHistograms.begin(), m_fullTimePerUnitHistograms.end(),
    [](auto & it) { it.second->Reset(); });
    std::for_each(m_processingTimePerUnitHistograms.begin(), m_processingTimePerUnitHistograms.end(),
    [](auto & it) { it.second->Reset(); });
    std::for_each(m_fullMemoryPerUnitHistograms.begin(), m_fullMemoryPerUnitHistograms.end(),
    [](auto & it) { it.second->Reset(); });
    m_processesPerUnitHistogram->Reset();

    m_processesPerUnitHistogram->Fill(m_hlt_unit);
  }
  m_processingTimePassiveVeto->Reset();
  m_processingTimeNotPassiveVeto->Reset();
  m_procTimeVsnSVDShaperDigitsPassiveVeto->Reset();
  m_procTimeVsnSVDShaperDigitsNotPassiveVeto->Reset();
  m_procTimeVsnCDCHitsPassiveVeto->Reset();
  m_procTimeVsnCDCHitsNotPassiveVeto->Reset();
  m_procTimeVsnECLDigitsPassiveVeto->Reset();
  m_procTimeVsnECLDigitsNotPassiveVeto->Reset();

}

