/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li, Thomas Hauth, Nils Braun                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/HistoModule.h>

#include <TH1F.h>

#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerVariables.h>

#include <framework/datastore/StoreObjPtr.h>

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
      std::map<std::string, std::vector<std::string>> m_param_cutResultIdentifiers;

      /// Which variables should be reported?
      std::vector<std::string> m_param_variableIdentifiers;

      /// Directory to put the generated histograms
      std::string m_param_histogramDirectoryName = "softwaretrigger";

      // Histograms
      /// histograms for the final sw trigger decisions for each base identifier
      std::map<std::string, TH1F*> m_cutResultHistograms;

      /// histograms for the software trigger variables in all calculators (although maybe not filled)
      std::map<std::string, TH1F*> m_triggerVariablesHistograms;

      // Datastore members
      /// STM cut results
      StoreObjPtr<SoftwareTriggerResult> m_triggerResult;
      /// STM cut variables
      StoreObjPtr<SoftwareTriggerVariables> m_variables;
    };
  }
}
