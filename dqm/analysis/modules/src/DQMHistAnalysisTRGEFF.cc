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

  /////////////////////////////////////////////////////////
  //get histo from TRGEFFDQMModule
  m_histList.clear();
  m_histList = {
    // Add more histogram names as needed
    {"TRGEFF/hPhi_psnecl", &m_hPhi_psnecl},                          {"TRGEFF/hPhi_psnecl_ftdf", &m_hPhi_psnecl_ftdf},
    {"TRGEFF/hPt_psnecl", &m_hPt_psnecl},                            {"TRGEFF/hPt_psnecl_ftdf", &m_hPt_psnecl_ftdf},
    {"TRGEFF/nobha_hPt_psnecl", &m_nobha_hPt_psnecl},                {"TRGEFF/nobha_hPt_psnecl_ftdf", &m_nobha_hPt_psnecl_ftdf},
    {"TRGEFF/hP3_z_psnecl", &m_hP3_z_psnecl},                        {"TRGEFF/hP3_z_psnecl_ftdf", &m_hP3_z_psnecl_ftdf},
    {"TRGEFF/hP3_y_psnecl", &m_hP3_y_psnecl},                        {"TRGEFF/hP3_y_psnecl_ftdf", &m_hP3_y_psnecl_ftdf},
    {"TRGEFF/nobha_hP3_z_psnecl", &m_nobha_hP3_z_psnecl},            {"TRGEFF/nobha_hP3_z_psnecl_ftdf", &m_nobha_hP3_z_psnecl_ftdf},
    {"TRGEFF/nobha_hP3_y_psnecl", &m_nobha_hP3_y_psnecl},            {"TRGEFF/nobha_hP3_y_psnecl_ftdf", &m_nobha_hP3_y_psnecl_ftdf},
    {"TRGEFF/fyo_dphi_psnecl", &m_fyo_dphi_psnecl},                  {"TRGEFF/fyo_dphi_psnecl_ftdf", &m_fyo_dphi_psnecl_ftdf},
    {"TRGEFF/nobha_fyo_dphi_psnecl", &m_nobha_fyo_dphi_psnecl},      {"TRGEFF/nobha_fyo_dphi_psnecl_ftdf", &m_nobha_fyo_dphi_psnecl_ftdf},
    {"TRGEFF/stt_phi_psnecl", &m_stt_phi_psnecl},                    {"TRGEFF/stt_phi_psnecl_ftdf", &m_stt_phi_psnecl_ftdf},
    {"TRGEFF/stt_P3_psnecl", &m_stt_P3_psnecl},                      {"TRGEFF/stt_P3_psnecl_ftdf", &m_stt_P3_psnecl_ftdf},
    {"TRGEFF/stt_theta_psnecl", &m_stt_theta_psnecl},                {"TRGEFF/stt_theta_psnecl_ftdf", &m_stt_theta_psnecl_ftdf},
    {"TRGEFF/nobha_stt_phi_psnecl", &m_nobha_stt_phi_psnecl},        {"TRGEFF/nobha_stt_phi_psnecl_ftdf", &m_nobha_stt_phi_psnecl_ftdf},
    {"TRGEFF/nobha_stt_P3_psnecl", &m_nobha_stt_P3_psnecl},          {"TRGEFF/nobha_stt_P3_psnecl_ftdf", &m_nobha_stt_P3_psnecl_ftdf},
    {"TRGEFF/nobha_stt_theta_psnecl", &m_nobha_stt_theta_psnecl},    {"TRGEFF/nobha_stt_theta_psnecl_ftdf", &m_nobha_stt_theta_psnecl_ftdf},
    {"TRGEFF/hie_E_psnecl", &m_hie_E_psnecl},                        {"TRGEFF/hie_E_psnecl_ftdf", &m_hie_E_psnecl_ftdf},
    {"TRGEFF/nobha_hie_E_psnecl", &m_nobha_hie_E_psnecl},            {"TRGEFF/nobha_hie_E_psnecl_ftdf", &m_nobha_hie_E_psnecl_ftdf},
    {"TRGEFF/ecltiming_E_psnecl", &m_ecltiming_E_psnecl},            {"TRGEFF/ecltiming_E_psnecl_ftdf", &m_ecltiming_E_psnecl_ftdf},
    {"TRGEFF/ecltiming_theta_psnecl", &m_ecltiming_theta_psnecl},    {"TRGEFF/ecltiming_theta_psnecl_ftdf", &m_ecltiming_theta_psnecl_ftdf},
    {"TRGEFF/ecltiming_phi_psnecl", &m_ecltiming_phi_psnecl},        {"TRGEFF/ecltiming_phi_psnecl_ftdf", &m_ecltiming_phi_psnecl_ftdf},
    {"TRGEFF/klmhit_phi_psnecl", &m_klmhit_phi_psnecl},              {"TRGEFF/klmhit_phi_psnecl_ftdf", &m_klmhit_phi_psnecl_ftdf},
    {"TRGEFF/klmhit_theta_psnecl", &m_klmhit_theta_psnecl},          {"TRGEFF/klmhit_theta_psnecl_ftdf", &m_klmhit_theta_psnecl_ftdf},
    {"TRGEFF/eklmhit_phi_psnecl", &m_eklmhit_phi_psnecl},            {"TRGEFF/eklmhit_phi_psnecl_ftdf", &m_eklmhit_phi_psnecl_ftdf},
    {"TRGEFF/eklmhit_theta_psnecl", &m_eklmhit_theta_psnecl},        {"TRGEFF/eklmhit_theta_psnecl_ftdf", &m_eklmhit_theta_psnecl_ftdf}


  };

  for (auto& [name, histPtr] : m_histList) {
    // B2INFO("The name for the histogram is   " << name);
    B2DEBUG(1, "The current histogram name is   " << name);  // Debug print
    *histPtr = (TH1F*)findHist(name);
    if (*histPtr == NULL) {
      B2WARNING("Histogram named " + name + " is not found.");
      return;
    }
  }


  /////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////
  //estimate efficiency

  // Update m_efficiencyPairs with found histograms
  m_efficiencyPairs.clear();
  m_efficiencyPairs = {
    // Add more histogram pairs as needed
    std::make_tuple(m_hPhi_psnecl_ftdf,               m_hPhi_psnecl,               &m_hPhi_eff),
    std::make_tuple(m_hPt_psnecl_ftdf,                m_hPt_psnecl,                &m_hPt_eff),
    std::make_tuple(m_nobha_hPt_psnecl_ftdf,          m_nobha_hPt_psnecl,          &m_nobha_hPt_eff),
    std::make_tuple(m_hP3_z_psnecl_ftdf,              m_hP3_z_psnecl,              &m_hP3_z_eff),
    std::make_tuple(m_hP3_y_psnecl_ftdf,              m_hP3_y_psnecl,              &m_hP3_y_eff),
    std::make_tuple(m_nobha_hP3_z_psnecl_ftdf,        m_nobha_hP3_z_psnecl,        &m_nobha_hP3_z_eff),
    std::make_tuple(m_nobha_hP3_y_psnecl_ftdf,        m_nobha_hP3_y_psnecl,        &m_nobha_hP3_y_eff),
    std::make_tuple(m_fyo_dphi_psnecl_ftdf,           m_fyo_dphi_psnecl,           &m_fyo_dphi_eff),
    std::make_tuple(m_nobha_fyo_dphi_psnecl_ftdf,     m_nobha_fyo_dphi_psnecl,     &m_nobha_fyo_dphi_eff),
    std::make_tuple(m_stt_phi_psnecl_ftdf,            m_stt_phi_psnecl,            &m_stt_phi_eff),
    std::make_tuple(m_stt_P3_psnecl_ftdf,             m_stt_P3_psnecl,             &m_stt_P3_eff),
    std::make_tuple(m_stt_theta_psnecl_ftdf,          m_stt_theta_psnecl,          &m_stt_theta_eff),
    std::make_tuple(m_nobha_stt_phi_psnecl_ftdf,      m_nobha_stt_phi_psnecl,      &m_nobha_stt_phi_eff),
    std::make_tuple(m_nobha_stt_P3_psnecl_ftdf,       m_nobha_stt_P3_psnecl,       &m_nobha_stt_P3_eff),
    std::make_tuple(m_nobha_stt_theta_psnecl_ftdf,    m_nobha_stt_theta_psnecl,    &m_nobha_stt_theta_eff),
    std::make_tuple(m_hie_E_psnecl_ftdf,              m_hie_E_psnecl,              &m_hie_E_eff),
    std::make_tuple(m_nobha_hie_E_psnecl_ftdf,        m_nobha_hie_E_psnecl,        &m_nobha_hie_E_eff),
    std::make_tuple(m_ecltiming_E_psnecl_ftdf,        m_ecltiming_E_psnecl,        &m_ecltiming_E_eff),
    std::make_tuple(m_ecltiming_theta_psnecl_ftdf,    m_ecltiming_theta_psnecl,    &m_ecltiming_theta_eff),
    std::make_tuple(m_ecltiming_phi_psnecl_ftdf,      m_ecltiming_phi_psnecl,      &m_ecltiming_phi_eff),
    std::make_tuple(m_klmhit_phi_psnecl_ftdf,         m_klmhit_phi_psnecl,         &m_klmhit_phi_eff),
    std::make_tuple(m_klmhit_theta_psnecl_ftdf,       m_klmhit_theta_psnecl,       &m_klmhit_theta_eff),
    std::make_tuple(m_eklmhit_phi_psnecl_ftdf,        m_eklmhit_phi_psnecl,        &m_eklmhit_phi_eff),
    std::make_tuple(m_eklmhit_theta_psnecl_ftdf,      m_eklmhit_theta_psnecl,      &m_eklmhit_theta_eff)

  };

  for (auto& [histFtdf, hist, efficiencyPtr] : m_efficiencyPairs) {
    if (histFtdf == nullptr) {
      B2WARNING("Histogram Ftdf is nullptr for efficiency calculation.");
      return;
    }
    if (hist == nullptr) {
      B2WARNING("Histogram hist is nullptr for efficiency calculation.");
      return;
    }

    if (*efficiencyPtr != nullptr) {
      delete *efficiencyPtr;
      *efficiencyPtr = nullptr;
    }
    if (TEfficiency::CheckConsistency(*histFtdf, *hist)) {
      *efficiencyPtr = new TEfficiency(*histFtdf, *hist);
    } else {
      B2WARNING("Histograms " << histFtdf->GetName() << " and " << hist->GetName() << " are not consistent for efficiency calculation.");
    }
  }


  // Update the m_canvasEfficiencyPairs
  m_canvasEfficiencyPairs.clear();
  m_canvasEfficiencyPairs = {
    // Add more pairs as needed
    std::make_tuple(m_cPhi_eff, m_hPhi_eff),
    std::make_tuple(m_cPt_eff, m_hPt_eff),
    std::make_tuple(m_nobha_cPt_eff, m_nobha_hPt_eff),
    std::make_tuple(m_cP3_z_eff, m_hP3_z_eff),
    std::make_tuple(m_cP3_y_eff, m_hP3_y_eff),
    std::make_tuple(m_nobha_cP3_z_eff, m_nobha_hP3_z_eff),
    std::make_tuple(m_nobha_cP3_y_eff, m_nobha_hP3_y_eff),
    std::make_tuple(m_c_fyo_dphi_eff, m_fyo_dphi_eff),
    std::make_tuple(m_c_nobha_fyo_dphi_eff, m_nobha_fyo_dphi_eff),
    std::make_tuple(m_c_stt_phi_eff, m_stt_phi_eff),
    std::make_tuple(m_c_stt_P3_eff, m_stt_P3_eff),
    std::make_tuple(m_c_stt_theta_eff, m_stt_theta_eff),
    std::make_tuple(m_c_nobha_stt_phi_eff, m_nobha_stt_phi_eff),
    std::make_tuple(m_c_nobha_stt_P3_eff, m_nobha_stt_P3_eff),
    std::make_tuple(m_c_nobha_stt_theta_eff, m_nobha_stt_theta_eff),
    std::make_tuple(m_c_hie_E_eff, m_hie_E_eff),
    std::make_tuple(m_c_nobha_hie_E_eff, m_nobha_hie_E_eff),
    std::make_tuple(m_c_ecltiming_E_eff, m_ecltiming_E_eff),
    std::make_tuple(m_c_ecltiming_theta_eff, m_ecltiming_theta_eff),
    std::make_tuple(m_c_ecltiming_phi_eff, m_ecltiming_phi_eff),
    std::make_tuple(m_c_klmhit_phi_eff, m_klmhit_phi_eff),
    std::make_tuple(m_c_klmhit_theta_eff, m_klmhit_theta_eff),
    std::make_tuple(m_c_eklmhit_phi_eff, m_eklmhit_phi_eff),
    std::make_tuple(m_c_eklmhit_theta_eff, m_eklmhit_theta_eff)



  };


  for (const auto& [canvas, efficiency] : m_canvasEfficiencyPairs) {
    if (canvas && efficiency) {
      canvas->Clear();
      canvas->cd();
      efficiency->Draw();
      canvas->Modified();
    } else {
      if (!canvas) {
        B2WARNING("Canvas object is null.");
      }
      if (!efficiency) {
        B2WARNING("TEfficiency object is null.");
      }
    }
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

