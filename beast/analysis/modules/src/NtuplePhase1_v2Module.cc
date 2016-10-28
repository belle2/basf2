/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <beast/analysis/modules/NtuplePhase1_v2Module.h>
#include <beast/analysis/modules/BEASTTree_v2.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework core


// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/core/RandomNumbers.h>

// DataStore classes
#include <framework/io/RootIOUtilities.h>
#include <framework/dataobjects/EventMetaData.h>


#include <TTimeStamp.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  using namespace RootIOUtilities;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(NtuplePhase1_v2)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  NtuplePhase1_v2Module::NtuplePhase1_v2Module() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Read SKB PVs, simulated measurements of BEAST sensors, and write scaled simulated Ntuple in BEAST phase 1 data format");


    // Add parameters
    addParam("inputFileNames", m_inputFileNames,
             "List of files with SKB PVs ");

    addParam("outputFileName", m_outputFileName, "Output file name");

    //addParam("input_ts", m_input_ts, "Input time stamp start and stop");

    addParam("input_I_HER", m_input_I_HER, "HER current");
    addParam("input_I_LER", m_input_I_LER, "LER current");

    addParam("input_P_HER", m_input_P_HER, "HER pressure");
    addParam("input_P_LER", m_input_P_LER, "LER pressure");

    addParam("input_sigma_HER", m_input_sigma_HER, "HER beam size");
    addParam("input_sigma_LER", m_input_sigma_LER, "LER beam size");

    addParam("input_bunchNb_HER", m_input_bunchNb_HER, "HER bunch number");
    addParam("input_bunchNb_LER", m_input_bunchNb_LER, "LER bunch number");

    addParam("input_data_bunchNb_HER", m_input_data_bunchNb_HER, "HER bunch number");
    addParam("input_data_bunchNb_LER", m_input_data_bunchNb_LER, "LER bunch number");

    addParam("input_LT_DIA_dose", m_input_LT_DIA_dose, "List of LT DIA dose ");
    addParam("input_HT_DIA_dose", m_input_HT_DIA_dose, "List of HT DIA dose ");
    addParam("input_LB_DIA_dose", m_input_LB_DIA_dose, "List of LB DIA dose ");
    addParam("input_HB_DIA_dose", m_input_HB_DIA_dose, "List of HB DIA dose ");
    addParam("input_LC_DIA_dose", m_input_LC_DIA_dose, "List of LC DIA dose ");
    addParam("input_HC_DIA_dose", m_input_HC_DIA_dose, "List of HC DIA dose ");

    addParam("input_LT_PIN_dose", m_input_LT_PIN_dose, "List of LT PIN dose ");
    addParam("input_HT_PIN_dose", m_input_HT_PIN_dose, "List of HT PIN dose ");
    addParam("input_LB_PIN_dose", m_input_LB_PIN_dose, "List of LB PIN dose ");
    addParam("input_HB_PIN_dose", m_input_HB_PIN_dose, "List of HB PIN dose ");
    addParam("input_LC_PIN_dose", m_input_LC_PIN_dose, "List of LC PIN dose ");
    addParam("input_HC_PIN_dose", m_input_HC_PIN_dose, "List of HC PIN dose ");

    addParam("input_LT_BGO_dose", m_input_LT_BGO_dose, "List of LT BGO dose ");
    addParam("input_HT_BGO_dose", m_input_HT_BGO_dose, "List of HT BGO dose ");
    addParam("input_LB_BGO_dose", m_input_LB_BGO_dose, "List of LB BGO dose ");
    addParam("input_HB_BGO_dose", m_input_HB_BGO_dose, "List of HB BGO dose ");
    addParam("input_LC_BGO_dose", m_input_LC_BGO_dose, "List of LC BGO dose ");
    addParam("input_HC_BGO_dose", m_input_HC_BGO_dose, "List of HC BGO dose ");

    addParam("input_LT_HE3_rate", m_input_LT_HE3_rate, "List of LT HE3 rate ");
    addParam("input_HT_HE3_rate", m_input_HT_HE3_rate, "List of HT HE3 rate ");
    addParam("input_LB_HE3_rate", m_input_LB_HE3_rate, "List of LB HE3 rate ");
    addParam("input_HB_HE3_rate", m_input_HB_HE3_rate, "List of HB HE3 rate ");
    addParam("input_LC_HE3_rate", m_input_LC_HE3_rate, "List of LC HE3 rate ");
    addParam("input_HC_HE3_rate", m_input_HC_HE3_rate, "List of HC HE3 rate ");

    addParam("input_LT_CSI_rate", m_input_LT_CSI_rate, "List of LT CSI rate ");
    addParam("input_HT_CSI_rate", m_input_HT_CSI_rate, "List of HT CSI rate ");
    addParam("input_LB_CSI_rate", m_input_LB_CSI_rate, "List of LB CSI rate ");
    addParam("input_HB_CSI_rate", m_input_HB_CSI_rate, "List of HB CSI rate ");
    addParam("input_LC_CSI_rate", m_input_LC_CSI_rate, "List of LC CSI rate ");
    addParam("input_HC_CSI_rate", m_input_HC_CSI_rate, "List of HC CSI rate ");

    addParam("input_LT_CSI_dose", m_input_LT_CSI_dose, "List of LT CSI dose ");
    addParam("input_HT_CSI_dose", m_input_HT_CSI_dose, "List of HT CSI dose ");
    addParam("input_LB_CSI_dose", m_input_LB_CSI_dose, "List of LB CSI dose ");
    addParam("input_HB_CSI_dose", m_input_HB_CSI_dose, "List of HB CSI dose ");
    addParam("input_LC_CSI_dose", m_input_LC_CSI_dose, "List of LC CSI dose ");
    addParam("input_HC_CSI_dose", m_input_HC_CSI_dose, "List of HC CSI dose ");

    // initialize other private data members
    m_file = NULL;
    m_tree = NULL;
    m_treeBEAST = NULL;
    m_beast.clear();

    m_numEntries = 0;
    m_entryCounter = 0;

  }

  NtuplePhase1_v2Module::~NtuplePhase1_v2Module()
  {
  }

  void NtuplePhase1_v2Module::initialize()
  {
    loadDictionaries();

    // expand possible wildcards
    m_inputFileNames = expandWordExpansions(m_inputFileNames);
    if (m_inputFileNames.empty()) {
      B2FATAL("No valid files specified!");
    }

    // check files
    TDirectory* dir = gDirectory;
    for (const string& fileName : m_inputFileNames) {
      TFile* f = TFile::Open(fileName.c_str(), "READ");
      if (!f or !f->IsOpen()) {
        B2FATAL("Couldn't open input file " + fileName);
      }
      delete f;
    }
    dir->cd();

    // get event TTree
    //m_tree = new TChain(c_treeNames[DataStore::c_Event].c_str());
    m_tree = new TChain("tout");
    for (const string& fileName : m_inputFileNames) {
      m_tree->AddFile(fileName.c_str());
    }
    m_numEvents = m_tree->GetEntries();
    if (m_numEvents == 0) B2ERROR(c_treeNames[DataStore::c_Event] << " has no entires");
    m_eventCount = 0;

    m_tree->SetBranchAddress("ts", &(m_beast.ts));
    m_tree->SetBranchAddress("event", &(m_beast.event));
    m_tree->SetBranchAddress("run", &(m_beast.run));
    m_tree->SetBranchAddress("subrun", &(m_beast.subrun));
    m_tree->SetBranchAddress("SKB_HER_injectionFlag", &(m_beast.SKB_HER_injectionFlag));
    m_tree->SetBranchAddress("SKB_LER_injectionFlag", &(m_beast.SKB_LER_injectionFlag));
    m_tree->SetBranchAddress("SKB_Status", &(m_beast.SKB_Status));
    m_tree->SetBranchAddress("SKB_HER_injectionRate", &(m_beast.SKB_HER_injectionRate));
    m_tree->SetBranchAddress("SKB_LER_injectionRate", &(m_beast.SKB_LER_injectionRate));
    m_tree->SetBranchAddress("SKB_HER_lifetime", &(m_beast.SKB_HER_lifetime));
    m_tree->SetBranchAddress("SKB_LER_lifetime", &(m_beast.SKB_LER_lifetime));
    m_tree->SetBranchAddress("SKB_LER_current", &(m_beast.SKB_LER_current));
    m_tree->SetBranchAddress("SKB_HER_current", &(m_beast.SKB_HER_current));
    m_tree->SetBranchAddress("SKB_LER_injectionEfficiency", &(m_beast.SKB_LER_injectionEfficiency));
    m_tree->SetBranchAddress("SKB_HER_injectionEfficiency", &(m_beast.SKB_HER_injectionEfficiency));
    m_tree->SetBranchAddress("SKB_beamLoss_ionChambers_mean", &(m_beast.SKB_beamLoss_ionChambers_mean));
    m_tree->SetBranchAddress("SKB_beamLoss_PINdiodes_mean", &(m_beast.SKB_beamLoss_PINdiodes_mean));
    m_tree->SetBranchAddress("SKB_beamLoss_nearCollimators", &(m_beast.SKB_beamLoss_nearCollimators));
    m_tree->SetBranchAddress("SKB_beamLoss_aroundMasks", &(m_beast.SKB_beamLoss_aroundMasks));
    m_tree->SetBranchAddress("SKB_LER_injectionCharge", &(m_beast.SKB_LER_injectionCharge));
    m_tree->SetBranchAddress("SKB_HER_injectionCharge", &(m_beast.SKB_HER_injectionCharge));
    m_tree->SetBranchAddress("SKB_LER_injectionRepetitionRate", &(m_beast.SKB_LER_injectionRepetitionRate));
    m_tree->SetBranchAddress("SKB_HER_injectionRepetitionRate", &(m_beast.SKB_HER_injectionRepetitionRate));
    m_tree->SetBranchAddress("SKB_LER_injectionNumberOfBunches", &(m_beast.SKB_LER_injectionNumberOfBunches));
    m_tree->SetBranchAddress("SKB_HER_injectionNumberOfBunches", &(m_beast.SKB_HER_injectionNumberOfBunches));
    m_tree->SetBranchAddress("SKB_LER_pressures", &(m_beast.SKB_LER_pressures));
    m_tree->SetBranchAddress("SKB_HER_pressures", &(m_beast.SKB_HER_pressures));
    m_tree->SetBranchAddress("SKB_LER_pressure_average", &(m_beast.SKB_LER_pressure_average));
    m_tree->SetBranchAddress("SKB_HER_pressure_average", &(m_beast.SKB_HER_pressure_average));
    m_tree->SetBranchAddress("SKB_HER_collimatorPositions_mm", &(m_beast.SKB_HER_collimatorPositions_mm));
    m_tree->SetBranchAddress("SKB_HER_collimatorPositions_DMM", &(m_beast.SKB_HER_collimatorPositions_DMM));
    m_tree->SetBranchAddress("SKB_HER_collimatorPositions_inX", &(m_beast.SKB_HER_collimatorPositions_inX));
    m_tree->SetBranchAddress("SKB_HER_collimatorPositions_inY", &(m_beast.SKB_HER_collimatorPositions_inY));
    m_tree->SetBranchAddress("SKB_HER_collimatorPositions_fromBeam", &(m_beast.SKB_HER_collimatorPositions_fromBeam));
    m_tree->SetBranchAddress("SKB_LER_collimatorPositions_mm", &(m_beast.SKB_LER_collimatorPositions_mm));
    m_tree->SetBranchAddress("SKB_LER_collimatorPositions_X", &(m_beast.SKB_LER_collimatorPositions_X));
    m_tree->SetBranchAddress("SKB_LER_collimatorPositions_Y", &(m_beast.SKB_LER_collimatorPositions_Y));
    m_tree->SetBranchAddress("SKB_LER_collimatorPositions_fromBeam", &(m_beast.SKB_LER_collimatorPositions_fromBeam));
    m_tree->SetBranchAddress("SKB_HER_beamSize_xray_X", &(m_beast.SKB_HER_beamSize_xray_X));
    m_tree->SetBranchAddress("SKB_HER_beamSize_xray_Y", &(m_beast.SKB_HER_beamSize_xray_Y));
    m_tree->SetBranchAddress("SKB_LER_beamSize_xray_X", &(m_beast.SKB_LER_beamSize_xray_X));
    m_tree->SetBranchAddress("SKB_LER_beamSize_xray_Y", &(m_beast.SKB_LER_beamSize_xray_Y));
    m_tree->SetBranchAddress("SKB_LER_beamSize_SR_X", &(m_beast.SKB_LER_beamSize_SR_X));
    m_tree->SetBranchAddress("SKB_LER_beamSize_SR_Y", &(m_beast.SKB_LER_beamSize_SR_Y));
    m_tree->SetBranchAddress("SKB_HER_beamSize_SR_X", &(m_beast.SKB_HER_beamSize_SR_X));
    m_tree->SetBranchAddress("SKB_HER_beamSize_SR_Y", &(m_beast.SKB_HER_beamSize_SR_Y));
    m_tree->SetBranchAddress("SKB_HER_integratedCurrent", &(m_beast.SKB_HER_integratedCurrent));
    m_tree->SetBranchAddress("SKB_LER_integratedCurrent", &(m_beast.SKB_LER_integratedCurrent));
    m_tree->SetBranchAddress("SKB_LER_partialPressures_D06", &(m_beast.SKB_LER_partialPressures_D06));
    m_tree->SetBranchAddress("SKB_LER_partialPressures_D02", &(m_beast.SKB_LER_partialPressures_D02));
    m_tree->SetBranchAddress("SKB_LER_pressures_local", &(m_beast.SKB_LER_pressures_local));

    m_numEntries = m_tree->GetEntries();
    cout << "m_numEntries " << m_numEntries << endl;
    m_entryCounter = 0;
    m_exp = 0;
    // data store objects registration

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.registerInDataStore();

    m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
    m_treeBEAST = new TTree("tout", "BEAST data tree (simulation)");
    m_treeBEAST->Branch("ts", &(m_beast.ts));

    m_treeBEAST->Branch("SKB_HER_injectionFlag", &(m_beast.SKB_HER_injectionFlag));
    m_treeBEAST->Branch("SKB_LER_injectionFlag", &(m_beast.SKB_LER_injectionFlag));
    m_treeBEAST->Branch("SKB_Status", &(m_beast.SKB_Status));
    m_treeBEAST->Branch("SKB_HER_injectionRate", &(m_beast.SKB_HER_injectionRate));
    m_treeBEAST->Branch("SKB_LER_injectionRate", &(m_beast.SKB_LER_injectionRate));
    m_treeBEAST->Branch("SKB_HER_lifetime", &(m_beast.SKB_HER_lifetime));
    m_treeBEAST->Branch("SKB_LER_lifetime", &(m_beast.SKB_LER_lifetime));
    m_treeBEAST->Branch("SKB_LER_current", &(m_beast.SKB_LER_current));
    m_treeBEAST->Branch("SKB_HER_current", &(m_beast.SKB_HER_current));
    m_treeBEAST->Branch("SKB_LER_injectionEfficiency", &(m_beast.SKB_LER_injectionEfficiency));
    m_treeBEAST->Branch("SKB_HER_injectionEfficiency", &(m_beast.SKB_HER_injectionEfficiency));
    m_treeBEAST->Branch("SKB_beamLoss_ionChambers_mean", &(m_beast.SKB_beamLoss_ionChambers_mean));
    m_treeBEAST->Branch("SKB_beamLoss_PINdiodes_mean", &(m_beast.SKB_beamLoss_PINdiodes_mean));
    m_treeBEAST->Branch("SKB_beamLoss_nearCollimators", &(m_beast.SKB_beamLoss_nearCollimators));
    m_treeBEAST->Branch("SKB_beamLoss_aroundMasks", &(m_beast.SKB_beamLoss_aroundMasks));
    m_treeBEAST->Branch("SKB_LER_injectionCharge", &(m_beast.SKB_LER_injectionCharge));
    m_treeBEAST->Branch("SKB_HER_injectionCharge", &(m_beast.SKB_HER_injectionCharge));
    m_treeBEAST->Branch("SKB_LER_injectionRepetitionRate", &(m_beast.SKB_LER_injectionRepetitionRate));
    m_treeBEAST->Branch("SKB_HER_injectionRepetitionRate", &(m_beast.SKB_HER_injectionRepetitionRate));
    m_treeBEAST->Branch("SKB_LER_injectionNumberOfBunches", &(m_beast.SKB_LER_injectionNumberOfBunches));
    m_treeBEAST->Branch("SKB_HER_injectionNumberOfBunches", &(m_beast.SKB_HER_injectionNumberOfBunches));
    m_treeBEAST->Branch("SKB_LER_pressures", &(m_beast.SKB_LER_pressures));
    m_treeBEAST->Branch("SKB_HER_pressures", &(m_beast.SKB_HER_pressures));
    m_treeBEAST->Branch("SKB_LER_pressure_average", &(m_beast.SKB_LER_pressure_average));
    m_treeBEAST->Branch("SKB_HER_pressure_average", &(m_beast.SKB_HER_pressure_average));
    m_treeBEAST->Branch("SKB_HER_collimatorPositions_mm", &(m_beast.SKB_HER_collimatorPositions_mm));
    m_treeBEAST->Branch("SKB_HER_collimatorPositions_DMM", &(m_beast.SKB_HER_collimatorPositions_DMM));
    m_treeBEAST->Branch("SKB_HER_collimatorPositions_inX", &(m_beast.SKB_HER_collimatorPositions_inX));
    m_treeBEAST->Branch("SKB_HER_collimatorPositions_inY", &(m_beast.SKB_HER_collimatorPositions_inY));
    m_treeBEAST->Branch("SKB_HER_collimatorPositions_fromBeam", &(m_beast.SKB_HER_collimatorPositions_fromBeam));
    m_treeBEAST->Branch("SKB_LER_collimatorPositions_mm", &(m_beast.SKB_LER_collimatorPositions_mm));
    m_treeBEAST->Branch("SKB_LER_collimatorPositions_X", &(m_beast.SKB_LER_collimatorPositions_X));
    m_treeBEAST->Branch("SKB_LER_collimatorPositions_Y", &(m_beast.SKB_LER_collimatorPositions_Y));
    m_treeBEAST->Branch("SKB_LER_collimatorPositions_fromBeam", &(m_beast.SKB_LER_collimatorPositions_fromBeam));
    m_treeBEAST->Branch("SKB_HER_beamSize_xray_X", &(m_beast.SKB_HER_beamSize_xray_X));
    m_treeBEAST->Branch("SKB_HER_beamSize_xray_Y", &(m_beast.SKB_HER_beamSize_xray_Y));
    m_treeBEAST->Branch("SKB_LER_beamSize_xray_X", &(m_beast.SKB_LER_beamSize_xray_X));
    m_treeBEAST->Branch("SKB_LER_beamSize_xray_Y", &(m_beast.SKB_LER_beamSize_xray_Y));
    m_treeBEAST->Branch("SKB_LER_beamSize_SR_X", &(m_beast.SKB_LER_beamSize_SR_X));
    m_treeBEAST->Branch("SKB_LER_beamSize_SR_Y", &(m_beast.SKB_LER_beamSize_SR_Y));
    m_treeBEAST->Branch("SKB_HER_beamSize_SR_X", &(m_beast.SKB_HER_beamSize_SR_X));
    m_treeBEAST->Branch("SKB_HER_beamSize_SR_Y", &(m_beast.SKB_HER_beamSize_SR_Y));
    m_treeBEAST->Branch("SKB_HER_integratedCurrent", &(m_beast.SKB_HER_integratedCurrent));
    m_treeBEAST->Branch("SKB_LER_integratedCurrent", &(m_beast.SKB_LER_integratedCurrent));
    m_treeBEAST->Branch("SKB_LER_partialPressures_D06", &(m_beast.SKB_LER_partialPressures_D06));
    m_treeBEAST->Branch("SKB_LER_partialPressures_D02", &(m_beast.SKB_LER_partialPressures_D02));
    m_treeBEAST->Branch("SKB_LER_pressures_local", &(m_beast.SKB_LER_pressures_local));
    m_treeBEAST->Branch("PIN_dose", &(m_beast.PIN_dose));
    m_treeBEAST->Branch("BGO_energy", &(m_beast.BGO_energy));
    m_treeBEAST->Branch("HE3_rate", &(m_beast.HE3_rate));
    m_treeBEAST->Branch("CSI_sumE", &(m_beast.CSI_sumE));
    m_treeBEAST->Branch("CSI_hitRate", &(m_beast.CSI_hitRate));
    m_treeBEAST->Branch("DIA_dose", &(m_beast.DIA_dose));

    /*
    m_treeBEAST->Branch("TPC_neutrons_N", &(m_beast.TPC_neutrons_N));
    m_treeBEAST->Branch("TPC_neutrons_tracks_E", &(m_beast.TPC_neutrons_tracks_E));
    m_treeBEAST->Branch("TPC_neutrons_phi", &(m_beast.TPC_neutrons_phi));
    m_treeBEAST->Branch("TPC_neutrons_theta", &(m_beast.TPC_neutrons_theta));
    m_treeBEAST->Branch("TPC_neutrons_length", &(m_beast.TPC_neutrons_length));
    m_treeBEAST->Branch("TPC_alphas_top_N", &(m_beast.TPC_alphas_top_N));
    m_treeBEAST->Branch("TPC_alphas_bot_N", &(m_beast.TPC_alphas_bot_N));
    m_treeBEAST->Branch("TPC_alphas_top_tracks_dEdx", &(m_beast.TPC_alphas_top_tracks_dEdx));
    m_treeBEAST->Branch("TPC_alphas_bot_tracks_dEdx", &(m_beast.TPC_alphas_bot_tracks_dEdx));
    m_treeBEAST->Branch("TPC_xrays_N", &(m_beast.TPC_xrays_N));
    m_treeBEAST->Branch("TPC_xrays_sumE", &(m_beast.TPC_xrays_sumE));
    */
    m_treeBEAST->Branch("SAD_I_HER", &(m_input_I_HER));
    m_treeBEAST->Branch("SAD_I_LER", &(m_input_I_LER));
    m_treeBEAST->Branch("SAD_P_HER", &(m_input_P_HER));
    m_treeBEAST->Branch("SAD_P_LER", &(m_input_P_LER));
    m_treeBEAST->Branch("SAD_sigma_HER", &(m_input_sigma_HER));
    m_treeBEAST->Branch("SAD_sigma_LER", &(m_input_sigma_LER));
    m_treeBEAST->Branch("SAD_bunchNb_HER", &(m_input_bunchNb_HER));
    m_treeBEAST->Branch("SAD_bunchNb_LER", &(m_input_bunchNb_LER));

    m_treeBEAST->Branch("MC_LT_DIA_dose", &(m_input_LT_DIA_dose));
    m_treeBEAST->Branch("MC_HT_DIA_dose", &(m_input_HT_DIA_dose));
    m_treeBEAST->Branch("MC_LC_DIA_dose", &(m_input_LC_DIA_dose));
    m_treeBEAST->Branch("MC_HC_DIA_dose", &(m_input_HC_DIA_dose));
    m_treeBEAST->Branch("MC_LB_DIA_dose", &(m_input_LB_DIA_dose));
    m_treeBEAST->Branch("MC_HB_DIA_dose", &(m_input_HB_DIA_dose));

    m_treeBEAST->Branch("MC_LT_PIN_dose", &(m_input_LT_PIN_dose));
    m_treeBEAST->Branch("MC_HT_PIN_dose", &(m_input_HT_PIN_dose));
    m_treeBEAST->Branch("MC_LC_PIN_dose", &(m_input_LC_PIN_dose));
    m_treeBEAST->Branch("MC_HC_PIN_dose", &(m_input_HC_PIN_dose));
    m_treeBEAST->Branch("MC_LB_PIN_dose", &(m_input_LB_PIN_dose));
    m_treeBEAST->Branch("MC_HB_PIN_dose", &(m_input_HB_PIN_dose));

    m_treeBEAST->Branch("MC_LT_BGO_dose", &(m_input_LT_BGO_dose));
    m_treeBEAST->Branch("MC_HT_BGO_dose", &(m_input_HT_BGO_dose));
    m_treeBEAST->Branch("MC_LC_BGO_dose", &(m_input_LC_BGO_dose));
    m_treeBEAST->Branch("MC_HC_BGO_dose", &(m_input_HC_BGO_dose));
    m_treeBEAST->Branch("MC_LB_BGO_dose", &(m_input_LB_BGO_dose));
    m_treeBEAST->Branch("MC_HB_BGO_dose", &(m_input_HB_BGO_dose));

    m_treeBEAST->Branch("MC_LT_HE3_rate", &(m_input_LT_HE3_rate));
    m_treeBEAST->Branch("MC_HT_HE3_rate", &(m_input_HT_HE3_rate));
    m_treeBEAST->Branch("MC_LC_HE3_rate", &(m_input_LC_HE3_rate));
    m_treeBEAST->Branch("MC_HC_HE3_rate", &(m_input_HC_HE3_rate));
    m_treeBEAST->Branch("MC_LB_HE3_rate", &(m_input_LB_HE3_rate));
    m_treeBEAST->Branch("MC_HB_HE3_rate", &(m_input_HB_HE3_rate));

    m_treeBEAST->Branch("MC_LT_CSI_rate", &(m_input_LT_CSI_rate));
    m_treeBEAST->Branch("MC_HT_CSI_rate", &(m_input_HT_CSI_rate));
    m_treeBEAST->Branch("MC_LC_CSI_rate", &(m_input_LC_CSI_rate));
    m_treeBEAST->Branch("MC_HC_CSI_rate", &(m_input_HC_CSI_rate));
    m_treeBEAST->Branch("MC_LB_CSI_rate", &(m_input_LB_CSI_rate));
    m_treeBEAST->Branch("MC_HB_CSI_rate", &(m_input_HB_CSI_rate));

    m_treeBEAST->Branch("MC_LT_CSI_dose", &(m_input_LT_CSI_dose));
    m_treeBEAST->Branch("MC_HT_CSI_dose", &(m_input_HT_CSI_dose));
    m_treeBEAST->Branch("MC_LC_CSI_dose", &(m_input_LC_CSI_dose));
    m_treeBEAST->Branch("MC_HC_CSI_dose", &(m_input_HC_CSI_dose));
    m_treeBEAST->Branch("MC_LB_CSI_dose", &(m_input_LB_CSI_dose));
    m_treeBEAST->Branch("MC_HB_CSI_dose", &(m_input_HB_CSI_dose));
  }


  void NtuplePhase1_v2Module::beginRun()
  {
  }


  void NtuplePhase1_v2Module::event()
  {
    m_beast.clear();
    // create data store objects

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.create();

    if (m_eventCount == m_numEvents) {
      evtMetaData->setEndOfData(); // event processing
      return;
    }

    m_tree->GetEntry(m_eventCount);

    double I_HER = 0;
    if (m_beast.SKB_HER_current != 0) I_HER = m_beast.SKB_HER_current->at(0);
    if (m_input_I_HER[1] > 0) I_HER += gRandom->Gaus(0, m_input_I_HER[1]);
    double I_LER = 0;
    if (m_beast.SKB_LER_current != 0) I_LER = m_beast.SKB_LER_current->at(0);
    if (m_input_I_LER[1] > 0) I_LER += gRandom->Gaus(0, m_input_I_LER[1]);
    double P_HER = 0;
    if (m_beast.SKB_HER_pressure_average != 0) P_HER = m_beast.SKB_HER_pressure_average->at(0) * 0.00750062 * 1e9;
    if (m_input_P_HER[1] > 0) P_HER += gRandom->Gaus(0, m_input_P_HER[1]);
    double P_LER = 0;
    if (m_beast.SKB_LER_pressure_average != 0) P_LER = m_beast.SKB_LER_pressure_average->at(0) * 0.00750062 * 1e9;
    if (m_input_P_LER[1] > 0) P_LER += gRandom->Gaus(0, m_input_P_LER[1]);
    double sigma_y_HER = 0;
    if (m_beast.SKB_HER_beamSize_xray_Y != 0) sigma_y_HER = m_beast.SKB_HER_beamSize_xray_Y->at(0);
    if (m_input_sigma_HER[1] > 0) sigma_y_HER += gRandom->Gaus(0, m_input_sigma_HER[1]);
    double sigma_y_LER = 0;
    if (m_beast.SKB_LER_beamSize_xray_Y != 0) sigma_y_LER = m_beast.SKB_LER_beamSize_xray_Y->at(0);
    if (m_input_sigma_LER[1] > 0) sigma_y_LER += gRandom->Gaus(0, m_input_sigma_LER[1]);
    double bunch_nb_HER = 0;
    if (m_beast.SKB_HER_injectionNumberOfBunches != 0) bunch_nb_HER = m_beast.SKB_HER_injectionNumberOfBunches->at(0);
    if (bunch_nb_HER == 0) bunch_nb_HER = m_input_data_bunchNb_HER;
    if (m_input_bunchNb_HER[1] > 0) bunch_nb_HER += gRandom->Gaus(0, m_input_bunchNb_HER[1]);
    double bunch_nb_LER = 0;
    if (m_beast.SKB_LER_injectionNumberOfBunches != 0) bunch_nb_LER = m_beast.SKB_LER_injectionNumberOfBunches->at(0);
    if (bunch_nb_LER == 0) bunch_nb_LER = m_input_data_bunchNb_LER;
    if (m_input_bunchNb_LER[1] > 0) bunch_nb_LER += gRandom->Gaus(0, m_input_bunchNb_LER[1]);
    /*
    cout << " I_HER = " << I_HER << " P_HER = " << P_HER << " sigma_y_HER = " << sigma_y_HER << " bunch_nb_HER = " << bunch_nb_HER <<
         endl;
    cout << " I_LER = " << I_LER << " P_LER = " << P_LER << " sigma_y_LER = " << sigma_y_LER << " bunch_nb_LER = " << bunch_nb_LER <<
         endl;
    */
    if (I_HER < 0) I_HER = 0;
    if (I_LER < 0) I_LER = 0;
    if (P_HER < 0) P_HER = 0;
    if (P_LER < 0) P_LER = 0;

    //Calculate Beam Gas scaling factor: Beam Gas \propo I x P => (IP)^data / (IP)^simu
    double ScaleFacBG_HER = 0;
    double ScaleFacBG_LER = 0;
    if (I_LER > 0 && P_LER > 0)
      ScaleFacBG_LER = I_LER * P_LER / (m_input_I_LER[0] * m_input_P_LER[0]); // bunch_nb_LER / m_input_bunchNb_LER[0];
    if (I_HER > 0 && P_HER > 0)
      ScaleFacBG_HER = I_HER * P_HER / (m_input_I_HER[0] * m_input_P_HER[0]); // bunch_nb_HER / m_input_bunchNb_HER[0];

    //Calculate Touschek scaling factor: Touschek \propo I^2 / (bunch_nb x sigma_y) => (I^2/(bunch_nb x sigma_y))^data / (I^2/(bunch_nb x sigma_y))^simu
    double ScaleFacTo_HER = 0;
    double ScaleFacTo_LER = 0;
    if (bunch_nb_LER > 0 && sigma_y_LER > 0)
      ScaleFacTo_LER = TMath::Power(I_LER / m_input_I_LER[0],
                                    2) / (bunch_nb_LER / m_input_bunchNb_LER[0]) / (sigma_y_LER / m_input_sigma_LER[0]);
    if (bunch_nb_HER > 0 && sigma_y_HER > 0)
      ScaleFacTo_HER = TMath::Power(I_HER / m_input_I_HER[0],
                                    2) / (bunch_nb_HER / m_input_bunchNb_HER[0]) / (sigma_y_HER / m_input_sigma_HER[0]);

    //cout << " factor BG LER " << ScaleFacBG_LER << " Toushek LER " << ScaleFacTo_LER << endl;
    //cout << " factor BG HER " << ScaleFacBG_HER << " Toushek HER " << ScaleFacTo_HER << endl;

    //Scale DIA
    for (int i = 0; i < (int)m_input_LT_DIA_dose.size(); i++) {
      double LBG = m_input_LB_DIA_dose[i] + m_input_LC_DIA_dose[i];
      double HBG = m_input_HB_DIA_dose[i] + m_input_HC_DIA_dose[i];
      double BG = LBG * ScaleFacBG_LER + HBG * ScaleFacBG_HER;
      double To = ScaleFacTo_LER * m_input_LT_DIA_dose[i] + ScaleFacTo_HER * m_input_HT_DIA_dose[i];
      m_beast.DIA_dose.push_back(BG + To);
    }
    //Scale PIN
    for (int i = 0; i < (int)m_input_LT_PIN_dose.size(); i++) {
      double LBG = m_input_LB_PIN_dose[i] + m_input_LC_PIN_dose[i];
      double HBG = m_input_HB_PIN_dose[i] + m_input_HC_PIN_dose[i];
      double BG = LBG * ScaleFacBG_LER + HBG * ScaleFacBG_HER;
      double To = ScaleFacTo_LER * m_input_LT_PIN_dose[i] + ScaleFacTo_HER * m_input_HT_PIN_dose[i];
      m_beast.PIN_dose.push_back(BG + To);
    }
    //Scale BGO
    for (int i = 0; i < (int)m_input_LT_BGO_dose.size(); i++) {
      double LBG = m_input_LB_BGO_dose[i] + m_input_LC_BGO_dose[i];
      double HBG = m_input_HB_BGO_dose[i] + m_input_HC_BGO_dose[i];
      double BG = LBG * ScaleFacBG_LER + HBG * ScaleFacBG_HER;
      double To = ScaleFacTo_LER * m_input_LT_BGO_dose[i] + ScaleFacTo_HER * m_input_HT_BGO_dose[i];
      m_beast.BGO_energy.push_back(BG + To);
    }
    //Scale HE3
    for (int i = 0; i < (int)m_input_LT_HE3_rate.size(); i++) {
      double LBG = m_input_LB_HE3_rate[i] + m_input_LC_HE3_rate[i];
      double HBG = m_input_HB_HE3_rate[i] + m_input_HC_HE3_rate[i];
      double BG = LBG * ScaleFacBG_LER + HBG * ScaleFacBG_HER;
      double To = ScaleFacTo_LER * m_input_LT_HE3_rate[i] + ScaleFacTo_HER * m_input_HT_HE3_rate[i];
      m_beast.HE3_rate.push_back(BG + To);
    }
    //Scale CSI
    for (int i = 0; i < (int)m_input_LT_CSI_dose.size(); i++) {
      double LBG = m_input_LB_CSI_dose[i] + m_input_LC_CSI_dose[i];
      double HBG = m_input_HB_CSI_dose[i] + m_input_HC_CSI_dose[i];
      double BG = LBG * ScaleFacBG_LER + HBG * ScaleFacBG_HER;
      double To = ScaleFacTo_LER * m_input_LT_CSI_dose[i] + ScaleFacTo_HER * m_input_HT_CSI_dose[i];
      m_beast.CSI_sumE.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_CSI_rate.size(); i++) {
      double LBG = m_input_LB_CSI_rate[i] + m_input_LC_CSI_rate[i];
      double HBG = m_input_HB_CSI_rate[i] + m_input_HC_CSI_rate[i];
      double BG = LBG * ScaleFacBG_LER + HBG * ScaleFacBG_HER;
      double To = ScaleFacTo_LER * m_input_LT_CSI_rate[i] + ScaleFacTo_HER * m_input_HT_CSI_rate[i];
      m_beast.CSI_hitRate.push_back(BG + To);
    }

    m_treeBEAST->Fill();

    // set event metadata
    //evtMetaData->setEvent(m_eventCount);
    //evtMetaData->setRun(m_run);
    //evtMetaData->setExperiment(m_exp);

    m_eventCount++;
  }


  void NtuplePhase1_v2Module::endRun()
  {
  }

  void NtuplePhase1_v2Module::terminate()
  {
    delete m_tree;
    m_file->cd();
    m_treeBEAST->Write();
    m_file->Close();
  }

  void NtuplePhase1_v2Module::printModuleParams() const
  {
  }

} // end Belle2 namespace

