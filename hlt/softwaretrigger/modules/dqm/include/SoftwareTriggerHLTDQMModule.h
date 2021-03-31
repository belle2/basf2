/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li, Thomas Hauth, Nils Braun, Markus Prim        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/HistoModule.h>

#include <TH1F.h>
#include <TH1D.h>

#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerVariables.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Module defining the STM histograms
    class SoftwareTriggerHLTDQMModule final : public HistoModule {
    public:
      //! Constructor / Destructor
      SoftwareTriggerHLTDQMModule();

      //! Module functions to be called from main process
      void initialize() override;

      //! Module functions to be called from event process
      void event() override;

      //! Histogram definition
      void defineHisto() override;

      //! Reset all histogram entries for a new run
      void beginRun() override;

    private:
      // Parameters
      /// Which cuts should be reported? Please remember to include the total_result also, if wanted.
      std::map<std::string, std::map<std::string, std::vector<std::string>>> m_param_cutResultIdentifiers;

      /// Which cuts should be ignored? This can be used to clear trigger lines from e.g. bhabha contamination.
      std::map<std::string, std::vector<std::string>> m_param_cutResultIdentifiersIgnored;

      /// Which cuts should be reported per unit?
      std::vector<std::string> m_param_cutResultIdentifiersPerUnit;

      /// Which L1 cuts should be reported?
      std::vector<std::string> m_param_l1Identifiers;

      /// Create total result histogram?
      bool m_param_create_total_result_histograms;

      /// Create exp/run/event number histograms?
      bool m_param_create_exp_run_event_histograms;

      /// Create HLT unit number histograms?
      bool m_param_create_hlt_unit_histograms;

      /// Create error flag histograms?
      bool m_param_create_error_flag_histograms;

      /// Which variables should be reported?
      std::vector<std::string> m_param_variableIdentifiers;

      /// Directory to put the generated histograms
      std::string m_param_histogramDirectoryName = "softwaretrigger";

      /// Location of the module in the path: before filter or after filter
      std::string m_param_pathLocation = "after filter";

      /// HLT unit number of the machine used
      int m_hlt_unit = 0;

      // Histograms
      /// histograms for the final sw trigger decisions for each base identifier
      std::map<std::string, TH1F*> m_cutResultHistograms;

      /// histograms for the final sw trigger decisions for each base identifier per unit
      std::map<std::string, TH1F*> m_cutResultPerUnitHistograms;

      /// histograms for the software trigger variables in all calculators (although maybe not filled)
      std::map<std::string, TH1F*> m_triggerVariablesHistograms;

      /// histogram with the L1 information
      std::map<std::string, TH1F*> m_l1Histograms;

      /// histograms with the run information
      std::map<std::string, TH1D*> m_runInfoHistograms;

      // Datastore members
      /// STM cut results
      StoreObjPtr<SoftwareTriggerResult> m_triggerResult;

      /// L1 cut results
      StoreObjPtr<TRGSummary> m_l1TriggerResult;

      /// STM cut variables
      StoreObjPtr<SoftwareTriggerVariables> m_variables;

      /// Event Info
      StoreObjPtr<EventMetaData> m_eventMetaData;

      /// Dataobjects
      DBObjPtr<TRGGDLDBFTDLBits> m_l1NameLookup;
    };

    /// HLT unit number information.
    namespace HLTUnit {
      /// Maximum number of HLT units used during the experiment
      static constexpr unsigned int max_hlt_units = 10;

      /// Location of HLT unit number information
      static constexpr char hlt_unit_file[] = "/home/usr/hltdaq/HLT.UnitNumber";
    };
  }
}
