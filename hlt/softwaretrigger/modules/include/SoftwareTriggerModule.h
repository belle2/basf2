#pragma once

#include <framework/core/Module.h>

#include <hlt/softwaretrigger/calculations/SoftwareTriggerCalculation.h>
#include <hlt/softwaretrigger/calculations/FastRecoCalculator.h>
#include <hlt/softwaretrigger/calculations/HLTCalculator.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Module to perform cuts on various variables in the event. The cuts can be defined
     * by elements loaded from the database. Each cut is executed and its result stored.
     *
     * The return value of this module is an integer, which is:
     * if reject is more important than accept:
     * * 1: if one of the accept cuts has a true result and none of the reject cuts is false ( = accepted)
     * * 0: if neither one of the accept cuts is true nor one of the reject cuts false ( = don't know)
     * * -1: if one of the reject cuts is false ( = rejected)
     *
     * Please note that the reject cuts override the accept cuts decision in this case!
     *
     * if accept is more important than reject:
     * * 1: if one of the accept cuts has a true result. ( = accepted)
     * * 0: if neither one of the accept cuts is true nor one of the reject cuts false ( = don't know)
     * * -1: if one of the reject cuts is false and none of the accept cuts is true ( = rejected)
     *
     * Please note that the accept cuts override the reject cuts decision in this case!
     *
     * What is more important can be controlled by the flag acceptOverridesReject, which is off by default (so reject is
     * more important than accept by default).
     */
    class SoftwareTriggerModule : public Module {
    public:
      /// Create a new module instance and set the parameters.
      SoftwareTriggerModule();

      /// Initialize/Require the DB object pointers and any needed store arrays.
      void initialize() override;

      /// Run over all cuts and check them. If one of the cuts yields true, give a positive return value of the module.
      void event() override;

      /// Check if the cut representations in the database have changed and download newer ones if needed.
      void beginRun() override;

      /// Store and delete the ttree if it was created.
      void terminate() override;

    private:
      // Parameters
      /// Base identifier for all cuts downloaded from database.
      std::string m_param_baseIdentifier = "hlt";
      /// List of identifiers for the different cuts.
      std::vector<std::string> m_param_cutIdentifiers;
      /// Store Object Pointer name for storing the trigger decision.
      std::string m_param_resultStoreArrayName = "SoftwareTriggerResults";
      /// Flag to control which class of cuts is more "important": accept cuts or reject cuts.
      bool m_param_acceptOverridesReject = false;
      /// Flag to also store the result of the calculations into a root file.
      bool m_param_storeDebugOutputToROOTFile = false;
      /// Prescale with which to save the results of the calculations into the DataStore.
      unsigned int m_param_preScaleStoreDebugOutputToDataStore = 0;
      /// Output file name for the debug output. Is only used if debug is turned on.
      std::string m_param_debugOutputFileName = "software_trigger_debug.root";
      /// Output store object name for the debug output. Is only used if debug is turned on.
      std::string m_param_debugOutputStoreObjName = "";

      // Object pools
      /// Store Object for storing the trigger decision.
      StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer;
      /// Internal handler object for the DB interface.
      SoftwareTriggerDBHandler m_dbHandler;
      /// Internal handler for the Calculations (will be set in initialize to the correct one).
      std::unique_ptr<SoftwareTriggerCalculation> m_calculation;
      /// TFile to store the debug TTree (or a nullptr if we do not save the debug output).
      std::unique_ptr<TFile> m_debugOutputFile;
      /// TTree to store the debug output (or a nullptr if we do not save the debug output).
      std::unique_ptr<TTree> m_debugTTree;
      /// TTree living in the datastore for debug reasons
      StoreObjPtr<SoftwareTriggerVariables> m_debugOutputStoreObject;

      /// Helper function to initialize the calculation by creating a new calculation object and
      /// requiring all store arrays.
      void initializeCalculation();

      /// Helper function to initliaze debug output creation by creating a TTree
      /// and an object in the data store if needed.
      void initializeDebugOutput();

      /// Helper function to perform the actual cut on the prefilled object and set the return value of the module.
      void makeCut(const SoftwareTriggerObject& prefilledObject);

      /// Helper function to store the calculated variables from the calculation either in the TTree or in the data store.
      void makeDebugOutput();
    };
  }
}