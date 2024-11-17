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
#include "TLatex.h"

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
  m_c_nobha_phi_z_eff     = new TCanvas("TRGEFF/c_nobha_phi_z_eff");
  m_c_nobha_phi_y_eff     = new TCanvas("TRGEFF/c_nobha_phi_y_eff");
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
  m_mon_trgeff->addCanvas(m_cPhi_eff);
  m_mon_trgeff->addCanvas(m_nobha_cPt_eff);
  m_mon_trgeff->addCanvas(m_nobha_cP3_z_eff);
  m_mon_trgeff->addCanvas(m_nobha_cP3_y_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_phi_z_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_phi_y_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_fyo_dphi_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_stt_phi_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_stt_P3_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_stt_theta_eff);
  m_mon_trgeff->addCanvas(m_c_nobha_hie_E_eff);
  m_mon_trgeff->addCanvas(m_c_ecltiming_E_eff);
  m_mon_trgeff->addCanvas(m_c_ecltiming_theta_eff);
  m_mon_trgeff->addCanvas(m_c_ecltiming_phi_eff);
  m_mon_trgeff->addCanvas(m_c_klmhit_phi_eff);
  m_mon_trgeff->addCanvas(m_c_klmhit_theta_eff);
  m_mon_trgeff->addCanvas(m_c_eklmhit_phi_eff);
  m_mon_trgeff->addCanvas(m_c_eklmhit_theta_eff);

  ///////////////////////////////////////////////////////
  m_efficiencyList.clear();
  m_efficiencyList = {
    // Histogram names, efficiency pointers,  corresponding canvases  title and X-title
    {"TRGEFF/hPt_psnecl",             &m_hPt_eff,               m_cPt_eff,                  "f bit",            "Pt (GeV)"},
    {"TRGEFF/hPhi_psnecl",            &m_hPhi_eff,              m_cPhi_eff,                 "f bit",            "#phi (degree)"},
    {"TRGEFF/nobha_hPt_psnecl",       &m_nobha_hPt_eff,         m_nobha_cPt_eff,            "nobha f bit",      "Pt (GeV)"},
    {"TRGEFF/hP3_z_psnecl",           &m_hP3_z_eff,             m_cP3_z_eff,                "z bit",            "Pt (GeV)"},
    {"TRGEFF/hP3_y_psnecl",           &m_hP3_y_eff,             m_cP3_y_eff,                "y bit",            "Pt (GeV)"},
    {"TRGEFF/nobha_hP3_z_psnecl",     &m_nobha_hP3_z_eff,       m_nobha_cP3_z_eff,          "nobha z bit",      "Pt (GeV)"},
    {"TRGEFF/nobha_hP3_y_psnecl",     &m_nobha_hP3_y_eff,       m_nobha_cP3_y_eff,          "nobha y bit",      "Pt (GeV)"},
    {"TRGEFF/nobha_phi_z_psnecl",     &m_nobha_phi_z_eff,       m_c_nobha_phi_z_eff,        "nobha z bit",      "#phi (degree)"},
    {"TRGEFF/nobha_phi_y_psnecl",     &m_nobha_phi_y_eff,       m_c_nobha_phi_y_eff,        "nobha y bit",      "#phi (degree)"},
    {"TRGEFF/fyo_dphi_psnecl",        &m_fyo_dphi_eff,          m_c_fyo_dphi_eff,           "fyo bit",          "#Delta#phi (degree)"},
    {"TRGEFF/nobha_fyo_dphi_psnecl",  &m_nobha_fyo_dphi_eff,    m_c_nobha_fyo_dphi_eff,     "nobha fyo bit",    "#Delta#phi (degree)"},
    {"TRGEFF/stt_phi_psnecl",         &m_stt_phi_eff,           m_c_stt_phi_eff,            "stt bit",          "#phi (degree)"},
    {"TRGEFF/stt_P3_psnecl",          &m_stt_P3_eff,            m_c_stt_P3_eff,             "stt bit",          "P (GeV)"},
    {"TRGEFF/stt_theta_psnecl",       &m_stt_theta_eff,         m_c_stt_theta_eff,          "stt bit",          "#theta (degree)"},
    {"TRGEFF/nobha_stt_phi_psnecl",   &m_nobha_stt_phi_eff,     m_c_nobha_stt_phi_eff,      "nobha stt bit",    "#phi (degree)"},
    {"TRGEFF/nobha_stt_P3_psnecl",    &m_nobha_stt_P3_eff,      m_c_nobha_stt_P3_eff,       "nobha stt bit",    "P (GeV)"},
    {"TRGEFF/nobha_stt_theta_psnecl", &m_nobha_stt_theta_eff,   m_c_nobha_stt_theta_eff,    "nobha stt bit",    "#theta (degree)"},
    {"TRGEFF/hie_E_psnecl",           &m_hie_E_eff,             m_c_hie_E_eff,              "hie bit",          "E (GeV)"},
    {"TRGEFF/nobha_hie_E_psnecl",     &m_nobha_hie_E_eff,       m_c_nobha_hie_E_eff,        "nobha hie bit",    "E (GeV)"},
    {"TRGEFF/ecltiming_E_psnecl",     &m_ecltiming_E_eff,       m_c_ecltiming_E_eff,        "ecltiming bit",    "E (GeV)"},
    {"TRGEFF/ecltiming_theta_psnecl", &m_ecltiming_theta_eff,   m_c_ecltiming_theta_eff,    "ecltiming bit",    "#theta (degree)"},
    {"TRGEFF/ecltiming_phi_psnecl",   &m_ecltiming_phi_eff,     m_c_ecltiming_phi_eff,      "ecltiming bit",    "#phi (degree)"},
    {"TRGEFF/klmhit_phi_psnecl",      &m_klmhit_phi_eff,        m_c_klmhit_phi_eff,         "klmhit bit",       "#phi (degree)"},
    {"TRGEFF/klmhit_theta_psnecl",    &m_klmhit_theta_eff,      m_c_klmhit_theta_eff,       "klmhit bit",       "#theta (degree)"},
    {"TRGEFF/eklmhit_phi_psnecl",     &m_eklmhit_phi_eff,       m_c_eklmhit_phi_eff,        "eklmhit bit",      "#phi (degree)"},
    {"TRGEFF/eklmhit_theta_psnecl",   &m_eklmhit_theta_eff,     m_c_eklmhit_theta_eff,      "eklmhit bit",      "#theta (degree)"}

    // Add more entries as needed
  };

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
    const std::string& name     = std::get<0>(entry);   // Get the histogram name
    TEfficiency** efficiencyPtr = std::get<1>(entry);   // Get the efficiency pointer
    TCanvas* canvas = std::get<2>(entry);               // Get the canvas pointer
    const std::string& title    = std::get<3>(entry);   // Get the histogram title
    const std::string& xtitle   = std::get<4>(entry);   // Get the histogram X-title

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

      hist->GetYaxis()->SetNdivisions(505);
      hist->GetXaxis()->SetTitleSize(0.04);
      hist->GetYaxis()->SetTitleSize(0.04);
      hist->GetYaxis()->SetLabelSize(0.03);
      hist->GetXaxis()->SetLabelSize(0.03);
      hist->GetYaxis()->SetTitleOffset(1.);
      hist->GetXaxis()->SetTitleOffset(1.);
      hist->GetYaxis()->SetTitleFont(42);
      hist->GetXaxis()->SetTitleFont(42);
      hist->GetYaxis()->SetLabelFont(42);
      hist->GetXaxis()->SetLabelFont(42);
      hist->GetXaxis()->SetTitle(xtitle.c_str());
      hist->GetYaxis()->SetTitle("#epsilon");
      hist->GetYaxis()->SetRangeUser(0, 1);
      // hist->GetXaxis()->CenterTitle(true);
      // hist->GetYaxis()->CenterTitle(true);

      TLatex hist_title;
      hist_title.SetTextAlign(23);
      hist_title.SetTextSize(0.04);
      hist_title.SetTextFont(42);
      double pos_titleX = (hist->GetXaxis()->GetXmin() + hist->GetXaxis()->GetXmax()) / 2; // X position of histogram title
      double pos_titleY = 1.1; // Y position of histogram title

      // Draw efficiency on canvas and update it
      canvas->Clear();
      canvas->cd();

      hist->Draw("axis");
      hist_title.DrawLatex(pos_titleX, pos_titleY, title.c_str());
      (*efficiencyPtr)->Draw("same");

      canvas->Modified();

    } else {
      B2WARNING("Histograms " << histFtdf->GetName() << " and " << hist->GetName() << " are not consistent for efficiency calculation.");
      canvas->Clear();
    }
  }

}

void DQMHistAnalysisTRGEFFModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisTRGEFF : endRun called");

  // Loop through m_efficiencyList and process the efficiency histogram
  for (auto& entry : m_efficiencyList) {
    const std::string& name = std::get<0>(entry);       // Get the histogram name
    TEfficiency** efficiencyPtr = std::get<1>(entry);   // Get the efficiency pointer

    TEfficiency* effHist = *efficiencyPtr;

    if (effHist) {
      /***********************************************************
       *     rebin the histogram for MonitoringObject            *
       ***********************************************************/
      TH1F* hist     = (TH1F*)effHist->GetTotalHistogram();
      TH1F* histFtdf = (TH1F*)effHist->GetPassedHistogram();

      Double_t* newBins = nullptr;
      int nBins = 1;

      if (name.find("TRGEFF/nobha_hie_E_psnecl") != std::string::npos) {
        newBins = new Double_t[4] {0, 0.6, 1.6, hist->GetXaxis()->GetXmax()}; // Allocate on heap
        nBins = 3;
      } else if (name.find("TRGEFF/nobha_stt_P3_psnecl") != std::string::npos) {
        newBins = new Double_t[4] {0, 0.5, 1, hist->GetXaxis()->GetXmax()};
        nBins = 3;
      } else if (name.find("TRGEFF/nobha_fyo_dphi_psnecl") != std::string::npos) {
        newBins = new Double_t[4] {0, 80, 100, hist->GetXaxis()->GetXmax()};
        nBins = 3;
      } else if (name.find("TRGEFF/klmhit_theta_psnecl") != std::string::npos) {
        newBins = new Double_t[2] {0, hist->GetXaxis()->GetXmax()}; // Assuming barrel is one bin
        nBins = 1;
      } else if (name.find("TRGEFF/eklmhit_theta_psnecl") != std::string::npos) {
        newBins = new Double_t[3] {0, 90, hist->GetXaxis()->GetXmax()}; // Assuming forward and backward are two bins
        nBins = 2;
      } else {
        newBins = new Double_t[2] {0, hist->GetXaxis()->GetXmax()}; // Average of all bins
        nBins = 1;
      }

      // Sort the bins if there are more than one
      if (nBins > 1) {
        std::sort(newBins, newBins + nBins + 1); // nBins + 1 because we need to sort the edges
      }

      TEfficiency* efficiencyRebinnedPtr = nullptr;

      // rebin the found histograms
      TH1F* histRebinned     = (TH1F*)hist->Rebin(nBins, (name + "_rebinned").c_str(), newBins);
      TH1F* histFtdfRebinned = (TH1F*)histFtdf->Rebin(nBins, (name + "_ftdf_rebinned").c_str(), newBins);

      // delete the allocated memory
      delete[] newBins;

      // Check consistency and create a new TEfficiency for rebinned histograms
      if (TEfficiency::CheckConsistency(*histFtdfRebinned, *histRebinned)) {
        efficiencyRebinnedPtr = new TEfficiency(*histFtdfRebinned, *histRebinned);

        // Clean the name: remove "TRGEFF/" prefix and "_psnecl" suffix for the name in m_efficiencyList
        std::string cleanName = name;

        // Find and erase "TRGEFF/" if it exists
        size_t prefixPos = cleanName.find("TRGEFF/");
        if (prefixPos != std::string::npos) {
          cleanName.erase(prefixPos, std::string("TRGEFF/").length());  // Remove "TRGEFF/"
        }

        // Find and erase "_psnecl" if it exists
        size_t suffixPos = cleanName.find("_psnecl");
        if (suffixPos != std::string::npos) {
          cleanName.erase(suffixPos, std::string("_psnecl").length());  // Remove "_psnecl"
        }

        int nbins = efficiencyRebinnedPtr->GetTotalHistogram()->GetNbinsX();
        for (int i = 1; i <= nbins; i++) {
          char varName[100];
          sprintf(varName, "%s_%i", cleanName.c_str(), i);
          B2DEBUG(1, "The name for MonitoringObject histogram is " << varName << "  " << efficiencyRebinnedPtr->GetEfficiency(
                    i) << "   " << efficiencyRebinnedPtr->GetEfficiencyErrorUp(i) << "   " << efficiencyRebinnedPtr->GetEfficiencyErrorLow(i));
          m_mon_trgeff->setVariable(varName,
                                    efficiencyRebinnedPtr->GetEfficiency(i),
                                    efficiencyRebinnedPtr->GetEfficiencyErrorUp(i),
                                    efficiencyRebinnedPtr->GetEfficiencyErrorLow(i));
        }

        if (efficiencyRebinnedPtr != nullptr) {
          delete efficiencyRebinnedPtr;
        }

      } else {
        B2WARNING("Rebinned histograms " << histFtdfRebinned->GetName() << " and " << histRebinned->GetName() <<
                  " are not consistent for efficiency calculation.");
      }
      // Delete the rebinned histograms
      delete histRebinned;
      delete histFtdfRebinned;

    } else {
      B2WARNING(std::string("Efficiency histogram is null for ") + name);
    }
  }


}

void DQMHistAnalysisTRGEFFModule::terminate()
{
  B2DEBUG(1, "terminate called");
}

