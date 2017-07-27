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
#include <beast/analysis/modules/NtuplePhase1_v6Module.h>
#include <beast/analysis/modules/BEASTTree_v5.h>

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
#include <TH1F.h>
#include <TH2F.h>
#include <TString.h>

using namespace std;

namespace Belle2 {

  using namespace RootIOUtilities;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(NtuplePhase1_v6)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  NtuplePhase1_v6Module::NtuplePhase1_v6Module() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Read SKB PVs, simulated measurements of BEAST sensors, and write scaled simulated Ntuple in BEAST phase 1 data format");


    // Add parameters
    addParam("inputFileNames", m_inputFileNames,
             "List of files with SKB PVs ");

    addParam("outputFileName", m_outputFileName, "Output file name");

    //addParam("input_ts", m_input_ts, "Input time stamp start and stop");

    addParam("input_Time_eqv", m_input_Time_eqv, "time-eqv");

    addParam("input_I_HER", m_input_I_HER, "HER current");
    addParam("input_I_LER", m_input_I_LER, "LER current");

    addParam("input_P_HER", m_input_P_HER, "HER pressure");
    addParam("input_P_LER", m_input_P_LER, "LER pressure");

    addParam("input_sigma_x_HER", m_input_sigma_x_HER, "HER beam size");
    addParam("input_sigma_x_LER", m_input_sigma_x_LER, "LER beam size");
    addParam("input_sigma_y_HER", m_input_sigma_y_HER, "HER beam size");
    addParam("input_sigma_y_LER", m_input_sigma_y_LER, "LER beam size");

    addParam("input_bunchNb_HER", m_input_bunchNb_HER, "HER bunch number");
    addParam("input_bunchNb_LER", m_input_bunchNb_LER, "LER bunch number");

    addParam("input_data_bunchNb_HER", m_input_data_bunchNb_HER, "HER bunch number");
    addParam("input_data_bunchNb_LER", m_input_data_bunchNb_LER, "LER bunch number");
    addParam("input_data_SingleBeam", m_input_data_SingleBeam, "LER/HER/Both");

    addParam("input_LT_SAD_RLR", m_input_LT_SAD_RLR, "SAD Low Ring Loss Rate");
    addParam("input_HT_SAD_RLR", m_input_HT_SAD_RLR, "SAD High Ring Loss Rate");
    addParam("input_LC_SAD_RLR", m_input_LC_SAD_RLR, "SAD Low Ring Loss Rate");
    addParam("input_HC_SAD_RLR", m_input_HC_SAD_RLR, "SAD High Ring Loss Rate");
    addParam("input_LB_SAD_RLR", m_input_LB_SAD_RLR, "SAD Low Ring Loss Rate");
    addParam("input_HB_SAD_RLR", m_input_HB_SAD_RLR, "SAD High Ring Loss Rate");
    addParam("input_LC_SAD_RLR_av", m_input_LC_SAD_RLR_av, "SAD Low Ring Loss Rate");
    addParam("input_HC_SAD_RLR_av", m_input_HC_SAD_RLR_av, "SAD High Ring Loss Rate");
    addParam("input_LB_SAD_RLR_av", m_input_LB_SAD_RLR_av, "SAD Low Ring Loss Rate");
    addParam("input_HB_SAD_RLR_av", m_input_HB_SAD_RLR_av, "SAD High Ring Loss Rate");

    addParam("input_BGSol", m_input_BGSol, "BG solution 0 or 1");
    addParam("input_ToSol", m_input_ToSol, "To solution 0 or 1");

    addParam("input_Z", m_input_Z, "Z number");

    addParam("input_GasCorrection", m_input_GasCorrection, "GasCorrection");

    addParam("input_part", m_input_part, "Part");

    addParam("inputHistoFileNames", m_inputHistoFileNames,
             "List of root files with histograms");

    addParam("inputRateHistoNames", m_inputRateHistoNames,
             "List of rate histograms");
    addParam("inputDoseHistoNames", m_inputDoseHistoNames,
             "List of dose histograms");
    addParam("inputRateHistoNamesVrs", m_inputRateHistoNamesVrs,
             "List of rate histograms");
    addParam("inputDoseHistoNamesVrs", m_inputDoseHistoNamesVrs,
             "List of dose histograms");


    addParam("input_PIN_width", m_input_PIN_width, "PIN width");
    addParam("input_HE3_EfCor", m_input_HE3_EfCor, "HE3 inefficiency correction");

    // initialize other private data members
    m_file = NULL;
    m_tree = NULL;
    m_treeBEAST = NULL;
    m_beast.clear();

    m_numEntries = 0;
    m_entryCounter = 0;

    m_DayBin = 0;
  }

  NtuplePhase1_v6Module::~NtuplePhase1_v6Module()
  {
  }

  void NtuplePhase1_v6Module::initialize()
  {
    loadDictionaries();
    // read TFile with histograms

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
    m_tree->SetBranchAddress("SKB_HER_injectionFlag_safe", &(m_beast.SKB_HER_injectionFlag_safe));
    m_tree->SetBranchAddress("SKB_LER_injectionFlag_safe", &(m_beast.SKB_LER_injectionFlag_safe));
    m_tree->SetBranchAddress("SKB_HER_abortFlag", &(m_beast.SKB_HER_abortFlag));
    m_tree->SetBranchAddress("SKB_LER_abortFlag", &(m_beast.SKB_LER_abortFlag));
    m_tree->SetBranchAddress("SKB_HER_abortFlag_safe", &(m_beast.SKB_HER_abortFlag_safe));
    m_tree->SetBranchAddress("SKB_LER_abortFlag_safe", &(m_beast.SKB_LER_abortFlag_safe));
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
    m_tree->SetBranchAddress("SKB_LER_pressures_corrected", &(m_beast.SKB_LER_pressures_corrected));
    m_tree->SetBranchAddress("SKB_HER_pressures_corrected", &(m_beast.SKB_HER_pressures_corrected));
    m_tree->SetBranchAddress("SKB_LER_pressure_average_corrected", &(m_beast.SKB_LER_pressure_average_corrected));
    m_tree->SetBranchAddress("SKB_HER_pressure_average_corrected", &(m_beast.SKB_HER_pressure_average_corrected));
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
    m_tree->SetBranchAddress("SKB_HER_correctedBeamSize_xray_Y", &(m_beast.SKB_HER_correctedBeamSize_xray_Y));
    m_tree->SetBranchAddress("SKB_LER_beamSize_xray_X", &(m_beast.SKB_LER_beamSize_xray_X));
    m_tree->SetBranchAddress("SKB_LER_beamSize_xray_Y", &(m_beast.SKB_LER_beamSize_xray_Y));
    m_tree->SetBranchAddress("SKB_LER_correctedBeamSize_xray_Y", &(m_beast.SKB_LER_correctedBeamSize_xray_Y));
    m_tree->SetBranchAddress("SKB_LER_beamSize_SR_X", &(m_beast.SKB_LER_beamSize_SR_X));
    m_tree->SetBranchAddress("SKB_LER_beamSize_SR_Y", &(m_beast.SKB_LER_beamSize_SR_Y));
    m_tree->SetBranchAddress("SKB_HER_beamSize_SR_X", &(m_beast.SKB_HER_beamSize_SR_X));
    m_tree->SetBranchAddress("SKB_HER_beamSize_SR_Y", &(m_beast.SKB_HER_beamSize_SR_Y));
    m_tree->SetBranchAddress("SKB_HER_integratedCurrent", &(m_beast.SKB_HER_integratedCurrent));
    m_tree->SetBranchAddress("SKB_LER_integratedCurrent", &(m_beast.SKB_LER_integratedCurrent));
    m_tree->SetBranchAddress("SKB_LER_partialPressures_D06", &(m_beast.SKB_LER_partialPressures_D06));
    m_tree->SetBranchAddress("SKB_LER_partialPressures_D02", &(m_beast.SKB_LER_partialPressures_D02));
    m_tree->SetBranchAddress("SKB_LER_pressures_local", &(m_beast.SKB_LER_pressures_local));
    m_tree->SetBranchAddress("SKB_HER_pressures_local", &(m_beast.SKB_HER_pressures_local));
    m_tree->SetBranchAddress("SKB_LER_pressures_local_corrected", &(m_beast.SKB_LER_pressures_local_corrected));
    m_tree->SetBranchAddress("SKB_HER_pressures_local_corrected", &(m_beast.SKB_HER_pressures_local_corrected));
    m_tree->SetBranchAddress("SKB_LER_Zeff_D02", &(m_beast.SKB_LER_Zeff_D02));
    m_tree->SetBranchAddress("SKB_LER_Zeff_D06", &(m_beast.SKB_LER_Zeff_D06));
    m_tree->SetBranchAddress("CSI_sumE", &(m_beast.CSI_data_sumE));
    m_tree->SetBranchAddress("BGO_dose", &(m_beast.BGO_data_dose));
    m_tree->SetBranchAddress("PIN_dose", &(m_beast.PIN_data_dose));
    m_tree->SetBranchAddress("DIA_dose", &(m_beast.DIA_data_dose));

    m_tree->SetBranchAddress("HE3_rate", &(m_beast.HE3_data_rate));
    m_tree->SetBranchAddress("CSI_hitRate", &(m_beast.CSI_data_rate));
    m_tree->SetBranchAddress("CSI_binnedE", &(m_beast.CSI_data_Ebin));
    m_tree->SetBranchAddress("SCI_rate", &(m_beast.QCSS_data_rate));
    m_tree->SetBranchAddress("CLW_N_MIPs_online", &(m_beast.CLAWS_data_rate));

    if (m_numEvents > 0) {
      m_tree->GetEntry(0);
      m_DayBin = (int)((m_beast.ts - 1454943600) / 60. / 60. / 24.);
    }

    // expand possible wildcards
    m_inputHistoFileNames = expandWordExpansions(m_inputHistoFileNames);
    if (m_inputFileNames.empty()) {
      B2FATAL("No valid files specified!");
    }

    fctRate_HB = new TF1("fctRate_HB", "[0] * x*x * log([1] / TMath::Power(x,1./3.) + [2])", 1.0, 19.0);
    fctRate_LB = new TF1("fctRate_LB", "[0] * x*x * log([1] / TMath::Power(x,1./3.) + [2])", 1.0, 19.0);
    fctRate_HC = new TF1("fctRate_HC", "[0] * x*x / TMath::Power( ([1] / TMath::Power(x,1./3.) + [2]), 2.)", 1.0, 19.0);
    fctRate_LC = new TF1("fctRate_LC", "[0] * x*x / TMath::Power( ([1] / TMath::Power(x,1./3.) + [2]), 2.)", 1.0, 19.0);
    fctRate_HB->SetParameters(0.183373, 0.117173, 1.23431);
    fctRate_LB->SetParameters(0.900838, 0.0455552, 1.10098);
    fctRate_HC->SetParameters(1.80992, -0.000115401, 8.4047);
    fctRate_LC->SetParameters(0.210872, -4.50637e-06, 1.64209);
    m_input_Z_scaling[0] = fctRate_HC->Eval(m_input_Z[0]) / fctRate_HC->Eval(7);
    m_input_Z_scaling[1] = fctRate_LC->Eval(m_input_Z[1]) / fctRate_LC->Eval(7);
    m_input_Z_scaling[2] = fctRate_HB->Eval(m_input_Z[2]) / fctRate_HB->Eval(7);
    m_input_Z_scaling[3] = fctRate_LB->Eval(m_input_Z[3]) / fctRate_LB->Eval(7);

    if (m_input_Z[0] == 0) m_input_Z_scaling[0] = 0;
    if (m_input_Z[1] == 0) m_input_Z_scaling[1] = 0;
    if (m_input_Z[2] == 0) m_input_Z_scaling[2] = 0;
    if (m_input_Z[3] == 0) m_input_Z_scaling[3] = 0;

    double volume = 0.;
    double rho = 0.;
    double mass = 0.;
    const double RadConv = 6.24e7; // 1 mrad = 6.24e7 MeV/kg

    // check files
    TDirectory* dirh = gDirectory;
    TFile* fh[6];
    int iter = 0;
    for (const TString& fileName : m_inputHistoFileNames) {
      fh[iter] = TFile::Open(fileName, "READ");
      if (!fh[iter] or !fh[iter]->IsOpen()) {
        B2FATAL("Couldn't open input file " + fileName);
      }
      if (fileName.Contains("Touschek") || fileName.Contains("Coulomb") || fileName.Contains("Brems")) {
        for (const TString& HistoRateName : m_inputRateHistoNames) {

          TH1F* h1D;
          if (HistoRateName.Contains("csi")) h1D = (TH1F*)fh[iter]->Get(TString::Format("csi_rate_%d", m_DayBin));
          else h1D = (TH1F*)fh[iter]->Get(HistoRateName);

          for (int i = 0; i < h1D->GetNbinsX(); i++) {
            double counts = h1D->GetBinContent(i + 1);
            double rate = counts / m_input_Time_eqv;

            if (fileName.Contains("Coulomb")) {
              if (fileName.Contains("HER")) rate *= m_input_Z_scaling[0];
              if (fileName.Contains("LER")) rate *= m_input_Z_scaling[1];
            }
            if (fileName.Contains("Brems")) {
              if (fileName.Contains("HER")) rate *= m_input_Z_scaling[2];
              if (fileName.Contains("LER")) rate *= m_input_Z_scaling[3];
            }

            if (HistoRateName.Contains("Def")) rate *= m_input_HE3_EfCor[i];

            if (fileName.Contains("HER")) {
              if (HistoRateName.Contains("qcss") && fileName.Contains("Touschek")) m_input_HT_QCSS_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("claws") && fileName.Contains("Touschek")) m_input_HT_CLAWS_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Touschek")) m_input_HT_CSI_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Touschek")) m_input_HT_HE3_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_rate") && fileName.Contains("Touschek")) m_input_HT_TPC_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_rate") && fileName.Contains("Touschek")) m_input_HT_TPC_angular_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_dose") && fileName.Contains("Touschek")) m_input_HT_TPC_angular_dose.push_back(rate); //Hz
              if (HistoRateName.Contains("qcss") && fileName.Contains("Brems")) m_input_HB_QCSS_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("claws") && fileName.Contains("Brems")) m_input_HB_CLAWS_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Brems")) m_input_HB_CSI_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Brems")) m_input_HB_HE3_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_rate") && fileName.Contains("Brems")) m_input_HB_TPC_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_rate") && fileName.Contains("Brems")) m_input_HB_TPC_angular_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_dose") && fileName.Contains("Brems")) m_input_HB_TPC_angular_dose_av.push_back(rate); //Hz
              if (HistoRateName.Contains("qcss") && fileName.Contains("Coulomb")) m_input_HC_QCSS_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("claws") && fileName.Contains("Coulomb")) m_input_HC_CLAWS_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Coulomb")) m_input_HC_CSI_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Coulomb")) m_input_HC_HE3_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_rate") && fileName.Contains("Coulomb")) m_input_HC_TPC_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_rate") && fileName.Contains("Coulomb")) m_input_HC_TPC_angular_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_dose") && fileName.Contains("Coulomb")) m_input_HC_TPC_angular_dose_av.push_back(rate); //Hz
            }
            if (fileName.Contains("LER")) {
              if (HistoRateName.Contains("qcss") && fileName.Contains("Touschek")) m_input_LT_QCSS_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("claws") && fileName.Contains("Touschek")) m_input_LT_CLAWS_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Touschek")) m_input_LT_CSI_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Touschek")) m_input_LT_HE3_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_rate") && fileName.Contains("Touschek")) m_input_LT_TPC_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_rate") && fileName.Contains("Touschek")) m_input_LT_TPC_angular_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_dose") && fileName.Contains("Touschek")) m_input_LT_TPC_angular_dose.push_back(rate); //Hz
              if (HistoRateName.Contains("qcss") && fileName.Contains("Brems")) m_input_LB_QCSS_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("claws") && fileName.Contains("Brems")) m_input_LB_CLAWS_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Brems")) m_input_LB_CSI_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Brems")) m_input_LB_HE3_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_rate") && fileName.Contains("Brems")) m_input_LB_TPC_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_rate") && fileName.Contains("Brems")) m_input_LB_TPC_angular_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_dose") && fileName.Contains("Brems")) m_input_LB_TPC_angular_dose_av.push_back(rate); //Hz
              if (HistoRateName.Contains("qcss") && fileName.Contains("Coulomb")) m_input_LC_QCSS_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("claws") && fileName.Contains("Coulomb")) m_input_LC_CLAWS_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Coulomb")) m_input_LC_CSI_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Coulomb")) m_input_LC_HE3_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_rate") && fileName.Contains("Coulomb")) m_input_LC_TPC_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_rate") && fileName.Contains("Coulomb")) m_input_LC_TPC_angular_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("tpc_angular_dose") && fileName.Contains("Coulomb")) m_input_LC_TPC_angular_dose_av.push_back(rate); //Hz
            }
          }
          delete h1D;
        }
        for (const TString& HistoDoseName : m_inputDoseHistoNames) {

          int imax = 0;
          if (HistoDoseName.Contains("csi")) imax = 18;
          if (HistoDoseName.Contains("bgo")) imax = 8;
          if (HistoDoseName.Contains("pin")) {
            imax = 64;
            volume = 0.265 * 0.265 * m_input_PIN_width; //cm^3
            rho = 2.32; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          if (HistoDoseName.Contains("dia")) {
            imax = 4;
            volume = 0.4 * 0.4 * 0.05; //cm^3
            rho = 3.53; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          if (HistoDoseName.Contains("dosi")) {
            imax = 4;
            volume = 0.265 * 0.265 * 0.01; //cm^3
            rho = 2.32; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          if (HistoDoseName.Contains("tpc")) {
            imax = 2;
            volume = 10.8537 * 2.0 * 1.68; //cm^3
            rho = 0.00066908; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          for (int i = 0; i < imax; i++) {

            TH1F* he;
            if (HistoDoseName.Contains("csi")) {
              he = (TH1F*)fh[iter]->Get(TString::Format("csi_dedep_%d_%d", i, m_DayBin));
            } else {
              he = (TH1F*)fh[iter]->Get(TString::Format("%s_%d", HistoDoseName.Data(), i));
            }

            //double step = ((double)he->GetXaxis()->GetXmax() - (double)he->GetXaxis()->GetXmin()) / ((double)he->GetNbinsX());
            double esum = 0;
            for (int j = 0; j < he->GetNbinsX(); j++) {
              double co = he->GetBinContent(j + 1);
              double va = he->GetXaxis()->GetBinCenter(j + 1);
              double esumbin = va * co;

              if (fileName.Contains("Coulomb")) {
                if (fileName.Contains("HER")) esumbin *= m_input_Z_scaling[0];
                if (fileName.Contains("LER")) esumbin *= m_input_Z_scaling[1];
              }
              if (fileName.Contains("Brems")) {
                if (fileName.Contains("HER")) esumbin *= m_input_Z_scaling[2];
                if (fileName.Contains("LER")) esumbin *= m_input_Z_scaling[3];
              }

              esum += esumbin;// / step;
              if (HistoDoseName.Contains("csi_energy")) {
                if (fileName.Contains("HER")) {
                  if (fileName.Contains("Touschek")) m_input_HT_CSI_dose_binE.push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                  if (fileName.Contains("Coulomb")) m_input_HC_CSI_dose_binE_av.push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                  if (fileName.Contains("Brems")) m_input_HB_CSI_dose_binE_av.push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                }
                if (fileName.Contains("LER")) {
                  if (fileName.Contains("Touschek")) m_input_LT_CSI_dose_binE.push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                  if (fileName.Contains("Coulomb")) m_input_LC_CSI_dose_binE_av.push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                  if (fileName.Contains("Brems")) m_input_LB_CSI_dose_binE_av.push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                }
              }
            }
            if (fileName.Contains("HER")) {
              if (HistoDoseName.Contains("csi") && HistoDoseName.Contains("edep")
                  && fileName.Contains("Touschek")) m_input_HT_CSI_dose.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Touschek")) m_input_HT_BGO_dose.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Touschek")) m_input_HT_PIN_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dosi")
                  && fileName.Contains("Touschek")) m_input_HT_DOSI.push_back(esum / m_input_Time_eqv / mass / RadConv);
              if (HistoDoseName.Contains("tpc_dose")
                  && fileName.Contains("Touschek")) m_input_HT_TPC_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              /*if (HistoDoseName.Contains("tpc_angular_dose")
                  && fileName.Contains("Touschek")) m_input_HT_TPC_angular_dose.push_back(esum / m_input_Time_eqv / mass / RadConv *
              1e-3); //keV to MeV*/
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Touschek")) m_input_HT_DIA_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("csi") && HistoDoseName.Contains("edep")
                  && fileName.Contains("Brems")) m_input_HB_CSI_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Brems")) m_input_HB_BGO_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Brems")) m_input_HB_PIN_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dosi")
                  && fileName.Contains("Brems")) m_input_HB_DOSI_av.push_back(esum / m_input_Time_eqv / mass / RadConv);
              if (HistoDoseName.Contains("tpc_dose")
                  && fileName.Contains("Brems")) m_input_HB_TPC_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              /*if (HistoDoseName.Contains("tpc_angular_dose")
                  && fileName.Contains("Brems")) m_input_HB_TPC_angular_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv *
              1e-3); //keV to MeV*/
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Brems")) m_input_HB_DIA_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("csi") && HistoDoseName.Contains("edep")
                  && fileName.Contains("Coulomb")) m_input_HC_CSI_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Coulomb")) m_input_HC_BGO_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Coulomb")) m_input_HC_PIN_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dosi")
                  && fileName.Contains("Coulomb")) m_input_HC_DOSI_av.push_back(esum / m_input_Time_eqv / mass / RadConv);
              if (HistoDoseName.Contains("tpc_dose")
                  && fileName.Contains("Coulomb")) m_input_HC_TPC_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              /*if (HistoDoseName.Contains("tpc_angular_dose")
                  && fileName.Contains("Coulomb")) m_input_HC_TPC_angular_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv *
              1e-3); //keV to MeV*/
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Coulomb")) m_input_HC_DIA_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
            }
            if (fileName.Contains("LER")) {
              if (HistoDoseName.Contains("csi") && HistoDoseName.Contains("edep")
                  && fileName.Contains("Touschek")) m_input_LT_CSI_dose.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Touschek")) m_input_LT_BGO_dose.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Touschek")) m_input_LT_PIN_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dosi")
                  && fileName.Contains("Touschek")) m_input_LT_DOSI.push_back(esum / m_input_Time_eqv / mass / RadConv);
              if (HistoDoseName.Contains("tpc_dose")
                  && fileName.Contains("Touschek")) m_input_LT_TPC_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              /*if (HistoDoseName.Contains("tpc_angular_dose")
                  && fileName.Contains("Touschek")) m_input_LT_TPC_angular_dose.push_back(esum / m_input_Time_eqv / mass / RadConv *
              1e-3); //keV to MeV*/
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Touschek")) m_input_LT_DIA_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("csi") && HistoDoseName.Contains("edep")
                  && fileName.Contains("Brems")) m_input_LB_CSI_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Brems")) m_input_LB_BGO_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Brems")) m_input_LB_PIN_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dosi")
                  && fileName.Contains("Brems")) m_input_LB_DOSI_av.push_back(esum / m_input_Time_eqv / mass / RadConv);
              if (HistoDoseName.Contains("tpc_dose")
                  && fileName.Contains("Brems")) m_input_LB_TPC_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              /*if (HistoDoseName.Contains("tpc_angular_dose")
                  && fileName.Contains("Brems")) m_input_LB_TPC_angular_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv *
              1e-3); //keV to MeV*/
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Brems")) m_input_LB_DIA_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("csi") && HistoDoseName.Contains("edep")
                  && fileName.Contains("Coulomb")) m_input_LC_CSI_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Coulomb")) m_input_LC_BGO_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Coulomb")) m_input_LC_PIN_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dosi")
                  && fileName.Contains("Coulomb")) m_input_LC_DOSI_av.push_back(esum / m_input_Time_eqv / mass / RadConv);
              if (HistoDoseName.Contains("tpc_dose")
                  && fileName.Contains("Coulomb")) m_input_LC_TPC_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              /*if (HistoDoseName.Contains("tpc_angular_dose")
                  && fileName.Contains("Coulomb")) m_input_LC_TPC_angular_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv *
              1e-3); //keV to MeV*/
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Coulomb")) m_input_LC_DIA_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
            }
            delete he;
          }
        }
      }
      if (fileName.Contains("Coulomb") || fileName.Contains("Brems")) {
        for (const TString& HistoRateName : m_inputRateHistoNamesVrs) {

          TH2F* h2D;
          if (HistoRateName.Contains("csi")) h2D = (TH2F*)fh[iter]->Get(TString::Format("csi_rs_drate_%d", m_DayBin));
          else h2D = (TH2F*)fh[iter]->Get(HistoRateName);

          for (int k = 0; k < h2D->GetNbinsY(); k++) {
            for (int i = 0; i < h2D->GetNbinsX(); i++) {
              double counts = h2D->GetBinContent(i + 1, k + 1);
              double rate = counts / m_input_Time_eqv;

              if (fileName.Contains("Coulomb")) {
                if (fileName.Contains("HER")) rate *= m_input_Z_scaling[0];
                if (fileName.Contains("LER")) rate *= m_input_Z_scaling[1];
              }
              if (fileName.Contains("Brems")) {
                if (fileName.Contains("HER")) rate *= m_input_Z_scaling[2];
                if (fileName.Contains("LER")) rate *= m_input_Z_scaling[3];
              }

              if (HistoRateName.Contains("Def")) rate *= m_input_HE3_EfCor[i];

              if (fileName.Contains("Coulomb_HER")) {
                if (HistoRateName.Contains("qcss")) m_input_HC_QCSS_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("claws")) m_input_HC_CLAWS_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("csi")) m_input_HC_CSI_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("Def")) m_input_HC_HE3_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_rate")) m_input_HC_TPC_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_angular_rate")) m_input_HC_TPC_angular_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_angular_dose")) m_input_HC_TPC_angular_dose[k].push_back(rate); //Hz
              }
              if (fileName.Contains("Coulomb_LER")) {
                if (HistoRateName.Contains("qcss")) m_input_LC_QCSS_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("claws")) m_input_LC_CLAWS_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("csi")) m_input_LC_CSI_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("Def")) m_input_LC_HE3_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_rate")) m_input_LC_TPC_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_angular_rate")) m_input_LC_TPC_angular_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_angular_dose")) m_input_LC_TPC_angular_dose[k].push_back(rate); //Hz
              }
              if (fileName.Contains("Brems_HER")) {
                if (HistoRateName.Contains("qcss")) m_input_HB_QCSS_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("claws")) m_input_HB_CLAWS_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("csi")) m_input_HB_CSI_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("Def")) m_input_HB_HE3_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_rate")) m_input_HB_TPC_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_angular_rate")) m_input_HB_TPC_angular_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_angular_dose")) m_input_HB_TPC_angular_dose[k].push_back(rate); //Hz
              }
              if (fileName.Contains("Brems_LER")) {
                if (HistoRateName.Contains("qcss")) m_input_LB_QCSS_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("claws")) m_input_LB_CLAWS_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("csi")) m_input_LB_CSI_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("Def")) m_input_LB_HE3_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_rate")) m_input_LB_TPC_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_angular_rate")) m_input_LB_TPC_angular_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("tpc_angular_dose")) m_input_LB_TPC_angular_dose[k].push_back(rate); //Hz
              }
            }
          }
          delete h2D;
        }

        for (const TString& HistoDoseName : m_inputDoseHistoNamesVrs) {
          int imax = 0;
          if (HistoDoseName.Contains("csi")) imax = 18;
          if (HistoDoseName.Contains("bgo")) imax = 8;
          if (HistoDoseName.Contains("pin")) {
            imax = 64;
            volume = 0.265 * 0.265 * m_input_PIN_width; //cm^3
            rho = 2.32; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          if (HistoDoseName.Contains("dia")) {
            imax = 4;
            volume = 0.4 * 0.4 * 0.05; //cm^3
            rho = 3.53; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          if (HistoDoseName.Contains("dosi")) {
            imax = 4;
            volume = 0.265 * 0.265 * 0.01; //cm^3
            rho = 2.32; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          if (HistoDoseName.Contains("tpc")) {
            imax = 2;
            volume = 10.8537 * 2.0 * 1.68; //cm^3
            rho = 0.00066908; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          for (int i = 0; i < imax; i++) {

            TH2F* he;
            if (HistoDoseName.Contains("csi")) {
              he = (TH2F*)fh[iter]->Get(TString::Format("csi_rs_dedep_%d_%d", i, m_DayBin));
            } else {
              he = (TH2F*)fh[iter]->Get(TString::Format("%s_%d", HistoDoseName.Data(), i));
            }

            //double step = ((double)he->GetXaxis()->GetXmax() - (double)he->GetXaxis()->GetXmin()) / ((double)he->GetNbinsX());
            for (int k = 0; k < he->GetNbinsY(); k++) {
              double esum = 0;
              for (int j = 0; j < he->GetNbinsX(); j++) {
                double co = he->GetBinContent(j + 1, k + 1);
                double va = he->GetXaxis()->GetBinCenter(j + 1);
                double esumbin = va * co;

                if (fileName.Contains("Coulomb")) {
                  if (fileName.Contains("HER")) esumbin *= m_input_Z_scaling[0];
                  if (fileName.Contains("LER")) esumbin *= m_input_Z_scaling[1];
                }
                if (fileName.Contains("Brems")) {
                  if (fileName.Contains("HER")) esumbin *= m_input_Z_scaling[2];
                  if (fileName.Contains("LER")) esumbin *= m_input_Z_scaling[3];
                }

                esum += esumbin;// / step;
                if (HistoDoseName.Contains("csi_energy")) {
                  if (fileName.Contains("HER")) {
                    if (fileName.Contains("Coulomb")) m_input_HC_CSI_dose_binE[k].push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                    if (fileName.Contains("Brems")) m_input_HB_CSI_dose_binE[k].push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                  }
                  if (fileName.Contains("LER")) {
                    if (fileName.Contains("Coulomb")) m_input_LC_CSI_dose_binE[k].push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                    if (fileName.Contains("Brems")) m_input_LB_CSI_dose_binE[k].push_back(esumbin / m_input_Time_eqv * 1e-3); //MeV to GeV
                  }
                }
              }

              if (fileName.Contains("Coulomb_HER")) {
                if (HistoDoseName.Contains("csi")
                    && HistoDoseName.Contains("edep")) m_input_HC_CSI_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("bgo")) m_input_HC_BGO_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("pin")) m_input_HC_PIN_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
                if (HistoDoseName.Contains("dosi")) m_input_HC_DOSI[k].push_back(esum / m_input_Time_eqv / mass / RadConv);
                if (HistoDoseName.Contains("tpc_dose")) m_input_HC_TPC_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv *
                      1e-3); //keV to MeV
                /*if (HistoDoseName.Contains("tpc_angular_dose")) m_input_HC_TPC_angular_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv *
                1e-3); //keV to MeV*/
                if (HistoDoseName.Contains("dia")) m_input_HC_DIA_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              }
              if (fileName.Contains("Coulomb_LER")) {
                if (HistoDoseName.Contains("csi")
                    && HistoDoseName.Contains("edep")) m_input_LC_CSI_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("bgo")) m_input_LC_BGO_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("pin")) m_input_LC_PIN_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
                if (HistoDoseName.Contains("dosi")) m_input_LC_DOSI[k].push_back(esum / m_input_Time_eqv / mass / RadConv);
                if (HistoDoseName.Contains("tpc_dose")) m_input_LC_TPC_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv *
                      1e-3); //keV to MeV
                /*if (HistoDoseName.Contains("tpc_angular_dose")) m_input_LC_TPC_angular_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv *
                1e-3); //keV to MeV*/
                if (HistoDoseName.Contains("dia")) m_input_LC_DIA_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              }
              if (fileName.Contains("Brems_HER")) {
                if (HistoDoseName.Contains("csi")
                    && HistoDoseName.Contains("edep")) m_input_HB_CSI_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("bgo")) m_input_HB_BGO_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("pin")) m_input_HB_PIN_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
                if (HistoDoseName.Contains("dosi")) m_input_HB_DOSI[k].push_back(esum / m_input_Time_eqv / mass / RadConv);
                if (HistoDoseName.Contains("tpc_dose")) m_input_HB_TPC_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv *
                      1e-3); //keV to MeV
                /*if (HistoDoseName.Contains("tpc_angular_dose")) m_input_HB_TPC_angular_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv *
                1e-3); //keV to MeV*/
                if (HistoDoseName.Contains("dia")) m_input_HB_DIA_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              }
              if (fileName.Contains("Brems_LER")) {
                if (HistoDoseName.Contains("csi")
                    && HistoDoseName.Contains("edep")) m_input_LB_CSI_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("bgo")) m_input_LB_BGO_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("pin")) m_input_LB_PIN_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
                if (HistoDoseName.Contains("dosi")) m_input_LB_DOSI[k].push_back(esum / m_input_Time_eqv / mass / RadConv);
                if (HistoDoseName.Contains("tpc_dose")) m_input_LB_TPC_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv *
                      1e-3); //keV to MeV
                /*if (HistoDoseName.Contains("tpc_angular_dose")) m_input_LB_TPC_angular_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv *
                1e-3); //keV to MeV*/
                if (HistoDoseName.Contains("dia")) m_input_LB_DIA_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              }
            }
            delete he;
          }
        }

      }
      iter++;
    }
    dirh->cd();
    dir->cd();
    m_numEntries = m_tree->GetEntries();
    cout << "m_numEntries " << m_numEntries << endl;
    m_entryCounter = 0;
    m_exp = 0;
    // data store objects registration

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.registerInDataStore();

    m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
    m_treeTruth = new TTree("truth", "Truth table (simulation)");

    m_treeBEAST = new TTree("tout", "BEAST data tree (simulation)");

    m_treeBEAST->Branch("ts", &(m_beast.ts));
    m_treeBEAST->Branch("event", &(m_beast.event));
    m_treeBEAST->Branch("run", &(m_beast.run));
    m_treeBEAST->Branch("subrun", &(m_beast.subrun));

    m_treeBEAST->Branch("SKB_HER_injectionFlag", &(m_beast.SKB_HER_injectionFlag));
    m_treeBEAST->Branch("SKB_LER_injectionFlag", &(m_beast.SKB_LER_injectionFlag));
    m_treeBEAST->Branch("SKB_HER_injectionFlag_safe", &(m_beast.SKB_HER_injectionFlag_safe));
    m_treeBEAST->Branch("SKB_LER_injectionFlag_safe", &(m_beast.SKB_LER_injectionFlag_safe));
    m_treeBEAST->Branch("SKB_HER_abortFlag", &(m_beast.SKB_HER_abortFlag));
    m_treeBEAST->Branch("SKB_LER_abortFlag", &(m_beast.SKB_LER_abortFlag));
    m_treeBEAST->Branch("SKB_HER_abortFlag_safe", &(m_beast.SKB_HER_abortFlag_safe));
    m_treeBEAST->Branch("SKB_LER_abortFlag_safe", &(m_beast.SKB_LER_abortFlag_safe));
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
    m_treeBEAST->Branch("SKB_LER_pressures_corrected", &(m_beast.SKB_LER_pressures_corrected));
    m_treeBEAST->Branch("SKB_HER_pressures_corrected", &(m_beast.SKB_HER_pressures_corrected));
    m_treeBEAST->Branch("SKB_LER_pressure_average_corrected", &(m_beast.SKB_LER_pressure_average_corrected));
    m_treeBEAST->Branch("SKB_HER_pressure_average_corrected", &(m_beast.SKB_HER_pressure_average_corrected));
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
    m_treeBEAST->Branch("SKB_HER_correctedBeamSize_xray_Y", &(m_beast.SKB_HER_correctedBeamSize_xray_Y));
    m_treeBEAST->Branch("SKB_LER_beamSize_xray_X", &(m_beast.SKB_LER_beamSize_xray_X));
    m_treeBEAST->Branch("SKB_LER_beamSize_xray_Y", &(m_beast.SKB_LER_beamSize_xray_Y));
    m_treeBEAST->Branch("SKB_LER_correctedBeamSize_xray_Y", &(m_beast.SKB_LER_correctedBeamSize_xray_Y));
    m_treeBEAST->Branch("SKB_LER_beamSize_SR_X", &(m_beast.SKB_LER_beamSize_SR_X));
    m_treeBEAST->Branch("SKB_LER_beamSize_SR_Y", &(m_beast.SKB_LER_beamSize_SR_Y));
    m_treeBEAST->Branch("SKB_HER_beamSize_SR_X", &(m_beast.SKB_HER_beamSize_SR_X));
    m_treeBEAST->Branch("SKB_HER_beamSize_SR_Y", &(m_beast.SKB_HER_beamSize_SR_Y));
    m_treeBEAST->Branch("SKB_HER_integratedCurrent", &(m_beast.SKB_HER_integratedCurrent));
    m_treeBEAST->Branch("SKB_LER_integratedCurrent", &(m_beast.SKB_LER_integratedCurrent));
    m_treeBEAST->Branch("SKB_LER_partialPressures_D06", &(m_beast.SKB_LER_partialPressures_D06));
    m_treeBEAST->Branch("SKB_LER_partialPressures_D02", &(m_beast.SKB_LER_partialPressures_D02));
    m_treeBEAST->Branch("SKB_LER_pressures_local", &(m_beast.SKB_LER_pressures_local));
    m_treeBEAST->Branch("SKB_HER_pressures_local", &(m_beast.SKB_HER_pressures_local));
    m_treeBEAST->Branch("SKB_LER_pressures_local_corrected", &(m_beast.SKB_LER_pressures_local_corrected));
    m_treeBEAST->Branch("SKB_HER_pressures_local_corrected", &(m_beast.SKB_HER_pressures_local_corrected));
    m_treeBEAST->Branch("SKB_LER_Zeff_D02", &(m_beast.SKB_LER_Zeff_D02));
    m_treeBEAST->Branch("SKB_LER_Zeff_D06", &(m_beast.SKB_LER_Zeff_D06));

    m_treeBEAST->Branch("CSI_data_sumE", &(m_beast.CSI_data_sumE));
    m_treeBEAST->Branch("BGO_data_dose", &(m_beast.BGO_data_dose));
    m_treeBEAST->Branch("PIN_data_dose", &(m_beast.PIN_data_dose));
    m_treeBEAST->Branch("DIA_data_dose", &(m_beast.DIA_data_dose));

    m_treeBEAST->Branch("HE3_data_rate", &(m_beast.HE3_data_rate));
    m_treeBEAST->Branch("CSI_data_rate", &(m_beast.CSI_data_rate));
    m_treeBEAST->Branch("QCSS_data_rate", &(m_beast.QCSS_data_rate));
    m_treeBEAST->Branch("CLAWS_data_rate", &(m_beast.CLAWS_data_rate));

    m_treeBEAST->Branch("CSI_data_Ebin", &(m_beast.CSI_data_Ebin));

    m_treeBEAST->Branch("PIN_dose", &(m_beast.PIN_dose));
    m_treeBEAST->Branch("BGO_energy", &(m_beast.BGO_energy));
    m_treeBEAST->Branch("HE3_rate", &(m_beast.HE3_rate));
    //m_treeBEAST->Branch("TPC_rate", &(m_beast.TPC_rate), "TPC_rate[2][5]/F");
    //m_treeBEAST->Branch("TPC_dose", &(m_beast.TPC_dose), "TPC_dose[2][5]/F");
    //m_treeBEAST->Branch("TPC_angular_rate", &(m_beast.TPC_angular_rate), "TPC_angular_rate[2][9][18]/F");
    //m_treeBEAST->Branch("TPC_angular_dose", &(m_beast.TPC_angular_dose), "TPC_angular_dose[2][9][18]/F");
    m_treeBEAST->Branch("CSI_sumE", &(m_beast.CSI_sumE));
    m_treeBEAST->Branch("CSI_Ebin", &(m_beast.CSI_Ebin));
    m_treeBEAST->Branch("CSI_hitRate", &(m_beast.CSI_hitRate));
    m_treeBEAST->Branch("DIA_dose", &(m_beast.DIA_dose));
    m_treeBEAST->Branch("CLAWS_rate", &(m_beast.CLAWS_rate));
    m_treeBEAST->Branch("QCSS_rate", &(m_beast.QCSS_rate));

    m_treeBEAST->Branch("DOSI_av", &(m_beast.DOSI_av));
    m_treeBEAST->Branch("DOSI", &(m_beast.DOSI));
    m_treeBEAST->Branch("PIN_dose_av", &(m_beast.PIN_dose_av));
    m_treeBEAST->Branch("BGO_energy_av", &(m_beast.BGO_energy_av));
    m_treeBEAST->Branch("HE3_rate_av", &(m_beast.HE3_rate_av));
    m_treeBEAST->Branch("TPC_rate_av", &(m_beast.TPC_rate_av), "TPC_rate_av[2][5]/F");
    //m_treeBEAST->Branch("TPC_dose_av", &(m_beast.TPC_dose_av), "TPC_dose_av[2][5]/F");
    m_treeBEAST->Branch("TPC_dose_av", &(m_beast.TPC_dose_av));
    m_treeBEAST->Branch("TPC_angular_rate_av", &(m_beast.TPC_angular_rate_av), "TPC_angular_rate_av[2][9][18]/F");
    m_treeBEAST->Branch("TPC_angular_dose_av", &(m_beast.TPC_angular_dose_av), "TPC_angular_dose_av[2][9][18]/F");
    m_treeBEAST->Branch("CSI_sumE_av", &(m_beast.CSI_sumE_av));
    m_treeBEAST->Branch("CSI_Ebin_av", &(m_beast.CSI_Ebin_av));
    m_treeBEAST->Branch("CSI_hitRate_av", &(m_beast.CSI_hitRate_av));
    m_treeBEAST->Branch("DIA_dose_av", &(m_beast.DIA_dose_av));
    m_treeBEAST->Branch("CLAWS_rate_av", &(m_beast.CLAWS_rate_av));
    m_treeBEAST->Branch("QCSS_rate_av", &(m_beast.QCSS_rate_av));

    m_treeBEAST->Branch("SAD_HER_lifetime", &(m_beast.SAD_HER_lifetime));
    m_treeBEAST->Branch("SAD_LER_lifetime", &(m_beast.SAD_LER_lifetime));
    m_treeBEAST->Branch("SAD_HER_lifetime_av", &(m_beast.SAD_HER_lifetime_av));
    m_treeBEAST->Branch("SAD_LER_lifetime_av", &(m_beast.SAD_LER_lifetime_av));
    m_treeBEAST->Branch("SAD_HER_RLR", &(m_beast.SAD_HER_RLR));
    m_treeBEAST->Branch("SAD_LER_RLR", &(m_beast.SAD_LER_RLR));
    m_treeBEAST->Branch("SKB_HER_RLR", &(m_beast.SKB_HER_RLR));
    m_treeBEAST->Branch("SKB_LER_RLR", &(m_beast.SKB_LER_RLR));
    m_treeBEAST->Branch("SAD_HER_RLR_av", &(m_beast.SAD_HER_RLR_av));
    m_treeBEAST->Branch("SAD_LER_RLR_av", &(m_beast.SAD_LER_RLR_av));

    m_treeBEAST->Branch("mc_reweight_LERT", &(m_beast.mc_reweight_LERT));
    m_treeBEAST->Branch("mc_reweight_LERC", &(m_beast.mc_reweight_LERC));
    m_treeBEAST->Branch("mc_reweight_LERB", &(m_beast.mc_reweight_LERB));
    m_treeBEAST->Branch("mc_reweight_HERT", &(m_beast.mc_reweight_HERT));
    m_treeBEAST->Branch("mc_reweight_HERC", &(m_beast.mc_reweight_HERC));
    m_treeBEAST->Branch("mc_reweight_HERB", &(m_beast.mc_reweight_HERB));


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
    m_treeTruth->Branch("SAD_I_HER", &(m_input_I_HER));
    m_treeTruth->Branch("SAD_I_LER", &(m_input_I_LER));
    m_treeTruth->Branch("SAD_P_HER", &(m_input_P_HER));
    m_treeTruth->Branch("SAD_P_LER", &(m_input_P_LER));
    m_treeTruth->Branch("SAD_sigma_x_HER", &(m_input_sigma_x_HER));
    m_treeTruth->Branch("SAD_sigma_x_LER", &(m_input_sigma_x_LER));
    m_treeTruth->Branch("SAD_sigma_y_HER", &(m_input_sigma_y_HER));
    m_treeTruth->Branch("SAD_sigma_y_LER", &(m_input_sigma_y_LER));
    m_treeTruth->Branch("SAD_bunchNb_HER", &(m_input_bunchNb_HER));
    m_treeTruth->Branch("SAD_bunchNb_LER", &(m_input_bunchNb_LER));

    for (int i = 0; i < 12; i ++) {
      m_treeTruth->Branch(TString::Format("MC_LC_DIA_dose_%d", i), &(m_input_LC_DIA_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_HC_DIA_dose_%d", i), &(m_input_HC_DIA_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_LB_DIA_dose_%d", i), &(m_input_LB_DIA_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_HB_DIA_dose_%d", i), &(m_input_HB_DIA_dose[i]));

      m_treeTruth->Branch(TString::Format("MC_LC_PIN_dose_%d", i), &(m_input_LC_PIN_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_HC_PIN_dose_%d", i), &(m_input_HC_PIN_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_LB_PIN_dose_%d", i), &(m_input_LB_PIN_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_HB_PIN_dose_%d", i), &(m_input_HB_PIN_dose[i]));

      m_treeTruth->Branch(TString::Format("MC_LC_BGO_dose_%d", i), &(m_input_LC_BGO_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_HC_BGO_dose_%d", i), &(m_input_HC_BGO_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_LB_BGO_dose_%d", i), &(m_input_LB_BGO_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_HB_BGO_dose_%d", i), &(m_input_HB_BGO_dose[i]));

      m_treeTruth->Branch(TString::Format("MC_LC_HE3_rate_%d", i), &(m_input_LC_HE3_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_HC_HE3_rate_%d", i), &(m_input_HC_HE3_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_LB_HE3_rate_%d", i), &(m_input_LB_HE3_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_HB_HE3_rate_%d", i), &(m_input_HB_HE3_rate[i]));

      //m_treeTruth->Branch(TString::Format("MC_LC_TPC_rate_%d", i), &(m_input_LC_TPC_rate[i]));
      //m_treeTruth->Branch(TString::Format("MC_HC_TPC_rate_%d", i), &(m_input_HC_TPC_rate[i]));
      //m_treeTruth->Branch(TString::Format("MC_LB_TPC_rate_%d", i), &(m_input_LB_TPC_rate[i]));
      //m_treeTruth->Branch(TString::Format("MC_HB_TPC_rate_%d", i), &(m_input_HB_TPC_rate[i]));

      //m_treeTruth->Branch(TString::Format("MC_LC_TPC_dose_%d", i), &(m_input_LC_TPC_dose[i]));
      //m_treeTruth->Branch(TString::Format("MC_HC_TPC_dose_%d", i), &(m_input_HC_TPC_dose[i]));
      //m_treeTruth->Branch(TString::Format("MC_LB_TPC_dose_%d", i), &(m_input_LB_TPC_dose[i]));
      //m_treeTruth->Branch(TString::Format("MC_HB_TPC_dose_%d", i), &(m_input_HB_TPC_dose[i]));

      //m_treeTruth->Branch(TString::Format("MC_LC_TPC_angular_rate_%d", i), &(m_input_LC_TPC_angular_rate[i]));
      //m_treeTruth->Branch(TString::Format("MC_HC_TPC_angular_rate_%d", i), &(m_input_HC_TPC_angular_rate[i]));
      //m_treeTruth->Branch(TString::Format("MC_LB_TPC_angular_rate_%d", i), &(m_input_LB_TPC_angular_rate[i]));
      //m_treeTruth->Branch(TString::Format("MC_HB_TPC_angular_rate_%d", i), &(m_input_HB_TPC_angular_rate[i]));

      //m_treeTruth->Branch(TString::Format("MC_LC_TPC_angular_dose_%d", i), &(m_input_LC_TPC_angular_dose[i]));
      //m_treeTruth->Branch(TString::Format("MC_HC_TPC_angular_dose_%d", i), &(m_input_HC_TPC_angular_dose[i]));
      //m_treeTruth->Branch(TString::Format("MC_LB_TPC_angular_dose_%d", i), &(m_input_LB_TPC_angular_dose[i]));
      //m_treeTruth->Branch(TString::Format("MC_HB_TPC_angular_dose_%d", i), &(m_input_HB_TPC_angular_dose[i]));

      m_treeTruth->Branch(TString::Format("MC_LC_CSI_rate_%d", i), &(m_input_LC_CSI_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_HC_CSI_rate_%d", i), &(m_input_HC_CSI_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_LB_CSI_rate_%d", i), &(m_input_LB_CSI_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_HB_CSI_rate_%d", i), &(m_input_HB_CSI_rate[i]));

      m_treeTruth->Branch(TString::Format("MC_LC_CSI_dose_%d", i), &(m_input_LC_CSI_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_HC_CSI_dose_%d", i), &(m_input_HC_CSI_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_LB_CSI_dose_%d", i), &(m_input_LB_CSI_dose[i]));
      m_treeTruth->Branch(TString::Format("MC_HB_CSI_dose_%d", i), &(m_input_HB_CSI_dose[i]));

      m_treeTruth->Branch(TString::Format("MC_LC_CLAWS_rate_%d", i), &(m_input_LC_CLAWS_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_HC_CLAWS_rate_%d", i), &(m_input_HC_CLAWS_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_LB_CLAWS_rate_%d", i), &(m_input_LB_CLAWS_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_HB_CLAWS_rate_%d", i), &(m_input_HB_CLAWS_rate[i]));

      m_treeTruth->Branch(TString::Format("MC_LC_QCSS_rate_%d", i), &(m_input_LC_QCSS_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_HC_QCSS_rate_%d", i), &(m_input_HC_QCSS_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_LB_QCSS_rate_%d", i), &(m_input_LB_QCSS_rate[i]));
      m_treeTruth->Branch(TString::Format("MC_HB_QCSS_rate_%d", i), &(m_input_HB_QCSS_rate[i]));
    }

    m_treeTruth->Branch("MC_LT_DIA_dose", &(m_input_LT_DIA_dose));
    m_treeTruth->Branch("MC_HT_DIA_dose", &(m_input_HT_DIA_dose));
    m_treeTruth->Branch("MC_LC_DIA_dose_av", &(m_input_LC_DIA_dose_av));
    m_treeTruth->Branch("MC_HC_DIA_dose_av", &(m_input_HC_DIA_dose_av));
    m_treeTruth->Branch("MC_LB_DIA_dose_av", &(m_input_LB_DIA_dose_av));
    m_treeTruth->Branch("MC_HB_DIA_dose_av", &(m_input_HB_DIA_dose_av));

    m_treeTruth->Branch("MC_LT_PIN_dose", &(m_input_LT_PIN_dose));
    m_treeTruth->Branch("MC_HT_PIN_dose", &(m_input_HT_PIN_dose));
    m_treeTruth->Branch("MC_LC_PIN_dose_av", &(m_input_LC_PIN_dose_av));
    m_treeTruth->Branch("MC_HC_PIN_dose_av", &(m_input_HC_PIN_dose_av));
    m_treeTruth->Branch("MC_LB_PIN_dose_av", &(m_input_LB_PIN_dose_av));
    m_treeTruth->Branch("MC_HB_PIN_dose_av", &(m_input_HB_PIN_dose_av));

    m_treeTruth->Branch("MC_LT_BGO_dose", &(m_input_LT_BGO_dose));
    m_treeTruth->Branch("MC_HT_BGO_dose", &(m_input_HT_BGO_dose));
    m_treeTruth->Branch("MC_LC_BGO_dose_av", &(m_input_LC_BGO_dose_av));
    m_treeTruth->Branch("MC_HC_BGO_dose_av", &(m_input_HC_BGO_dose_av));
    m_treeTruth->Branch("MC_LB_BGO_dose_av", &(m_input_LB_BGO_dose_av));
    m_treeTruth->Branch("MC_HB_BGO_dose_av", &(m_input_HB_BGO_dose_av));

    m_treeTruth->Branch("MC_LT_HE3_rate", &(m_input_LT_HE3_rate));
    m_treeTruth->Branch("MC_HT_HE3_rate", &(m_input_HT_HE3_rate));
    m_treeTruth->Branch("MC_LC_HE3_rate_av", &(m_input_LC_HE3_rate_av));
    m_treeTruth->Branch("MC_HC_HE3_rate_av", &(m_input_HC_HE3_rate_av));
    m_treeTruth->Branch("MC_LB_HE3_rate_av", &(m_input_LB_HE3_rate_av));
    m_treeTruth->Branch("MC_HB_HE3_rate_av", &(m_input_HB_HE3_rate_av));

    //m_treeTruth->Branch("MC_LT_TPC_rate", &(m_input_LT_TPC_rate));
    //m_treeTruth->Branch("MC_HT_TPC_rate", &(m_input_HT_TPC_rate));
    m_treeTruth->Branch("MC_LC_TPC_rate_av", &(m_input_LC_TPC_rate_av));
    m_treeTruth->Branch("MC_HC_TPC_rate_av", &(m_input_HC_TPC_rate_av));
    m_treeTruth->Branch("MC_LB_TPC_rate_av", &(m_input_LB_TPC_rate_av));
    m_treeTruth->Branch("MC_HB_TPC_rate_av", &(m_input_HB_TPC_rate_av));

    //m_treeTruth->Branch("MC_LT_TPC_dose", &(m_input_LT_TPC_dose));
    //m_treeTruth->Branch("MC_HT_TPC_dose", &(m_input_HT_TPC_dose));
    m_treeTruth->Branch("MC_LC_TPC_dose_av", &(m_input_LC_TPC_dose_av));
    m_treeTruth->Branch("MC_HC_TPC_dose_av", &(m_input_HC_TPC_dose_av));
    m_treeTruth->Branch("MC_LB_TPC_dose_av", &(m_input_LB_TPC_dose_av));
    m_treeTruth->Branch("MC_HB_TPC_dose_av", &(m_input_HB_TPC_dose_av));

    //m_treeTruth->Branch("MC_LT_TPC_angular_rate", &(m_input_LT_TPC_angular_rate));
    //m_treeTruth->Branch("MC_HT_TPC_angular_rate", &(m_input_HT_TPC_angular_rate));
    m_treeTruth->Branch("MC_LC_TPC_angular_rate_av", &(m_input_LC_TPC_angular_rate_av));
    m_treeTruth->Branch("MC_HC_TPC_angular_rate_av", &(m_input_HC_TPC_angular_rate_av));
    m_treeTruth->Branch("MC_LB_TPC_angular_rate_av", &(m_input_LB_TPC_angular_rate_av));
    m_treeTruth->Branch("MC_HB_TPC_angular_rate_av", &(m_input_HB_TPC_angular_rate_av));

    //m_treeTruth->Branch("MC_LT_TPC_angular_dose", &(m_input_LT_TPC_angular_dose));
    //m_treeTruth->Branch("MC_HT_TPC_angular_dose", &(m_input_HT_TPC_angular_dose));
    m_treeTruth->Branch("MC_LC_TPC_angular_dose_av", &(m_input_LC_TPC_angular_dose_av));
    m_treeTruth->Branch("MC_HC_TPC_angular_dose_av", &(m_input_HC_TPC_angular_dose_av));
    m_treeTruth->Branch("MC_LB_TPC_angular_dose_av", &(m_input_LB_TPC_angular_dose_av));
    m_treeTruth->Branch("MC_HB_TPC_angular_dose_av", &(m_input_HB_TPC_angular_dose_av));

    m_treeTruth->Branch("MC_LT_CSI_rate", &(m_input_LT_CSI_rate));
    m_treeTruth->Branch("MC_HT_CSI_rate", &(m_input_HT_CSI_rate));
    m_treeTruth->Branch("MC_LC_CSI_rate_av", &(m_input_LC_CSI_rate_av));
    m_treeTruth->Branch("MC_HC_CSI_rate_av", &(m_input_HC_CSI_rate_av));
    m_treeTruth->Branch("MC_LB_CSI_rate_av", &(m_input_LB_CSI_rate_av));
    m_treeTruth->Branch("MC_HB_CSI_rate_av", &(m_input_HB_CSI_rate_av));

    m_treeTruth->Branch("MC_LT_CSI_dose", &(m_input_LT_CSI_dose));
    m_treeTruth->Branch("MC_HT_CSI_dose", &(m_input_HT_CSI_dose));
    m_treeTruth->Branch("MC_LC_CSI_dose_av", &(m_input_LC_CSI_dose_av));
    m_treeTruth->Branch("MC_HC_CSI_dose_av", &(m_input_HC_CSI_dose_av));
    m_treeTruth->Branch("MC_LB_CSI_dose_av", &(m_input_LB_CSI_dose_av));
    m_treeTruth->Branch("MC_HB_CSI_dose_av", &(m_input_HB_CSI_dose_av));

    m_treeTruth->Branch("MC_LT_CLAWS_rate", &(m_input_LT_CLAWS_rate));
    m_treeTruth->Branch("MC_HT_CLAWS_rate", &(m_input_HT_CLAWS_rate));
    m_treeTruth->Branch("MC_LC_CLAWS_rate_av", &(m_input_LC_CLAWS_rate_av));
    m_treeTruth->Branch("MC_HC_CLAWS_rate_av", &(m_input_HC_CLAWS_rate_av));
    m_treeTruth->Branch("MC_LB_CLAWS_rate_av", &(m_input_LB_CLAWS_rate_av));
    m_treeTruth->Branch("MC_HB_CLAWS_rate_av", &(m_input_HB_CLAWS_rate_av));

    m_treeTruth->Branch("MC_LT_QCSS_rate", &(m_input_LT_QCSS_rate));
    m_treeTruth->Branch("MC_HT_QCSS_rate", &(m_input_HT_QCSS_rate));
    m_treeTruth->Branch("MC_LC_QCSS_rate_av", &(m_input_LC_QCSS_rate_av));
    m_treeTruth->Branch("MC_HC_QCSS_rate_av", &(m_input_HC_QCSS_rate_av));
    m_treeTruth->Branch("MC_LB_QCSS_rate_av", &(m_input_LB_QCSS_rate_av));
    m_treeTruth->Branch("MC_HB_QCSS_rate_av", &(m_input_HB_QCSS_rate_av));

    m_treeTruth->Fill();
  }


  void NtuplePhase1_v6Module::beginRun()
  {
  }


  void NtuplePhase1_v6Module::event()
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

    double Zeff_LER = 0;
    if (m_beast.SKB_LER_Zeff_D02 != 0 && m_beast.SKB_LER_Zeff_D02->size() > 0) Zeff_LER = m_beast.SKB_LER_Zeff_D02->at(0);
    //cout << "Zeff_DO2 " << Zeff_LER << endl;
    double Zeff_LC = 1;
    double Zeff_LB = 1;
    if (Zeff_LER == 0) {
      Zeff_LER = 2.7;
      //Zeff_LC = 1.0;
      //Zeff_LB = 1.0;
    } else if (Zeff_LER > 0 && Zeff_LER < 40) {
      Zeff_LC = fctRate_LC->Eval(Zeff_LER) / fctRate_LC->Eval(7) / m_input_Z_scaling[1];
      Zeff_LB = fctRate_LB->Eval(Zeff_LER) / fctRate_LB->Eval(7) / m_input_Z_scaling[3];
    }
    //cout << "Zeff_LC  " << Zeff_LC  << " Zeff_LB " << Zeff_LB << " Zeff_LER " << Zeff_LER << endl;
    if (m_input_Z[1] == 2.7 && m_input_Z[3] == 2.7) {
      Zeff_LC = 1;
      Zeff_LB = 1;
    }
    double Zeff_HC = m_input_Z_scaling[0];
    double Zeff_HB = m_input_Z_scaling[2];

    double I_HER = 0;
    if (m_beast.SKB_HER_current != 0 && m_beast.SKB_HER_current->size() > 0) I_HER = m_beast.SKB_HER_current->at(0);
    if (m_input_I_HER[1] > 0) I_HER += gRandom->Gaus(0, m_input_I_HER[1]);
    double I_LER = 0;
    if (m_beast.SKB_LER_current != 0 && m_beast.SKB_LER_current->size() > 0) I_LER = m_beast.SKB_LER_current->at(0);
    if (m_input_I_LER[1] > 0) I_LER += gRandom->Gaus(0, m_input_I_LER[1]);
    double P_HER = 0;
    if (m_beast.SKB_HER_pressure_average != 0
        && m_beast.SKB_HER_pressure_average->size() > 0) P_HER = m_beast.SKB_HER_pressure_average->at(
                0) * 0.00750062 * 1e9 * m_input_GasCorrection[0];
    if (m_input_P_HER[1] > 0) P_HER += gRandom->Gaus(0, m_input_P_HER[1]);
    double P_LER = 0;
    if (m_beast.SKB_LER_pressure_average != 0
        && m_beast.SKB_LER_pressure_average->size() > 0) P_LER = m_beast.SKB_LER_pressure_average->at(
                0) * 0.00750062 * 1e9 * m_input_GasCorrection[1];
    if (m_input_P_LER[1] > 0) P_LER += gRandom->Gaus(0, m_input_P_LER[1]);
    double P_corrected_HER = 0;
    if (m_beast.SKB_HER_pressure_average_corrected != 0
        && m_beast.SKB_HER_pressure_average_corrected->size() > 0) P_corrected_HER = m_beast.SKB_HER_pressure_average_corrected->at(
                0) * 0.00750062 * 1e9;
    if (m_input_P_HER[1] > 0) P_corrected_HER += gRandom->Gaus(0, m_input_P_HER[1]);
    double P_corrected_LER = 0;
    if (m_beast.SKB_LER_pressure_average_corrected != 0
        && m_beast.SKB_LER_pressure_average_corrected->size() > 0) P_corrected_LER = m_beast.SKB_LER_pressure_average_corrected->at(
                0) * 0.00750062 * 1e9;
    if (m_input_P_LER[1] > 0) P_corrected_LER += gRandom->Gaus(0, m_input_P_LER[1]);

    double sigma_y_HER = 0;
    double sigma_x_HER = 0;
    if (m_beast.SKB_HER_correctedBeamSize_xray_Y != 0
        && m_beast.SKB_HER_beamSize_xray_Y != 0
        && m_beast.SKB_HER_correctedBeamSize_xray_Y->size() > 0) {
      sigma_y_HER = m_beast.SKB_HER_correctedBeamSize_xray_Y->at(0);

      if (m_beast.SKB_HER_beamSize_SR_X != 0 && m_beast.SKB_HER_beamSize_SR_Y
          && m_beast.SKB_HER_beamSize_SR_X->size() > 0
          && m_beast.SKB_HER_beamSize_SR_Y->size() > 0)
        sigma_x_HER = m_beast.SKB_HER_correctedBeamSize_xray_Y->at(0) *  m_beast.SKB_HER_beamSize_SR_X->at(
                        0) /  m_beast.SKB_HER_beamSize_SR_Y->at(0);
    }
    /*
    if (m_beast.SKB_HER_beamSize_xray_Y != 0
        && m_beast.SKB_HER_beamSize_xray_Y != 0
        && m_beast.SKB_HER_beamSize_xray_Y->size() > 0) sigma_y_HER = m_beast.SKB_HER_beamSize_xray_Y->at(0);
    */
    if (m_input_sigma_x_HER[1] > 0) sigma_x_HER += gRandom->Gaus(0, m_input_sigma_x_HER[1]);
    if (m_input_sigma_y_HER[1] > 0) sigma_y_HER += gRandom->Gaus(0, m_input_sigma_y_HER[1]);
    double sigma_y_LER = 0;
    double sigma_x_LER = 0;
    if (m_beast.SKB_LER_correctedBeamSize_xray_Y != 0
        && m_beast.SKB_LER_beamSize_xray_Y != 0
        && m_beast.SKB_LER_correctedBeamSize_xray_Y->size() > 0) {
      sigma_y_LER = m_beast.SKB_LER_correctedBeamSize_xray_Y->at(0);

      if (m_beast.SKB_LER_beamSize_SR_X != 0 && m_beast.SKB_LER_beamSize_SR_Y
          && m_beast.SKB_LER_beamSize_SR_X->size() > 0
          && m_beast.SKB_LER_beamSize_SR_Y->size() > 0)
        sigma_x_LER = m_beast.SKB_LER_correctedBeamSize_xray_Y->at(0) *  m_beast.SKB_LER_beamSize_SR_X->at(
                        0) /  m_beast.SKB_LER_beamSize_SR_Y->at(0);
    }
    /*
    if (m_beast.SKB_LER_beamSize_xray_Y != 0
        && m_beast.SKB_LER_beamSize_xray_Y != 0
        && m_beast.SKB_LER_beamSize_xray_Y->size() > 0) sigma_y_LER = m_beast.SKB_LER_beamSize_xray_Y->at(0);
    */
    if (m_input_sigma_x_LER[1] > 0) sigma_x_LER += gRandom->Gaus(0, m_input_sigma_x_LER[1]);
    if (m_input_sigma_y_LER[1] > 0) sigma_y_LER += gRandom->Gaus(0, m_input_sigma_y_LER[1]);
    double bunch_nb_HER = 0;
    if (m_beast.SKB_HER_injectionNumberOfBunches != 0
        && m_beast.SKB_HER_injectionNumberOfBunches->size() > 0) bunch_nb_HER = m_beast.SKB_HER_injectionNumberOfBunches->at(0);
    if (bunch_nb_HER == 0) bunch_nb_HER = m_input_data_bunchNb_HER;
    if (m_input_bunchNb_HER[1] > 0) bunch_nb_HER += gRandom->Gaus(0, m_input_bunchNb_HER[1]);
    double bunch_nb_LER = 0;
    if (m_beast.SKB_LER_injectionNumberOfBunches != 0
        && m_beast.SKB_LER_injectionNumberOfBunches->size() > 0) bunch_nb_LER = m_beast.SKB_LER_injectionNumberOfBunches->at(0);
    if (bunch_nb_LER == 0) bunch_nb_LER = m_input_data_bunchNb_LER;
    if (m_input_bunchNb_LER[1] > 0) bunch_nb_LER += gRandom->Gaus(0, m_input_bunchNb_LER[1]);

    /*
    if (I_HER < 0 || I_HER > 2100.) I_HER = 0;
    if (I_LER < 0 || I_LER > 2100.) I_LER = 0;
    if (P_HER < 0 || P_HER > 760.) P_HER = 0;
    if (P_LER < 0 || P_LER > 760.) P_LER = 0;
    */
    if (I_HER < 0) I_HER = 0;
    if (I_LER < 0) I_LER = 0;
    if (P_HER < 0) P_HER = 0;
    if (P_LER < 0) P_LER = 0;
    if (P_corrected_HER < 0) P_corrected_HER = 0;
    if (P_corrected_LER < 0) P_corrected_LER = 0;

    if (m_input_data_SingleBeam == "LER") {
      I_HER = 0;
      P_HER = 0;
      P_corrected_HER = 0;
    } else if (m_input_data_SingleBeam == "HER") {
      I_LER = 0;
      P_LER = 0;
      P_corrected_LER = 0;
    }

    double Ib_HER = 0;
    if (bunch_nb_HER > 0) Ib_HER = I_HER / bunch_nb_HER * 1e-3; // mA -> A
    double Ib_LER = 0;
    if (bunch_nb_LER > 0) Ib_LER = I_LER / bunch_nb_LER * 1e-3; // mA -> A

    double Nb_HER = 0;
    if (Ib_HER > 0) Nb_HER = Ib_HER * 3000. / TMath::C() / (1.6e-19);
    double Nb_LER = 0;
    if (Ib_LER > 0) Nb_LER = Ib_LER * 3000. / TMath::C() / (1.6e-19);

    double RLR_HER = 0;
    if (m_beast.SKB_HER_lifetime != 0 && m_beast.SKB_HER_lifetime->size() > 0 && Nb_HER > 0) {
      RLR_HER = Nb_HER / (m_beast.SKB_HER_lifetime->at(0) * 60.) * 1e-9 * bunch_nb_HER;
      m_beast.SKB_HER_RLR.push_back(RLR_HER);
    }
    double RLR_LER = 0;
    if (m_beast.SKB_LER_lifetime != 0 && m_beast.SKB_LER_lifetime->size() > 0 && Nb_LER > 0) {
      RLR_LER = Nb_LER / (m_beast.SKB_LER_lifetime->at(0) * 60.) * 1e-9 * bunch_nb_LER;
      m_beast.SKB_LER_RLR.push_back(RLR_LER);
    }

    //Calculate Beam Gas scaling factor:
    //                       solution 0: Beam Gas \propo I x P => (IP)^data / (IP)^simu
    //                       solution 1: Beam Gas \propo I x P => (I_bP)^data / (I_bP)^simu where I_b = current / #bunch
    double ScaleFacBGav_HER = 0;
    double ScaleFacBGav_LER = 0;
    /*
    if (I_LER > 0 && P_LER > 0) {
      if (m_input_BGSol == 0
          && bunch_nb_LER > 0) ScaleFacBGav_LER = I_LER * P_LER / (m_input_I_LER[0] * m_input_P_LER[0]) / bunch_nb_LER *
                                                    m_input_bunchNb_LER[0];
      if (m_input_BGSol == 1 && bunch_nb_LER > 0) ScaleFacBGav_LER = I_LER * P_LER / (m_input_I_LER[0] * m_input_P_LER[0]);
    }
    if (I_HER > 0 && P_HER > 0) {
      if (m_input_BGSol == 0
          && bunch_nb_HER > 0) ScaleFacBGav_HER = I_HER * P_HER / (m_input_I_HER[0] * m_input_P_HER[0]) / bunch_nb_HER *
                                                    m_input_bunchNb_HER[0];
      if (m_input_BGSol == 1 && bunch_nb_HER > 0) ScaleFacBGav_HER = I_HER * P_HER / (m_input_I_HER[0] * m_input_P_HER[0]);
    }
    */
    if (I_LER > 0 && P_corrected_LER > 0) {
      if (m_input_BGSol == 0
          && bunch_nb_LER > 0) ScaleFacBGav_LER = I_LER * P_corrected_LER / (m_input_I_LER[0] * m_input_P_LER[0]) / bunch_nb_LER *
                                                    m_input_bunchNb_LER[0];
      if (m_input_BGSol == 1 && bunch_nb_LER > 0) ScaleFacBGav_LER = I_LER * P_corrected_LER / (m_input_I_LER[0] * m_input_P_LER[0]);
    }
    if (I_HER > 0 && P_corrected_HER > 0) {
      if (m_input_BGSol == 0
          && bunch_nb_HER > 0) ScaleFacBGav_HER = I_HER * P_corrected_HER / (m_input_I_HER[0] * m_input_P_HER[0]) / bunch_nb_HER *
                                                    m_input_bunchNb_HER[0];
      if (m_input_BGSol == 1 && bunch_nb_HER > 0) ScaleFacBGav_HER = I_HER * P_corrected_HER / (m_input_I_HER[0] * m_input_P_HER[0]);
    }

    //Calculate Beam Gas scaling factor: Beam Gas \propo I x P => (IP)^data / (IP)^simu
    double ScaleFacBG_HER[12];
    double ScaleFacBG_LER[12];

    if (m_beast.SKB_HER_pressures != 0 && m_input_GasCorrection[0] != 1) {
      for (int i = 0; i < (int)m_beast.SKB_HER_pressures->size(); i++) {
        ScaleFacBG_HER[i] = 0;
        double iP_HER = 0;
        iP_HER = m_beast.SKB_HER_pressures->at(i) * 0.00750062 * 1e9 * m_input_GasCorrection[0];
        if (m_input_P_HER[1] > 0) iP_HER += gRandom->Gaus(0, m_input_P_HER[1]);
        if (iP_HER < 0 || iP_HER > 260.) iP_HER = 0;
        if (I_HER > 0 && iP_HER > 0) {
          if (m_input_BGSol == 0
              && bunch_nb_HER > 0) ScaleFacBG_HER[i] = I_HER * iP_HER / (m_input_I_HER[0] * m_input_P_HER[0]) / bunch_nb_HER *
                                                         m_input_bunchNb_HER[0];
          if (m_input_BGSol == 1 && bunch_nb_HER > 0) ScaleFacBG_HER[i] = I_HER * iP_HER / (m_input_I_HER[0] * m_input_P_HER[0]);
        }
      }
    }
    if (m_beast.SKB_LER_pressures != 0 && m_input_GasCorrection[1] != 1) {
      for (int i = 0; i < (int)m_beast.SKB_LER_pressures->size(); i++) {
        ScaleFacBG_LER[i] = 0;
        double iP_LER = 0;
        iP_LER = m_beast.SKB_LER_pressures->at(i) * 0.00750062 * 1e9 * m_input_GasCorrection[1];
        if (m_input_P_LER[1] > 0) iP_LER += gRandom->Gaus(0, m_input_P_LER[1]);
        if (iP_LER < 0 || iP_LER > 260.) iP_LER = 0;
        if (I_LER > 0 && iP_LER > 0) {
          if (m_input_BGSol == 0
              && bunch_nb_LER > 0) ScaleFacBG_LER[i] = I_LER * iP_LER / (m_input_I_LER[0] * m_input_P_LER[0]) / bunch_nb_LER *
                                                         m_input_bunchNb_LER[0];
          if (m_input_BGSol == 1 && bunch_nb_LER > 0) ScaleFacBG_LER[i] = I_LER * iP_LER / (m_input_I_LER[0] * m_input_P_LER[0]);
        }
      }
    }

    if (m_beast.SKB_HER_pressures_corrected != 0 && m_input_GasCorrection[0] == 1) {
      for (int i = 0; i < (int)m_beast.SKB_HER_pressures_corrected->size(); i++) {
        ScaleFacBG_HER[i] = 0;
        double iP_HER = 0;
        iP_HER = m_beast.SKB_HER_pressures_corrected->at(i) * 0.00750062 * 1e9 * m_input_GasCorrection[0];
        if (m_input_P_HER[1] > 0) iP_HER += gRandom->Gaus(0, m_input_P_HER[1]);
        if (iP_HER < 0 || iP_HER > 260.) iP_HER = 0;
        if (I_HER > 0 && iP_HER > 0) {
          if (m_input_BGSol == 0
              && bunch_nb_HER > 0) ScaleFacBG_HER[i] = I_HER * iP_HER / (m_input_I_HER[0] * m_input_P_HER[0]) / bunch_nb_HER *
                                                         m_input_bunchNb_HER[0];
          if (m_input_BGSol == 1 && bunch_nb_HER > 0) ScaleFacBG_HER[i] = I_HER * iP_HER / (m_input_I_HER[0] * m_input_P_HER[0]);
        }
      }
    }
    if (m_beast.SKB_LER_pressures_corrected != 0 && m_input_GasCorrection[1] == 1) {
      for (int i = 0; i < (int)m_beast.SKB_LER_pressures_corrected->size(); i++) {
        ScaleFacBG_LER[i] = 0;
        double iP_LER = 0;
        iP_LER = m_beast.SKB_LER_pressures_corrected->at(i) * 0.00750062 * 1e9 * m_input_GasCorrection[1];
        if (m_input_P_LER[1] > 0) iP_LER += gRandom->Gaus(0, m_input_P_LER[1]);
        if (iP_LER < 0 || iP_LER > 260.) iP_LER = 0;
        if (I_LER > 0 && iP_LER > 0) {
          if (m_input_BGSol == 0
              && bunch_nb_LER > 0) ScaleFacBG_LER[i] = I_LER * iP_LER / (m_input_I_LER[0] * m_input_P_LER[0]) / bunch_nb_LER *
                                                         m_input_bunchNb_LER[0];
          if (m_input_BGSol == 1 && bunch_nb_LER > 0) ScaleFacBG_LER[i] = I_LER * iP_LER / (m_input_I_LER[0] * m_input_P_LER[0]);
        }
      }
    }
    //Calculate Touschek scaling factor: Touschek \propo I^2 / (bunch_nb x sigma_y) => (I^2/(bunch_nb x sigma_y))^data / (I^2/(bunch_nb x sigma_y))^simu
    double ScaleFacTo_HER = 0;
    double ScaleFacTo_LER = 0;
    if (bunch_nb_LER > 0 && sigma_y_LER > 0 && I_LER > 0) {
      if (m_input_ToSol == 0) ScaleFacTo_LER = TMath::Power(I_LER / m_input_I_LER[0],
                                                              2) / (bunch_nb_LER / m_input_bunchNb_LER[0]) / (sigma_y_LER / m_input_sigma_y_LER[0]);
      else if (m_input_ToSol == 1) ScaleFacTo_LER = TMath::Power(I_LER / m_input_I_LER[0],
                                                                   2) / (bunch_nb_LER / m_input_bunchNb_LER[0]) /
                                                      (sqrt(sigma_y_LER * sigma_x_LER / m_input_sigma_y_LER[0] / m_input_sigma_x_LER[0]));
    }
    if (bunch_nb_HER > 0 && sigma_y_HER > 0 && I_HER > 0) {
      if (m_input_ToSol == 0) ScaleFacTo_HER = TMath::Power(I_HER / m_input_I_HER[0],
                                                              2) / (bunch_nb_HER / m_input_bunchNb_HER[0]) / (sigma_y_HER / m_input_sigma_y_HER[0]);
      else if (m_input_ToSol == 1) ScaleFacTo_HER = TMath::Power(I_HER / m_input_I_HER[0],
                                                                   2) / (bunch_nb_HER / m_input_bunchNb_HER[0]) /
                                                      (sqrt(sigma_y_HER * sigma_x_HER / m_input_sigma_y_HER[0] / m_input_sigma_x_HER[0]));
    }

    //Save reweight information for Touschek and BeamGas-Coulomb & -Brems
    m_beast.mc_reweight_LERT.push_back(ScaleFacTo_LER);
    m_beast.mc_reweight_HERT.push_back(ScaleFacTo_HER);
    for (int i = 0; i < 12; i ++) {
      m_beast.mc_reweight_HERC.push_back(Zeff_HC * ScaleFacBG_HER[i]);
      m_beast.mc_reweight_HERB.push_back(Zeff_HB * ScaleFacBG_HER[i]);
      if (Zeff_LC != 1)
        m_beast.mc_reweight_LERC.push_back(Zeff_LC * ScaleFacBG_LER[i]);
      else
        m_beast.mc_reweight_LERC.push_back(m_input_Z_scaling[1] * ScaleFacBG_LER[i]);
      if (Zeff_LB != 1)
        m_beast.mc_reweight_LERB.push_back(Zeff_LB * ScaleFacBG_LER[i]);
      else
        m_beast.mc_reweight_LERB.push_back(m_input_Z_scaling[3] * ScaleFacBG_LER[i]);
    }
    //Only Touschek LER + HER
    if (m_input_part == 0) {
      ScaleFacBGav_HER = 0;
      ScaleFacBGav_LER = 0;
      for (int i = 0; i < 12; i ++) {
        ScaleFacBG_HER[i] = 0;
        ScaleFacBG_LER[i] = 0;
      }
    }
    //Only Beam Gas
    if (m_input_part == 1) {
      ScaleFacTo_HER = 0;
      ScaleFacTo_LER = 0;
    }
    /*
    //Only Beam Gas - LER Brems
    if (m_input_part == 2) {
      ScaleFacTo_HER = 0;
      ScaleFacTo_LER = 0;
      Zeff_LC = 0;
      Zeff_HB = 0;
      Zeff_HC = 0;
    }
    //Only Beam Gas - LER Coulomb
    if (m_input_part == 3) {
      ScaleFacTo_HER = 0;
      ScaleFacTo_LER = 0;
      Zeff_LB = 0;
      Zeff_HB = 0;
      Zeff_HC = 0;
    }
    //Only Beam Gas - HER Brems
    if (m_input_part == 4) {
      ScaleFacTo_HER = 0;
      ScaleFacTo_LER = 0;
      Zeff_HC = 0;
      Zeff_LB = 0;
      Zeff_LC = 0;
    }
    //Only Beam Gas - HER Coulomb
    if (m_input_part == 5) {
      ScaleFacTo_HER = 0;
      ScaleFacTo_LER = 0;
      Zeff_HB = 0;
      Zeff_LB = 0;
      Zeff_LC = 0;
    }
    */
    //Scale LER SAD RLR
    for (int i = 0; i < (int)m_input_LT_SAD_RLR.size(); i++) {
      float LBG = m_input_LB_SAD_RLR_av[i] * Zeff_LB + m_input_LC_SAD_RLR_av[i] * Zeff_LC;
      float BG = LBG * ScaleFacBGav_LER;
      float To = ScaleFacTo_LER * m_input_LT_SAD_RLR[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.SAD_LER_RLR_av.push_back(BG + To);
      m_beast.SAD_LER_lifetime_av.push_back(Nb_LER / (BG + To) * 1e-9 / 60. * bunch_nb_LER);
      BG = 0;
      LBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0;
        if (m_input_LB_SAD_RLR.size() > 0) {
          LBG = m_input_LB_SAD_RLR[j] * Zeff_LB + m_input_LC_SAD_RLR[j] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.SAD_LER_RLR.push_back(BG + To);
      m_beast.SAD_LER_lifetime.push_back(Nb_LER / (BG + To) * 1e-9 / 60. * bunch_nb_LER);
    }

    //Scale HER SAD RLR
    for (int i = 0; i < (int)m_input_HT_SAD_RLR.size(); i++) {
      float HBG = m_input_HB_SAD_RLR_av[i] + m_input_HC_SAD_RLR_av[i];
      float BG = HBG * ScaleFacBGav_HER;
      float To = ScaleFacTo_HER * m_input_HT_SAD_RLR[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.SAD_HER_RLR_av.push_back(BG + To);
      m_beast.SAD_HER_lifetime_av.push_back(Nb_HER / (BG + To) * 1e-9 / 60. * bunch_nb_HER);
      BG = 0;
      HBG = 0;
      for (int j = 0; j < 12; j++) {
        HBG = 0;
        if (m_input_HB_SAD_RLR.size() > 0) {
          HBG = m_input_HB_SAD_RLR[j] + m_input_HC_SAD_RLR[j];
          BG += HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.SAD_HER_RLR.push_back(BG + To);
      m_beast.SAD_HER_lifetime.push_back(Nb_HER / (BG + To) * 1e-9 / 60. * bunch_nb_HER);
    }

    //Scale DIA
    for (int i = 0; i < (int)m_input_LT_DIA_dose.size(); i++) {
      double LBG = m_input_LB_DIA_dose_av[i] + m_input_LC_DIA_dose_av[i];
      double HBG = m_input_HB_DIA_dose_av[i] + m_input_HC_DIA_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_DIA_dose[i] + ScaleFacTo_HER * m_input_HT_DIA_dose[i];
      /*cout << "ch # " << i
           << " av LB " << m_input_LB_DIA_dose_av[i] << " LC " <<  m_input_LC_DIA_dose_av[i]
           << " HB " << m_input_HB_DIA_dose_av[i] << " HC " <<  m_input_HC_DIA_dose_av[i]
           << " LT " << m_input_LT_DIA_dose[i] << " HT " <<  m_input_HT_DIA_dose[i]
           << endl;*/
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.DIA_dose_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_DIA_dose[j].size() > 0) {
          //LBG = m_input_LB_DIA_dose[j][i] + m_input_LC_DIA_dose[j][i];
          HBG = m_input_HB_DIA_dose[j][i] + m_input_HC_DIA_dose[j][i];
          LBG = m_input_LB_DIA_dose[j][i] * Zeff_LB + m_input_LC_DIA_dose[j][i] * Zeff_LC;
          /*cout << "section " << j
               << " LB " << m_input_LB_DIA_dose[j][i] << " LC " << m_input_LC_DIA_dose[j][i] << " HB " <<  m_input_HB_DIA_dose[j][i] << " HC " <<
               m_input_HC_DIA_dose[j][i] << endl;*/
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.DIA_dose.push_back(BG + To);
    }

    //Scale PIN
    for (int i = 0; i < (int)m_input_LT_PIN_dose.size(); i++) {
      double LBG = m_input_LB_PIN_dose_av[i] + m_input_LC_PIN_dose_av[i];
      double HBG = m_input_HB_PIN_dose_av[i] + m_input_HC_PIN_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_PIN_dose[i] + ScaleFacTo_HER * m_input_HT_PIN_dose[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.PIN_dose_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_PIN_dose[j].size() > 0) {
          //LBG = m_input_LB_PIN_dose[j][i] + m_input_LC_PIN_dose[j][i];
          HBG = m_input_HB_PIN_dose[j][i] + m_input_HC_PIN_dose[j][i];
          LBG = m_input_LB_PIN_dose[j][i] * Zeff_LB + m_input_LC_PIN_dose[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.PIN_dose.push_back(BG + To);
    }

    //Scale DOSI
    for (int i = 0; i < (int)m_input_LT_DOSI.size(); i++) {
      //cout << "LB : " << m_input_LB_DOSI_av[i] << " LC " << m_input_LC_DOSI_av[i] << endl;
      //cout << "HB : " << m_input_HB_DOSI_av[i] << " HC " << m_input_HC_DOSI_av[i] << endl;
      //cout << "HT : " << m_input_HT_DOSI[i] << " LT " << m_input_LT_DOSI[i] << endl;
      double LBG = m_input_LB_DOSI_av[i] + m_input_LC_DOSI_av[i];
      double HBG = m_input_HB_DOSI_av[i] + m_input_HC_DOSI_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_DOSI[i] + ScaleFacTo_HER * m_input_HT_DOSI[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.DOSI_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_DOSI[j].size() > 0) {
          //LBG = m_input_LB_DOSI[j][i] + m_input_LC_DOSI[j][i];
          HBG = m_input_HB_DOSI[j][i] + m_input_HC_DOSI[j][i];
          LBG = m_input_LB_DOSI[j][i] * Zeff_LB + m_input_LC_DOSI[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.DOSI.push_back(BG + To);
    }

    //Scale BGO
    for (int i = 0; i < (int)m_input_LT_BGO_dose.size(); i++) {
      double LBG = m_input_LB_BGO_dose_av[i] + m_input_LC_BGO_dose_av[i];
      double HBG = m_input_HB_BGO_dose_av[i] + m_input_HC_BGO_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_BGO_dose[i] + ScaleFacTo_HER * m_input_HT_BGO_dose[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.BGO_energy_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_BGO_dose[j].size() > 0) {
          //LBG = m_input_LB_BGO_dose[j][i] + m_input_LC_BGO_dose[j][i];
          HBG = m_input_HB_BGO_dose[j][i] + m_input_HC_BGO_dose[j][i];
          LBG = m_input_LB_BGO_dose[j][i] * Zeff_LB + m_input_LC_BGO_dose[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.BGO_energy.push_back(BG + To);
    }
    int he3order[4];
    if (m_beast.ts > 1464868800) {
      he3order[0] = 0;
      he3order[1] = 3;
      he3order[2] = 2;
      he3order[3] = 1;
    } else {
      he3order[0] = 0;
      he3order[1] = 1;
      he3order[2] = 2;
      he3order[3] = 3;
    }
    //Scale HE3
    for (int i = 0; i < (int)m_input_LT_HE3_rate.size(); i++) {
      double LBG = m_input_LB_HE3_rate_av[he3order[i]] + m_input_LC_HE3_rate_av[he3order[i]];
      double HBG = m_input_HB_HE3_rate_av[he3order[i]] + m_input_HC_HE3_rate_av[he3order[i]];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_HE3_rate[he3order[i]] + ScaleFacTo_HER * m_input_HT_HE3_rate[he3order[i]];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.HE3_rate_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_HE3_rate[j].size() > 0) {
          //LBG = m_input_LB_HE3_rate[j][he3order[i]] + m_input_LC_HE3_rate[j][he3order[i]];
          HBG = m_input_HB_HE3_rate[j][he3order[i]] + m_input_HC_HE3_rate[j][he3order[i]];
          LBG = m_input_LB_HE3_rate[j][he3order[i]] * Zeff_LB + m_input_LC_HE3_rate[j][he3order[i]] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.HE3_rate.push_back(BG + To);
    }

    //Scale TPC
    for (int i = 0; i < (int)m_input_LT_TPC_rate.size(); i++) {
      double LBG = m_input_LB_TPC_rate_av[i] + m_input_LC_TPC_rate_av[i];
      double HBG = m_input_HB_TPC_rate_av[i] + m_input_HC_TPC_rate_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_TPC_rate[i] + ScaleFacTo_HER * m_input_HT_TPC_rate[i];
      int tpc_ch = (int)(i / 5);
      int n_type = i - 5 * tpc_ch;
      m_beast.TPC_rate_av[tpc_ch][n_type] = (BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_TPC_rate[j].size() > 0) {
          //LBG = m_input_LB_TPC_rate[j][i] + m_input_LC_TPC_rate[j][i];
          HBG = m_input_HB_TPC_rate[j][i] + m_input_HC_TPC_rate[j][i];
          LBG = m_input_LB_TPC_rate[j][i] * Zeff_LB + m_input_LC_TPC_rate[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //m_beast.TPC_rate[tpc_ch][n_type] = (BG + To);
    }


    //Scale TPC_dose
    for (int i = 0; i < (int)m_input_LT_TPC_dose.size(); i++) {
      double LBG = m_input_LB_TPC_dose_av[i] + m_input_LC_TPC_dose_av[i];
      double HBG = m_input_HB_TPC_dose_av[i] + m_input_HC_TPC_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_TPC_dose[i] + ScaleFacTo_HER * m_input_HT_TPC_dose[i];
      //int tpc_ch = (int)(i / 5);
      //int n_type = i - 5 * tpc_ch;
      //m_beast.TPC_dose_av[tpc_ch][n_type] = (BG + To);
      m_beast.TPC_dose_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_TPC_dose[j].size() > 0) {
          //LBG = m_input_LB_TPC_dose[j][i] + m_input_LC_TPC_dose[j][i];
          HBG = m_input_HB_TPC_dose[j][i] + m_input_HC_TPC_dose[j][i];
          LBG = m_input_LB_TPC_dose[j][i] * Zeff_LB + m_input_LC_TPC_dose[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //m_beast.TPC_dose[tpc_ch][n_type] = (BG + To);
    }

    //Scale TPC_angular
    for (int i = 0; i < (int)m_input_LT_TPC_angular_rate.size(); i++) {
      double LBG = m_input_LB_TPC_angular_rate_av[i] + m_input_LC_TPC_angular_rate_av[i];
      double HBG = m_input_HB_TPC_angular_rate_av[i] + m_input_HC_TPC_angular_rate_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_TPC_angular_rate[i] + ScaleFacTo_HER * m_input_HT_TPC_angular_rate[i];
      int tpc_ch = (int)(i / (9 * 18));
      int angle = i - (9 * 18) * tpc_ch;
      int i_theta = (int)(angle / 18);
      int i_phi = angle - 9 * i_theta;
      m_beast.TPC_angular_rate_av[tpc_ch][i_theta][i_phi] = (BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_TPC_angular_rate[j].size() > 0) {
          //LBG = m_input_LB_TPC_angular_rate[j][i] + m_input_LC_TPC_angular_rate[j][i];
          HBG = m_input_HB_TPC_angular_rate[j][i] + m_input_HC_TPC_angular_rate[j][i];
          LBG = m_input_LB_TPC_angular_rate[j][i] * Zeff_LB + m_input_LC_TPC_angular_rate[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //m_beast.TPC_angular_rate[tpc_ch][n_type] = (BG + To);
    }

    //Scale TPC_angular_dose
    for (int i = 0; i < (int)m_input_LT_TPC_angular_dose.size(); i++) {
      double LBG = m_input_LB_TPC_angular_dose_av[i] + m_input_LC_TPC_angular_dose_av[i];
      double HBG = m_input_HB_TPC_angular_dose_av[i] + m_input_HC_TPC_angular_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_TPC_angular_dose[i] + ScaleFacTo_HER * m_input_HT_TPC_angular_dose[i];
      int tpc_ch = (int)(i / (9 * 18));
      int angle = i - (9 * 18) * tpc_ch;
      int i_theta = (int)(angle / 18);
      int i_phi = angle - 9 * i_theta;
      m_beast.TPC_angular_dose_av[tpc_ch][i_theta][i_phi] = (BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_TPC_angular_dose[j].size() > 0) {
          //LBG = m_input_LB_TPC_angular_dose[j][i] + m_input_LC_TPC_angular_dose[j][i];
          HBG = m_input_HB_TPC_angular_dose[j][i] + m_input_HC_TPC_angular_dose[j][i];
          LBG = m_input_LB_TPC_angular_dose[j][i] * Zeff_LB + m_input_LC_TPC_angular_dose[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //m_beast.TPC_angular_dose[tpc_ch][n_type] = (BG + To);
    }


    //Scale CLAWS
    for (int i = 0; i < (int)m_input_LT_CLAWS_rate.size(); i++) {
      double LBG = m_input_LB_CLAWS_rate_av[i] + m_input_LC_CLAWS_rate_av[i];
      double HBG = m_input_HB_CLAWS_rate_av[i] + m_input_HC_CLAWS_rate_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_CLAWS_rate[i] + ScaleFacTo_HER * m_input_HT_CLAWS_rate[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.CLAWS_rate_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_CLAWS_rate[j].size() > 0) {
          //LBG = m_input_LB_CLAWS_rate[j][i] + m_input_LC_CLAWS_rate[j][i];
          HBG = m_input_HB_CLAWS_rate[j][i] + m_input_HC_CLAWS_rate[j][i];
          LBG = m_input_LB_CLAWS_rate[j][i] * Zeff_LB + m_input_LC_CLAWS_rate[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.CLAWS_rate.push_back(BG + To);
    }

    //Scale QCSS
    for (int i = 0; i < (int)m_input_LT_QCSS_rate.size(); i++) {
      double LBG = m_input_LB_QCSS_rate_av[i] + m_input_LC_QCSS_rate_av[i];
      double HBG = m_input_HB_QCSS_rate_av[i] + m_input_HC_QCSS_rate_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_QCSS_rate[i] + ScaleFacTo_HER * m_input_HT_QCSS_rate[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.QCSS_rate_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_QCSS_rate[j].size() > 0) {
          //LBG = m_input_LB_QCSS_rate[j][i] + m_input_LC_QCSS_rate[j][i];
          HBG = m_input_HB_QCSS_rate[j][i] + m_input_HC_QCSS_rate[j][i];
          LBG = m_input_LB_QCSS_rate[j][i] * Zeff_LB + m_input_LC_QCSS_rate[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.QCSS_rate.push_back(BG + To);
    }

    //Scale CSI
    for (int i = 0; i < (int)m_input_LT_CSI_dose.size(); i++) {
      double LBG = m_input_LB_CSI_dose_av[i] + m_input_LC_CSI_dose_av[i];
      double HBG = m_input_HB_CSI_dose_av[i] + m_input_HC_CSI_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_CSI_dose[i] + ScaleFacTo_HER * m_input_HT_CSI_dose[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.CSI_sumE_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_CSI_dose[j].size() > 0) {
          //LBG = m_input_LB_CSI_dose[j][i] + m_input_LC_CSI_dose[j][i];
          HBG = m_input_HB_CSI_dose[j][i] + m_input_HC_CSI_dose[j][i];
          LBG = m_input_LB_CSI_dose[j][i] * Zeff_LB + m_input_LC_CSI_dose[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.CSI_sumE.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_CSI_dose_binE.size(); i++) {
      double LBG = m_input_LB_CSI_dose_binE_av[i] + m_input_LC_CSI_dose_binE_av[i];
      double HBG = m_input_HB_CSI_dose_binE_av[i] + m_input_HC_CSI_dose_binE_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_CSI_dose_binE[i] + ScaleFacTo_HER * m_input_HT_CSI_dose_binE[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.CSI_Ebin_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_CSI_dose_binE[j].size() > 0) {
          //LBG = m_input_LB_CSI_dose_binE[j][i] + m_input_LC_CSI_dose_binE[j][i];
          HBG = m_input_HB_CSI_dose_binE[j][i] + m_input_HC_CSI_dose_binE[j][i];
          LBG = m_input_LB_CSI_dose_binE[j][i] * Zeff_LB + m_input_LC_CSI_dose_binE[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.CSI_Ebin.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_CSI_rate.size(); i++) {
      double LBG = m_input_LB_CSI_rate_av[i] + m_input_LC_CSI_rate_av[i];
      double HBG = m_input_HB_CSI_rate_av[i] + m_input_HC_CSI_rate_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_CSI_rate[i] + ScaleFacTo_HER * m_input_HT_CSI_rate[i];
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.CSI_hitRate_av.push_back(BG + To);
      BG = 0; LBG = 0; HBG = 0;
      for (int j = 0; j < 12; j++) {
        LBG = 0; HBG = 0;
        if (m_input_LB_CSI_rate[j].size() > 0) {
          //LBG = m_input_LB_CSI_rate[j][i] + m_input_LC_CSI_rate[j][i];
          HBG = m_input_HB_CSI_rate[j][i] + m_input_HC_CSI_rate[j][i];
          LBG = m_input_LB_CSI_rate[j][i] * Zeff_LB + m_input_LC_CSI_rate[j][i] * Zeff_LC;
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      //if (TMath::IsNaN(To)) To = 0;
      //if (TMath::IsNaN(BG)) BG = 0;
      m_beast.CSI_hitRate.push_back(BG + To);
    }

    m_treeBEAST->Fill();

    // set event metadata
    //evtMetaData->setEvent(m_eventCount);
    //evtMetaData->setRun(m_run);
    //evtMetaData->setExperiment(m_exp);

    m_eventCount++;

  }


  void NtuplePhase1_v6Module::endRun()
  {
  }

  void NtuplePhase1_v6Module::terminate()
  {
    delete m_tree;
    m_file->cd();
    m_treeBEAST->Write();
    m_treeTruth->Write();
    m_file->Close();
  }

  void NtuplePhase1_v6Module::printModuleParams() const
  {
  }

} // end Belle2 namespace
