/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li, Thomas Hauth, Nils Braun, Markus Prim        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/modules/dqm/SoftwareTriggerHLTDQMModule.h>

#include <TDirectory.h>

#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>

#include <framework/core/ModuleParam.templateDetails.h>

#include <algorithm>
#include <fstream>

using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(SoftwareTriggerHLTDQM)

SoftwareTriggerHLTDQMModule::SoftwareTriggerHLTDQMModule() : HistoModule()
{
  setDescription("Monitor Physics Trigger");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Fill in the default values of the module parameters
  m_param_variableIdentifiers = {};

  m_param_cutResultIdentifiers["filter"]["filter"] = {"total_result"};

  addParam("cutResultIdentifiers", m_param_cutResultIdentifiers,
           "Which cuts should be reported? Please remember to include the total_result also, if wanted.",
           m_param_cutResultIdentifiers);

  addParam("cutResultIdentifiersIgnored", m_param_cutResultIdentifiersIgnored,
           "Which cuts should be ignored? This will display cleaner trigger lines, e.g. to clear them from bhabha contamination. "
           "Vetoes on skims do not apply in filter plot and vice versa.",
           m_param_cutResultIdentifiersIgnored);

  addParam("cutResultIdentifiersPerUnit", m_param_cutResultIdentifiersPerUnit,
           "Which cuts should be reported per unit?",
           m_param_cutResultIdentifiersPerUnit);

  addParam("variableIdentifiers", m_param_variableIdentifiers,
           "Which variables should be reported?",
           m_param_variableIdentifiers);

  addParam("l1Identifiers", m_param_l1Identifiers,
           "Which l1 identifiers to report?",
           m_param_l1Identifiers);

  addParam("createTotalResultHistograms", m_param_create_total_result_histograms,
           "Create total result histogram?",
           true);

  addParam("createExpRunEventHistograms", m_param_create_exp_run_event_histograms,
           "Create exp/run/event histograms?",
           true);

  addParam("createHLTUnitHistograms", m_param_create_hlt_unit_histograms,
           "Create HLT unit histograms?",
           false);

  addParam("createErrorFlagHistograms", m_param_create_error_flag_histograms,
           "Create Error Flag histograms?",
           false);

  addParam("histogramDirectoryName", m_param_histogramDirectoryName,
           "SoftwareTrigger DQM histograms will be put into this directory", m_param_histogramDirectoryName);

  addParam("pathLocation", m_param_pathLocation,
           "Location of the module in the path: before filter or after filter", m_param_pathLocation);
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

    const std::string& title = cutIdentifier.first;
    const auto& mapVal = *(m_param_cutResultIdentifiers[title].begin());
    const std::string& baseIdentifier = mapVal.first;
    const int numberOfFlags = mapVal.second.size();

    if (m_param_histogramDirectoryName == "softwaretrigger_skim_nobhabha") {
      if (title == baseIdentifier)
        m_cutResultHistograms.emplace(title,
                                      new TH1F((title + "_nobhabha").c_str(), ("Events triggered in HLT " + baseIdentifier).c_str(),
                                               numberOfFlags, 0,
                                               numberOfFlags));
      else
        m_cutResultHistograms.emplace(title,
                                      new TH1F((baseIdentifier + "_" + title + "_nobhabha").c_str(),
                                               ("Events triggered in HLT " + baseIdentifier + " : " + title).c_str(),
                                               numberOfFlags, 0,
                                               numberOfFlags));
    } else {
      if (title == baseIdentifier)
        m_cutResultHistograms.emplace(title,
                                      new TH1F(title.c_str(), ("Events triggered in HLT " + baseIdentifier).c_str(),
                                               numberOfFlags, 0,
                                               numberOfFlags));
      else
        m_cutResultHistograms.emplace(title,
                                      new TH1F((baseIdentifier + "_" + title).c_str(), ("Events triggered in HLT " + baseIdentifier + " : " + title).c_str(),
                                               numberOfFlags, 0,
                                               numberOfFlags));
    }
    m_cutResultHistograms[title]->SetXTitle("");
    m_cutResultHistograms[title]->SetOption("hist");
    m_cutResultHistograms[title]->SetStats(false);
    m_cutResultHistograms[title]->SetMinimum(0);
  }

  // We add one for the total result
  if (m_param_create_total_result_histograms) {
    m_cutResultHistograms.emplace("total_result",
                                  new TH1F("total_result", "Total Result of HLT (absolute numbers)", 1, 0, 0));
    m_cutResultHistograms["total_result"]->SetXTitle("Total Cut Result");
    m_cutResultHistograms["total_result"]->SetOption("hist");
    m_cutResultHistograms["total_result"]->SetStats(false);
    m_cutResultHistograms["total_result"]->SetMinimum(0);
  }

  for (const std::string& trigger : m_param_l1Identifiers) {
    m_l1Histograms.emplace(trigger, new TH1F(trigger.c_str(), ("Events triggered in L1 " + trigger).c_str(), 1, 0, 0));
    m_l1Histograms[trigger]->SetXTitle("");
    m_l1Histograms[trigger]->SetOption("hist");
    m_l1Histograms[trigger]->SetStats(false);
    m_l1Histograms[trigger]->SetMinimum(0);
  }

  // And also one for the total numbers
  if (m_param_create_total_result_histograms) {
    m_l1Histograms.emplace("l1_total_result",
                           new TH1F("l1_total_result", "Events triggered in L1 (total results)", 1, 0, 0));
    m_l1Histograms["l1_total_result"]->SetXTitle("Total L1 Cut Result");
    m_l1Histograms["l1_total_result"]->SetOption("hist");
    m_l1Histograms["l1_total_result"]->SetStats(false);
    m_l1Histograms["l1_total_result"]->SetMinimum(0);
  }

  if (m_param_create_exp_run_event_histograms) {
    m_runInfoHistograms.emplace("run_number", new TH1F("run_number", "Run Number", 100, 0, 10000));
    m_runInfoHistograms.emplace("event_number", new TH1F("event_number", "Event Number", 100, 0, 1'000'000));
    m_runInfoHistograms.emplace("experiment_number", new TH1F("experiment_number", "Experiment Number", 50, 0, 50));
  }

  if (m_param_create_hlt_unit_histograms) {
    if (m_param_histogramDirectoryName != "softwaretrigger_before_filter") {
      m_runInfoHistograms.emplace("hlt_unit_number", new TH1F("hlt_unit_number_after_filter",
                                                              ("Number of events per HLT unit " + m_param_pathLocation).c_str(), HLTUnit::max_hlt_units + 1, 0,
                                                              HLTUnit::max_hlt_units + 1));
    } else {
      m_runInfoHistograms.emplace("hlt_unit_number", new TH1F("hlt_unit_number",
                                                              ("Number of events per HLT unit " + m_param_pathLocation).c_str(), HLTUnit::max_hlt_units + 1, 0,
                                                              HLTUnit::max_hlt_units + 1));
    }
    m_runInfoHistograms["hlt_unit_number"]->SetMinimum(0);

    for (const auto& cutIdentifierPerUnit : m_param_cutResultIdentifiersPerUnit) {
      m_cutResultPerUnitHistograms.emplace(cutIdentifierPerUnit , new TH1F((cutIdentifierPerUnit + "_per_unit").c_str(),
                                           ("Events triggered per unit in HLT : " + cutIdentifierPerUnit).c_str(), HLTUnit::max_hlt_units + 1, 0,
                                           HLTUnit::max_hlt_units + 1));
      m_cutResultPerUnitHistograms[cutIdentifierPerUnit]->SetXTitle("HLT unit number");
      m_cutResultPerUnitHistograms[cutIdentifierPerUnit]->SetOption("histe");
      m_cutResultPerUnitHistograms[cutIdentifierPerUnit]->SetMinimum(0);
    }

  }

  if (m_param_create_error_flag_histograms) {
    m_runInfoHistograms.emplace("error_flag", new TH1F("error_flag", "Error Flag", 4, 0, 4));
    m_runInfoHistograms["error_flag"]->SetStats(false);
    m_runInfoHistograms["error_flag"]->SetOption("hist");
    m_runInfoHistograms["error_flag"]->SetMinimum(0);
  }

  if (oldDirectory) {
    oldDirectory->cd();
  }
}

void SoftwareTriggerHLTDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  if (m_param_create_hlt_unit_histograms) {
    std::ifstream file;
    file.open(HLTUnit::hlt_unit_file);
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
      const std::string& title = cutIdentifier.first;
      const auto& mapVal = *(m_param_cutResultIdentifiers[title].begin());
      const std::string& baseIdentifier = mapVal.first;
      const auto& cuts = mapVal.second;

      // check if we want to ignore it
      bool skip = false;
      const auto& cutsIgnored = m_param_cutResultIdentifiersIgnored[baseIdentifier];

      for (const std::string& cutTitleIgnored : cutsIgnored) {
        const std::string& cutNameIgnored = cutTitleIgnored.substr(0, cutTitleIgnored.find("\\"));
        const std::string& fullCutIdentifierIgnored = SoftwareTriggerDBHandler::makeFullCutName(baseIdentifier, cutNameIgnored);

        const auto resultsIgnored = m_triggerResult->getResults();
        auto const cutEntryIgnored = resultsIgnored.find(fullCutIdentifierIgnored);

        if (cutEntryIgnored != resultsIgnored.end()) {
          if (cutEntryIgnored->second > 0) skip = true;
        }
      }

      for (const std::string& cutTitle : cuts) {
        const std::string& cutName = cutTitle.substr(0, cutTitle.find("\\"));
        const std::string& fullCutIdentifier = SoftwareTriggerDBHandler::makeFullCutName(baseIdentifier, cutName);

        // check if the cutResult is in the list, be graceful when not available
        // Create results object instead of calling .find() on a temporary object. This will cause undefined behaviour
        // when checking again the .end() pointer, when the .end() pointer is also created from a temporary object.
        const auto results = m_triggerResult->getResults();
        auto const cutEntry = results.find(fullCutIdentifier);

        if (cutEntry != results.end()) {
          const int cutResult = cutEntry->second;
          m_cutResultHistograms[title]->Fill(cutTitle.c_str(), cutResult > 0 and not skip);
        }
      }

      if (m_param_create_total_result_histograms) {
        if (title == baseIdentifier) {
          const std::string& totalCutIdentifier = SoftwareTriggerDBHandler::makeTotalResultName(baseIdentifier);
          const int cutResult = static_cast<int>(m_triggerResult->getResult(totalCutIdentifier));

          m_cutResultHistograms["total_result"]->Fill(totalCutIdentifier.c_str(), cutResult > 0);
        }
      }
    }

    if (m_param_create_total_result_histograms) {
      const bool totalResult = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_triggerResult);
      m_cutResultHistograms["total_result"]->Fill("total_result", totalResult > 0);
    }
  }

  if (m_l1TriggerResult.isValid() and m_l1NameLookup.isValid() and m_triggerResult.isValid()) {
    for (const std::string& l1Trigger : m_param_l1Identifiers) {
      const int triggerBit = m_l1NameLookup->getoutbitnum(l1Trigger.c_str());
      if (triggerBit < 0) {
        B2WARNING("Can not find L1 trigger with name " << l1Trigger);
        continue;
      }
      const bool triggerResult = m_l1TriggerResult->testPsnm(triggerBit);
      if (m_param_create_total_result_histograms) {
        m_l1Histograms["l1_total_result"]->Fill(l1Trigger.c_str(), triggerResult);
      }

      if (not triggerResult) {
        continue;
      }

      for (auto const& cutIdentifier : m_param_cutResultIdentifiers) {
        const std::string& title = cutIdentifier.first;
        const auto& mapVal = *(m_param_cutResultIdentifiers[title].begin());
        const std::string& baseIdentifier = mapVal.first;
        const auto& cuts = mapVal.second;

        for (const std::string& cutTitle : cuts) {
          const std::string& cutName = cutTitle.substr(0, cutTitle.find("\\"));
          const std::string& fullCutIdentifier = SoftwareTriggerDBHandler::makeFullCutName(baseIdentifier, cutName);

          // check if the cutResult is in the list, be graceful when not available
          const auto results = m_triggerResult->getResults();
          auto const cutEntry = results.find(fullCutIdentifier);

          if (cutEntry != results.end()) {
            const int cutResult = cutEntry->second;
            m_l1Histograms[l1Trigger]->Fill(cutTitle.c_str(), cutResult > 0);
          }
        }
      }

      const bool totalResult = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_triggerResult);
      m_l1Histograms[l1Trigger]->Fill("hlt_result", totalResult > 0);
      m_l1Histograms[l1Trigger]->LabelsDeflate("X");
    }
  }

  if (m_eventMetaData.isValid() and m_param_create_exp_run_event_histograms) {
    m_runInfoHistograms["run_number"]->Fill(m_eventMetaData->getRun());
    m_runInfoHistograms["event_number"]->Fill(m_eventMetaData->getEvent());
    m_runInfoHistograms["experiment_number"]->Fill(m_eventMetaData->getExperiment());
  }

  if (m_eventMetaData.isValid() and m_param_create_error_flag_histograms) {
    m_runInfoHistograms["error_flag"]->Fill("B2LinkPacketCRCError",
                                            (bool)(m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkPacketCRCError));
    m_runInfoHistograms["error_flag"]->Fill("B2LinkEventCRCError",
                                            (bool)(m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkEventCRCError));
    m_runInfoHistograms["error_flag"]->Fill("HLTCrash",
                                            (bool)(m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_HLTCrash));
    m_runInfoHistograms["error_flag"]->Fill("ReconstructionAbort",
                                            (bool)(m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_ReconstructionAbort));
  }

  if (m_param_create_hlt_unit_histograms) {
    m_runInfoHistograms["hlt_unit_number"]->Fill(m_hlt_unit);

    if (m_triggerResult.isValid()) {
      for (const std::string& cutIdentifierPerUnit : m_param_cutResultIdentifiersPerUnit) {
        const std::string& cutName = cutIdentifierPerUnit.substr(0, cutIdentifierPerUnit.find("\\"));
        const std::string& fullCutIdentifier = SoftwareTriggerDBHandler::makeFullCutName("filter", cutName);

        // check if the cutResult is in the list, be graceful when not available
        const auto results = m_triggerResult->getResults();
        auto const cutEntry = results.find(fullCutIdentifier);

        if (cutEntry != results.end()) {
          const int cutResult = cutEntry->second;
          m_cutResultPerUnitHistograms[cutIdentifierPerUnit]->Fill(m_hlt_unit, cutResult > 0);
        }
      }
    }
  }
}

void SoftwareTriggerHLTDQMModule::beginRun()
{
  std::for_each(m_cutResultHistograms.begin(), m_cutResultHistograms.end(),
  [](auto & it) { it.second->Reset(); });
  std::for_each(m_cutResultPerUnitHistograms.begin(), m_cutResultPerUnitHistograms.end(),
  [](auto & it) { it.second->Reset(); });
  std::for_each(m_triggerVariablesHistograms.begin(), m_triggerVariablesHistograms.end(),
  [](auto & it) { it.second->Reset(); });
  std::for_each(m_l1Histograms.begin(), m_l1Histograms.end(),
  [](auto & it) { it.second->Reset(); });
  std::for_each(m_runInfoHistograms.begin(), m_runInfoHistograms.end(),
  [](auto & it) { it.second->Reset(); });
}

