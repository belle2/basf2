#include <skim/softwaretrigger/modules/SoftwareTriggerModule.h>

using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(SoftwareTrigger)


/// Create a new module instance and set the parameters.
SoftwareTriggerModule::SoftwareTriggerModule() : Module()
{
  setDescription("Module to perform cuts on various variables in the event. The cuts can be defined "
                 "by elements loaded from the database. Each cut is executed, its result stored "
                 "and the return value of this module is set to true of one of the cuts give a "
                 "positive result.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("baseIdentifier", m_param_baseIdentifier, "Base identifier for all cuts downloaded from database. "
           "The full db name of the cuts will be <base_identifier>/<cut_identifier>. You can only choose one identifier "
           "to make clear that all chosen cuts belong together (and should be calculated together).",
           m_param_baseIdentifier);

  addParam("cutIdentifiers", m_param_cutIdentifiers, "List of identifiers for the different cuts. The module will "
           "look for database entries with the form <base_identifier>/<cut_identifier> for each cut_identifier in the "
           "list of strings you provide here. Make sure to choose those wisely as the modules return value depends "
           "on these cuts.");

  addParam("resultStoreArrayName", m_param_resultStoreArrayName, "Store Object Pointer name for storing the "
           "trigger decision.", m_param_resultStoreArrayName);

  addParam("storeDebugOutput", m_param_storeDebugOutput, "Flag to save the results of the calculations leading "
           "to the the trigger decisions into a ROOT file. The file path and name of this file can be handled by the "
           "debugOutputFileName parameter.", m_param_storeDebugOutput);

  addParam("debugOutputFileName", m_param_debugOutputFileName, "File path and name of the ROOT "
           "file, in which the results of the calculation are stored, if storeDebugOutput is "
           "turned on. Please note that already present files will be overridden. "
           "ATTENTION: This debugging mode does not work in parallel processing.", m_param_debugOutputFileName);
}

/// Initialize/Require the DB object pointers and any needed store arrays.
void SoftwareTriggerModule::initialize()
{
  m_resultStoreObjectPointer.registerInDataStore(m_param_resultStoreArrayName);

  m_dbHandler.initialize(m_param_baseIdentifier, m_param_cutIdentifiers);
}

/// Run over all cuts and check them. If one of the cuts yields true, give a positive return value of the module.
void SoftwareTriggerModule::event()
{
  m_resultStoreObjectPointer.create();

  // TODO: Do the calculation!
  SoftwareTriggerObject prefilledObject;

  for (const auto& cutWithName : m_dbHandler.getCutsWithNames()) {
    const std::string& cutIdentifier = cutWithName.first;
    const auto& cut = cutWithName.second;
    const bool cutResult = cut->checkPreScaled(prefilledObject);
    m_resultStoreObjectPointer->addResult(cutIdentifier, cutResult);
  }

  setReturnValue(m_resultStoreObjectPointer->getTotalResult());
}

/// Check if the cut representations in the database have changed and download newer ones if needed.
void SoftwareTriggerModule::beginRun()
{
  m_dbHandler.checkForChangedDBEntries();
}