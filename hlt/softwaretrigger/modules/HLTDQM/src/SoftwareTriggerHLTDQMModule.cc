//+
// File : SoftwareTriggerHLTDQMModule.cc
// Description : Module to monitor physics trigger on HLT
//
// Author : Chunhua LI, Thomas Hauth
// Date : 13 - Oct - 2016
//-
#include <hlt/softwaretrigger/modules/HLTDQM/SoftwareTriggerHLTDQMModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerVariables.h>
#include <framework/datastore/StoreObjPtr.h>

#include <TDirectory.h>

#include <map>
#include <string>
#include <iostream>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>

using namespace Belle2;
using namespace SoftwareTrigger;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SoftwareTriggerHLTDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SoftwareTriggerHLTDQMModule::SoftwareTriggerHLTDQMModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Physics Trigger");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("triggerVariables", m_param_triggerVariables, "List of trigger variables"
           "that should be written into histograms. The option preScaleStoreDebugOutputToDataStore of the "
           "SoftwareTriggerModule needs to be enabled for this to work.", m_param_triggerVariables);

  addParam("baseIdentifier", m_param_baseIdentifier, "Base identifier for all variables and cut results reported",
           m_param_baseIdentifier);

  addParam("cutIdentifiers", m_param_cutIdentifiers, "List of identifiers for the different cuts to include in the plotting",
           m_param_cutIdentifiers);

  addParam("histogramDirectoryName", m_param_histogramDirectoryName,
           "SoftwareTrigger DQM histograms will be put into this directory", m_param_histogramDirectoryName);

}

void SoftwareTriggerHLTDQMModule::defineHisto()
{
  // move to the specified folder
  TDirectory* oldDir = nullptr;

  if (!m_param_histogramDirectoryName.empty()) {
    oldDir = gDirectory;
    TDirectory* histDir = oldDir->mkdir(m_param_histogramDirectoryName.c_str());
    histDir->cd();
  }

  for (auto const& s : m_param_triggerVariables) {
    // todo: make correct range
    auto pVarName = new TH1F(s.c_str(), s.c_str(), 50, 0, 50);

    pVarName->SetXTitle(("SoftwareTriggerVariable " + s).c_str());
    m_triggerVariablesHistogram[s] = pVarName;
  }

  for (auto const& s : m_param_cutIdentifiers) {
    // can hold the values -1, 0, 1
    auto pCutResultHistogram = new TH1F(s.c_str(), s.c_str(), 3, -1.5 , 1.5);
    pCutResultHistogram->SetXTitle(("Cut Result for " + s).c_str());
    m_cutResultsHistogram[s] = pCutResultHistogram;
  }

  m_totalResultHistogram = new TH1F("Total Cut Result", "Total Cut Result", 3, -1.5 , 1.5);
  m_totalResultHistogram->SetXTitle("Total Cut Result");

  if (oldDir)
    oldDir->cd();
}


void SoftwareTriggerHLTDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void SoftwareTriggerHLTDQMModule::event()
{
  // load and plot the output of the variable computation
  StoreObjPtr<SoftwareTriggerVariables> variables;

  // this might be pre-scaled for performance reasons in the final configuration, therefore this structure
  // might not be filled in every event
  if (variables.isValid()) {
    for (auto& varHistPair : m_triggerVariablesHistogram) {

      const auto fullName = m_param_baseIdentifier + "_" + varHistPair.first;

      // try to load this variable from the computed trigger variables
      if (!variables->has(fullName)) {
        B2FATAL("Variable " << fullName << " configured for SoftwareTriggerDQM plotting is not available");
      } else {
        const auto value = variables->getVariable(fullName);
        varHistPair.second->Fill(value);
      }
    }
  }

  // load and plot the Cut results
  StoreObjPtr<SoftwareTriggerResult> triggerResult;

  if (triggerResult.isValid()) {
    for (auto const& cutIdentifier : m_param_cutIdentifiers) {
      auto fullCutIndenfitier = "software_trigger_cut&" + m_param_baseIdentifier + "&" + cutIdentifier;
      auto cutResult = triggerResult->getResult(fullCutIndenfitier);

      int cutResultInt = 0;
      if (cutResult == SoftwareTriggerCutResult::c_accept) {
        cutResultInt = 1;
      } else if (cutResult == SoftwareTriggerCutResult::c_noResult) {
        cutResultInt = 0;
      } else if (cutResult == SoftwareTriggerCutResult::c_reject) {
        cutResultInt = -1;
      } else {
        B2FATAL("Unsupported SoftwareTriggerCutResult value");
      }

      m_cutResultsHistogram.at(cutIdentifier)->Fill(cutResultInt);
    }

    // todo: support override rejects option
    m_totalResultHistogram->Fill(FinalTriggerDecisionCalculator::getFinalTriggerDecision(*triggerResult));
  }

  /*
   * Legacy code which needs to be replaced by adding the proper
   * triggerVariables to the configuration
  const auto& variableMap = variables->get();
  double numberofTracks = variableMap.at("hlt_nTracksLE");
  double visibleEnergy = variableMap.at("hlt_VisibleEnergyLE");
  double totalEnergy = variableMap.at("hlt_EtotLE");
  h_NTrack->Fill(numberofTracks);
  h_ESum->Fill(totalEnergy);
  h_EVis->Fill(visibleEnergy);


  StoreObjPtr<SoftwareTriggerResult> result;
  //Fill h_hlt_map and h_hlt_unique_map
  std::string hlt_trigger[] = {"hadron", "bhabha", "tau_tau", "2_tracks", "1_track1_cluster", "mu_mu", "gamma_gamma"};
  int numberelement = sizeof(hlt_trigger) / sizeof(hlt_trigger[0]);
  int record_unique[2] = {0, -1};
  for (int i = 1; i <= numberelement; i++) {
    std::string cutname = "software_trigger_cut&hlt&accept_" + hlt_trigger[i - 1];
    int hltres = (int)result->getResult(cutname);
    if (hltres) {
      h_hlt_map->Fill(i);
      record_unique[0]++;
      record_unique[1] = i;
    }
  }
  if (record_unique[0] == 1) h_hlt_unique_map->Fill(record_unique[1]);

  //Fill histgrom of intermediate states
  std::string dqm_part[] = {"D0", "Dplus", "Dstar", "Jpsiee", "Jpsimumu"};
  int dqmsize = sizeof(dqm_part) / sizeof(dqm_part[0]);
  for (int i = 0; i < dqmsize; i++) {
    std::string dqmcutname = "software_trigger_cut&calib&accept_dqm_" + dqm_part[i];
    int dqmres = (int)result->getResult(dqmcutname);
    if (dqmres) {
      std::string varname = "calib_dqm_" + dqm_part[i] + "_M";
      double mass = variableMap.at(varname);
      h_dqm[i]->Fill(mass);
    }
  }
  */
}
