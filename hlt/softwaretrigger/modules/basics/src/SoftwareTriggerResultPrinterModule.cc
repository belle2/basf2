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
  setDescription("Print (and optionally write out) the software trigger "
                 "results in an easily accessible summary table.");

  addParam("storeResultsToDisk", m_param_storeResultsToDisk,
           "Write out all results to disk also (instead of just printing them",
           m_param_storeResultsToDisk);

  addParam("debugOutputFileName", m_param_debugOutputFileName, "File path and name of the ROOT "
           "file, in which the results of the calculation are stored, if storeResultsToDisk is "
           "turned on. Please note that already present files will be overridden. ",
           m_param_debugOutputFileName);
}

void SoftwareTriggerResultPrinterModule::initialize()
{
  m_resultStoreObjectPointer.isRequired();
  m_l1Result.isRequired();
}

void SoftwareTriggerResultPrinterModule::terminate()
{
  B2RESULT("Results of SoftwareTrigger:");
  B2RESULT("\t" << std::setw(50) << "Name" << "\t" << std::setw(30) << "Accepted" << "\t" << std::setw(30)
           << "Rejected");
  for (auto& cutResult : m_passedEventsPerTrigger) {
    const std::string& cutName = cutResult.first;
    const double passedEvents = static_cast<double>(cutResult.second[SoftwareTriggerCutResult::c_accept]);
    const double rejectedEvents = static_cast<double>(cutResult.second[SoftwareTriggerCutResult::c_reject]);

    std::string printName = cutName;
    boost::replace_all(printName, "software_trigger_cut&", "");

    B2RESULT("\t" << std::setw(50) << printName.substr(0, 50) << "\t"
             << std::setw(8) << passedEvents << "/" << std::setw(8) << m_numberOfEvents << " (" <<
             std::setw(8) << std::setprecision(6) << 100 * passedEvents / m_numberOfEvents << " %)" << "\t"
             << std::setw(8) << rejectedEvents << "/" << std::setw(8) << m_numberOfEvents << " (" <<
             std::setw(8) << std::setprecision(6) << 100 * rejectedEvents / m_numberOfEvents << " %)" << "\t");

    if (m_passedEventsPerTriggerNonPrescaled.find(cutName) != m_passedEventsPerTriggerNonPrescaled.end()) {
      auto cutResultNoPre = m_passedEventsPerTriggerNonPrescaled[cutName];
      const double passedEventsNoPre = static_cast<double>(cutResultNoPre[SoftwareTriggerCutResult::c_accept]);
      const double rejectedEventsNoPre = static_cast<double>(cutResultNoPre[SoftwareTriggerCutResult::c_reject]);

      const std::string& cutNameNoPre = "not prescaled: " + printName;

      B2RESULT("\t" << std::setw(50) << cutNameNoPre.substr(0, 50) << "\t"
               << std::setw(8) << passedEventsNoPre << "/" << std::setw(8) << m_numberOfEvents << " (" <<
               std::setw(8) << std::setprecision(6) << 100 * passedEventsNoPre / m_numberOfEvents << " %)" << "\t"
               << std::setw(8) << rejectedEventsNoPre << "/" << std::setw(8) << m_numberOfEvents << " (" <<
               std::setw(8) << std::setprecision(6) << 100 * rejectedEventsNoPre / m_numberOfEvents << " %)" << "\t");
    }
  }

  if (m_param_storeResultsToDisk) {
    auto debugOutputFile = std::unique_ptr<TFile>(TFile::Open(m_param_debugOutputFileName.c_str(), "RECREATE"));
    if (not debugOutputFile) {
      B2FATAL("Could not open debug output file. Aborting.");
    }
    auto debugTTree = std::make_unique<TTree>("software_trigger_results", "software_trigger_results");
    if (not debugTTree) {
      B2FATAL("Could not create debug output tree. Aborting.");
    }

    bool accepted;

    debugTTree->Branch("accept_or_reject", &accepted);
    debugTTree->Branch("total_events", &m_numberOfEvents);
    std::vector<double> numberOfEvents(m_passedEventsPerTrigger.size());

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

    accepted = false;
    counter = 0;
    for (auto& cutResult : m_passedEventsPerTrigger) {
      // cppcheck-suppress unreadVariable // the variable is used in the Fill() method below
      numberOfEvents[counter] = static_cast<double>(cutResult.second[SoftwareTriggerCutResult::c_reject]);
      counter++;
    }

    debugTTree->Fill();

    debugOutputFile->cd();
    debugOutputFile->Write();
    debugTTree.reset();
    debugOutputFile.reset();
  }
}

void SoftwareTriggerResultPrinterModule::event()
{
  m_numberOfEvents++;

  for (const auto& triggerResult : m_resultStoreObjectPointer->getResultPairs()) {
    auto cutResults = triggerResult.second;
    m_passedEventsPerTrigger[triggerResult.first][static_cast<SoftwareTriggerCutResult >(cutResults.first)]++;
    m_passedEventsPerTriggerNonPrescaled[triggerResult.first][static_cast<SoftwareTriggerCutResult >(cutResults.second)]++;
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
