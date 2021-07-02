/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/softwaretrigger/modules/basics/SoftwareTriggerModule.h>
#include <hlt/softwaretrigger/core/utilities.h>
#include <hlt/softwaretrigger/calculations/FilterCalculator.h>
#include <hlt/softwaretrigger/calculations/SkimSampleCalculator.h>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <TFile.h>

using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(SoftwareTrigger)


/// Create a new module instance and set the parameters.
SoftwareTriggerModule::SoftwareTriggerModule() : Module(), m_resultStoreObjectPointer("", DataStore::c_Event)
{
  setDescription("Module to perform cuts on various variables in the event. The cuts can be defined\n"
                 "by elements loaded from the database. Each cut is executed and its result stored.\n"
                 "The return value of this module is a bool, which is either true (accept the event) or false (reject it).\n"
                 "It is defined from the results of the cuts in the given trigger menu, which are all evaluated\n"
                 "and the trigger mode (accept mode or not).\n"
                 "if not in accept mode (= reject mode):\n"
                 "* 1: if one of the accept cuts has a true result and none of the reject cuts is false ( = accepted)\n"
                 "* 0: if neither one of the accept cuts is true nor one of the reject cuts false ( = don't know) or\n"
                 "*    if one of the reject cuts is false ( = rejected)\n"
                 "In short: event accepted <=> (#true accept cuts > 0) && (#false reject cuts == 0)\n"
                 "Please note that the reject cuts override the accept cuts decision in this case!\n"
                 "if in accept mode\n"
                 "* 1: if one of the accept cuts has a true result. ( = accepted) or\n"
                 "*    if neither one of the accept cuts is true nor one of the reject cuts false ( = don't know)\n"
                 "* 0: if one of the reject cuts is false and none of the accept cuts is true ( = rejected)\n"
                 "Please note that the accept cuts override the reject cuts decision in this case!\n"
                 "In short: event accepted <=> (#true accept cuts > 0) || (#false reject cuts == 0)");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("baseIdentifier", m_param_baseIdentifier, "Base identifier for all cuts downloaded from database. "
           "The full db name of the cuts will be <base_identifier>/<cut_identifier>. You can only choose one identifier "
           "to make clear that all chosen cuts belong together (and should be calculated together).",
           m_param_baseIdentifier);

  addParam("resultStoreArrayName", m_param_resultStoreArrayName, "Store Object Pointer name for storing the "
           "trigger decision.", m_param_resultStoreArrayName);

  addParam("storeDebugOutputToROOTFile", m_param_storeDebugOutputToROOTFile, "Flag to save the results of the calculations leading "
           "to the trigger decisions into a ROOT file. The file path and name of this file can be handled by the "
           "debugOutputFileName parameter. Not supported during parallel processing.", m_param_storeDebugOutputToROOTFile);

  addParam("preScaleStoreDebugOutputToDataStore", m_param_preScaleStoreDebugOutputToDataStore,
           "Prescale with which to save the results of the calculations leading "
           "to the trigger decisions into the DataStore. Leave to zero, to not store them at all.",
           m_param_preScaleStoreDebugOutputToDataStore);

  addParam("debugOutputFileName", m_param_debugOutputFileName, "File path and name of the ROOT "
           "file, in which the results of the calculation are stored, if storeDebugOutput is "
           "turned on. Please note that already present files will be overridden. "
           "ATTENTION: This file debugging mode does not work in parallel processing.", m_param_debugOutputFileName);
  addParam("useRandomNumbersForPreScale", m_param_useRandomNumbersForPreScale, "Flag to use random numbers (True) "
           "or a counter (False) for applying the prescale.", m_param_useRandomNumbersForPreScale);
}

void SoftwareTriggerModule::initialize()
{
  m_resultStoreObjectPointer.registerInDataStore(m_param_resultStoreArrayName);
  m_dbHandler.reset(new SoftwareTriggerDBHandler(m_param_baseIdentifier));

  initializeCalculation();
  initializeDebugOutput();
}

void SoftwareTriggerModule::beginRun()
{
  m_dbHandler->checkForChangedDBEntries();
  // Initialize always the internal counters at the beginning of each run.
  if (not m_param_useRandomNumbersForPreScale) {
    initializeCounters();
  }
}

void SoftwareTriggerModule::terminate()
{
  if (m_debugTTree) {
    m_debugOutputFile->cd();
    m_debugOutputFile->Write();
    m_debugTTree.reset();
    m_debugOutputFile.reset();
  }
}

/// Run over all cuts and check them. If one of the cuts yields true, give a positive return value of the module.
void SoftwareTriggerModule::event()
{
  if (not m_resultStoreObjectPointer.isValid()) {
    m_resultStoreObjectPointer.construct();
  }

  if (m_param_preScaleStoreDebugOutputToDataStore > 0 and not m_debugOutputStoreObject.isValid()) {
    m_debugOutputStoreObject.construct();
  }

  B2DEBUG(20, "Doing the calculation...");
  const SoftwareTriggerObject& prefilledObject = m_calculation->fillInCalculations();
  B2DEBUG(20, "Successfully finished the calculation.");

  makeCut(prefilledObject);
  makeDebugOutput();
}

void SoftwareTriggerModule::initializeCalculation()
{
  if (m_param_baseIdentifier == "filter") {
    m_calculation.reset(new FilterCalculator());
  } else if (m_param_baseIdentifier == "skim") {
    m_calculation.reset(new SkimSampleCalculator());
  } else {
    B2FATAL("You gave an invalid base identifier " << m_param_baseIdentifier << ".");
  }

  m_calculation->requireStoreArrays();
}

void SoftwareTriggerModule::initializeDebugOutput()
{
  if (m_param_storeDebugOutputToROOTFile) {
    m_debugOutputFile.reset(TFile::Open(m_param_debugOutputFileName.c_str(), "RECREATE"));
    if (not m_debugOutputFile) {
      B2FATAL("Could not open debug output file. Aborting.");
    }
    m_debugTTree.reset(new TTree("software_trigger_results", "software_trigger_results"));
    if (not m_debugTTree) {
      B2FATAL("Could not create debug output tree. Aborting.");
    }
  }

  if (m_param_preScaleStoreDebugOutputToDataStore > 0) {
    m_debugOutputStoreObject.registerInDataStore(m_param_debugOutputStoreObjName);
  }
}

void SoftwareTriggerModule::initializeCounters()
{
  // Clear the vector of counters...
  m_counters.clear();
  // ... and then initialize it with "numberOfCuts" 0s.
  size_t numberOfCuts = (m_dbHandler->getCutsWithNames()).size();
  m_counters.assign(numberOfCuts, 0);
}

void SoftwareTriggerModule::makeCut(const SoftwareTriggerObject& prefilledObject)
{
  // Define the pointer to the counter and the index of each counter in m_counters.
  uint32_t* counter{nullptr};
  size_t counterIndex{0};
  // Check all cuts with the prefilled object and write them back into the data store.
  for (const auto& cutWithName : m_dbHandler->getCutsWithNames()) {
    const std::string& cutIdentifier = cutWithName.first;
    const auto& cut = cutWithName.second;
    B2DEBUG(20, "Next processing cut " << cutIdentifier << " (" << cut->decompile() << ")");
    if (not m_param_useRandomNumbersForPreScale) {
      counter = &m_counters.at(counterIndex++);
    }
    const auto& [prescaledCutResult, nonPrescaledCutResult] = cut->check(prefilledObject, counter);
    m_resultStoreObjectPointer->addResult(cutIdentifier, prescaledCutResult, nonPrescaledCutResult);
  }

  // Also add the module result ( = the result of all cuts with this basename) for later reference.
  const SoftwareTriggerCutResult& moduleResult =
    FinalTriggerDecisionCalculator::getModuleResult(*m_resultStoreObjectPointer, m_param_baseIdentifier,
                                                    m_dbHandler->getAcceptOverridesReject());
  const std::string& moduleResultIdentifier = SoftwareTriggerDBHandler::makeTotalResultName(m_param_baseIdentifier);
  m_resultStoreObjectPointer->addResult(moduleResultIdentifier, moduleResult);

  // Return the trigger decision up to here
  bool totalResult = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer, true);
  const std::string& totalResultIdentifier = SoftwareTriggerDBHandler::makeTotalResultName();
  if (totalResult) {
    m_resultStoreObjectPointer->addResult(totalResultIdentifier, SoftwareTriggerCutResult::c_accept);
  } else {
    m_resultStoreObjectPointer->addResult(totalResultIdentifier, SoftwareTriggerCutResult::c_reject);
  }
  setReturnValue(totalResult);
}

void SoftwareTriggerModule::makeDebugOutput()
{
  if (m_param_storeDebugOutputToROOTFile) {
    B2DEBUG(20, "Storing debug output to file as requested.");
    m_calculation->writeDebugOutput(m_debugTTree);
    B2DEBUG(20, "Finished storing the debug output to file.");
  }

  if (makePreScale(m_param_preScaleStoreDebugOutputToDataStore)) {
    B2DEBUG(20, "Storing debug output to DataStore as requested.");
    m_calculation->addDebugOutput(m_debugOutputStoreObject, m_param_baseIdentifier);
  }
}
