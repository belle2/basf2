/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li, Thomas Hauth, Nils Braun                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/modules/dqm/SoftwareTriggerHLTDQMModule.h>

#include <TDirectory.h>

#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>

#include <framework/core/ModuleParam.templateDetails.h>

#include <algorithm>

using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(SoftwareTriggerHLTDQM)

SoftwareTriggerHLTDQMModule::SoftwareTriggerHLTDQMModule() : HistoModule()
{
  setDescription("Monitor Physics Trigger");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Fill in the default values of the module parameters
  m_param_variableIdentifiers = {};

  m_param_cutResultIdentifiers["filter"] = {"total_result"};

  addParam("cutResultIdentifiers", m_param_cutResultIdentifiers,
           "Which cuts should be reported? Please remember to include the total_result also, if wanted.",
           m_param_cutResultIdentifiers);

  addParam("variableIdentifiers", m_param_variableIdentifiers,
           "Which variables should be reported?",
           m_param_variableIdentifiers);

  addParam("l1Identifiers", m_param_l1Identifiers,
           "Which l1 identifiers to report?",
           m_param_l1Identifiers);

  addParam("histogramDirectoryName", m_param_histogramDirectoryName,
           "SoftwareTrigger DQM histograms will be put into this directory", m_param_histogramDirectoryName);
}

void SoftwareTriggerHLTDQMModule::defineHisto()
{
  TDirectory* oldDirectory = nullptr;

  if (!m_param_histogramDirectoryName.empty()) {
    oldDirectory = gDirectory;
    TDirectory* histDir = oldDirectory->mkdir(m_param_histogramDirectoryName.c_str());
    histDir->cd();
  }

  for (auto const& variable : m_param_variableIdentifiers) {
    // todo: make correct range
    const unsigned int numberOfBins = 50;
    const double lowerX = 0;
    const double upperX = 50;
    m_triggerVariablesHistograms.emplace(variable, new TH1F(variable.c_str(), variable.c_str(), numberOfBins, lowerX, upperX));
    m_triggerVariablesHistograms[variable]->SetXTitle(("SoftwareTriggerVariable " + variable).c_str());
  }

  for (const auto& cutIdentifier : m_param_cutResultIdentifiers) {
    const std::string& baseIdentifier = cutIdentifier.first;

    m_cutResultHistograms.emplace(baseIdentifier,
                                  new TH1F(baseIdentifier.c_str(), ("Events triggered in HLT baseIdentifier " + baseIdentifier).c_str(), 0, 0, 0));
    m_cutResultHistograms[baseIdentifier]->SetXTitle(("Prescaled Cut Result for " + baseIdentifier).c_str());
    m_cutResultHistograms[baseIdentifier]->SetOption("bar");
    m_cutResultHistograms[baseIdentifier]->SetFillStyle(0);
    m_cutResultHistograms[baseIdentifier]->SetStats(false);
  }

  // We add one for the total result
  m_cutResultHistograms.emplace("total_result",
                                new TH1F("total_result", "Total Result of HLT (absolute numbers)", 0, 0, 0));
  m_cutResultHistograms["total_result"]->SetXTitle("Total Cut Result");
  m_cutResultHistograms["total_result"]->SetOption("bar");
  m_cutResultHistograms["total_result"]->SetFillStyle(0);
  m_cutResultHistograms["total_result"]->SetStats(false);

  for (const std::string& trigger : m_param_l1Identifiers) {
    m_l1Histograms.emplace(trigger, new TH1F(trigger.c_str(), ("Events triggered in L1 " + trigger).c_str(), 0, 0, 0));
    m_l1Histograms[trigger]->SetXTitle(("HLT Result for L1: " + trigger).c_str());
    m_l1Histograms[trigger]->SetOption("bar");
    m_l1Histograms[trigger]->SetFillStyle(0);
    m_l1Histograms[trigger]->SetStats(false);
  }

  // And also one for the total numbers
  m_l1Histograms.emplace("l1_total_result",
                         new TH1F("l1_total_result", "Events triggered in L1 (total results)", 0, 0, 0));
  m_l1Histograms["l1_total_result"]->SetXTitle("Total L1 Cut Result");
  m_l1Histograms["l1_total_result"]->SetOption("bar");
  m_l1Histograms["l1_total_result"]->SetFillStyle(0);
  m_l1Histograms["l1_total_result"]->SetStats(false);

  m_runInfoHistograms.emplace("run_number", new TH1F("run_number", "Run Number", 100, 0, 10000));
  m_runInfoHistograms.emplace("event_number", new TH1F("event_number", "Event Number", 100, 0, 1'000'000));
  m_runInfoHistograms.emplace("experiment_number", new TH1F("experiment_number", "Experiment Number", 50, 0, 50));

  if (oldDirectory) {
    oldDirectory->cd();
  }
}

void SoftwareTriggerHLTDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void SoftwareTriggerHLTDQMModule::event()
{
  // this might be pre-scaled for performance reasons in the final configuration, therefore this structure
  // might not be filled in every event
  if (m_variables.isValid()) {
    for (auto& variableNameAndTH1F : m_triggerVariablesHistograms) {
      const std::string& variable = variableNameAndTH1F.first;
      TH1F* histogram = variableNameAndTH1F.second;

      // try to load this variable from the computed trigger variables
      if (not m_variables->has(variable)) {
        B2ERROR("Variable " << variable << " configured for SoftwareTriggerDQM plotting is not available");
      } else {
        const double value = m_variables->getVariable(variable);
        histogram->Fill(value);
      }
    }
  }

  if (m_triggerResult.isValid()) {
    for (auto const& cutIdentifier : m_param_cutResultIdentifiers) {
      const std::string& baseIdentifier = cutIdentifier.first;
      const auto& cuts = cutIdentifier.second;

      for (const std::string& cutTitle : cuts) {
        const std::string& cutName = cutTitle.substr(0, cutTitle.find("\\"));
        const std::string& fullCutIdentifier = SoftwareTriggerDBHandler::makeFullCutName(baseIdentifier, cutName);

        // check if the cutResult is in the list, be graceful when not available
        auto const cutEntry = m_triggerResult->getResults().find(fullCutIdentifier);

        if (cutEntry != m_triggerResult->getResults().end()) {
          const int cutResult = cutEntry->second;
          m_cutResultHistograms[baseIdentifier]->Fill(cutTitle.c_str(), cutResult > 0);
        }
      }

      const std::string& totalCutIdentifier = SoftwareTriggerDBHandler::makeTotalResultName(baseIdentifier);
      const int cutResult = static_cast<int>(m_triggerResult->getResult(totalCutIdentifier));

      m_cutResultHistograms["total_result"]->Fill(totalCutIdentifier.c_str(), cutResult > 0);
    }

    const bool totalResult = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_triggerResult);
    m_cutResultHistograms["total_result"]->Fill("total_result", totalResult > 0);
  }

  if (m_l1TriggerResult.isValid() and m_l1NameLookup.isValid() and m_triggerResult.isValid()) {
    for (const std::string& l1Trigger : m_param_l1Identifiers) {
      const int triggerBit = m_l1NameLookup->getoutbitnum(l1Trigger.c_str());
      if (triggerBit < 0) {
        B2WARNING("Can not find L1 trigger with name " << l1Trigger);
        continue;
      }
      const bool triggerResult = m_l1TriggerResult->testPsnm(triggerBit);
      m_l1Histograms["l1_total_result"]->Fill(l1Trigger.c_str(), triggerResult);

      if (not triggerResult) {
        continue;
      }

      for (auto const& cutIdentifier : m_param_cutResultIdentifiers) {
        const std::string& baseIdentifier = cutIdentifier.first;
        const auto& cuts = cutIdentifier.second;

        for (const std::string& cutTitle : cuts) {
          const std::string& cutName = cutTitle.substr(0, cutTitle.find("\\"));
          const std::string& fullCutIdentifier = SoftwareTriggerDBHandler::makeFullCutName(baseIdentifier, cutName);

          // check if the cutResult is in the list, be graceful when not available
          auto const cutEntry = m_triggerResult->getResults().find(fullCutIdentifier);

          if (cutEntry != m_triggerResult->getResults().end()) {
            const int cutResult = cutEntry->second;
            m_l1Histograms[l1Trigger]->Fill(cutTitle.c_str(), cutResult > 0);
          }
        }
      }

      const bool totalResult = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_triggerResult);
      m_l1Histograms[l1Trigger]->Fill("hlt_result", totalResult > 0);
    }
  }

  if (m_eventMetaData.isValid()) {
    m_runInfoHistograms["run_number"]->Fill(m_eventMetaData->getRun());
    m_runInfoHistograms["event_number"]->Fill(m_eventMetaData->getEvent());
    m_runInfoHistograms["experiment_number"]->Fill(m_eventMetaData->getExperiment());
  }
}

void SoftwareTriggerHLTDQMModule::beginRun()
{
  std::for_each(m_cutResultHistograms.begin(), m_cutResultHistograms.end(),
  [](auto & it) { it.second->Reset(); });
  std::for_each(m_triggerVariablesHistograms.begin(), m_triggerVariablesHistograms.end(),
  [](auto & it) { it.second->Reset(); });
  std::for_each(m_l1Histograms.begin(), m_l1Histograms.end(),
  [](auto & it) { it.second->Reset(); });
  std::for_each(m_runInfoHistograms.begin(), m_runInfoHistograms.end(),
  [](auto & it) { it.second->Reset(); });
}
