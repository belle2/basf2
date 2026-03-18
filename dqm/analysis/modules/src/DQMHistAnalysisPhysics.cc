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
#include <RooRealVar.h>
#include <RooDataHist.h>
#include "RooCBShape.h"
#include "RooCrystalBall.h"
#include "RooChebychev.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooFitResult.h"

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
  addParam("minEntriesUPSmumu", m_minEntriesUPSmumu, "minimum number of new entries for last time slot for Ups(mumu)", 1000);
  addParam("minEntriesUPSee", m_minEntriesUPSee, "minimum number of new entries for last time slot for Ups(ee)", 1000);

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
  m_cmUPSmumu_text = new TPaveText(0.7, 0.8, 0.9, 0.9, "NDC");
  m_cmUPSmumu_text->SetFillColor(0);
  m_cmUPSmumu_text->SetFillStyle(0);
  m_cmUPSmumu_text->SetTextAlign(12);
  m_cmUPSmumu_text->SetBorderSize(0);
  m_cmUPSmumu_text->SetTextSize(0.026);
  m_cmUPSee_text = new TPaveText(0.7, 0.8, 0.9, 0.9, "NDC");
  m_cmUPSee_text->SetFillColor(0);
  m_cmUPSee_text->SetFillStyle(0);
  m_cmUPSee_text->SetTextAlign(12);
  m_cmUPSee_text->SetBorderSize(0);
  m_cmUPSee_text->SetTextSize(0.026);
  m_ratio_text = new TPaveText(0.55, 0.6, 0.85, 0.9, "NDC");
  m_ratio_text->SetFillColor(0);
  m_ratio_text->SetFillStyle(0);
  m_ratio_text->SetTextAlign(12);
  m_ratio_text->SetBorderSize(0);
  m_ratio_text->SetTextSize(0.026);

  addDeltaPar("PhysicsObjects", "mUPSe", HistDelta::c_Entries, m_minEntriesUPSee, 1);
  registerEpicsPV(m_pvPrefix + "mUPSee_mean", "mUPSee_mean");
  registerEpicsPV(m_pvPrefix + "mUPSee_width", "mUPSee_width");
  m_cmUPSee = new TCanvas("PhysicsObjects/fit_mUPSee");

  addDeltaPar("PhysicsObjects", "mUPS", HistDelta::c_Entries, m_minEntriesUPSmumu, 1);
  registerEpicsPV(m_pvPrefix + "mUPSmumu_mean", "mUPSmumu_mean");
  registerEpicsPV(m_pvPrefix + "mUPSmumu_width", "mUPSmumu_width");
  m_cmUPSmumu = new TCanvas("PhysicsObjects/fit_mUPSmumu");

  //new ratio hadronb2_tight/bhabha_all
  addDeltaPar("PhysicsObjects", "physicsresults", HistDelta::c_Events, 3000000, 1); // update each 10000 events
  registerEpicsPV(m_pvPrefix + "hadronb2_tight_over_bhabha_all", "hadronb2_tight_over_bhabha_all");

  B2DEBUG(20, "DQMHistAnalysisPhysics: initialized.");
}


void DQMHistAnalysisPhysicsModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisPhysics: beginRun called.");
}

void DQMHistAnalysisPhysicsModule::fitUpsilonFromHisto(TH1* histo, TPaveText* text, std::string parts, std::string prefix,
                                                       std::string pvname)
{
  double xMin = histo->GetXaxis()->GetXmin();
  double xMax = histo->GetXaxis()->GetXmax();
  RooRealVar m("m", parts.c_str(), xMin, xMax);

  RooDataHist data("data", "histogram", m, histo);

  RooRealVar mean("mean", "mass", 10.579, 10.5, 10.7);
  RooRealVar sigma("sigma", "resolution", 0.05, 0.01, 0.15);
  RooRealVar alphaL("alphaL", "left tail alpha", 1.5, 0.1, 5);
  RooRealVar nL("nL", "left tail n", 1.0, 0.1, 10);
  RooRealVar alphaR("alphaR", "right tail alpha", 2.0, 0.1, 5);
  RooRealVar nR("nR", "right tail n", 1.0, 0.1, 10);

  RooCrystalBall signal("signal", "Double CB", m, mean, sigma, sigma, alphaL, nL, alphaR, nR);

  RooRealVar a0("a0", "poly constant", 0.0, -1.0, 1.0);
  RooChebychev bkg("bkg", "Background PDF", m, RooArgList(a0));

  // Set the max to 2x the total entries to ensure the fit never hits a ceiling
  double nMax = histo->GetEntries() * 2.0;
  RooRealVar nsig("nsig", "yield of signal", histo->GetEntries() * 0.3, 0, nMax);
  RooRealVar nbkg("nbkg", "yield of background", histo->GetEntries() * 0.7, 0, nMax);

  RooAddPdf model("model", "sig+bkg", RooArgList(signal, bkg), RooArgList(nsig, nbkg));

  model.fitTo(data, RooFit::Extended(kTRUE));

  RooPlot* frame = m.frame();
  data.plotOn(frame, RooFit::DrawOption("B"), RooFit::FillColor(kGray), RooFit::LineWidth(1), RooFit::MarkerSize(0),
              RooFit::XErrorSize(0), RooFit::DataError(RooAbsData::None), RooFit::LineStyle(kSolid), RooFit::LineColor(kBlack));
  model.plotOn(frame, RooFit::LineColor(kBlue), RooFit::LineWidth(1));
  frame->SetTitle("Upsilon(4S) Mass Fit");
  frame->GetXaxis()->SetTitle((parts).c_str());
  frame->Draw();

  auto measuredMass = mean.getVal();
  auto massUncertainty = mean.getError();
  auto measuredWidth = sigma.getVal();

  text->Clear();
  double mean_mUPS = histo->GetMean();
  text->AddText(Form("mean : %.3f", float(mean_mUPS)));
  text->AddText(Form("fit mean : %.3f +-%.3f", float(measuredMass), float(massUncertainty)));
  text->AddText(Form("fit width : %.3f", float(measuredWidth)));
  text->Draw();
  setEpicsPV(pvname + "_mean", measuredMass);
  setEpicsPV(pvname + "_width", measuredWidth);
  m_monObj->setVariable(prefix + "mass", measuredMass, massUncertainty);
  m_monObj->setVariable(prefix + "width", measuredWidth);

}

void DQMHistAnalysisPhysicsModule::event()
{


  bool m_IsPhysicsRun = (getRunType() == "physics") || (getRunType() == "debug");
  if (m_IsPhysicsRun == true) {

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
      double error_hadron_bhabha = -10.;
      double error_hadronb2_bhabha = -10.;
      double error_hadronb2_tight_bhabha = -10.;
      double error_mumu_tight_bhabha = -10.;


      if (bhabha_all_ntot != 0) {
        ratio_hadron_bhabha = had_ntot / bhabha_all_ntot;
        error_hadron_bhabha = ratio_hadron_bhabha * sqrt((1 / had_ntot) + (1 / bhabha_all_ntot));
        ratio_hadronb2_bhabha = hadb2_ntot / bhabha_all_ntot;
        error_hadronb2_bhabha = ratio_hadronb2_bhabha * sqrt((1 / hadb2_ntot) + (1 / bhabha_all_ntot));
        ratio_hadronb2_tight_bhabha = hadb2_tight_ntot / bhabha_all_ntot;
        error_hadronb2_tight_bhabha = ratio_hadronb2_tight_bhabha * sqrt((1 / hadb2_tight_ntot) + (1 / bhabha_all_ntot));
        ratio_mumu_tight_bhabha = mumu_tight_ntot / bhabha_all_ntot;
        error_mumu_tight_bhabha = ratio_mumu_tight_bhabha * sqrt((1 / mumu_tight_ntot) + (1 / bhabha_all_ntot));
      }
      m_ratio_text->AddText(Form("hadronb2_tight/bhabha: %.4f +/- %.4f", float(ratio_hadronb2_tight_bhabha),
                                 float(error_hadronb2_tight_bhabha)));
      m_ratio_text->AddText(Form("hadronb2/bhabha: %.4f +/- %.4f", float(ratio_hadronb2_bhabha), float(error_hadronb2_bhabha)));
      m_ratio_text->AddText(Form("mumu_tight/bhabha: %.4f +/- %.4f", float(ratio_mumu_tight_bhabha), float(error_mumu_tight_bhabha)));
      m_ratio_text->AddText(Form("hadron/bhabha: %.4f +/- %.4f", float(ratio_hadron_bhabha), float(error_hadron_bhabha)));

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

    if (m_cmUPSmumu) {
      auto hmUPSmumu = getDelta("PhysicsObjects/mUPS");// check if updated
      if (hmUPSmumu) {
        m_cmUPSmumu->cd();
        fitUpsilonFromHisto(hmUPSmumu, m_cmUPSmumu_text, "M(#mu#mu) [GeV/c^2]", "UPSmumu", m_pvPrefix + "mUPSmumu");
        m_cmUPSmumu->Modified();
        m_cmUPSmumu->Update();
        UpdateCanvas(m_cmUPSmumu);
      } else {
        hmUPSmumu = findHist("PhysicsObjects/mUPS", true);// only if updated
        if (hmUPSmumu and hmUPSmumu->GetEntries() < m_minEntriesUPSmumu) {
          // only if integral plot is below delta entries
          m_cmUPSmumu->cd();
          m_cmUPSmumu->Clear();
          hmUPSmumu->Draw("hist");
        }
      }
    }
    if (m_cmUPSee) {
      auto hmUPSee = getDelta("PhysicsObjects/mUPSe");// check if updated
      if (hmUPSee) {
        m_cmUPSee->cd();
        fitUpsilonFromHisto(hmUPSee, m_cmUPSee_text, "M(ee) [GeV/c^2]", "UPSee", m_pvPrefix + "mUPSee");
        m_cmUPSee->Modified();
        m_cmUPSee->Update();
        UpdateCanvas(m_cmUPSee);
      } else {
        hmUPSee = findHist("PhysicsObjects/mUPSe", true);// only if updated
        if (hmUPSee and hmUPSee->GetEntries() < m_minEntriesUPSee) {
          // only if integral plot is below delta entries
          m_cmUPSee->cd();
          m_cmUPSee->Clear();
          hmUPSee->Draw("hist");
        }
      }
    }
    auto* m_cphysicsresults = findCanvas("PhysicsObjects/c_physicsresults");
    if (m_cphysicsresults) {
      m_cphysicsresults->cd();
      m_ratio_text->Draw();
      m_cphysicsresults->Modified();
      m_cphysicsresults->Update();
    }
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
    m_monObj->setVariable("hadronb2_tight_hlt", hadb2_tight_ntot);
    m_monObj->setVariable("bhabha_all_hlt", bhabha_all_ntot);

  }
}

void DQMHistAnalysisPhysicsModule::terminate()
{

  B2DEBUG(20, "DQMHistAnalysisPhysics: terminate called");
}

