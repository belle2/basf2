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
#include <beast/analysis/modules/NtuplePhase1Module.h>
#include <beast/analysis/modules/SuperKEKBTree.h>
#include <beast/analysis/modules/BEASTTree.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework core


// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

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

  REG_MODULE(NtuplePhase1)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  NtuplePhase1Module::NtuplePhase1Module() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Read SKB PVs, simulated measurements of BEAST sensors, and write scaled simulated Ntuple in BEAST phase 1 data format");


    // Add parameters
    addParam("inputFileNames", m_inputFileNames,
             "List of files with SKB PVs ");

    addParam("outputFileName", m_outputFileName, "Output file name");

    addParam("input_I_HER", m_input_I_HER, "HER current");
    addParam("input_I_LER", m_input_I_LER, "LER current");

    addParam("input_P_HER", m_input_P_HER, "HER pressure");
    addParam("input_P_LER", m_input_P_LER, "LER pressure");

    addParam("input_sigma_HER", m_input_sigma_HER, "HER beam size");
    addParam("input_sigma_LER", m_input_sigma_LER, "LER beam size");

    addParam("input_bunchNb_HER", m_input_bunchNb_HER, "HER bunch number");
    addParam("input_bunchNb_LER", m_input_bunchNb_LER, "LER bunch number");

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
    m_skb.clear();
    m_beast.clear();

    m_numEntries = 0;
    m_entryCounter = 0;

  }

  NtuplePhase1Module::~NtuplePhase1Module()
  {
  }

  void NtuplePhase1Module::initialize()
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

    m_tree->SetBranchAddress("ts", &(m_skb.ts));
    m_tree->SetBranchAddress("BM_DCCT_HCUR", &(m_skb.BM_DCCT_HCUR));
    m_tree->SetBranchAddress("BM_DCCT_LCUR", &(m_skb.BM_DCCT_LCUR));

    m_tree->SetBranchAddress("CGLINJ_BEAM_GATE_STATUS", &(m_skb.CGLINJ_BEAM_GATE_STATUS));
    m_tree->SetBranchAddress("CGHINJ_BEAM_GATE_STATUS", &(m_skb.CGHINJ_BEAM_GATE_STATUS));
    /*
    m_tree->SetBranchAddress("VAHBMD_BEAMDOSE_VAL", &(m_skb.VAHBMD_BEAMDOSE_VAL));
    m_tree->SetBranchAddress("VALBMD_BEAMDOSE_VAL", &(m_skb.VALBMD_BEAMDOSE_VAL));
    */
    m_tree->SetBranchAddress("VAHCCG_HER_PRES_AVG", &(m_skb.VAHCCG_HER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_LER_PRES_AVG", &(m_skb.VALCCG_LER_PRES_AVG));

    m_tree->SetBranchAddress("VALCCG_D01_LER_PRES_AVG", &(m_skb.VALCCG_D01_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D02_LER_PRES_AVG", &(m_skb.VALCCG_D02_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D03_LER_PRES_AVG", &(m_skb.VALCCG_D03_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D04_LER_PRES_AVG", &(m_skb.VALCCG_D04_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D05_LER_PRES_AVG", &(m_skb.VALCCG_D05_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D06_LER_PRES_AVG", &(m_skb.VALCCG_D06_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D07_LER_PRES_AVG", &(m_skb.VALCCG_D07_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D08_LER_PRES_AVG", &(m_skb.VALCCG_D08_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D09_LER_PRES_AVG", &(m_skb.VALCCG_D09_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D10_LER_PRES_AVG", &(m_skb.VALCCG_D10_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D11_LER_PRES_AVG", &(m_skb.VALCCG_D11_LER_PRES_AVG));
    m_tree->SetBranchAddress("VALCCG_D12_LER_PRES_AVG", &(m_skb.VALCCG_D12_LER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D01_HER_PRES_AVG", &(m_skb.VAHCCG_D01_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D02_HER_PRES_AVG", &(m_skb.VAHCCG_D02_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D03_HER_PRES_AVG", &(m_skb.VAHCCG_D03_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D04_HER_PRES_AVG", &(m_skb.VAHCCG_D04_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D05_HER_PRES_AVG", &(m_skb.VAHCCG_D05_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D06_HER_PRES_AVG", &(m_skb.VAHCCG_D06_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D07_HER_PRES_AVG", &(m_skb.VAHCCG_D07_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D08_HER_PRES_AVG", &(m_skb.VAHCCG_D08_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D09_HER_PRES_AVG", &(m_skb.VAHCCG_D09_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D10_HER_PRES_AVG", &(m_skb.VAHCCG_D10_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D11_HER_PRES_AVG", &(m_skb.VAHCCG_D11_HER_PRES_AVG));
    m_tree->SetBranchAddress("VAHCCG_D12_HER_PRES_AVG", &(m_skb.VAHCCG_D12_HER_PRES_AVG));

    m_tree->SetBranchAddress("BMHXRM_BEAM_SIGMAX", &(m_skb.BMHXRM_BEAM_SIGMAX));
    m_tree->SetBranchAddress("BMLXRM_BEAM_SIGMAX", &(m_skb.BMLXRM_BEAM_SIGMAX));
    m_tree->SetBranchAddress("BMHXRM_BEAM_SIGMAY", &(m_skb.BMHXRM_BEAM_SIGMAY));
    m_tree->SetBranchAddress("BMLXRM_BEAM_SIGMAY", &(m_skb.BMLXRM_BEAM_SIGMAY));
    m_tree->SetBranchAddress("CGHINJ_BKSEL_NOB_SET", &(m_skb.CGHINJ_BKSEL_NOB_SET));
    m_tree->SetBranchAddress("CGLINJ_BKSEL_NOB_SET", &(m_skb.CGLINJ_BKSEL_NOB_SET));
    /*
    m_tree->SetBranchAddress("VALCCG_D02_L18_PRES", &(m_skb.VALCCG_D02_L18_PRES));
    m_tree->SetBranchAddress("VALCCG_D02_L19_PRES", &(m_skb.VALCCG_D02_L19_PRES));
    m_tree->SetBranchAddress("VALCCG_D02_L20_PRES", &(m_skb.VALCCG_D02_L20_PRES));
    m_tree->SetBranchAddress("VALCCG_D02_L21_PRES", &(m_skb.VALCCG_D02_L21_PRES));
    m_tree->SetBranchAddress("VALCCG_D02_L22_PRES", &(m_skb.VALCCG_D02_L22_PRES));
    m_tree->SetBranchAddress("VALCCG_D02_L23_PRES", &(m_skb.VALCCG_D02_L23_PRES));
    m_tree->SetBranchAddress("VALCCG_D02_L24_PRES", &(m_skb.VALCCG_D02_L24_PRES));
    m_tree->SetBranchAddress("VALCCG_D02_L25_PRES", &(m_skb.VALCCG_D02_L25_PRES));
    m_tree->SetBranchAddress("VALCCG_D02_L26_PRES", &(m_skb.VALCCG_D02_L26_PRES));
    m_tree->SetBranchAddress("VALCCG_D07_L00_PRES", &(m_skb.VALCCG_D07_L00_PRES));
    m_tree->SetBranchAddress("VALCCG_D07_L01_PRES", &(m_skb.VALCCG_D07_L01_PRES));
    m_tree->SetBranchAddress("VALCCG_D07_L02_PRES", &(m_skb.VALCCG_D07_L02_PRES));
    m_tree->SetBranchAddress("VALCCG_D10_L01_PRES", &(m_skb.VALCCG_D10_L01_PRES));
    m_tree->SetBranchAddress("VALCCG_D10_L02_PRES", &(m_skb.VALCCG_D10_L02_PRES));
    m_tree->SetBranchAddress("VALCCG_D10_L03_PRES", &(m_skb.VALCCG_D10_L03_PRES));
    m_tree->SetBranchAddress("VALCCG_D10_L04_PRES", &(m_skb.VALCCG_D10_L04_PRES));
    m_tree->SetBranchAddress("VALCCG_D10_L05_PRES", &(m_skb.VALCCG_D10_L05_PRES));
    m_tree->SetBranchAddress("VALCCG_D10_L06_PRES", &(m_skb.VALCCG_D10_L06_PRES));
    m_tree->SetBranchAddress("VALCCG_D10_L07_PRES", &(m_skb.VALCCG_D10_L07_PRES));
    m_tree->SetBranchAddress("VALCCG_D10_L08_PRES", &(m_skb.VALCCG_D10_L08_PRES));
    m_tree->SetBranchAddress("VALCCG_D11_L19_PRES", &(m_skb.VALCCG_D11_L19_PRES));
    m_tree->SetBranchAddress("VALCCG_D11_L20_PRES", &(m_skb.VALCCG_D11_L20_PRES));
    m_tree->SetBranchAddress("VALCCG_D11_L21_PRES", &(m_skb.VALCCG_D11_L21_PRES));
    m_tree->SetBranchAddress("VALCCG_D11_L22_PRES", &(m_skb.VALCCG_D11_L22_PRES));
    m_tree->SetBranchAddress("VALCCG_D11_L23_PRES", &(m_skb.VALCCG_D11_L23_PRES));
    m_tree->SetBranchAddress("VALCCG_D11_L24_PRES", &(m_skb.VALCCG_D11_L24_PRES));
    m_tree->SetBranchAddress("VALCCG_D11_L25_PRES", &(m_skb.VALCCG_D11_L25_PRES));
    m_tree->SetBranchAddress("VALCLM_D06H3OUT_CSS_CLM_POS", &(m_skb.VALCLM_D06H3OUT_CSS_CLM_POS));
    m_tree->SetBranchAddress("VALCLM_D06H3IN_CSS_CLM_POS", &(m_skb.VALCLM_D06H3IN_CSS_CLM_POS));
    m_tree->SetBranchAddress("VALCLM_D06H4OUT_CSS_CLM_POS", &(m_skb.VALCLM_D06H4OUT_CSS_CLM_POS));
    m_tree->SetBranchAddress("VALCLM_D06H4IN_CSS_CLM_POS", &(m_skb.VALCLM_D06H4IN_CSS_CLM_POS));
    m_tree->SetBranchAddress("CGLINJ_EFFICIENCY", &(m_skb.CGLINJ_EFFICIENCY));
    m_tree->SetBranchAddress("CGHINJ_EFFICIENCY", &(m_skb.CGHINJ_EFFICIENCY));
    m_tree->SetBranchAddress("COpLER_BEAM_LIFE", &(m_skb.COpLER_BEAM_LIFE));
    m_tree->SetBranchAddress("COeHER_BEAM_LIFE", &(m_skb.COeHER_BEAM_LIFE));
    m_tree->SetBranchAddress("BM_DCCT_HLIFE", &(m_skb.BM_DCCT_HLIFE));
    m_tree->SetBranchAddress("BMHDCCT_LIFE", &(m_skb.BMHDCCT_LIFE));
    m_tree->SetBranchAddress("BM_DCCT_LLIFE", &(m_skb.BM_DCCT_LLIFE));
    m_tree->SetBranchAddress("BMLDCCT_LIFE", &(m_skb.BMLDCCT_LIFE));
    */

    m_numEntries = m_tree->GetEntries();
    m_entryCounter = 0;
    m_exp = 0;
    // data store objects registration

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.registerInDataStore();

    m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
    m_treeBEAST = new TTree("tout", "BEAST data tree (simulation)");
    m_treeBEAST->Branch("ts", &(m_beast.ts));
    m_treeBEAST->Branch("SKB_LER_current", &(m_beast.SKB_LER_current));
    m_treeBEAST->Branch("SKB_HER_current", &(m_beast.SKB_HER_current));
    m_treeBEAST->Branch("SKB_LER_pressure", &(m_beast.SKB_LER_pressure));
    m_treeBEAST->Branch("SKB_HER_pressure", &(m_beast.SKB_HER_pressure));
    m_treeBEAST->Branch("SKB_LER_averagePressure", &(m_beast.SKB_LER_averagePressure));
    m_treeBEAST->Branch("SKB_HER_averagePressure", &(m_beast.SKB_HER_averagePressure));
    m_treeBEAST->Branch("SKB_HER_collimatorBeamPositionX", &(m_beast.SKB_HER_collimatorBeamPositionX));
    m_treeBEAST->Branch("SKB_HER_collimatorBeamPositionY", &(m_beast.SKB_HER_collimatorBeamPositionY));
    m_treeBEAST->Branch("SKB_LER_collimatorBeamPositionX", &(m_beast.SKB_LER_collimatorBeamPositionX));
    m_treeBEAST->Branch("SKB_LER_collimatorBeamPositionY", &(m_beast.SKB_LER_collimatorBeamPositionY));
    m_treeBEAST->Branch("SKB_HER_BeamSizeX", &(m_beast.SKB_HER_BeamSizeX));
    m_treeBEAST->Branch("SKB_HER_BeamSizeY", &(m_beast.SKB_HER_BeamSizeY));
    m_treeBEAST->Branch("SKB_LER_BeamSizeX", &(m_beast.SKB_LER_BeamSizeX));
    m_treeBEAST->Branch("SKB_LER_BeamSizeY", &(m_beast.SKB_LER_BeamSizeY));
    m_treeBEAST->Branch("SKB_LER_injection_OneBunch_BucketNumber", &(m_beast.SKB_LER_injection_OneBunch_BucketNumber));
    m_treeBEAST->Branch("SKB_HER_injection_OneBunch_BucketNumber", &(m_beast.SKB_HER_injection_OneBunch_BucketNumber));
    m_treeBEAST->Branch("SKB_HER_injectionFlag", &(m_beast.SKB_HER_injectionFlag));
    m_treeBEAST->Branch("SKB_LER_injectionFlag", &(m_beast.SKB_LER_injectionFlag));

    m_treeBEAST->Branch("PIN_dose", &(m_beast.PIN_dose));
    m_treeBEAST->Branch("BGO_energy", &(m_beast.BGO_energy));
    m_treeBEAST->Branch("HE3_rate", &(m_beast.HE3_rate));
    m_treeBEAST->Branch("CSI_sumE", &(m_beast.CSI_sumE));
    m_treeBEAST->Branch("CSI_hitRate", &(m_beast.CSI_hitRate));
    m_treeBEAST->Branch("DIA_dose", &(m_beast.DIA_dose));
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


  void NtuplePhase1Module::beginRun()
  {
  }


  void NtuplePhase1Module::event()
  {
    m_skb.clear();
    m_beast.clear();
    // create data store objects

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.create();

    if (m_eventCount == m_numEvents) {
      evtMetaData->setEndOfData(); // event processing
      return;
    }

    m_tree->GetEntry(m_eventCount);

    float I_HER = m_skb.BM_DCCT_HCUR;
    float I_LER = m_skb.BM_DCCT_LCUR;
    float P_HER = (m_skb.VAHCCG_D01_HER_PRES_AVG +
                   m_skb.VAHCCG_D02_HER_PRES_AVG +
                   m_skb.VAHCCG_D03_HER_PRES_AVG +
                   m_skb.VAHCCG_D04_HER_PRES_AVG +
                   m_skb.VAHCCG_D05_HER_PRES_AVG +
                   m_skb.VAHCCG_D06_HER_PRES_AVG +
                   m_skb.VAHCCG_D07_HER_PRES_AVG +
                   m_skb.VAHCCG_D08_HER_PRES_AVG +
                   m_skb.VAHCCG_D09_HER_PRES_AVG +
                   m_skb.VAHCCG_D10_HER_PRES_AVG +
                   m_skb.VAHCCG_D11_HER_PRES_AVG +
                   m_skb.VAHCCG_D12_HER_PRES_AVG) / 12. * 0.00750062 * 1e9;
    float P_LER = (m_skb.VALCCG_D01_LER_PRES_AVG +
                   m_skb.VALCCG_D02_LER_PRES_AVG +
                   m_skb.VALCCG_D03_LER_PRES_AVG +
                   m_skb.VALCCG_D04_LER_PRES_AVG +
                   m_skb.VALCCG_D05_LER_PRES_AVG +
                   m_skb.VALCCG_D06_LER_PRES_AVG +
                   m_skb.VALCCG_D07_LER_PRES_AVG +
                   m_skb.VALCCG_D08_LER_PRES_AVG +
                   m_skb.VALCCG_D09_LER_PRES_AVG +
                   m_skb.VALCCG_D10_LER_PRES_AVG +
                   m_skb.VALCCG_D11_LER_PRES_AVG +
                   m_skb.VALCCG_D12_LER_PRES_AVG) / 12. * 0.00750062 * 1e9;
    const float sigma_y_HER = m_skb.BMHXRM_BEAM_SIGMAY;
    const float sigma_y_LER = m_skb.BMLXRM_BEAM_SIGMAY;
    const float bunch_nb_HER = m_skb.CGHINJ_BKSEL_NOB_SET;
    const float bunch_nb_LER = m_skb.CGLINJ_BKSEL_NOB_SET;
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
    float coefBG_HER = 0;
    float coefBG_LER = 0;
    float coefTo_HER = 0;
    float coefTo_LER = 0;
    if (I_LER > 0 && P_LER > 0)
      coefBG_LER = I_LER * P_LER / (m_input_I_LER[0] * m_input_P_LER[0]);
    if (I_HER > 0 && P_HER > 0)
      coefBG_HER = I_HER * P_HER / (m_input_I_HER[0] * m_input_P_HER[0]);
    if (bunch_nb_LER > 0 && sigma_y_LER > 0)
      coefTo_LER = TMath::Power(I_LER / m_input_I_LER[0],
                                2) / (bunch_nb_LER / m_input_bunchNb_LER[0]) / (sigma_y_LER / m_input_sigma_LER[0]);
    if (bunch_nb_HER > 0 && sigma_y_HER > 0)
      coefTo_HER = TMath::Power(I_HER / m_input_I_HER[0],
                                2) / (bunch_nb_HER / m_input_bunchNb_HER[0]) / (sigma_y_HER / m_input_sigma_HER[0]);
    for (int i = 0; i < (int)m_input_LT_DIA_dose.size(); i++) {
      float LBG = m_input_LB_DIA_dose[i] + m_input_LC_DIA_dose[i];
      float HBG = m_input_HB_DIA_dose[i] + m_input_HC_DIA_dose[i];
      float BG = LBG * coefBG_LER + HBG * coefBG_HER;
      float To = coefTo_LER * m_input_LB_DIA_dose[i] + coefTo_HER * m_input_HB_DIA_dose[i];
      m_beast.DIA_dose.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_PIN_dose.size(); i++) {
      float LBG = m_input_LB_PIN_dose[i] + m_input_LC_PIN_dose[i];
      float HBG = m_input_HB_PIN_dose[i] + m_input_HC_PIN_dose[i];
      float BG = LBG * coefBG_LER + HBG * coefBG_HER;
      float To = coefTo_LER * m_input_LB_PIN_dose[i] + coefTo_HER * m_input_HB_PIN_dose[i];
      m_beast.PIN_dose.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_BGO_dose.size(); i++) {
      float LBG = m_input_LB_BGO_dose[i] + m_input_LC_BGO_dose[i];
      float HBG = m_input_HB_BGO_dose[i] + m_input_HC_BGO_dose[i];
      float BG = LBG * coefBG_LER + HBG * coefBG_HER;
      float To = coefTo_LER * m_input_LB_BGO_dose[i] + coefTo_HER * m_input_HB_BGO_dose[i];
      m_beast.BGO_energy.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_HE3_rate.size(); i++) {
      float LBG = m_input_LB_HE3_rate[i] + m_input_LC_HE3_rate[i];
      float HBG = m_input_HB_HE3_rate[i] + m_input_HC_HE3_rate[i];
      float BG = LBG * coefBG_LER + HBG * coefBG_HER;
      float To = coefTo_LER * m_input_LB_HE3_rate[i] + coefTo_HER * m_input_HB_HE3_rate[i];
      m_beast.HE3_rate.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_CSI_dose.size(); i++) {
      float LBG = m_input_LB_CSI_dose[i] + m_input_LC_CSI_dose[i];
      float HBG = m_input_HB_CSI_dose[i] + m_input_HC_CSI_dose[i];
      float BG = LBG * coefBG_LER + HBG * coefBG_HER;
      float To = coefTo_LER * m_input_LB_CSI_dose[i] + coefTo_HER * m_input_HB_CSI_dose[i];
      m_beast.CSI_sumE.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_CSI_rate.size(); i++) {
      float LBG = m_input_LB_CSI_rate[i] + m_input_LC_CSI_rate[i];
      float HBG = m_input_HB_CSI_rate[i] + m_input_HC_CSI_rate[i];
      float BG = LBG * coefBG_LER + HBG * coefBG_HER;
      float To = coefTo_LER * m_input_LB_CSI_rate[i] + coefTo_HER * m_input_HB_CSI_rate[i];
      m_beast.CSI_hitRate.push_back(BG + To);
    }

    m_beast.ts = m_skb.ts;
    m_beast.SKB_HER_current.push_back(m_skb.BM_DCCT_HCUR);
    m_beast.SKB_LER_current.push_back(m_skb.BM_DCCT_LCUR);
    m_beast.SKB_LER_averagePressure.push_back(P_LER/*m_skb.VALCCG_LER_PRES_AVG*/);
    m_beast.SKB_HER_averagePressure.push_back(P_HER/*m_skb.VAHCCG_HER_PRES_AVG*/);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D01_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D02_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D03_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D04_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D05_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D06_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D07_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D08_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D09_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D10_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D11_LER_PRES_AVG);
    m_beast.SKB_LER_pressure.push_back(m_skb.VALCCG_D12_LER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D01_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D02_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D03_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D04_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D05_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D06_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D07_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D08_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D09_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D10_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D11_HER_PRES_AVG);
    m_beast.SKB_HER_pressure.push_back(m_skb.VAHCCG_D12_HER_PRES_AVG);
    m_beast.SKB_HER_collimatorBeamPositionX.push_back(m_skb.BMHXRM_BEAM_SIGMAX);
    m_beast.SKB_HER_collimatorBeamPositionY.push_back(m_skb.BMHXRM_BEAM_SIGMAY);
    m_beast.SKB_LER_collimatorBeamPositionX.push_back(m_skb.BMLXRM_BEAM_SIGMAX);
    m_beast.SKB_LER_collimatorBeamPositionY.push_back(m_skb.BMLXRM_BEAM_SIGMAY);
    m_beast.SKB_HER_BeamSizeX.push_back(m_skb.BMHXRM_BEAM_SIGMAX);
    m_beast.SKB_HER_BeamSizeY.push_back(sigma_y_HER/*m_skb.BMHXRM_BEAM_SIGMAY*/);
    m_beast.SKB_LER_BeamSizeX.push_back(m_skb.BMLXRM_BEAM_SIGMAX);
    m_beast.SKB_LER_BeamSizeY.push_back(sigma_y_LER/*m_skb.BMLXRM_BEAM_SIGMAY*/);
    m_beast.SKB_LER_injection_OneBunch_BucketNumber.push_back(bunch_nb_LER/*m_skb.CGLINJ_BKSEL_NOB_SET*/);
    m_beast.SKB_HER_injection_OneBunch_BucketNumber.push_back(bunch_nb_HER/*m_skb.CGHINJ_BKSEL_NOB_SET*/);
    m_beast.SKB_HER_injectionFlag.push_back(m_skb.CGHINJ_BEAM_GATE_STATUS);
    m_beast.SKB_LER_injectionFlag.push_back(m_skb.CGLINJ_BEAM_GATE_STATUS);
    m_treeBEAST->Fill();

    // set event metadata
    //evtMetaData->setEvent(m_eventCount);
    //evtMetaData->setRun(m_run);
    //evtMetaData->setExperiment(m_exp);

    m_eventCount++;
  }


  void NtuplePhase1Module::endRun()
  {
  }

  void NtuplePhase1Module::terminate()
  {
    delete m_tree;
    m_file->cd();
    m_treeBEAST->Write();
    m_file->Close();
  }

  void NtuplePhase1Module::printModuleParams() const
  {
  }

} // end Belle2 namespace

