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
#include <mdst/dbobjects/DBRepresentationOfSoftwareTriggerCut.h>
#include <framework/database/DBObjPtr.h>

#include <TFile.h>
#include <TTree.h>

#include <boost/algorithm/string/replace.hpp>
#include <memory>


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
  bool cut;

  debugTTree->Branch("cut", &cut);
  debugTTree->Branch("prescaled", &prescaled);
  debugTTree->Branch("accept_or_reject", &accepted);
  debugTTree->Branch("total_events", &m_numberOfEvents);
  std::vector<double> value(m_passedEventsPerTrigger.size());

  cut = true;
  prescaled = true;
  accepted = true;
  unsigned int counter = 0;
  for (auto& cutResult : m_passedEventsPerTrigger) {
    std::string cutName = cutResult.first;
    boost::replace_all(cutName, "&", "_");
    debugTTree->Branch(cutName.c_str(), &value.at(counter));

    value[counter] = static_cast<double>(cutResult.second[SoftwareTriggerCutResult::c_accept]);
    counter++;
  }
  debugTTree->Fill();

  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  cut = true;
  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  prescaled = true;
  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  accepted = false;
  counter = 0;
  for (auto& cutResult : m_passedEventsPerTrigger) {
    value[counter] = static_cast<double>(cutResult.second[SoftwareTriggerCutResult::c_reject]);
    counter++;
  }
  debugTTree->Fill();

  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  cut = true;
  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  prescaled = false;
  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  accepted = true;
  counter = 0;
  for (auto& cutResult : m_passedEventsPerTrigger) {
    const auto& cutName = cutResult.first;
    if (m_passedEventsPerTriggerNonPrescaled.find(cutName) == m_passedEventsPerTriggerNonPrescaled.end()) {
      value[counter] = NAN;
    } else {
      value[counter] = static_cast<double>(m_passedEventsPerTriggerNonPrescaled[cutName][SoftwareTriggerCutResult::c_accept]);
    }
    counter++;
  }
  debugTTree->Fill();

  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  cut = true;
  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  prescaled = false;
  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  accepted = false;
  counter = 0;
  for (auto& cutResult : m_passedEventsPerTrigger) {
    const auto& cutName = cutResult.first;
    if (m_passedEventsPerTriggerNonPrescaled.find(cutName) == m_passedEventsPerTriggerNonPrescaled.end()) {
      value[counter] = NAN;
    } else {
      value[counter] = static_cast<double>(m_passedEventsPerTriggerNonPrescaled[cutName][SoftwareTriggerCutResult::c_reject]);
    }
    counter++;
  }
  debugTTree->Fill();

  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  cut = false;
  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  prescaled = false;
  // cppcheck-suppress redundantAssignment; the variable is used in the Fill() method below
  accepted = false;
  counter = 0;
  for (auto& cutResult : m_passedEventsPerTrigger) {
    const auto& cutName = cutResult.first;
    if (m_prescales.find(cutName) == m_prescales.end()) {
      value[counter] = NAN;
    } else {
      value[counter] = static_cast<double>(m_prescales[cutName]);
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

  for (const auto& [cutName, cutResults] : m_resultStoreObjectPointer->getResultPairs()) {
    m_passedEventsPerTrigger[cutName][static_cast<SoftwareTriggerCutResult >(cutResults.first)]++;

    // This does only make sense for non-total results (as they are prescaled)
    if (cutName.find("total_result") == std::string::npos) {
      m_passedEventsPerTriggerNonPrescaled[cutName][static_cast<SoftwareTriggerCutResult >(cutResults.second)]++;

      DBObjPtr<DBRepresentationOfSoftwareTriggerCut> downloadedCut(cutName);
      if (downloadedCut) {
        m_prescales[cutName] = downloadedCut->getPreScaleFactor();
      }
    }
  }

  const bool eventAccepted = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer);
  if (eventAccepted) {
    m_passedEventsPerTrigger["final_decision"][SoftwareTriggerCutResult::c_accept]++;
  } else {
    m_passedEventsPerTrigger["final_decision"][SoftwareTriggerCutResult::c_reject]++;
  }

  if (m_l1Result.isValid()) {
    const bool l1Accepted = m_l1Result->test();
    if (l1Accepted) {
      m_passedEventsPerTrigger["l1_decision"][SoftwareTriggerCutResult::c_accept]++;
    } else {
      m_passedEventsPerTrigger["l1_decision"][SoftwareTriggerCutResult::c_reject]++;
    }
  } else {
    if (m_eventMetaDataPtr)
      B2WARNING("Uncaught exception encountered: Trying to access StoreObjPtr object 'TRGSummary' (durability: event), which was not created in exp/run/evt: "
                << LogVar("exp", m_eventMetaDataPtr->getExperiment())
                << LogVar("run", m_eventMetaDataPtr->getRun())
                << LogVar("event", m_eventMetaDataPtr->getEvent()));
  }
}
