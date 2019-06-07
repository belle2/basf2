/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/softwaretrigger/modules/basics/SoftwareTriggerResultPrinterModule.h>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>

#include <TFile.h>
#include <TTree.h>

#include <boost/algorithm/string/replace.hpp>
#include <memory>
#include <iomanip>

using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(SoftwareTriggerResultPrinter)


SoftwareTriggerResultPrinterModule::SoftwareTriggerResultPrinterModule()
  : Module()
{
  setDescription("Write out the software trigger results in an easily accessible summary table to disk.");

  addParam("outputFileName", m_param_outputFileName, "File path and name of the ROOT "
           "file, in which the results of the calculation are stored. Please note that already present files will be overridden. ",
           m_param_outputFileName);
}

void SoftwareTriggerResultPrinterModule::initialize()
{
  m_resultStoreObjectPointer.isRequired();
  m_l1Result.isRequired();
}

void SoftwareTriggerResultPrinterModule::terminate()
{

  auto debugOutputFile = std::unique_ptr<TFile>(TFile::Open(m_param_outputFileName.c_str(), "RECREATE"));
  if (not debugOutputFile) {
    B2FATAL("Could not open debug output file. Aborting.");
  }
  auto debugTTree = std::make_unique<TTree>("software_trigger_results", "software_trigger_results");
  if (not debugTTree) {
    B2FATAL("Could not create debug output tree. Aborting.");
  }

  bool prescaled;
  bool accepted;

  debugTTree->Branch("prescaled", &prescaled);
  debugTTree->Branch("accept_or_reject", &accepted);
  debugTTree->Branch("total_events", &m_numberOfEvents);
  std::vector<double> numberOfEvents(m_passedEventsPerTrigger.size());

  prescaled = true;
  accepted = true;
  unsigned int counter = 0;
  for (auto& cutResult : m_passedEventsPerTrigger) {
    std::string cutName = cutResult.first;
    boost::replace_all(cutName, "&", "_");
    debugTTree->Branch(cutName.c_str(), &numberOfEvents.at(counter));

    numberOfEvents[counter] = static_cast<double>(cutResult.second[SoftwareTriggerCutResult::c_accept]);
    counter++;
  }
  debugTTree->Fill();

  prescaled = true;
  accepted = false;
  counter = 0;
  for (auto& cutResult : m_passedEventsPerTrigger) {
    // cppcheck-suppress unreadVariable // the variable is used in the Fill() method below
    numberOfEvents[counter] = static_cast<double>(cutResult.second[SoftwareTriggerCutResult::c_reject]);
    counter++;
  }
  debugTTree->Fill();

  prescaled = false;
  accepted = true;
  counter = 0;
  for (auto& cutResult : m_passedEventsPerTrigger) {
    const auto& cutName = cutResult.first;
    if (m_passedEventsPerTriggerNonPrescaled.find(cutName) == m_passedEventsPerTriggerNonPrescaled.end()) {
      numberOfEvents[counter] = NAN;
    } else {
      numberOfEvents[counter] = static_cast<double>(m_passedEventsPerTriggerNonPrescaled[cutName][SoftwareTriggerCutResult::c_accept]);
    }
    counter++;
  }
  debugTTree->Fill();

  prescaled = false;
  accepted = false;
  counter = 0;
  for (auto& cutResult : m_passedEventsPerTrigger) {
    const auto& cutName = cutResult.first;
    if (m_passedEventsPerTriggerNonPrescaled.find(cutName) == m_passedEventsPerTriggerNonPrescaled.end()) {
      numberOfEvents[counter] = NAN;
    } else {
      numberOfEvents[counter] = static_cast<double>(m_passedEventsPerTriggerNonPrescaled[cutName][SoftwareTriggerCutResult::c_reject]);
    }
    counter++;
  }
  debugTTree->Fill();

  debugOutputFile->cd();
  debugOutputFile->Write();
  debugTTree.reset();
  debugOutputFile.reset();
}

void SoftwareTriggerResultPrinterModule::event()
{
  m_numberOfEvents++;

  for (const auto& triggerResult : m_resultStoreObjectPointer->getResultPairs()) {
    auto cutResults = triggerResult.second;
    m_passedEventsPerTrigger[triggerResult.first][static_cast<SoftwareTriggerCutResult >(cutResults.first)]++;

    // This does only make sense for non-total results (as they are prescaled)
    if (triggerResult.first.find("total_result") == std::string::npos) {
      m_passedEventsPerTriggerNonPrescaled[triggerResult.first][static_cast<SoftwareTriggerCutResult >(cutResults.second)]++;
    }
  }

  const bool eventAccepted = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer);
  if (eventAccepted) {
    m_passedEventsPerTrigger["final_decision"][SoftwareTriggerCutResult::c_accept]++;
  } else {
    m_passedEventsPerTrigger["final_decision"][SoftwareTriggerCutResult::c_reject]++;
  }

  const bool l1Accepted = m_l1Result->test();
  if (l1Accepted) {
    m_passedEventsPerTrigger["l1_decision"][SoftwareTriggerCutResult::c_accept]++;
  } else {
    m_passedEventsPerTrigger["l1_decision"][SoftwareTriggerCutResult::c_reject]++;
  }
}
