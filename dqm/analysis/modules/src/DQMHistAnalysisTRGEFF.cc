/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <dqm/analysis/modules/DQMHistAnalysisTRGEFF.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <TLine.h>
#include <TClass.h>
#include <TStyle.h>
#include <TROOT.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisTRGEFF);

DQMHistAnalysisTRGEFFModule::DQMHistAnalysisTRGEFFModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("Modify and analyze the data quality histograms of TRGEFF");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("alert", m_enableAlert, "Enable color alert", true);
}

DQMHistAnalysisTRGEFFModule::~DQMHistAnalysisTRGEFFModule()
{

}

void DQMHistAnalysisTRGEFFModule::initialize()
{
  gROOT->cd();

  m_cPt_eff               = new TCanvas("TRGEFF/c_hPt_eff");
  m_cPhi_eff              = new TCanvas("TRGEFF/c_hPhi_eff");
  m_nobha_cPt_eff         = new TCanvas("TRGEFF/c_nobha_hPt_eff");
  m_cP3_z_eff             = new TCanvas("TRGEFF/c_hP3_z_eff");
  m_cP3_y_eff             = new TCanvas("TRGEFF/c_hP3_y_eff");
  m_nobha_cP3_z_eff       = new TCanvas("TRGEFF/c_nobha_hP3_z_eff");
  m_nobha_cP3_y_eff       = new TCanvas("TRGEFF/c_nobha_hP3_y_eff");
  m_c_fyo_dphi_eff        = new TCanvas("TRGEFF/c_fyo_dphi_eff");
  m_c_nobha_fyo_dphi_eff  = new TCanvas("TRGEFF/c_nobha_fyo_dphi_eff");
  m_c_stt_phi_eff         = new TCanvas("TRGEFF/c_stt_phi_eff");
  m_c_stt_P3_eff          = new TCanvas("TRGEFF/c_stt_P3_eff");
  m_c_stt_theta_eff       = new TCanvas("TRGEFF/c_stt_theta_eff");
  m_c_nobha_stt_phi_eff   = new TCanvas("TRGEFF/c_nobha_stt_phi_eff");
  m_c_nobha_stt_P3_eff    = new TCanvas("TRGEFF/c_nobha_stt_P3_eff");
  m_c_nobha_stt_theta_eff = new TCanvas("TRGEFF/c_nobha_stt_theta_eff");
  m_c_hie_E_eff           = new TCanvas("TRGEFF/c_hie_E_eff");
  m_c_nobha_hie_E_eff     = new TCanvas("TRGEFF/c_nobha_hie_E_eff");
  m_c_ecltiming_E_eff     = new TCanvas("TRGEFF/c_ecltiming_E_eff");
  m_c_ecltiming_theta_eff = new TCanvas("TRGEFF/c_ecltiming_theta_eff");
  m_c_ecltiming_phi_eff   = new TCanvas("TRGEFF/c_ecltiming_phi_eff");
  m_c_klmhit_phi_eff      = new TCanvas("TRGEFF/c_klmhit_phi_eff");
  m_c_klmhit_theta_eff    = new TCanvas("TRGEFF/c_klmhit_theta_eff");
  m_c_eklmhit_phi_eff     = new TCanvas("TRGEFF/c_eklmhit_phi_eff");
  m_c_eklmhit_theta_eff   = new TCanvas("TRGEFF/c_eklmhit_theta_eff");

  m_efficiencyList.clear();
  m_efficiencyList = {
    // Histogram names, efficiency pointers, and corresponding canvases
    {"TRGEFF/hPt_psnecl",             &m_hPt_eff,               m_cPt_eff},
    {"TRGEFF/hPhi_psnecl",            &m_hPhi_eff,              m_cPhi_eff},
    {"TRGEFF/nobha_hPt_psnecl",       &m_nobha_hPt_eff,         m_nobha_cPt_eff},
    {"TRGEFF/hP3_z_psnecl",           &m_hP3_z_eff,             m_cP3_z_eff},
    {"TRGEFF/hP3_y_psnecl",           &m_hP3_y_eff,             m_cP3_y_eff},
    {"TRGEFF/nobha_hP3_z_psnecl",     &m_nobha_hP3_z_eff,       m_nobha_cP3_z_eff},
    {"TRGEFF/nobha_hP3_y_psnecl",     &m_nobha_hP3_y_eff,       m_nobha_cP3_y_eff},
    {"TRGEFF/fyo_dphi_psnecl",        &m_fyo_dphi_eff,          m_c_fyo_dphi_eff},
    {"TRGEFF/nobha_fyo_dphi_psnecl",  &m_nobha_fyo_dphi_eff,    m_c_nobha_fyo_dphi_eff},
    {"TRGEFF/stt_phi_psnecl",         &m_stt_phi_eff,           m_c_stt_phi_eff},
    {"TRGEFF/stt_P3_psnecl",          &m_stt_P3_eff,            m_c_stt_P3_eff},
    {"TRGEFF/stt_theta_psnecl",       &m_stt_theta_eff,         m_c_stt_theta_eff},
    {"TRGEFF/nobha_stt_phi_psnecl",   &m_nobha_stt_phi_eff,     m_c_nobha_stt_phi_eff},
    {"TRGEFF/nobha_stt_P3_psnecl",    &m_nobha_stt_P3_eff,      m_c_nobha_stt_P3_eff},
    {"TRGEFF/nobha_stt_theta_psnecl", &m_nobha_stt_theta_eff,   m_c_nobha_stt_theta_eff},
    {"TRGEFF/hie_E_psnecl",           &m_hie_E_eff,             m_c_hie_E_eff},
    {"TRGEFF/nobha_hie_E_psnecl",     &m_nobha_hie_E_eff,       m_c_nobha_hie_E_eff},
    {"TRGEFF/ecltiming_E_psnecl",     &m_ecltiming_E_eff,       m_c_ecltiming_E_eff},
    {"TRGEFF/ecltiming_theta_psnecl", &m_ecltiming_theta_eff,   m_c_ecltiming_theta_eff},
    {"TRGEFF/ecltiming_phi_psnecl",   &m_ecltiming_phi_eff,     m_c_ecltiming_phi_eff},
    {"TRGEFF/klmhit_phi_psnecl",      &m_klmhit_phi_eff,        m_c_klmhit_phi_eff},
    {"TRGEFF/klmhit_theta_psnecl",    &m_klmhit_theta_eff,      m_c_klmhit_theta_eff},
    {"TRGEFF/eklmhit_phi_psnecl",     &m_eklmhit_phi_eff,       m_c_eklmhit_phi_eff},
    {"TRGEFF/eklmhit_theta_psnecl",   &m_eklmhit_theta_eff,     m_c_eklmhit_theta_eff}

    // Add more entries as needed
  };


  /////////////////
  // the MonitoringObject
  m_mon_trgeff = getMonitoringObject("trg");
  m_mon_trgeff->addCanvas(m_cPt_eff);
  m_mon_trgeff->addCanvas(m_cPhi_eff);
  m_mon_trgeff->addCanvas(m_nobha_cPt_eff);
  m_mon_trgeff->addCanvas(m_cP3_z_eff);
  m_mon_trgeff->addCanvas(m_cP3_y_eff);
  m_mon_trgeff->addCanvas(m_nobha_cP3_z_eff);
  m_mon_trgeff->addCanvas(m_nobha_cP3_y_eff);
  m_mon_trgeff->addCanvas(m_c_fyo_dphi_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_fyo_dphi_eff);
  m_mon_trgeff->addCanvas(m_c_stt_phi_eff);
  m_mon_trgeff->addCanvas(m_c_stt_P3_eff);
  m_mon_trgeff->addCanvas(m_c_stt_theta_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_stt_phi_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_stt_P3_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_stt_theta_eff);
  m_mon_trgeff->addCanvas(m_c_hie_E_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_hie_E_eff);
  m_mon_trgeff->addCanvas(m_c_ecltiming_E_eff);
  m_mon_trgeff->addCanvas(m_c_ecltiming_theta_eff);
  m_mon_trgeff->addCanvas(m_c_ecltiming_phi_eff);
  m_mon_trgeff->addCanvas(m_c_klmhit_phi_eff);
  m_mon_trgeff->addCanvas(m_c_klmhit_theta_eff);
  m_mon_trgeff->addCanvas(m_c_eklmhit_phi_eff);
  m_mon_trgeff->addCanvas(m_c_eklmhit_theta_eff);


  B2DEBUG(1, "DQMHistAnalysisTRGEFF: initialized.");
}


void DQMHistAnalysisTRGEFFModule::event()
{
  B2DEBUG(1, "DQMHistAnalysisTRGEFF: event start.");
  m_IsPhysicsRun = (getRunType() == "physics");
  m_IsCosmicRun  = (getRunType() == "cosmic");
  m_IsDebugRun   = (getRunType() == "debug");

  //if (m_IsPhysicsRun == true || m_IsCosmicRun == true || m_IsDebugRun == true)

  B2DEBUG(1, "DQMHistAnalysisTRGEFF: fill histogram");

  for (auto& entry : m_efficiencyList) {
    const std::string& name     = std::get<0>(entry);       // Get the histogram name
    TEfficiency** efficiencyPtr = std::get<1>(entry);   // Get the efficiency pointer
    TCanvas* canvas = std::get<2>(entry);               // Get the canvas pointer

    B2DEBUG(1, "The current histogram name is   " << name);  // Debug print
    TH1F* hist = (TH1F*)findHist(name);
    TH1F* histFtdf = (TH1F*)findHist(name + "_ftdf");
    if (hist == nullptr || histFtdf == nullptr) {
      B2WARNING("Histogram for " + name + " or its Ftdf counterpart is not found.");
      return;
    }

    // Delete old efficiency if it exists
    if (*efficiencyPtr != nullptr) {
      delete *efficiencyPtr;
      *efficiencyPtr = nullptr;
    }

    // Check consistency and create a new TEfficiency
    if (TEfficiency::CheckConsistency(*histFtdf, *hist)) {
      *efficiencyPtr = new TEfficiency(*histFtdf, *hist);
    } else {
      B2WARNING("Histograms " << histFtdf->GetName() << " and " << hist->GetName() << " are not consistent for efficiency calculation.");
    }

    // Draw efficiency on canvas and update it
    canvas->Clear();
    canvas->cd();
    (*efficiencyPtr)->Draw();
    canvas->Modified();
  }

}

void DQMHistAnalysisTRGEFFModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisTRGEFF : endRun called");

  // Vector to hold histogram names and their corresponding efficiency histograms
  std::vector<std::tuple<const char*, TEfficiency*>> efficiencyHistograms = {
    // Add more pairs as needed
    {"m_hPhi_eff", m_hPhi_eff},
    {"m_hPt_eff", m_hPt_eff},
    {"m_nobha_hPt_eff", m_nobha_hPt_eff},
    {"m_hP3_z_eff", m_hP3_z_eff},
    {"m_hP3_y_eff", m_hP3_y_eff},
    {"m_nobha_hP3_z_eff", m_nobha_hP3_z_eff},
    {"m_nobha_hP3_y_eff", m_nobha_hP3_y_eff},
    {"m_fyo_dphi_eff", m_fyo_dphi_eff},
    {"m_nobha_fyo_dphi_eff", m_nobha_fyo_dphi_eff},
    {"m_stt_phi_eff", m_stt_phi_eff},
    {"m_stt_P3_eff", m_stt_P3_eff},
    {"m_stt_theta_eff", m_stt_theta_eff},
    {"m_nobha_stt_phi_eff", m_nobha_stt_phi_eff},
    {"m_nobha_stt_P3_eff", m_nobha_stt_P3_eff},
    {"m_nobha_stt_theta_eff", m_nobha_stt_theta_eff},
    {"m_hie_E_eff", m_hie_E_eff},
    {"m_nobha_hie_E_eff", m_nobha_hie_E_eff},
    {"m_ecltiming_E_eff", m_ecltiming_E_eff},
    {"m_ecltiming_theta_eff", m_ecltiming_theta_eff},
    {"m_ecltiming_phi_eff", m_ecltiming_phi_eff},
    {"m_klmhit_phi_eff", m_klmhit_phi_eff},
    {"m_klmhit_theta_eff", m_klmhit_theta_eff},
    {"m_eklmhit_phi_eff", m_eklmhit_phi_eff},
    {"m_eklmhit_theta_eff", m_eklmhit_theta_eff}


  };

  // Loop through each histogram and process the efficiency data
  for (const auto& [name, effHist] : efficiencyHistograms) {
    if (effHist) {
      int nbins = effHist->GetTotalHistogram()->GetNbinsX();
      for (int i = 0; i < nbins; i++) {
        char varName[100];
        sprintf(varName, "%s_%i", name, i);
        m_mon_trgeff->setVariable(varName,
                                  effHist ? effHist->GetEfficiency(i) : 0,
                                  effHist ? effHist->GetEfficiencyErrorUp(i) : -1,
                                  effHist ? effHist->GetEfficiencyErrorLow(i) : -1);
      }
    } else {
      char warningMessage[200];
      sprintf(warningMessage, "Efficiency histogram is null for %s", name);
      B2WARNING(warningMessage);
    }
  }
}

void DQMHistAnalysisTRGEFFModule::terminate()
{
  B2DEBUG(1, "terminate called");
}

