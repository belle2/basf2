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
#include <beast/analysis/modules/NtuplePhase1_v4Module.h>
#include <beast/analysis/modules/BEASTTree_v3.h>

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

  REG_MODULE(NtuplePhase1_v4)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  NtuplePhase1_v4Module::NtuplePhase1_v4Module() : Module()
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

    addParam("input_sigma_HER", m_input_sigma_HER, "HER beam size");
    addParam("input_sigma_LER", m_input_sigma_LER, "LER beam size");

    addParam("input_bunchNb_HER", m_input_bunchNb_HER, "HER bunch number");
    addParam("input_bunchNb_LER", m_input_bunchNb_LER, "LER bunch number");

    addParam("input_data_bunchNb_HER", m_input_data_bunchNb_HER, "HER bunch number");
    addParam("input_data_bunchNb_LER", m_input_data_bunchNb_LER, "LER bunch number");

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

    // initialize other private data members
    m_file = NULL;
    m_tree = NULL;
    m_treeBEAST = NULL;
    m_beast.clear();

    m_numEntries = 0;
    m_entryCounter = 0;

  }

  NtuplePhase1_v4Module::~NtuplePhase1_v4Module()
  {
  }

  void NtuplePhase1_v4Module::initialize()
  {
    loadDictionaries();
    // read TFile with histograms

    // expand possible wildcards
    m_inputHistoFileNames = expandWordExpansions(m_inputHistoFileNames);
    if (m_inputFileNames.empty()) {
      B2FATAL("No valid files specified!");
    }

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
          TH1F* h1D = (TH1F*)fh[iter]->Get(HistoRateName);
          for (int i = 0; i < h1D->GetNbinsX(); i++) {
            double counts = h1D->GetBinContent(i + 1);
            double rate = counts / m_input_Time_eqv;
            /*if (HistoRateName.Contains("csi")) {
            cout << fileName << " rate " <<  rate << " counts " << counts << " m_input_Time_eqv " << m_input_Time_eqv << endl;
            }*/
            if (fileName.Contains("HER")) {
              if (HistoRateName.Contains("csi") && fileName.Contains("Touschek")) m_input_HT_CSI_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Touschek")) m_input_HT_HE3_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Brems")) m_input_HB_CSI_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Brems")) m_input_HB_HE3_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Coulomb")) m_input_HC_CSI_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Coulomb")) m_input_HC_HE3_rate_av.push_back(rate); //Hz
            }
            if (fileName.Contains("LER")) {
              if (HistoRateName.Contains("csi") && fileName.Contains("Touschek")) m_input_LT_CSI_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Touschek")) m_input_LT_HE3_rate.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Brems")) m_input_LB_CSI_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Brems")) m_input_LB_HE3_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("csi") && fileName.Contains("Coulomb")) m_input_LC_CSI_rate_av.push_back(rate); //Hz
              if (HistoRateName.Contains("Def") && fileName.Contains("Coulomb")) m_input_LC_HE3_rate_av.push_back(rate); //Hz
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
            volume = 0.265 * 0.265 * 0.01; //cm^3
            rho = 2.32; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          if (HistoDoseName.Contains("dia")) {
            imax = 4;
            volume = 0.4 * 0.4 * 0.05; //cm^3
            rho = 3.53; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          for (int i = 0; i < imax; i++) {
            TH1F* he = (TH1F*)fh[iter]->Get(TString::Format("%s_%d", HistoDoseName.Data(), i));
            double esum = 0;
            for (int j = 0; j < he->GetNbinsX(); j++) {
              double co = he->GetBinContent(j + 1);
              double va = he->GetXaxis()->GetBinCenter(j + 1);
              esum += va * co;
            }
            if (fileName.Contains("HER")) {
              if (HistoDoseName.Contains("csi")
                  && fileName.Contains("Touschek")) m_input_HT_CSI_dose.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Touschek")) m_input_HT_BGO_dose.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Touschek")) m_input_HT_PIN_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Touschek")) m_input_HT_DIA_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("csi")
                  && fileName.Contains("Brems")) m_input_HB_CSI_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Brems")) m_input_HB_BGO_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Brems")) m_input_HB_PIN_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Brems")) m_input_HB_DIA_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("csi")
                  && fileName.Contains("Coulomb")) m_input_HC_CSI_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Coulomb")) m_input_HC_BGO_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Coulomb")) m_input_HC_PIN_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Coulomb")) m_input_HC_DIA_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
            }
            if (fileName.Contains("LER")) {
              if (HistoDoseName.Contains("csi")
                  && fileName.Contains("Touschek")) m_input_LT_CSI_dose.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Touschek")) m_input_LT_BGO_dose.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Touschek")) m_input_LT_PIN_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Touschek")) m_input_LT_DIA_dose.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("csi")
                  && fileName.Contains("Brems")) m_input_LB_CSI_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Brems")) m_input_LB_BGO_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Brems")) m_input_LB_PIN_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Brems")) m_input_LB_DIA_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("csi")
                  && fileName.Contains("Coulomb")) m_input_LC_CSI_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("bgo")
                  && fileName.Contains("Coulomb")) m_input_LC_BGO_dose_av.push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
              if (HistoDoseName.Contains("pin")
                  && fileName.Contains("Coulomb")) m_input_LC_PIN_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              if (HistoDoseName.Contains("dia")
                  && fileName.Contains("Coulomb")) m_input_LC_DIA_dose_av.push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
            }
            delete he;
          }
        }
      }
      if (fileName.Contains("Coulomb") || fileName.Contains("Brems")) {
        for (const TString& HistoRateName : m_inputRateHistoNamesVrs) {
          TH2F* h2D = (TH2F*)fh[iter]->Get(HistoRateName);
          for (int k = 0; k < h2D->GetNbinsY(); k++) {
            for (int i = 0; i < h2D->GetNbinsX(); i++) {
              double counts = h2D->GetBinContent(i + 1, k + 1);
              double rate = counts / m_input_Time_eqv;
              if (fileName.Contains("Coulomb_HER")) {
                if (HistoRateName.Contains("csi")) m_input_HC_CSI_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("Def")) m_input_HC_HE3_rate[k].push_back(rate); //Hz
              }
              if (fileName.Contains("Coulomb_LER")) {
                if (HistoRateName.Contains("csi")) m_input_LC_CSI_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("Def")) m_input_LC_HE3_rate[k].push_back(rate); //Hz
              }
              if (fileName.Contains("Brems_HER")) {
                if (HistoRateName.Contains("csi")) m_input_HB_CSI_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("Def")) m_input_HB_HE3_rate[k].push_back(rate); //Hz
              }
              if (fileName.Contains("Brems_LER")) {
                if (HistoRateName.Contains("csi")) m_input_LB_CSI_rate[k].push_back(rate); //Hz
                if (HistoRateName.Contains("Def")) m_input_LB_HE3_rate[k].push_back(rate); //Hz
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
            volume = 0.265 * 0.265 * 0.01; //cm^3
            rho = 2.32; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          if (HistoDoseName.Contains("dia")) {
            imax = 4;
            volume = 0.4 * 0.4 * 0.05; //cm^3
            rho = 3.53; //g/cm^3
            mass = rho * volume * 1e-3; //g to kg
          }
          for (int i = 0; i < imax; i++) {
            TH2F* he = (TH2F*)fh[iter]->Get(TString::Format("%s_%d", HistoDoseName.Data(), i));
            for (int k = 0; k < he->GetNbinsY(); k++) {
              double esum = 0;
              for (int j = 0; j < he->GetNbinsX(); j++) {
                double co = he->GetBinContent(j + 1, k + 1);
                double va = he->GetXaxis()->GetBinCenter(j + 1);
                esum += va * co;
              }

              if (fileName.Contains("Coulomb_HER")) {
                if (HistoDoseName.Contains("csi")) m_input_HC_CSI_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("bgo")) m_input_HC_BGO_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("pin")) m_input_HC_PIN_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
                if (HistoDoseName.Contains("dia")) m_input_HC_DIA_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              }
              if (fileName.Contains("Coulomb_LER")) {
                if (HistoDoseName.Contains("csi")) m_input_LC_CSI_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("bgo")) m_input_LC_BGO_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("pin")) m_input_LC_PIN_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
                if (HistoDoseName.Contains("dia")) m_input_LC_DIA_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              }
              if (fileName.Contains("Brems_HER")) {
                if (HistoDoseName.Contains("csi")) m_input_HB_CSI_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("bgo")) m_input_HB_BGO_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("pin")) m_input_HB_PIN_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
                if (HistoDoseName.Contains("dia")) m_input_HB_DIA_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
              }
              if (fileName.Contains("Brems_LER")) {
                if (HistoDoseName.Contains("csi")) m_input_LB_CSI_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("bgo")) m_input_LB_BGO_dose[k].push_back(esum / m_input_Time_eqv * 1e-3); //MeV to GeV
                if (HistoDoseName.Contains("pin")) m_input_LB_PIN_dose[k].push_back(esum / m_input_Time_eqv / mass / RadConv * 1e-3); //keV to MeV
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
    m_tree->SetBranchAddress("PIN_dose", &(m_beast.PIN_dose));
    m_tree->SetBranchAddress("BGO_energy", &(m_beast.BGO_energy));
    m_tree->SetBranchAddress("HE3_rate", &(m_beast.HE3_rate));
    m_tree->SetBranchAddress("CSI_sumE", &(m_beast.CSI_sumE));
    m_tree->SetBranchAddress("CSI_hitRate", &(m_beast.CSI_hitRate));
    m_tree->SetBranchAddress("DIA_dose", &(m_beast.DIA_dose));

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
    m_treeBEAST->Branch("PIN_dose_mc", &(m_beast.PIN_dose_mc));
    m_treeBEAST->Branch("BGO_energy_mc", &(m_beast.BGO_energy_mc));
    m_treeBEAST->Branch("HE3_rate_mc", &(m_beast.HE3_rate_mc));
    m_treeBEAST->Branch("CSI_sumE_mc", &(m_beast.CSI_sumE_mc));
    m_treeBEAST->Branch("CSI_hitRate_mc", &(m_beast.CSI_hitRate_mc));
    m_treeBEAST->Branch("DIA_dose_mc", &(m_beast.DIA_dose_mc));

    m_treeBEAST->Branch("PIN_dose_mc_av", &(m_beast.PIN_dose_mc_av));
    m_treeBEAST->Branch("BGO_energy_mc_av", &(m_beast.BGO_energy_mc_av));
    m_treeBEAST->Branch("HE3_rate_mc_av", &(m_beast.HE3_rate_mc_av));
    m_treeBEAST->Branch("CSI_sumE_mc_av", &(m_beast.CSI_sumE_mc_av));
    m_treeBEAST->Branch("CSI_hitRate_mc_av", &(m_beast.CSI_hitRate_mc_av));
    m_treeBEAST->Branch("DIA_dose_mc_av", &(m_beast.DIA_dose_mc_av));

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
    /*
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
    */
  }


  void NtuplePhase1_v4Module::beginRun()
  {
  }


  void NtuplePhase1_v4Module::event()
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
    double ScaleFacBGav_HER = 0;
    double ScaleFacBGav_LER = 0;
    if (I_LER > 0 && P_LER > 0)
      ScaleFacBGav_LER = I_LER * P_LER / (m_input_I_LER[0] * m_input_P_LER[0]); // bunch_nb_LER / m_input_bunchNb_LER[0];
    if (I_HER > 0 && P_HER > 0)
      ScaleFacBGav_HER = I_HER * P_HER / (m_input_I_HER[0] * m_input_P_HER[0]); // bunch_nb_HER / m_input_bunchNb_HER[0];

    //Calculate Beam Gas scaling factor: Beam Gas \propo I x P => (IP)^data / (IP)^simu
    double ScaleFacBG_HER[12];
    double ScaleFacBG_LER[12];
    for (int i = 0; i < 12; i++) {
      ScaleFacBG_HER[i] = 0;
      ScaleFacBG_LER[i] = 0;
      double iP_HER = 0;
      if (m_beast.SKB_HER_pressures != 0) iP_HER = m_beast.SKB_HER_pressures->at(i) * 0.00750062 * 1e9;
      if (m_input_P_HER[1] > 0) iP_HER += gRandom->Gaus(0, m_input_P_HER[1]);
      double iP_LER = 0;
      if (m_beast.SKB_LER_pressures != 0) iP_LER = m_beast.SKB_LER_pressures->at(i) * 0.00750062 * 1e9;
      if (m_input_P_LER[1] > 0) iP_LER += gRandom->Gaus(0, m_input_P_LER[1]);
      if (iP_HER < 0) iP_HER = 0;
      if (iP_LER < 0) iP_LER = 0;
      if (I_LER > 0 && iP_LER > 0)
        ScaleFacBG_LER[i] = I_LER * iP_LER / (m_input_I_LER[0] * m_input_P_LER[0]); // bunch_nb_LER / m_input_bunchNb_LER[0];
      if (I_HER > 0 && iP_HER > 0)
        ScaleFacBG_HER[i] = I_HER * iP_HER / (m_input_I_HER[0] * m_input_P_HER[0]); // bunch_nb_HER / m_input_bunchNb_HER[0];
    }
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
      double LBG = m_input_LB_DIA_dose_av[i] + m_input_LC_DIA_dose_av[i];
      double HBG = m_input_HB_DIA_dose_av[i] + m_input_HC_DIA_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_DIA_dose[i] + ScaleFacTo_HER * m_input_HT_DIA_dose[i];
      m_beast.DIA_dose_mc_av.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_DIA_dose.size(); i++) {
      double BG = 0;
      for (int j = 0; j < 12; j++) {
        if (m_input_LB_DIA_dose[j].size() > 0) {
          double LBG = m_input_LB_DIA_dose[j][i] + m_input_LC_DIA_dose[j][i];
          double HBG = m_input_HB_DIA_dose[j][i] + m_input_HC_DIA_dose[j][i];
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      double To = ScaleFacTo_LER * m_input_LT_DIA_dose[i] + ScaleFacTo_HER * m_input_HT_DIA_dose[i];
      m_beast.DIA_dose_mc.push_back(BG + To);
    }

    //Scale PIN
    for (int i = 0; i < (int)m_input_LT_PIN_dose.size(); i++) {
      double LBG = m_input_LB_PIN_dose_av[i] + m_input_LC_PIN_dose_av[i];
      double HBG = m_input_HB_PIN_dose_av[i] + m_input_HC_PIN_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_PIN_dose[i] + ScaleFacTo_HER * m_input_HT_PIN_dose[i];
      m_beast.PIN_dose_mc_av.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_PIN_dose.size(); i++) {
      double BG = 0;
      for (int j = 0; j < 12; j++) {
        if (m_input_LB_PIN_dose[j].size() > 0) {
          double LBG = m_input_LB_PIN_dose[j][i] + m_input_LC_PIN_dose[j][i];
          double HBG = m_input_HB_PIN_dose[j][i] + m_input_HC_PIN_dose[j][i];
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      double To = ScaleFacTo_LER * m_input_LT_PIN_dose[i] + ScaleFacTo_HER * m_input_HT_PIN_dose[i];
      m_beast.PIN_dose_mc.push_back(BG + To);
    }

    //Scale BGO
    for (int i = 0; i < (int)m_input_LT_BGO_dose.size(); i++) {
      double LBG = m_input_LB_BGO_dose_av[i] + m_input_LC_BGO_dose_av[i];
      double HBG = m_input_HB_BGO_dose_av[i] + m_input_HC_BGO_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_BGO_dose[i] + ScaleFacTo_HER * m_input_HT_BGO_dose[i];
      m_beast.BGO_energy_mc_av.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_BGO_dose.size(); i++) {
      double BG = 0;
      for (int j = 0; j < 12; j++) {
        if (m_input_LB_BGO_dose[j].size() > 0) {
          double LBG = m_input_LB_BGO_dose[j][i] + m_input_LC_BGO_dose[j][i];
          double HBG = m_input_HB_BGO_dose[j][i] + m_input_HC_BGO_dose[j][i];
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      double To = ScaleFacTo_LER * m_input_LT_BGO_dose[i] + ScaleFacTo_HER * m_input_HT_BGO_dose[i];
      m_beast.BGO_energy_mc.push_back(BG + To);
    }

    //Scale HE3
    for (int i = 0; i < (int)m_input_LT_HE3_rate.size(); i++) {
      double LBG = m_input_LB_HE3_rate_av[i] + m_input_LC_HE3_rate_av[i];
      double HBG = m_input_HB_HE3_rate_av[i] + m_input_HC_HE3_rate_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_HE3_rate[i] + ScaleFacTo_HER * m_input_HT_HE3_rate[i];
      m_beast.HE3_rate_mc_av.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_HE3_rate.size(); i++) {
      double BG = 0;
      for (int j = 0; j < 12; j++) {
        if (m_input_LB_HE3_rate[j].size() > 0) {
          double LBG = m_input_LB_HE3_rate[j][i] + m_input_LC_HE3_rate[j][i];
          double HBG = m_input_HB_HE3_rate[j][i] + m_input_HC_HE3_rate[j][i];
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      double To = ScaleFacTo_LER * m_input_LT_HE3_rate[i] + ScaleFacTo_HER * m_input_HT_HE3_rate[i];
      m_beast.HE3_rate_mc.push_back(BG + To);
    }
    //Scale CSI
    for (int i = 0; i < (int)m_input_LT_CSI_dose.size(); i++) {
      double LBG = m_input_LB_CSI_dose_av[i] + m_input_LC_CSI_dose_av[i];
      double HBG = m_input_HB_CSI_dose_av[i] + m_input_HC_CSI_dose_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_CSI_dose[i] + ScaleFacTo_HER * m_input_HT_CSI_dose[i];
      m_beast.CSI_sumE_mc_av.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_CSI_rate.size(); i++) {
      double LBG = m_input_LB_CSI_rate_av[i] + m_input_LC_CSI_rate_av[i];
      double HBG = m_input_HB_CSI_rate_av[i] + m_input_HC_CSI_rate_av[i];
      double BG = LBG * ScaleFacBGav_LER + HBG * ScaleFacBGav_HER;
      double To = ScaleFacTo_LER * m_input_LT_CSI_rate[i] + ScaleFacTo_HER * m_input_HT_CSI_rate[i];
      m_beast.CSI_hitRate_mc_av.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_CSI_dose.size(); i++) {
      double BG = 0;
      for (int j = 0; j < 12; j++) {
        if (m_input_LB_CSI_dose[j].size() > 0) {
          double LBG = m_input_LB_CSI_dose[j][i] + m_input_LC_CSI_dose[j][i];
          double HBG = m_input_HB_CSI_dose[j][i] + m_input_HC_CSI_dose[j][i];
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      double To = ScaleFacTo_LER * m_input_LT_CSI_dose[i] + ScaleFacTo_HER * m_input_HT_CSI_dose[i];
      m_beast.CSI_sumE_mc.push_back(BG + To);
    }
    for (int i = 0; i < (int)m_input_LT_CSI_rate.size(); i++) {
      double BG = 0;
      for (int j = 0; j < 12; j++) {
        if (m_input_LB_CSI_rate[j].size() > 0) {
          double LBG = m_input_LB_CSI_rate[j][i] + m_input_LC_CSI_rate[j][i];
          double HBG = m_input_HB_CSI_rate[j][i] + m_input_HC_CSI_rate[j][i];
          BG += LBG * ScaleFacBG_LER[j] + HBG * ScaleFacBG_HER[j];
        }
      }
      double To = ScaleFacTo_LER * m_input_LT_CSI_rate[i] + ScaleFacTo_HER * m_input_HT_CSI_rate[i];
      m_beast.CSI_hitRate_mc.push_back(BG + To);
    }

    m_treeBEAST->Fill();

    // set event metadata
    //evtMetaData->setEvent(m_eventCount);
    //evtMetaData->setRun(m_run);
    //evtMetaData->setExperiment(m_exp);

    m_eventCount++;

  }


  void NtuplePhase1_v4Module::endRun()
  {
  }

  void NtuplePhase1_v4Module::terminate()
  {
    delete m_tree;
    m_file->cd();
    m_treeBEAST->Write();
    m_file->Close();
  }

  void NtuplePhase1_v4Module::printModuleParams() const
  {
  }

} // end Belle2 namespace

