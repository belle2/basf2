#pragma once

#include <framework/core/Module.h>

#include <skim/softwaretrigger/dataobjects/SoftwareTriggerResult.h>
#include <skim/softwaretrigger/core/SoftwareTriggerDBHandler.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace SoftwareTrigger {
    class SoftwareTriggerModule : public Module {
    public:
      /// Create a new module instance and set the parameters.
      SoftwareTriggerModule();

      /// Initialize/Require the DB object pointers and any needed store arrays.
      virtual void initialize() override;

      /// Run over all cuts and check them. If one of the cuts yields true, give a positive return value of the module.
      virtual void event() override;

      /// Check if the cut representations in the database have changed and download newer ones if needed.
      virtual void beginRun() override;

    private:
      // Parameters
      /// Base identifier for all cuts downloaded from database.
      std::string m_param_baseIdentifier = "hlt";
      /// List of identifiers for the different cuts.
      std::vector<std::string> m_param_cutIdentifiers;
      /// Store Object Pointer name for storing the trigger decision.
      std::string m_param_resultStoreArrayName = "SoftwareTriggerResults";
      /// Flag to also store the result of the calculations.
      bool m_param_storeDebugOutput = false;
      /// Output file name for the debug output. Is only used if debug is turned on.
      std::string m_param_debugOutputFileName = "software_trigger_debug.root";

      /// Store Object for storing the trigger decision.
      StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer;

      /// Internal handler object for the DB interface.
      SoftwareTriggerDBHandler m_dbHandler;
    };
  }
}