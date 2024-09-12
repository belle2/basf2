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

  //CDCTRG nTSFHits total
  addDeltaPar("TRGGRL", "h_wirecnt_sum", HistDelta::c_Entries, 1000, 1);
  registerEpicsPV(m_pvPrefix + "CDCTRG_meanTSFHits_total", "CDCTRG_meanTSFHits_total");

  //CDCTRG nTSFHits Injection BG Clean region
  addDeltaPar("TRGGRL", "h_wirecnt_sum_clean", HistDelta::c_Entries, 1000, 1);
  registerEpicsPV(m_pvPrefix + "CDCTRG_meanTSFHits_clean", "CDCTRG_meanTSFHits_clean");

  //CDCTRG nTSFHits HER Injection region
  addDeltaPar("TRGGRL", "h_wirecnt_sum_injHER", HistDelta::c_Entries, 1000, 1);
  registerEpicsPV(m_pvPrefix + "CDCTRG_meanTSFHits_injHER", "CDCTRG_meanTSFHits_injHER");

  //CDCTRG nTSFHits LER Injection region
  addDeltaPar("TRGGRL", "h_wirecnt_sum_injLER", HistDelta::c_Entries, 1000, 1);
  registerEpicsPV(m_pvPrefix + "CDCTRG_meanTSFHits_injLER", "CDCTRG_meanTSFHits_injLER");

  //ecl timing –cdc timing
  addDeltaPar("TRGGDL", "hGDL_ns_cdcToecl_all", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "ecltiming_cdctiming", "ecltiming_cdctiming");

  //top timing – ecl timing
  addDeltaPar("TRGGDL", "hGDL_ns_topToecl_all", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "toptiming_ecltiming", "toptiming_ecltiming");

  // top timing – cdc timing
  addDeltaPar("TRGGDL", "hGDL_ns_topTocdc_all", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "toptiming_cdctiming", "toptiming_cdctiming");

  // gdll1-ecl timing
  addDeltaPar("TRGGDL", "hGDL_eclTogdlL1_all", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "gdll1_ecltiming", "gdll1_ecltiming");

  //gdll1-cdctiming
  addDeltaPar("TRGGDL", "hGDL_cdcTogdlL1_all", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "gdll1_cdctiming", "gdll1_cdctiming");

  //gdll1-toptiming
  addDeltaPar("TRGGDL", "hGDL_topTogdlL1_all", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "gdll1_toptiming", "gdll1_toptiming");

  //barrel klm latency
  addDeltaPar("TRGGDL", "hGDL_itd_klm_hit_rise_all", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "barrel_klm_latency", "barrel_klm_latency");

  //endcap klm latency
  addDeltaPar("TRGGDL", "hGDL_itd_eklm_hit_rise_all", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "endcap_klm_latency", "endcap_klm_latency");

  //hadronb2_over_bhabha_all
  addDeltaPar("softwaretrigger", "skim", HistDelta::c_Entries, 1000000, 1); // update each 1000000 entries
  registerEpicsPV(m_pvPrefix + "hadronb2_over_bhabha_all", "hadronb2_over_bhabha_all");

  //mumu2trk_over_bhabha_all
  addDeltaPar("softwaretrigger", "skim", HistDelta::c_Entries, 1000000, 1); // update each 1000000 entries
  registerEpicsPV(m_pvPrefix + "mumu2trk_over_bhabha_all", "mumu2trk_over_bhabha_all");

  //hadronb2_over_mumu2trk
  addDeltaPar("softwaretrigger", "skim", HistDelta::c_Entries, 1000000, 1); // update each 1000000 entries
  registerEpicsPV(m_pvPrefix + "hadronb2_over_mumu2trk", "hadronb2_over_mumu2trk");

  //ECLTRG_deadch
  addDeltaPar("TRG", "h_TCId", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_deadch", "ECLTRG_deadch");

  //ECLTRG N(TC) total
  addDeltaPar("TRG", "h_n_TChit_event_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_total", "ECLTRG_meanTC_total");

  //ECLTRG N(TC) clean
  addDeltaPar("TRG", "h_n_TChit_clean_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_clean", "ECLTRG_meanTC_clean");

  //ECLTRG N(TC) HER Injection region
  addDeltaPar("TRG", "h_n_TChit_injHER_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_injHER", "ECLTRG_meanTC_injHER");

  //ECLTRG N(TC) LER Injection region
  addDeltaPar("TRG", "h_n_TChit_injLER_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_injLER", "ECLTRG_meanTC_injLER");

  //ECLTRG N(TC) total from the forward endcap
  addDeltaPar("TRG", "h_n_TChit_FWD_event_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_FWD_total", "ECLTRG_meanTC_FWD_total");

  //ECLTRG N(TC) clean from the forward endcap
  addDeltaPar("TRG", "h_n_TChit_FWD_clean_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_FWD_clean", "ECLTRG_meanTC_FWD_clean");

  //ECLTRG N(TC) HER Injection region from the forward endcap
  addDeltaPar("TRG", "h_n_TChit_FWD_injHER_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_FWD_injHER", "ECLTRG_meanTC_FWD_injHER");

  //ECLTRG N(TC) LER Injection region from the forward endcap
  addDeltaPar("TRG", "h_n_TChit_FWD_injLER_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_FWD_injLER", "ECLTRG_meanTC_FWD_injLER");

  //ECLTRG N(TC) total from the barrel
  addDeltaPar("TRG", "h_n_TChit_BRL_event_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_BRL_total", "ECLTRG_meanTC_BRL_total");

  //ECLTRG N(TC) clean from the barrel
  addDeltaPar("TRG", "h_n_TChit_BRL_clean_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_BRL_clean", "ECLTRG_meanTC_BRL_clean");

  //ECLTRG N(TC) HER Injection region from the barrel
  addDeltaPar("TRG", "h_n_TChit_BRL_injHER_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_BRL_injHER", "ECLTRG_meanTC_BRL_injHER");

  //ECLTRG N(TC) LER Injection region from the barrel
  addDeltaPar("TRG", "h_n_TChit_BRL_injLER_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_BRL_injLER", "ECLTRG_meanTC_BRL_injLER");

  //ECLTRG N(TC) total from the backward endcap
  addDeltaPar("TRG", "h_n_TChit_BWD_event_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_BWD_total", "ECLTRG_meanTC_BWD_total");

  //ECLTRG N(TC) clean from the backward endcap
  addDeltaPar("TRG", "h_n_TChit_BWD_clean_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_BWD_clean", "ECLTRG_meanTC_BWD_clean");

  //ECLTRG N(TC) HER Injection region from the backward endcap
  addDeltaPar("TRG", "h_n_TChit_BWD_injHER_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_BWD_injHER", "ECLTRG_meanTC_BWD_injHER");

  //ECLTRG N(TC) LER Injection region from the backward endcap
  addDeltaPar("TRG", "h_n_TChit_BWD_injLER_clkgrp", HistDelta::c_Entries, 10000, 1); // update each 10000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_meanTC_BWD_injLER", "ECLTRG_meanTC_BWD_injLER");

  //CDCTRG_deadch
//  m_canvas_CDCTRG_deadch = new TCanvas("CDCTRG_deadch");
  addDeltaPar("TRGCDCTNN", "NeuroHWInTSID", HistDelta::c_Entries, 100000, 1); // update each 100000 entries
  registerEpicsPV(m_pvPrefix + "CDCTRG_deadch", "CDCTRG_deadch");

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

  bool m_IsPhysicsRun = (getRunType() == "physics") || (getRunType() == "cosmic") || (getRunType() == "debug");
  if (m_IsPhysicsRun == true) {

    //update comL1-GDLL1
    auto hist_comL1_GDLL1 = getDelta("TRGGDL", "hGDL_gdlL1TocomL1_all", 0, true);// only if updated
    if (hist_comL1_GDLL1) {
      double comL1_GDLL1_mean = 0.0;
      hist_comL1_GDLL1->Draw();
      comL1_GDLL1_mean = hist_comL1_GDLL1->GetMean();
      B2DEBUG(1, "comL1_GDLL1_mean:" << comL1_GDLL1_mean);
      setEpicsPV("comL1_GDLL1_mean", comL1_GDLL1_mean);
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
//    TH1F* hist_ECLTRG_clone = (TH1F*)hist_ECLTRG_peak->Clone();//Clone the histogram.
      double ECLTRG_peak = 0.0;
      hist_ECLTRG_peak->Draw();
//    int bin_ECLTRG_peak = hist_ECLTRG_peak->GetMaximumBin();
//    ECLTRG_peak = hist_ECLTRG_peak->GetXaxis()->GetBinCenter(bin_ECLTRG_peak);
      ECLTRG_peak = hist_ECLTRG_peak->GetMean();

      /*
          while(ECLTRG_peak<-1310){
             hist_ECLTRG_clone->SetBinContent(bin_ECLTRG_peak, 0);
             bin_ECLTRG_peak = hist_ECLTRG_clone->GetMaximumBin();
             ECLTRG_peak = hist_ECLTRG_clone->GetXaxis()->GetBinCenter(bin_ECLTRG_peak);
          }
      */

      B2DEBUG(1, "ECLTRG_peak:" << ECLTRG_peak);
      setEpicsPV("ECLTRG_peak", ECLTRG_peak);
//    delete hist_ECLTRG_clone;
    }

// update ECLTRG deadch
    auto hist_ECLTRG_deadch =  getDelta("TRG", "h_TCId", 0, true);// only if updated
    if (hist_ECLTRG_deadch) {
      hist_ECLTRG_deadch->Draw();
      int numberOfBins = hist_ECLTRG_deadch->GetNbinsX();
      double average = 0;
      for (int i = 80; i < 512; i++) {
        average += hist_ECLTRG_deadch->GetBinContent(i);
      }
      average = 1.0 * average / (512 - 80);
      int ECLTRG_deadch = 0;
//          for (int i = 2; i <= numberOfBins - 1; i++) {
//            if (hist_ECLTRG_deadch->GetBinContent(i) <= 0.01 * hist_ECLTRG_deadch->GetMaximum()) { ECLTRG_deadch += 1; }
//          }
      for (int i = 2; i <= numberOfBins - 1; i++) {
        if (hist_ECLTRG_deadch->GetBinContent(i) <= 0.01 * average) { ECLTRG_deadch += 1; }
      }
      B2DEBUG(1, "ECLTRG_deadch:" << ECLTRG_deadch);
      setEpicsPV("ECLTRG_deadch", ECLTRG_deadch);
    }

// update TOPTRG deadch
    auto hist_TOPTRG_deadch =  getDelta("TRGGRL", "h_slot_TOP", 0, true);// only if updated
    if (hist_TOPTRG_deadch) {
      hist_TOPTRG_deadch->Draw();
      int numberOfBins = hist_TOPTRG_deadch->GetNbinsX();
      int TOPTRG_deadch = 0;
      for (int i = 2; i <= numberOfBins - 1; i++) {
        if (hist_TOPTRG_deadch->GetBinContent(i) <= 0.01 * hist_TOPTRG_deadch->GetMaximum()) { TOPTRG_deadch += 1; }
      }
      B2DEBUG(1, "TOPTRG_deadch:" << TOPTRG_deadch);
      setEpicsPV("TOPTRG_deadch", TOPTRG_deadch);
    }

//  m_canvas_CDCTRG_2D_peak->Clear();
//  m_canvas_CDCTRG_2D_peak->cd(0);
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

// update CDCTRG deadch
    auto hist_CDCTRG_deadch =  getDelta("TRGCDCTNN", "NeuroHWInTSID", 0, true);// only if updated
    if (hist_CDCTRG_deadch) {
      hist_CDCTRG_deadch->Draw();
      int numberOfBins = hist_CDCTRG_deadch->GetNbinsX();
//    int YMax = hist_CDCTRG_deadch->GetBinContent(hist_CDCTRG_deadch->GetMaximumBin());
      int CDCTRG_deadch = 0;
      for (int i = 3; i <= numberOfBins; i++) {
        if (hist_CDCTRG_deadch->GetBinContent(i) <= 0.01 * hist_CDCTRG_deadch->GetMaximum()) {CDCTRG_deadch += 1;}
//      if (hist_CDCTRG_deadch->GetBinContent(i) <= 0) {CDCTRG_deadch += 1;}
      }
      B2DEBUG(1, "CDCTRG_deadch:" << CDCTRG_deadch);
      setEpicsPV("CDCTRG_deadch", CDCTRG_deadch);
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

// update ecltiming_cdctiming
    auto hist_ecltiming_cdctiming =  getDelta("TRGGDL", "hGDL_ns_cdcToecl_all", 0, true);// only if updated
    if (hist_ecltiming_cdctiming) {
      double ecltiming_cdctiming = 0.0;
      hist_ecltiming_cdctiming->Draw();
      int bin_ecltiming_cdctiming = hist_ecltiming_cdctiming->GetMaximumBin();
      ecltiming_cdctiming = hist_ecltiming_cdctiming->GetXaxis()->GetBinCenter(bin_ecltiming_cdctiming);
      B2DEBUG(1, "ecltiming_cdctiming:" << ecltiming_cdctiming);
      setEpicsPV("ecltiming_cdctiming", ecltiming_cdctiming);//Peak
    }

// update toptiming_ecltiming
    auto hist_toptiming_ecltiming =  getDelta("TRGGDL", "hGDL_ns_topToecl_all", 0, true);// only if updated
    if (hist_toptiming_ecltiming) {
      double toptiming_ecltiming = 0.0;
      hist_toptiming_ecltiming->Draw();
      int bin_toptiming_ecltiming = hist_toptiming_ecltiming->GetMaximumBin();
      toptiming_ecltiming = hist_toptiming_ecltiming->GetXaxis()->GetBinCenter(bin_toptiming_ecltiming);
      B2DEBUG(1, "toptiming_ecltiming:" << toptiming_ecltiming);
      setEpicsPV("toptiming_ecltiming", toptiming_ecltiming);//Peak
    }

// update toptiming_cdctiming
    auto hist_toptiming_cdctiming =  getDelta("TRGGDL", "hGDL_ns_topTocdc_all", 0, true);// only if updated
    if (hist_toptiming_cdctiming) {
      double toptiming_cdctiming = 0.0;
      hist_toptiming_cdctiming->Draw();
      int bin_toptiming_cdctiming = hist_toptiming_cdctiming->GetMaximumBin();
      toptiming_cdctiming = hist_toptiming_cdctiming->GetXaxis()->GetBinCenter(bin_toptiming_cdctiming);
      B2DEBUG(1, "toptiming_cdctiming:" << toptiming_cdctiming);
      setEpicsPV("toptiming_cdctiming", toptiming_cdctiming);//Peak
    }

// update gdll1_ecltiming
    auto hist_gdll1_ecltiming =  getDelta("TRGGDL", "hGDL_eclTogdlL1_all", 0, true);// only if updated
    if (hist_gdll1_ecltiming) {
      double gdll1_ecltiming = 0.0;
      hist_gdll1_ecltiming->Draw();
      int bin_gdll1_ecltiming = hist_gdll1_ecltiming->GetMaximumBin();
      gdll1_ecltiming = hist_gdll1_ecltiming->GetXaxis()->GetBinCenter(bin_gdll1_ecltiming);
      B2DEBUG(1, "gdll1_ecltiming:" << gdll1_ecltiming);
      setEpicsPV("gdll1_ecltiming", gdll1_ecltiming);//Peak
    }

// update gdll1_cdctiming
    auto hist_gdll1_cdctiming =  getDelta("TRGGDL", "hGDL_cdcTogdlL1_all", 0, true);// only if updated
    if (hist_gdll1_cdctiming) {
      double gdll1_cdctiming = 0.0;
      hist_gdll1_cdctiming->Draw();
      int bin_gdll1_cdctiming = hist_gdll1_cdctiming->GetMaximumBin();
      gdll1_cdctiming = hist_gdll1_cdctiming->GetXaxis()->GetBinCenter(bin_gdll1_cdctiming);
      B2DEBUG(1, "gdll1_cdctiming:" << gdll1_cdctiming);
      setEpicsPV("gdll1_cdctiming", gdll1_cdctiming);//Peak
    }

// update gdll1_toptiming
    auto hist_gdll1_toptiming =  getDelta("TRGGDL", "hGDL_topTogdlL1_all", 0, true);// only if updated
    if (hist_gdll1_toptiming) {
      double gdll1_toptiming = 0.0;
      hist_gdll1_toptiming->Draw();
      int bin_gdll1_toptiming = hist_gdll1_toptiming->GetMaximumBin();
      gdll1_toptiming = hist_gdll1_toptiming->GetXaxis()->GetBinCenter(bin_gdll1_toptiming);
      B2DEBUG(1, "gdll1_toptiming:" << gdll1_toptiming);
      setEpicsPV("gdll1_toptiming", gdll1_toptiming);//Peak
    }

// update barrel_klm_latency
    auto hist_barrel_klm_latency =  getDelta("TRGGDL", "hGDL_itd_klm_hit_rise_all", 0, true);// only if updated
    if (hist_barrel_klm_latency) {
      double barrel_klm_latency = 0.0;
      hist_barrel_klm_latency->Draw();
      int bin_barrel_klm_latency = hist_barrel_klm_latency->GetMaximumBin();
      barrel_klm_latency = hist_barrel_klm_latency->GetXaxis()->GetBinCenter(bin_barrel_klm_latency);
      B2DEBUG(1, "barrel_klm_latency:" << barrel_klm_latency);
      setEpicsPV("barrel_klm_latency", barrel_klm_latency);//Peak
    }

// update endcap_klm_latency
    auto hist_endcap_klm_latency =  getDelta("TRGGDL", "hGDL_itd_eklm_hit_rise_all", 0, true);// only if updated
    if (hist_endcap_klm_latency) {
      double endcap_klm_latency = 0.0;
      hist_endcap_klm_latency->Draw();
      int bin_endcap_klm_latency = hist_endcap_klm_latency->GetMaximumBin();
      endcap_klm_latency = hist_endcap_klm_latency->GetXaxis()->GetBinCenter(bin_endcap_klm_latency);
      B2DEBUG(1, "endcap_klm_latency:" << endcap_klm_latency);
      setEpicsPV("endcap_klm_latency", endcap_klm_latency);//Peak
    }

    bool m_IsPhysicsRun_ratio = (getRunType() == "physics");
    if (m_IsPhysicsRun_ratio == true) {


// update #hadronb2/#bhabha_all
      auto hist_hadronb2_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
      if (hist_hadronb2_over_bhabha_all) {
        hist_hadronb2_over_bhabha_all->Draw();
        if (hist_hadronb2_over_bhabha_all->GetBinContent(hist_hadronb2_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all")) != 0) {
          double hadronb2_over_bhabha_all = 0.0;
          hadronb2_over_bhabha_all = hist_hadronb2_over_bhabha_all->GetBinContent(
                                       hist_hadronb2_over_bhabha_all->GetXaxis()->FindBin("accept_hadronb2")) / hist_hadronb2_over_bhabha_all->GetBinContent(
                                       hist_hadronb2_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));

          B2DEBUG(1, "hadronb2_over_bhabha_all:" << hadronb2_over_bhabha_all);
          setEpicsPV("hadronb2_over_bhabha_all", hadronb2_over_bhabha_all);
        }
      }

// update #mumu2trk/#bhabha_all
      auto hist_mumu2trk_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
      if (hist_mumu2trk_over_bhabha_all) {
        hist_mumu2trk_over_bhabha_all->Draw();
        if (hist_mumu2trk_over_bhabha_all->GetBinContent(hist_mumu2trk_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all")) != 0) {
          double mumu2trk_over_bhabha_all = 0.0;
          mumu2trk_over_bhabha_all = hist_mumu2trk_over_bhabha_all->GetBinContent(
                                       hist_mumu2trk_over_bhabha_all->GetXaxis()->FindBin("accept_mumu_2trk")) / hist_mumu2trk_over_bhabha_all->GetBinContent(
                                       hist_mumu2trk_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
          B2DEBUG(1, "mumu2trk_over_bhabha_all:" << mumu2trk_over_bhabha_all);
          setEpicsPV("mumu2trk_over_bhabha_all", mumu2trk_over_bhabha_all);
        }

      }

// update #hadronb2/#mumu2trk
      auto hist_hadronb2_over_mumu2trk =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
      if (hist_hadronb2_over_mumu2trk) {
        hist_hadronb2_over_mumu2trk->Draw();
        if (hist_hadronb2_over_mumu2trk->GetBinContent(
              hist_hadronb2_over_mumu2trk->GetXaxis()->FindBin("accept_mumu_2trk")) != 0) {
          double hadronb2_over_mumu2trk = 0.0;
          hadronb2_over_mumu2trk = hist_hadronb2_over_mumu2trk->GetBinContent(
                                     hist_hadronb2_over_mumu2trk->GetXaxis()->FindBin("accept_hadronb2")) / hist_hadronb2_over_mumu2trk->GetBinContent(
                                     hist_hadronb2_over_mumu2trk->GetXaxis()->FindBin("accept_mumu_2trk"));
          B2DEBUG(1, "hadronb2_over_mumu2trk:" << hadronb2_over_mumu2trk);
          setEpicsPV("hadronb2_over_mumu2trk", hadronb2_over_mumu2trk);
        }

      }



// update #mumu_tight/#bhabha_all
      auto hist_mumu_tight_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
      if (hist_mumu_tight_over_bhabha_all) {
        hist_mumu_tight_over_bhabha_all->Draw();
        if (hist_mumu_tight_over_bhabha_all->GetBinContent(
              hist_mumu_tight_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all")) != 0) {
          double mumu_tight_over_bhabha_all = 0.0;
          mumu_tight_over_bhabha_all = hist_mumu_tight_over_bhabha_all->GetBinContent(
                                         hist_mumu_tight_over_bhabha_all->GetXaxis()->FindBin("accept_mumutight")) / hist_mumu_tight_over_bhabha_all->GetBinContent(
                                         hist_mumu_tight_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
          B2DEBUG(1, "mumu_tight_over_bhabha_all:" << mumu_tight_over_bhabha_all);
          setEpicsPV("mumu_tight_over_bhabha_all", mumu_tight_over_bhabha_all);
        }

      }

// update #gammagamma/#bhabha_all
      auto hist_gammagamma_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
      if (hist_gammagamma_over_bhabha_all) {
        hist_gammagamma_over_bhabha_all->Draw();
        if (hist_gammagamma_over_bhabha_all->GetBinContent(
              hist_gammagamma_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all")) != 0) {
          double gammagamma_over_bhabha_all = 0.0;
          gammagamma_over_bhabha_all = hist_gammagamma_over_bhabha_all->GetBinContent(
                                         hist_gammagamma_over_bhabha_all->GetXaxis()->FindBin("accept_gamma_gamma")) / hist_gammagamma_over_bhabha_all->GetBinContent(
                                         hist_gammagamma_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
          B2DEBUG(1, "gammagamma_over_bhabha_all:" << gammagamma_over_bhabha_all);
          setEpicsPV("gammagamma_over_bhabha_all", gammagamma_over_bhabha_all);
        }

      }

// update #tautau2trk/#bhabha_all
      auto hist_tautau2trk_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
      if (hist_tautau2trk_over_bhabha_all) {
        hist_tautau2trk_over_bhabha_all->Draw();
        if (hist_tautau2trk_over_bhabha_all->GetBinContent(
              hist_tautau2trk_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all")) != 0) {
          double tautau2trk_over_bhabha_all = 0.0;
          tautau2trk_over_bhabha_all = hist_tautau2trk_over_bhabha_all->GetBinContent(
                                         hist_tautau2trk_over_bhabha_all->GetXaxis()->FindBin("accept_tau_2trk")) / hist_tautau2trk_over_bhabha_all->GetBinContent(
                                         hist_tautau2trk_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
          B2DEBUG(1, "tautau2trk_over_bhabha_all:" << tautau2trk_over_bhabha_all);
          setEpicsPV("tautau2trk_over_bhabha_all", tautau2trk_over_bhabha_all);
        }

      }

// update #hadron/#bhabha_all
      auto hist_hadron_over_bhabha_all =  getDelta("softwaretrigger", "skim", 0, true);// only if updated
      if (hist_hadron_over_bhabha_all) {
        hist_hadron_over_bhabha_all->Draw();
        if (hist_hadron_over_bhabha_all->GetBinContent(
              hist_hadron_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all")) != 0) {
          double hadron_over_bhabha_all = 0.0;
          hadron_over_bhabha_all = hist_hadron_over_bhabha_all->GetBinContent(
                                     hist_hadron_over_bhabha_all->GetXaxis()->FindBin("accept_hadron")) / hist_hadron_over_bhabha_all->GetBinContent(
                                     hist_hadron_over_bhabha_all->GetXaxis()->FindBin("accept_bhabha_all"));
          B2DEBUG(1, "hadron_over_bhabha_all:" << hadron_over_bhabha_all);
          setEpicsPV("hadron_over_bhabha_all", hadron_over_bhabha_all);
        }

      }

    }

    auto hist_nTSFHits_total = getDelta("TRGGRL", "h_wirecnt_sum", 0, true);
    if (hist_nTSFHits_total) {
      double mean = hist_nTSFHits_total->GetMean();
      B2DEBUG(1, "CDCTRG_meanTSFHits_total:" << mean);
      setEpicsPV("CDCTRG_meanTSFHits_total", mean);
    }

    auto hist_nTSFHits_clean = getDelta("TRGGRL", "h_wirecnt_sum_clean", 0, true);
    if (hist_nTSFHits_clean) {
      double mean = hist_nTSFHits_clean->GetMean();
      B2DEBUG(1, "CDCTRG_meanTSFHits_clean:" << mean);
      setEpicsPV("CDCTRG_meanTSFHits_clean", mean);
    }

    auto hist_nTSFHits_injHER = getDelta("TRGGRL", "h_wirecnt_sum_injHER", 0, true);
    if (hist_nTSFHits_injHER) {
      double mean = hist_nTSFHits_injHER->GetMean();
      B2DEBUG(1, "CDCTRG_meanTSFHits_injHER:" << mean);
      setEpicsPV("CDCTRG_meanTSFHits_injHER", mean);
    }

    auto hist_nTSFHits_injLER = getDelta("TRGGRL", "h_wirecnt_sum_injLER", 0, true);
    if (hist_nTSFHits_injLER) {
      double mean = hist_nTSFHits_injLER->GetMean();
      B2DEBUG(1, "CDCTRG_meanTSFHits_injLER:" << mean);
      setEpicsPV("CDCTRG_meanTSFHits_injLER", mean);
    }

    auto hist_nTC_total = getDelta("TRG", "h_n_TChit_event_clkgrp", 0, true);
    if (hist_nTC_total) {
      double mean = hist_nTC_total->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_total:" << mean);
      setEpicsPV("ECLTRG_meanTC_total", mean);
    }

    auto hist_nTC_clean = getDelta("TRG", "h_n_TChit_clean_clkgrp", 0, true);
    if (hist_nTC_clean) {
      double mean = hist_nTC_clean->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_clean:" << mean);
      setEpicsPV("ECLTRG_meanTC_clean", mean);
    }

    auto hist_nTC_injHER = getDelta("TRG", "h_n_TChit_injHER_clkgrp", 0, true);
    if (hist_nTC_injHER) {
      double mean = hist_nTC_injHER->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_injHER:" << mean);
      setEpicsPV("ECLTRG_meanTC_injHER", mean);
    }

    auto hist_nTC_injLER = getDelta("TRG", "h_n_TChit_injLER_clkgrp", 0, true);
    if (hist_nTC_injLER) {
      double mean = hist_nTC_injLER->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_injLER:" << mean);
      setEpicsPV("ECLTRG_meanTC_injLER", mean);
    }

    auto hist_nTC_FWD_total = getDelta("TRG", "h_n_TChit_FWD_event_clkgrp", 0, true);
    if (hist_nTC_FWD_total) {
      double mean = hist_nTC_FWD_total->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_FWD_total:" << mean);
      setEpicsPV("ECLTRG_meanTC_FWD_total", mean);
    }

    auto hist_nTC_FWD_clean = getDelta("TRG", "h_n_TChit_FWD_clean_clkgrp", 0, true);
    if (hist_nTC_FWD_clean) {
      double mean = hist_nTC_FWD_clean->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_FWD_clean:" << mean);
      setEpicsPV("ECLTRG_meanTC_FWD_clean", mean);
    }

    auto hist_nTC_FWD_injHER = getDelta("TRG", "h_n_TChit_FWD_injHER_clkgrp", 0, true);
    if (hist_nTC_FWD_injHER) {
      double mean = hist_nTC_FWD_injHER->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_FWD_injHER:" << mean);
      setEpicsPV("ECLTRG_meanTC_FWD_injHER", mean);
    }

    auto hist_nTC_FWD_injLER = getDelta("TRG", "h_n_TChit_FWD_injLER_clkgrp", 0, true);
    if (hist_nTC_FWD_injLER) {
      double mean = hist_nTC_FWD_injLER->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_FWD_injLER:" << mean);
      setEpicsPV("ECLTRG_meanTC_FWD_injLER", mean);
    }

    auto hist_nTC_BRL_total = getDelta("TRG", "h_n_TChit_BRL_event_clkgrp", 0, true);
    if (hist_nTC_BRL_total) {
      double mean = hist_nTC_BRL_total->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_BRL_total:" << mean);
      setEpicsPV("ECLTRG_meanTC_BRL_total", mean);
    }

    auto hist_nTC_BRL_clean = getDelta("TRG", "h_n_TChit_BRL_clean_clkgrp", 0, true);
    if (hist_nTC_BRL_clean) {
      double mean = hist_nTC_BRL_clean->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_BRL_clean:" << mean);
      setEpicsPV("ECLTRG_meanTC_BRL_clean", mean);
    }

    auto hist_nTC_BRL_injHER = getDelta("TRG", "h_n_TChit_BRL_injHER_clkgrp", 0, true);
    if (hist_nTC_BRL_injHER) {
      double mean = hist_nTC_BRL_injHER->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_BRL_injHER:" << mean);
      setEpicsPV("ECLTRG_meanTC_BRL_injHER", mean);
    }

    auto hist_nTC_BRL_injLER = getDelta("TRG", "h_n_TChit_BRL_injLER_clkgrp", 0, true);
    if (hist_nTC_BRL_injLER) {
      double mean = hist_nTC_BRL_injLER->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_BRL_injLER:" << mean);
      setEpicsPV("ECLTRG_meanTC_BRL_injLER", mean);
    }

    auto hist_nTC_BWD_total = getDelta("TRG", "h_n_TChit_BWD_event_clkgrp", 0, true);
    if (hist_nTC_BWD_total) {
      double mean = hist_nTC_BWD_total->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_BWD_total:" << mean);
      setEpicsPV("ECLTRG_meanTC_BWD_total", mean);
    }

    auto hist_nTC_BWD_clean = getDelta("TRG", "h_n_TChit_BWD_clean_clkgrp", 0, true);
    if (hist_nTC_BWD_clean) {
      double mean = hist_nTC_BWD_clean->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_BWD_clean:" << mean);
      setEpicsPV("ECLTRG_meanTC_BWD_clean", mean);
    }

    auto hist_nTC_BWD_injHER = getDelta("TRG", "h_n_TChit_BWD_injHER_clkgrp", 0, true);
    if (hist_nTC_BWD_injHER) {
      double mean = hist_nTC_BWD_injHER->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_BWD_injHER:" << mean);
      setEpicsPV("ECLTRG_meanTC_BWD_injHER", mean);
    }

    auto hist_nTC_BWD_injLER = getDelta("TRG", "h_n_TChit_BWD_injLER_clkgrp", 0, true);
    if (hist_nTC_BWD_injLER) {
      double mean = hist_nTC_BWD_injLER->GetMean();
      B2DEBUG(1, "ECLTRG_meanTC_BWD_injLER:" << mean);
      setEpicsPV("ECLTRG_meanTC_BWD_injLER", mean);
    }


    // Tag canvas as updated ONLY if things have changed.
    UpdateCanvas(m_canvas->GetName(), hist != nullptr);

    // this if left over from jsroot, may not be needed anymore (to check)
    m_canvas->Update();
  }

}

void DQMHistAnalysisTRGModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: terminate called");
}

