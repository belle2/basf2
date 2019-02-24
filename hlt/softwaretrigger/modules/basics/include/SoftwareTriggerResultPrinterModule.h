/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <memory>

class TFile;
class TTree;

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Print (and optionally write out) the software trigger results
     * in an easily accessible summary table.
     */
    class SoftwareTriggerResultPrinterModule : public Module {
    public:
      /// Create a new module instance and set the parameters.
      SoftwareTriggerResultPrinterModule();

      /// default virtual constructor because this is a derived class using virtual methods
      virtual ~SoftwareTriggerResultPrinterModule() = default;

      /// Require the needed store object.
      void initialize() override;

      /// Write out the cuts if wanted and sum them up
      void event() override;

      /// Store and delete the ttree if it was created. Print out the summed results
      void terminate() override;

    private:
      /// Output file name for the debug output. Is only used if debug is turned on.
      std::string m_param_debugOutputFileName = "software_trigger_results.root";
      /// Write out all results to disk also
      bool m_param_storeResultsToDisk = false;

      /// Store Object for reading the trigger decision.
      StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer;
      /// TFile to store the debug TTree (or a nullptr if we do not save the debug output).
      std::unique_ptr<TFile> m_debugOutputFile;
      /// TTree to store the debug output (or a nullptr if we do not save the debug output).
      std::unique_ptr<TTree> m_debugTTree;
      /// Internal map of summed results
      std::map<std::string, std::map<SoftwareTriggerCutResult, unsigned int>> m_passedEventsPerTrigger;
      /// Internal counter for the number of seen events
      unsigned int m_numberOfEvents = 0;
    };
  }
}
