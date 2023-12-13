/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisTRGModule.cc
// Description : Module for TRG
//-


#include <dqm/analysis/modules/DQMHistAnalysisTRG.h>
#include <TROOT.h>
#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTRG);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTRGModule::DQMHistAnalysisTRGModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  // Parameter definition
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("TRG:"));
  B2DEBUG(1, "DQMHistAnalysisTRGModule: Constructor done.");

}

DQMHistAnalysisTRGModule::~DQMHistAnalysisTRGModule()
{
  // destructor not needed
  // EPICS singleton deletion not urgent -> can be done by framework
}

void DQMHistAnalysisTRGModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: initialized.");

  gROOT->cd();
  m_canvas = new TCanvas("TRG/c_Test");
  //comL1-GDLL1
  addDeltaPar("TRGGDL", "hGDL_gdlL1TocomL1_all", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "comL1_GDLL1_mean", "comL1_GDLL1_mean");

  //ECLTRG timing
  addDeltaPar("EventT0DQMdir", "m_histEventT0_TOP_hadron_L1_ECLTRG", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_timing_mean", "ECLTRG_timing_mean");

  //CDCTRG event timing
  addDeltaPar("EventT0DQMdir", "m_histEventT0_TOP_hadron_L1_CDCTRG", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "CDCTRG_timing_mean", "CDCTRG_timing_mean");

  //TOPTRG event timing
  addDeltaPar("EventT0DQMdir", "m_histEventT0_TOP_hadron_L1_TOPTRG", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "TOPTRG_timing_mean", "TOPTRG_timing_mean");

  //ECLTRG_peak
  addDeltaPar("TRGGRL", "h_ECLL1", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_peak", "ECLTRG_peak");

  //CDCTRG_2D_peak
  addDeltaPar("TRGGRL", "h_CDCL1", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "CDCTRG_2D_peak", "CDCTRG_2D_peak");

  //NN_peak
  addDeltaPar("TRGGRL", "h_CDCNNL1", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "NN_peak", "NN_peak");

  //CDCTRG_TSF_peak
  addDeltaPar("TRGGRL", "h_TSFL1", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "CDCTRG_TSF_peak", "CDCTRG_TSF_peak");

  //KLMTRG_peak
  addDeltaPar("TRGGRL", "h_KLML1", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "KLMTRG_peak", "KLMTRG_peak");

  //TOPTRG_peak
  addDeltaPar("TRGGRL", "h_TOPL1", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "TOPTRG_peak", "TOPTRG_peak");


  //hadronb2_over_bhabha_all
  addDeltaPar("softwaretrigger", "skim", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "hadronb2_over_bhabha_all", "hadronb2_over_bhabha_all");

  //mumu2trk_over_bhabha_all
  addDeltaPar("softwaretrigger", "skim", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "mumu2trk_over_bhabha_all", "mumu2trk_over_bhabha_all");

  //hadronb2_over_mumu2trk
  addDeltaPar("softwaretrigger", "skim", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "hadronb2_over_mumu2trk", "hadronb2_over_mumu2trk");

  //deadch_c_h_TCId
  addDeltaPar("TRG", "c_h_TCId", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "deadch_c_h_TCId", "deadch_c_h_TCId");

  //deadch_c_NeuroHWInTSID
  addDeltaPar("TRGCDCTNN", "c_NeuroHWInTSID", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "deadch_c_NeuroHWInTSID", "deadch_c_NeuroHWInTSID");

  //update PV
//  updateEpicsPVs(
//    5.0); // -> now trigger update. this may be optional, framework can take care unless we want to now the result immediately
}

void DQMHistAnalysisTRGModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: beginRun called.");
}

void DQMHistAnalysisTRGModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: endRun called.");
}

void DQMHistAnalysisTRGModule::event()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: event called.");
  doHistAnalysis();
}

void DQMHistAnalysisTRGModule::doHistAnalysis()
{
  m_canvas->Clear();
  m_canvas->cd(0);
  //update comL1-GDLL1
  auto hist_comL1_GDLL1 = getDelta("TRGGDL", "hGDL_gdlL1TocomL1_all", 0, true);// only if updated
  if (hist_comL1_GDLL1) {
    double comL1_GDLL1_mean = 0.0;
    hist_comL1_GDLL1->Draw();
    comL1_GDLL1_mean = hist_comL1_GDLL1->GetMean();
    B2DEBUG(1, "comL1_GDLL1_mean:" << comL1_GDLL1_mean);
    setEpicsPV("comL1_GDLL1_mean", comL1_GDLL1_mean);
//    updateEpicsPVs(
//      5.0); // -> now trigger update. this may be optional, framework can take care unless we want to now the result immediately
  }

  //update ECLTRG timing
  auto hist =  getDelta("EventT0DQMdir", "m_histEventT0_TOP_hadron_L1_ECLTRG", 0, true);// only if updated
  if (hist) {
    double ECLTRG_timing_mean = 0.0;
    hist->Draw();
    ECLTRG_timing_mean = hist->GetMean();
    B2DEBUG(1, "ECLTRG_timing_mean:" << ECLTRG_timing_mean);
    setEpicsPV("ECLTRG_timing_mean", ECLTRG_timing_mean);
  }


  //update CDCTRG timing
  auto histCDCTRG =  getDelta("EventT0DQMdir", "m_histEventT0_TOP_hadron_L1_CDCTRG", 0, true);// only if updated
  if (histCDCTRG) {
    double CDCTRG_timing_mean = 0.0;
    histCDCTRG->Draw();
    CDCTRG_timing_mean = histCDCTRG->GetMean();
    B2DEBUG(1, "CDCTRG_timing_mean:" << CDCTRG_timing_mean);
    setEpicsPV("CDCTRG_timing_mean", CDCTRG_timing_mean);
  }
  cout << "histCDCTRG = " << histCDCTRG << endl;

  //update TOPTRG timing
  auto histTOPTRG =  getDelta("EventT0DQMdir", "m_histEventT0_TOP_hadron_L1_TOPTRG", 0, true);// only if updated
  if (histTOPTRG) {
    double TOPTRG_timing_mean = 0.0;
    histTOPTRG->Draw();
    TOPTRG_timing_mean = histTOPTRG->GetMean();
    B2DEBUG(1, "TOPTRG_timing_mean:" << TOPTRG_timing_mean);
    setEpicsPV("TOPTRG_timing_mean", TOPTRG_timing_mean);
  }

// update ECLTRG peak
  auto hist_ECLTRG_peak =  getDelta("TRGGRL", "h_ECLL1", 0, true);// only if updated
  if (hist_ECLTRG_peak) {
    double ECLTRG_peak = 0.0;
    hist_ECLTRG_peak->Draw();
    int bin_ECLTRG_peak = hist_ECLTRG_peak->GetMaximumBin();
    ECLTRG_peak = hist_ECLTRG_peak->GetXaxis()->GetBinCenter(bin_ECLTRG_peak);
    B2DEBUG(1, "ECLTRG_peak:" << ECLTRG_peak);
    setEpicsPV("ECLTRG_peak", ECLTRG_peak);

  }

// update CDCTRG 2D peak
  auto hist_CDCTRG_2D_peak =  getDelta("TRGGRL", "h_CDCL1", 0, true);// only if updated
  if (hist_CDCTRG_2D_peak) {
    double CDCTRG_2D_peak = 0.0;
    hist_CDCTRG_2D_peak->Draw();
    int bin_CDCTRG_2D_peak = hist_CDCTRG_2D_peak->GetMaximumBin();
    CDCTRG_2D_peak = hist_CDCTRG_2D_peak->GetXaxis()->GetBinCenter(bin_CDCTRG_2D_peak);
    B2DEBUG(1, "CDCTRG_2D_peak:" << CDCTRG_2D_peak);
    setEpicsPV("CDCTRG_2D_peak", CDCTRG_2D_peak);

  }

// update CDCTRG NN peak
  auto hist_NN_peak =  getDelta("TRGGRL", "h_CDCNNL1", 0, true);// only if updated
  if (hist_NN_peak) {
    double NN_peak = 0.0;
    hist_NN_peak->Draw();
    int bin_NN_peak = hist_NN_peak->GetMaximumBin();
    NN_peak = hist_NN_peak->GetXaxis()->GetBinCenter(bin_NN_peak);
    B2DEBUG(1, "NN_peak:" << NN_peak);
    setEpicsPV("NN_peak", NN_peak);

  }

// update CDCTRG TSF
  auto hist_CDCTRG_TSF_peak =  getDelta("TRGGRL", "h_TSFL1", 0, true);// only if updated
  if (hist_CDCTRG_TSF_peak) {
    double CDCTRG_TSF_peak = 0.0;
    hist_CDCTRG_TSF_peak->Draw();
    int bin_CDCTRG_TSF_peak = hist_CDCTRG_TSF_peak->GetMaximumBin();
    CDCTRG_TSF_peak = hist_CDCTRG_TSF_peak->GetXaxis()->GetBinCenter(bin_CDCTRG_TSF_peak);
    B2DEBUG(1, "CDCTRG_TSF_peak:" << CDCTRG_TSF_peak);
    setEpicsPV("CDCTRG_TSF_peak", CDCTRG_TSF_peak);

  }

// update KLMTRG
  auto hist_KLMTRG_peak =  getDelta("TRGGRL", "h_KLML1", 0, true);// only if updated
  if (hist_KLMTRG_peak) {
    double KLMTRG_peak = 0.0;
    hist_KLMTRG_peak->Draw();
    int bin_KLMTRG_peak = hist_KLMTRG_peak->GetMaximumBin();
    KLMTRG_peak = hist_KLMTRG_peak->GetXaxis()->GetBinCenter(bin_KLMTRG_peak);
    B2DEBUG(1, "KLMTRG_peak:" << KLMTRG_peak);
    setEpicsPV("KLMTRG_peak", KLMTRG_peak);

  }

// update TOPTRG
  auto hist_TOPTRG_peak =  getDelta("TRGGRL", "h_TOPL1", 0, true);// only if updated
  if (hist_TOPTRG_peak) {
    double TOPTRG_peak = 0.0;
    hist_TOPTRG_peak->Draw();
    int bin_TOPTRG_peak = hist_TOPTRG_peak->GetMaximumBin();
    TOPTRG_peak = hist_TOPTRG_peak->GetXaxis()->GetBinCenter(bin_TOPTRG_peak);
    B2DEBUG(1, "TOPTRG_peak:" << TOPTRG_peak);
    setEpicsPV("TOPTRG_peak", TOPTRG_peak);

  }

// update #hadronb2/#bhabha_all
  auto hist_hadronb2_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
  if (hist_hadronb2_over_bhabha_all) {
    double hadronb2_over_bhabha_all = 0.0;
    hist_hadronb2_over_bhabha_all->Draw();
    hadronb2_over_bhabha_all = hist_hadronb2_over_bhabha_all->GetBinContent(
                                 hist_hadronb2_over_bhabha_all->GetXaxis()->FindBin("accept_hadronb2")) / hist_hadronb2_over_bhabha_all->GetBinContent(
                                 hist_hadronb2_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));

    B2DEBUG(1, "hadronb2_over_bhabha_all:" << hadronb2_over_bhabha_all);
    setEpicsPV("hadronb2_over_bhabha_all", hadronb2_over_bhabha_all);

  }

// update #mumu2trk/#bhabha_all
  auto hist_mumu2trk_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
  if (hist_mumu2trk_over_bhabha_all) {
    double mumu2trk_over_bhabha_all = 0.0;
    hist_mumu2trk_over_bhabha_all->Draw();
    mumu2trk_over_bhabha_all = hist_mumu2trk_over_bhabha_all->GetBinContent(
                                 hist_mumu2trk_over_bhabha_all->GetXaxis()->FindBin("accept_mumu_2trk")) / hist_mumu2trk_over_bhabha_all->GetBinContent(
                                 hist_mumu2trk_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
    B2DEBUG(1, "mumu2trk_over_bhabha_all:" << mumu2trk_over_bhabha_all);
    setEpicsPV("mumu2trk_over_bhabha_all", mumu2trk_over_bhabha_all);

  }

// update #hadronb2/#mumu2trk
  auto hist_hadronb2_over_mumu2trk =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
  if (hist_hadronb2_over_mumu2trk) {
    double hadronb2_over_mumu2trk = 0.0;
    hist_hadronb2_over_mumu2trk->Draw();
    hadronb2_over_mumu2trk = hist_hadronb2_over_mumu2trk->GetBinContent(
                               hist_hadronb2_over_mumu2trk->GetXaxis()->FindBin("accept_hadronb2")) / hist_hadronb2_over_mumu2trk->GetBinContent(
                               hist_hadronb2_over_mumu2trk->GetXaxis()->FindBin("accept_mumu_2trk"));
    B2DEBUG(1, "hadronb2_over_mumu2trk:" << hadronb2_over_mumu2trk);
    setEpicsPV("hadronb2_over_mumu2trk", hadronb2_over_mumu2trk);

  }

// update #deadch_c_h_TCId
  auto hist_c_h_TCId =  getDelta("TRG", "c_h_TCId", 0, true);// only if updated
  if (hist_c_h_TCId) {
    double deadch_c_h_TCId = 0.0;
    hist_c_h_TCId->Draw();
    deadch_c_h_TCId = hist_c_h_TCId->GetMaximum() * 0.01;
    B2DEBUG(1, "deadch_c_h_TCId:" << deadch_c_h_TCId);
    setEpicsPV("deadch_c_h_TCId", deadch_c_h_TCId);

  }

// update #deadch_c_NeuroHWInTSID
  auto hist_c_NeuroHWInTSID =  getDelta("TRGCDCTNN", "c_NeuroHWInTSID", 0, true);// only if updated
  if (hist_c_NeuroHWInTSID) {
    double deadch_c_NeuroHWInTSID = 0.0;
    hist_c_NeuroHWInTSID->Draw();
    deadch_c_NeuroHWInTSID = hist_c_NeuroHWInTSID->GetMaximum() * 0.01;
    B2DEBUG(1, "deadch_c_NeuroHWInTSID:" << deadch_c_NeuroHWInTSID);
    setEpicsPV("deadch_c_NeuroHWInTSID", deadch_c_NeuroHWInTSID);

  }


// update #mumu_tight/#bhabha_all
  auto hist_mumu_tight_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
  if (hist_mumu_tight_over_bhabha_all) {
    double mumu_tight_over_bhabha_all = 0.0;
    hist_mumu_tight_over_bhabha_all->Draw();
    mumu_tight_over_bhabha_all = hist_mumu_tight_over_bhabha_all->GetBinContent(
                                   hist_mumu_tight_over_bhabha_all->GetXaxis()->FindBin("accept_mumutight")) / hist_mumu_tight_over_bhabha_all->GetBinContent(
                                   hist_mumu_tight_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
    B2DEBUG(1, "mumu_tight_over_bhabha_all:" << mumu_tight_over_bhabha_all);
    setEpicsPV("mumu_tight_over_bhabha_all", mumu_tight_over_bhabha_all);

  }

// update #gammagamma/#bhabha_all
  auto hist_gammagamma_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
  if (hist_gammagamma_over_bhabha_all) {
    double gammagamma_over_bhabha_all = 0.0;
    hist_gammagamma_over_bhabha_all->Draw();
    gammagamma_over_bhabha_all = hist_gammagamma_over_bhabha_all->GetBinContent(
                                   hist_gammagamma_over_bhabha_all->GetXaxis()->FindBin("accept_gamma_gamma")) / hist_gammagamma_over_bhabha_all->GetBinContent(
                                   hist_gammagamma_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
    B2DEBUG(1, "gammagamma_over_bhabha_all:" << gammagamma_over_bhabha_all);
    setEpicsPV("gammagamma_over_bhabha_all", gammagamma_over_bhabha_all);

  }

// update #tautau2trk/#bhabha_all
  auto hist_tautau2trk_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
  if (hist_tautau2trk_over_bhabha_all) {
    double tautau2trk_over_bhabha_all = 0.0;
    hist_tautau2trk_over_bhabha_all->Draw();
    tautau2trk_over_bhabha_all = hist_tautau2trk_over_bhabha_all->GetBinContent(
                                   hist_tautau2trk_over_bhabha_all->GetXaxis()->FindBin("accept_tau_2trk")) / hist_tautau2trk_over_bhabha_all->GetBinContent(
                                   hist_tautau2trk_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
    B2DEBUG(1, "tautau2trk_over_bhabha_all:" << tautau2trk_over_bhabha_all);
    setEpicsPV("tautau2trk_over_bhabha_all", tautau2trk_over_bhabha_all);

  }

// update #hadron/#bhabha_all
  auto hist_hadron_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
  if (hist_hadron_over_bhabha_all) {
    double hadron_over_bhabha_all = 0.0;
    hist_hadron_over_bhabha_all->Draw();
    hadron_over_bhabha_all = hist_hadron_over_bhabha_all->GetBinContent(
                               hist_hadron_over_bhabha_all->GetXaxis()->FindBin("accept_hadron")) / hist_hadron_over_bhabha_all->GetBinContent(
                               hist_hadron_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
    B2DEBUG(1, "hadron_over_bhabha_all:" << hadron_over_bhabha_all);
    setEpicsPV("hadron_over_bhabha_all", hadron_over_bhabha_all);

  }



  // Tag canvas as updated ONLY if things have changed.
  UpdateCanvas(m_canvas->GetName(), hist != nullptr);

  // this if left over from jsroot, may not be needed anymore (to check)
  m_canvas->Update();

}

void DQMHistAnalysisTRGModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: terminate called");
}
