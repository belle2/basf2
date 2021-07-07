/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <string>
#include <memory>

class TFile;
class TTree;

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Write out the software trigger results in an easily accessible summary table.
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
      /// Output file name for the debug output.
      std::string m_param_outputFileName = "software_trigger_results.root";

      /// Store Object for reading the trigger decision.
      StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer;
      /// Store Object for reading the l1 result
      StoreObjPtr<TRGSummary> m_l1Result;
      /// EventMetaData is used by processEvent()/processCore().
      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
      /// TFile to store the debug TTree
      std::unique_ptr<TFile> m_debugOutputFile;
      /// TTree to store the debug output
      std::unique_ptr<TTree> m_debugTTree;
      /// Internal map of summed results
      std::map<std::string, std::map<SoftwareTriggerCutResult, unsigned int>> m_passedEventsPerTrigger;
      /// Internal map of summed results
      std::map<std::string, std::map<SoftwareTriggerCutResult, unsigned int>> m_passedEventsPerTriggerNonPrescaled;
      /// Internal map of prescales
      std::map<std::string, unsigned int> m_prescales;
      /// Internal counter for the number of seen events
      unsigned int m_numberOfEvents = 0;
    };
  }
}
