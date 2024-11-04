/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPhysics.cc
// Description : DQM module, for the physics histograms at hlt level
//-


#include <dqm/analysis/modules/DQMHistAnalysisPhysics.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPhysics);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPhysicsModule::DQMHistAnalysisPhysicsModule() : DQMHistAnalysisModule()
{
  // Description
  // Parameter definition
  setDescription("DQM Analysis for Physics histograms");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("Physics:"));

  B2DEBUG(20, "DQMHistAnalysisPhysics: Constructor done.");
}

DQMHistAnalysisPhysicsModule::~DQMHistAnalysisPhysicsModule()
{
}

void DQMHistAnalysisPhysicsModule::initialize()
{
  gROOT->cd();
  m_monObj = getMonitoringObject("physics_hlt");
  //new text for plots
  m_cmUPS_text = new TPaveText(0.7, 0.8, 0.9, 0.9, "NDC");
  m_cmUPS_text->SetFillColor(0);
  m_cmUPS_text->SetFillStyle(0);
  m_cmUPS_text->SetTextAlign(12);
  m_cmUPS_text->SetBorderSize(0);
  m_cmUPS_text->SetTextSize(0.026);
  m_cmUPSe_text = new TPaveText(0.7, 0.8, 0.9, 0.9, "NDC");
  m_cmUPSe_text->SetFillColor(0);
  m_cmUPSe_text->SetFillStyle(0);
  m_cmUPSe_text->SetTextAlign(12);
  m_cmUPSe_text->SetBorderSize(0);
  m_cmUPSe_text->SetTextSize(0.026);
  m_ratio_text = new TPaveText(0.6, 0.6, 0.9, 0.9, "NDC");
  m_ratio_text->SetFillColor(0);
  m_ratio_text->SetFillStyle(0);
  m_ratio_text->SetTextAlign(12);
  m_ratio_text->SetBorderSize(0);
  m_ratio_text->SetTextSize(0.026);

  //new ratio hadronb2_tight/bhabha_all
  addDeltaPar("PhysicsObjects", "physicsresults", HistDelta::c_Events, 3000000, 1); // update each 10000 events
  registerEpicsPV(m_pvPrefix + "hadronb2_tight_over_bhabha_all", "hadronb2_tight_over_bhabha_all");

  B2DEBUG(20, "DQMHistAnalysisPhysics: initialized.");
}


void DQMHistAnalysisPhysicsModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisPhysics: beginRun called.");
}

void DQMHistAnalysisPhysicsModule::event()
{
  m_cmUPS_text->Clear();
  auto m_hmUPS = findHist("PhysicsObjects/mUPS", true);// check if updated
  if (m_hmUPS) {
    double mean_mUPS = m_hmUPS->GetMean();
    m_cmUPS_text->AddText(Form("mean : %.2f", float(mean_mUPS)));

  }
  m_cmUPSe_text->Clear();
  auto m_hmUPSe = findHist("PhysicsObjects/mUPSe", true);// check if updated
  if (m_hmUPSe) {
    double mean_mUPSe = m_hmUPSe->GetMean();
    m_cmUPSe_text->AddText(Form("mean : %.2f", float(mean_mUPSe)));
  }
  m_ratio_text->Clear();
  auto m_hphysicsresults = findHist("PhysicsObjects/physicsresults", true);// check if updated
  if (m_hphysicsresults) {
    double had_ntot = m_hphysicsresults->GetBinContent(2);
    double hadb2_ntot = m_hphysicsresults->GetBinContent(3);
    double hadb2_tight_ntot = m_hphysicsresults->GetBinContent(4);
    double mumu_tight_ntot = m_hphysicsresults->GetBinContent(5);
    double bhabha_all_ntot = m_hphysicsresults->GetBinContent(6);

    double ratio_hadron_bhabha = 0.;
    double ratio_hadronb2_bhabha = 0.;
    double ratio_hadronb2_tight_bhabha = 0.;
    double ratio_mumu_tight_bhabha = 0.;

    if (bhabha_all_ntot != 0) {
      ratio_hadron_bhabha = had_ntot / bhabha_all_ntot;
      ratio_hadronb2_bhabha = hadb2_ntot / bhabha_all_ntot;
      ratio_hadronb2_tight_bhabha = hadb2_tight_ntot / bhabha_all_ntot;
      ratio_mumu_tight_bhabha = mumu_tight_ntot / bhabha_all_ntot;
    }
    m_ratio_text->AddText(Form("hadronb2_tight/bhabha: %.4f", float(ratio_hadronb2_tight_bhabha)));
    m_ratio_text->AddText(Form("hadronb2/bhabha: %.4f", float(ratio_hadronb2_bhabha)));
    m_ratio_text->AddText(Form("mumu_tight/bhabha: %.4f", float(ratio_mumu_tight_bhabha)));
    m_ratio_text->AddText(Form("hadron/bhabha: %.4f", float(ratio_hadron_bhabha)));

  }
  // for pv #new hadronb2_tight/#bhabha_all
  auto hist_hadronb2_tight_over_bhabha_all =  getDelta("PhysicsObjects", "physicsresults", 0, true);// only if updated
  if (hist_hadronb2_tight_over_bhabha_all) {
    if (hist_hadronb2_tight_over_bhabha_all->GetBinContent(6) != 0) {
      double hadronb2_tight_over_bhabha_all = hist_hadronb2_tight_over_bhabha_all->GetBinContent(4) /
                                              hist_hadronb2_tight_over_bhabha_all->GetBinContent(6);
      B2DEBUG(1, "hadronb2_tight_over_bhabha_all:" << hadronb2_tight_over_bhabha_all);
      setEpicsPV("hadronb2_tight_over_bhabha_all", hadronb2_tight_over_bhabha_all);
    }
  }

  auto* m_cmUPS = findCanvas("PhysicsObjects/c_mUPS");
  if (m_cmUPS) {
    m_cmUPS->cd();
    m_cmUPS_text->Draw();
    m_cmUPS->Modified();
    m_cmUPS->Update();
  }
  auto* m_cmUPSe = findCanvas("PhysicsObjects/c_mUPSe");
  if (m_cmUPSe) {
    m_cmUPSe->cd();
    m_cmUPSe_text->Draw();
    m_cmUPSe->Modified();
    m_cmUPSe->Update();
  }
  auto* m_cphysicsresults = findCanvas("PhysicsObjects/c_physicsresults");
  if (m_cphysicsresults) {
    m_cphysicsresults->cd();
    m_ratio_text->Draw();
    m_cphysicsresults->Modified();
    m_cphysicsresults->Update();
  }
}
void DQMHistAnalysisPhysicsModule::endRun()
{
  auto m_hphysicsresults = findHist("PhysicsObjects/physicsresults");
  if (m_hphysicsresults) {
    double had_ntot = m_hphysicsresults->GetBinContent(2);
    double hadb2_ntot = m_hphysicsresults->GetBinContent(3);
    double hadb2_tight_ntot = m_hphysicsresults->GetBinContent(4);
    double mumu_tight_ntot = m_hphysicsresults->GetBinContent(5);
    double bhabha_all_ntot = m_hphysicsresults->GetBinContent(6);
    double ratio_hadron_bhabha_final = 0.;
    double ratio_hadronb2_bhabha_final = 0.;
    double ratio_hadronb2_tight_bhabha_final = 0.;
    double ratio_mumu_tight_bhabha_final = 0.;

    if (bhabha_all_ntot != 0) {
      ratio_hadron_bhabha_final = had_ntot / bhabha_all_ntot;
      ratio_hadronb2_bhabha_final = hadb2_ntot / bhabha_all_ntot;
      ratio_hadronb2_tight_bhabha_final = hadb2_tight_ntot / bhabha_all_ntot;
      ratio_mumu_tight_bhabha_final = mumu_tight_ntot / bhabha_all_ntot;
    }
    m_monObj->setVariable("ratio_hadron_bhabha_hlt", ratio_hadron_bhabha_final);
    m_monObj->setVariable("ratio_hadronb2_bhabha_hlt", ratio_hadronb2_bhabha_final);
    m_monObj->setVariable("ratio_hadronb2_tight_bhabha_hlt", ratio_hadronb2_tight_bhabha_final);
    m_monObj->setVariable("ratio_mumu_tight_bhabha_hlt", ratio_mumu_tight_bhabha_final);

  }
}

void DQMHistAnalysisPhysicsModule::terminate()
{

  B2DEBUG(20, "DQMHistAnalysisPhysics: terminate called");
}

