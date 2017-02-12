//+
// File : SoftwareTriggerDQMModule.cc
// Description : Module to monitor physics trigger on HLT
//
// Author : Chunhua LI,
// Date : 13 - Oct - 2016
//-
#include <hlt/softwaretrigger/modules/SoftwareTriggerDQMModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerVariables.h>
#include <map>
#include <string>
#include <iostream>

using namespace std;
using namespace Belle2;
using namespace SoftwareTrigger;


//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SoftwareTriggerDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SoftwareTriggerDQMModule::SoftwareTriggerDQMModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Physics Trigger");
  setPropertyFlags(c_ParallelProcessingCertified);

}



SoftwareTriggerDQMModule::~SoftwareTriggerDQMModule()
{
}

void SoftwareTriggerDQMModule::defineHisto()
{

  h_NTrack = new TH1F("NTracks", "Number of Charged Tracks", 50, 0, 50);
  h_NTrack->SetXTitle("Number of tracks");

  h_NCluster = new TH1F("NCluster", "Number of neutral clusters in ECL", 500, 0, 500);
  h_NCluster->SetXTitle("Number of clusters");

  h_ESum = new TH1F("EnergySum", "Total deposited energy in ECL", 500, 0.0, 50.0);
  h_ESum->SetXTitle("Total deposited energy");

  h_EVis = new TH1F("EnergyVisible", "Total visible energy in detectors", 500, 0.0, 50.0);
  h_EVis->SetXTitle("Total visible energy");

  h_fast_reco_map = new TH1F("FastRecoMap", "Fast_Reco", 50, 0, 50);
  h_fast_reco_map->SetXTitle("Trigger Lines");
  h_fast_reco_map->SetYTitle("Events");

  h_fast_reco_unique_map = new TH1F("FastRecoUniqueMap", "Uniquely triggered by each Fast_Reco line", 50, 0, 50);
  h_fast_reco_unique_map->SetXTitle("Trigger Lines");
  h_fast_reco_unique_map->SetYTitle("Events");

  h_hlt_map = new TH1F("HltTriggerMap", "hlt", 50, 0, 50);
  h_hlt_map->SetXTitle("Trigger Lines");
  h_hlt_map->SetYTitle("Events");

  h_hlt_unique_map = new TH1F("HltUniqueTrigger", "Uniquely triggered by each hlt line", 50, 0, 50);
  h_hlt_unique_map->SetXTitle("Trigger Lines");
  h_hlt_unique_map->SetYTitle("Events");

  h_dqm[0] = new TH1F("D0_mass", "D0->K-#pi+", 100, 1.8, 1.92);
  h_dqm[0]->SetXTitle("M(K #pi)");

  h_dqm[1] = new TH1F("Dplus_mass", "D+->K-#pi+#pi+", 100, 1.8, 1.92);
  h_dqm[1]->SetXTitle("M(K #pi #pi)");

  h_dqm[2] = new TH1F("Dstar_mass", "D*+ -> D0 #pi+, D0->K-#pi+ ", 100, 1.95, 2.05);
  h_dqm[2]->SetXTitle("M(D0 pi+)");

  h_dqm[3] = new TH1F("Jpsiee_mass", "J/#psi->ee", 100, 2.9, 3.2);
  h_dqm[3]->SetXTitle("M(ee)");

  h_dqm[4] = new TH1F("Jpsimumu_mass", "J/#psi->#mu#mu", 100, 2.9, 3.2);
  h_dqm[4]->SetXTitle("M(#mu#mu)");

}


void SoftwareTriggerDQMModule::initialize()
{
  REG_HISTOGRAM
}


void SoftwareTriggerDQMModule::beginRun()
{
  B2INFO("beginRun called.");
}




void SoftwareTriggerDQMModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void SoftwareTriggerDQMModule::terminate()
{
  B2INFO("terminate called");
}



//
// User defined functions
//


void SoftwareTriggerDQMModule::event()
{

  StoreObjPtr<SoftwareTriggerVariables> variables;
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



}
