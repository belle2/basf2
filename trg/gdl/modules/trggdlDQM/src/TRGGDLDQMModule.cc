//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGGDLModule.cc
// Section  : TRG GDL
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : A trigger module for TRG GDL
//---------------------------------------------------------------
// 1.00 : 2017/05/08 : First version
//---------------------------------------------------------------
#include <trg/gdl/modules/trggdlDQM/TRGGDLDQMModule.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dbobjects/RunInfo.h>
#include <framework/datastore/DataStore.h>

#include <TDirectory.h>
#include <TRandom3.h>
#include <TPostScript.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <framework/logging/Logger.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Belle2;
using namespace GDL;

REG_MODULE(TRGGDLDQM);

/*
   Fired data in TrgBit not available because
   of absence of TRGSummary class.
   TrgBit class only for bit configuration.
*/


TRGGDLDQMModule::TRGGDLDQMModule() : HistoModule()
{

  setDescription("DQM for GDL Trigger system");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("eventByEventTimingHistRecord", m_eventByEventTimingHistRecord,
           "Recording event by event timing distribution histogram or not",
           false);
  addParam("dumpVcdFile", m_dumpVcdFile,
           "Dumping vcd file or not",
           false);
  addParam("bitConditionToDumpVcd", m_bitConditionToDumpVcd,
           "Condition for vcd. alg format with '!' and '+'.",
           string(""));
  addParam("vcdEventStart", m_vcdEventStart,
           "Start equential event number",
           unsigned(0));
  addParam("vcdNumberOfEvents", m_vcdNumberOfEvents,
           "Number of events to dump vcd file",
           unsigned(10));
  addParam("bitNameOnBinLabel", m_bitNameOnBinLabel,
           "Put bitname on BinLabel",
           false);
  addParam("generatePostscript", m_generatePostscript,
           "Genarete postscript file or not",
           false);
  addParam("postScriptName", m_postScriptName,
           "postscript file name",
           string("gdldqm.ps"));
  B2DEBUG(20, "eventByEventTimingFlag(" << m_eventByEventTimingHistRecord
          << "), m_dumpVcdFile(" << m_dumpVcdFile
          << "), m_bitConditionToDumpVcd(" << m_bitConditionToDumpVcd
          << "), m_vcdEventStart(" << m_vcdEventStart
          << "), m_vcdNumberOfEvents(" << m_vcdNumberOfEvents);

}

void TRGGDLDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = NULL;
  dirDQM = oldDir->mkdir("TRGGDL");
  dirDQM->cd();

  h_c8_gdlL1TocomL1  = new TH1I("hGDL_gdlL1TocomL1",  "comL1 - gdlL1 [clk8ns]", 100, 0, 100);
  h_c8_gdlL1TocomL1->GetXaxis()->SetTitle("clk8ns");
  h_c8_eclTogdlL1    = new TH1I("hGDL_eclTogdlL1",    "gdlL1 - ecl_timing [clk8ns]", 500, 0, 500);
  h_c8_eclTogdlL1->GetXaxis()->SetTitle("clk8ns");
  h_c8_ecl8mToGDL    = new TH1I("hGDL_ecl8mToGDL",    "gdlIn^{8MHz} - ecl_timing [clk8ns]", 500, 0, 500);
  h_c8_ecl8mToGDL->GetXaxis()->SetTitle("clk8ns");
  h_c8_eclToGDL      = new TH1I("hGDL_eclToGDL",      "gdlIn - ecl_timing [clk8ns]", 500, 0, 500);
  h_c8_eclToGDL->GetXaxis()->SetTitle("clk8ns");
  h_c2_cdcTocomL1 = new TH1I("hGDL_cdcTocomL1", "comL1 - cdc_timing [clk2ns]", 520, 0, 5200);
  h_c2_cdcTocomL1->GetXaxis()->SetTitle("clk2ns");
  h_ns_cdcTocomL1 = new TH1D("hGDL_ns_cdcTocomL1", "comL1 - cdc_timing [ns]", 2600, 0, 10400);
  h_ns_cdcTocomL1->GetXaxis()->SetTitle("ns");
  h_ns_cdcTocomL1->GetYaxis()->SetTitle("evt / 4ns");
  h_ns_cdcTogdlL1 = new TH1D("hGDL_ns_cdcTogdlL1", "gdlL1 - cdc_timing [ns]", 2600, 0, 10400);
  h_ns_cdcTogdlL1->GetXaxis()->SetTitle("ns");
  h_ns_cdcTogdlL1->GetYaxis()->SetTitle("evt / 4ns");
  h_ns_topToecl = new TH1D("hGDL_ns_topToecl", "ecl_timing - top_timing [ns]", 800, 0, 4000);
  h_ns_topToecl->GetXaxis()->SetTitle("ns");
  h_ns_topToecl->GetYaxis()->SetTitle("evt / 5ns");
  h_ns_topTocdc = new TH1D("hGDL_ns_topTocdc", "cdc_timing - top_timing [ns]", 800, 0, 4000);
  h_ns_topTocdc->GetXaxis()->SetTitle("ns");
  h_ns_topTocdc->GetYaxis()->SetTitle("evt / 5ns");
  h_c2_cdcToecl = new TH1I("hGDL_cdcToecl", "ecl_timing - cdc_timing [clk2ns]", 1000, 0, 2000);
  h_c2_cdcToecl->GetXaxis()->SetTitle("clk2ns");
  h_ns_cdcToecl = new TH1D("hGDL_ns_cdcToecl", "ecl_timing - cdc_timing [ns]", 2000, 0, 4000);
  h_ns_cdcToecl->GetXaxis()->SetTitle("ns");
  h_ns_cdcToecl->GetYaxis()->SetTitle("evt / 2ns");
  TrgBit tb;
  h_itd = new TH1I("hGDL_itd", "itd", tb.getNumOfInputs() + 1, -1, tb.getNumOfInputs());
  h_ftd = new TH1I("hGDL_ftd", "ftd", tb.getNumOfOutputs() + 1, -1, tb.getNumOfOutputs());
  h_psn = new TH1I("hGDL_psn", "psn", tb.getNumOfOutputs() + 1, -1, tb.getNumOfOutputs());
  // output extra
  h_psn_extra = new TH1I("hGDL_psn_extra", "psn extra", n_output_extra, 0, n_output_extra);
  for (int i = 0; i < n_output_extra; i++) {
    h_psn_extra->GetXaxis()->SetBinLabel(i + 1, output_extra[i]);
  }
  // rise/fall
  for (unsigned i = 0; i < tb.getNumOfInputs(); i++) {
    h_itd_rise[i] = new TH1I(Form("hGDL_itd_%s_rise", tb.getInputBitName(i)),
                             Form("itd%d(%s) rising", i, tb.getInputBitName(i)), 48, 0, 48);
    h_itd_rise[i]->SetLineColor(kRed);
    h_itd_fall[i] = new TH1I(Form("hGDL_itd_%s_fall", tb.getInputBitName(i)),
                             Form("itd%d(%s) falling", i, tb.getInputBitName(i)), 48, 0, 48);
    h_itd_fall[i]->SetLineColor(kGreen);
    if (m_bitNameOnBinLabel) {
      h_itd->GetXaxis()->SetBinLabel(h_itd->GetXaxis()->FindBin(i + 0.5), tb.getInputBitName(i));
    }
  }
  for (unsigned i = 0; i < tb.getNumOfOutputs(); i++) {
    h_ftd->GetXaxis()->SetBinLabel(h_ftd->GetXaxis()->FindBin(i + 0.5), tb.getOutputBitName(i));
    h_ftd_rise[i] = new TH1I(Form("hGDL_ftd_%s_rise", tb.getOutputBitName(i)),
                             Form("ftd%d(%s) rising", i, tb.getOutputBitName(i)), 48, 0, 48);
    h_ftd_rise[i]->SetLineColor(kRed);
    h_ftd_fall[i] = new TH1I(Form("hGDL_ftd_%s_fall", tb.getOutputBitName(i)),
                             Form("ftd%d(%s) falling", i, tb.getOutputBitName(i)), 48, 0, 48);
    h_ftd_fall[i]->SetLineColor(kGreen);
    h_psn->GetXaxis()->SetBinLabel(h_psn->GetXaxis()->FindBin(i + 0.5), tb.getOutputBitName(i));
    h_psn_rise[i] = new TH1I(Form("hGDL_psn_%s_rise", tb.getOutputBitName(i)),
                             Form("psn%d(%s) rising", i, tb.getOutputBitName(i)), 48, 0, 48);
    h_psn_rise[i]->SetLineColor(kRed);
    h_psn_fall[i] = new TH1I(Form("hGDL_psn_%s_fall", tb.getOutputBitName(i)),
                             Form("psn%d(%s) falling", i, tb.getOutputBitName(i)), 48, 0, 48);
    h_psn_fall[i]->SetLineColor(kGreen);
    if (m_bitNameOnBinLabel) {
      h_ftd->GetXaxis()->SetBinLabel(h_ftd->GetXaxis()->FindBin(i + 0.5), tb.getOutputBitName(i));
      h_psn->GetXaxis()->SetBinLabel(h_psn->GetXaxis()->FindBin(i + 0.5), tb.getOutputBitName(i));
    }
  }
  h_timtype = new TH1I("hGDL_timtype", "timtype", 7, 0, 7);

  oldDir->cd();
}

void TRGGDLDQMModule::beginRun()
{

  dirDQM->cd();

  h_c8_gdlL1TocomL1->Reset();
  h_c8_eclTogdlL1->Reset();
  h_c8_ecl8mToGDL->Reset();
  h_c8_eclToGDL->Reset();
  h_c2_cdcTocomL1->Reset();
  h_ns_cdcTocomL1->Reset();
  h_ns_cdcTogdlL1->Reset();
  h_ns_topToecl->Reset();
  h_ns_topTocdc->Reset();
  h_c2_cdcToecl->Reset();
  h_ns_cdcToecl->Reset();
  h_itd->Reset();
  h_ftd->Reset();
  h_psn->Reset();
  h_timtype->Reset();

  oldDir->cd();
}

void TRGGDLDQMModule::initialize()
{

  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();
  REG_HISTOGRAM
  defineHisto();

}

void TRGGDLDQMModule::endRun()
{
  if (m_generatePostscript) {
    TPostScript* ps = new TPostScript(m_postScriptName.c_str(), 112);
    TrgBit tb;
    gStyle->SetOptStat(0);
    TCanvas c1("c1", "", 0, 0, 500, 300);
    c1.cd();
    for (unsigned i = 0; i < tb.getNumOfInputs(); i++) {
      h_itd_rise[i]->SetTitle(Form("itd%d(%s) rising(red), falling(green)",
                                   i, tb.getInputBitName(i)));
      h_itd_rise[i]->Draw();
      h_itd_fall[i]->Draw("same");
      c1.Update();
    }
    h_itd->GetXaxis()->SetRange(h_itd->GetXaxis()->FindBin(0.5),
                                h_itd->GetXaxis()->FindBin(tb.getNumOfInputs() - 0.5));
    h_itd->Draw();
    c1.Update();
    h_ftd->GetXaxis()->SetRange(h_ftd->GetXaxis()->FindBin(0.5),
                                h_ftd->GetXaxis()->FindBin(tb.getNumOfOutputs() - 0.5));
    h_ftd->Draw();
    c1.Update();
    h_psn->GetXaxis()->SetRange(h_psn->GetXaxis()->FindBin(0.5),
                                h_psn->GetXaxis()->FindBin(tb.getNumOfOutputs() - 0.5));
    h_psn->Draw();
    c1.Update();
    h_ftd->SetTitle("ftd(green), psnm(red)");
    h_ftd->SetFillColor(kGreen);
    h_ftd->SetBarWidth(0.4);
    h_ftd->Draw("bar");
    h_psn->SetFillColor(kRed);
    h_psn->SetBarWidth(0.4);
    h_psn->SetBarOffset(0.5);
    h_psn->Draw("bar,same");

    c1.Update();
    h_timtype->Draw();
    c1.Update();
    h_c8_gdlL1TocomL1->Draw();
    c1.Update();
    h_c8_eclTogdlL1->Draw();
    c1.Update();
    h_c8_ecl8mToGDL->Draw();
    c1.Update();
    h_c8_eclToGDL->Draw();
    c1.Update();
    h_c2_cdcTocomL1->Draw();
    c1.Update();
    h_ns_cdcTocomL1->Draw();
    c1.Update();
    h_ns_cdcTogdlL1->Draw();
    c1.Update();
    h_ns_topToecl->Draw();
    c1.Update();
    h_ns_topTocdc->Draw();
    c1.Update();
    h_c2_cdcToecl->Draw();
    c1.Update();
    h_ns_cdcToecl->Draw();
    c1.Update();

    ps->Close();
  }
}

void TRGGDLDQMModule::event()
{

  TrgBit tb;
  static unsigned nvcd = 0;
  static bool begin_run = true;

  int n_leafs = 0;
  int n_leafsExtra = 0;
  int ee_psn[10] = {0};
  int ee_ftd[10] = {0};
  int ee_itd[10] = {0};
  unsigned nword_input = 3;
  unsigned nword_output = 3;
  unsigned _e_timtype = 0;
  void (*setPointer)(TRGGDLUnpackerStore * store, int** bitArray);
  unsigned _e_gdll1rvc = 0;
  StoreArray<TRGGDLUnpackerStore> entAry;
  if (!entAry || !entAry.getEntries()) return;

  int nconf = entAry[0]->m_conf;

  if (nconf == 6) {
    nword_input = 5;
    n_clocks = GDLCONF6::nClks;
    n_leafs = GDLCONF6::nLeafs;
    n_leafsExtra = GDLCONF6::nLeafsExtra;
    ee_itd[0] = GDLCONF6::e_itd0; ee_itd[1] = GDLCONF6::e_itd1; ee_itd[2] = GDLCONF6::e_itd2;
    ee_itd[3] = GDLCONF6::e_itd3; ee_itd[4] = GDLCONF6::e_itd4;
    ee_psn[0] = GDLCONF6::e_psn0; ee_psn[1] = GDLCONF6::e_psn1; ee_psn[2] = GDLCONF6::e_psn2;
    ee_ftd[0] = GDLCONF6::e_ftd0; ee_ftd[1] = GDLCONF6::e_ftd1; ee_ftd[2] = GDLCONF6::e_ftd2;
    _e_timtype = GDLCONF6::e_timtype;
    setPointer = GDLCONF6::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF6::e_gdll1rvc;
  } else if (nconf == 5) {
    n_clocks = GDLCONF5::nClks;
    n_leafs = GDLCONF5::nLeafs;
    n_leafsExtra = GDLCONF5::nLeafsExtra;
    ee_itd[0] = GDLCONF5::e_itd0; ee_itd[1] = GDLCONF5::e_itd1; ee_itd[2] = GDLCONF5::e_itd2;
    ee_psn[0] = GDLCONF5::e_psn0; ee_psn[1] = GDLCONF5::e_psn1; ee_psn[2] = GDLCONF5::e_psn2;
    ee_ftd[0] = GDLCONF5::e_ftd0; ee_ftd[1] = GDLCONF5::e_ftd1; ee_ftd[2] = GDLCONF5::e_ftd2;
    _e_timtype = GDLCONF5::e_timtype;
    setPointer = GDLCONF5::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF5::e_gdll1rvc;
  } else if (nconf == 4) {
    n_clocks = GDLCONF4::nClks;
    n_leafs = GDLCONF4::nLeafs;
    n_leafsExtra = GDLCONF4::nLeafsExtra;
    ee_itd[0] = GDLCONF4::e_itd0; ee_itd[1] = GDLCONF4::e_itd1; ee_itd[2] = GDLCONF4::e_itd2;
    ee_psn[0] = GDLCONF4::e_psn0; ee_psn[1] = GDLCONF4::e_psn1; ee_psn[2] = GDLCONF4::e_psn2;
    ee_ftd[0] = GDLCONF4::e_ftd0; ee_ftd[1] = GDLCONF4::e_ftd1; ee_ftd[2] = GDLCONF4::e_ftd2;
    _e_timtype = GDLCONF4::e_timtype;
    setPointer = GDLCONF4::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF4::e_gdll1rvc;
  } else if (nconf == 3) {
    n_clocks = GDLCONF3::nClks;
    n_leafs = GDLCONF3::nLeafs;
    n_leafsExtra = GDLCONF3::nLeafsExtra;
    ee_itd[0] = GDLCONF3::e_itd0; ee_itd[1] = GDLCONF3::e_itd1; ee_itd[2] = GDLCONF3::e_itd2;
    ee_psn[0] = GDLCONF3::e_psn0; ee_psn[1] = GDLCONF3::e_psn1; ee_psn[2] = GDLCONF3::e_psn2;
    ee_ftd[0] = GDLCONF3::e_ftd0; ee_ftd[1] = GDLCONF3::e_ftd1; ee_ftd[2] = GDLCONF3::e_ftd2;
    _e_timtype = GDLCONF3::e_timtype;
    setPointer = GDLCONF3::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF3::e_gdll1rvc;
  } else if (nconf == 2) {
    n_clocks = GDLCONF2::nClks;
    n_leafs = GDLCONF2::nLeafs;
    n_leafsExtra = GDLCONF2::nLeafsExtra;
    ee_itd[0] = GDLCONF2::e_itd0; ee_itd[1] = GDLCONF2::e_itd1; ee_itd[2] = GDLCONF2::e_itd2;
    ee_psn[0] = GDLCONF2::e_psn0; ee_psn[1] = GDLCONF2::e_psn1; ee_psn[2] = GDLCONF2::e_psn2;
    ee_ftd[0] = GDLCONF2::e_ftd0; ee_ftd[1] = GDLCONF2::e_ftd1; ee_ftd[2] = GDLCONF2::e_ftd2;
    _e_timtype = GDLCONF2::e_timtype;
    setPointer = GDLCONF2::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF2::e_gdll1rvc;
  } else if (nconf == 1) {
    n_clocks = GDLCONF1::nClks;
    n_leafs = GDLCONF1::nLeafs;
    n_leafsExtra = GDLCONF1::nLeafsExtra;
    ee_itd[0] = GDLCONF1::e_itd0; ee_itd[1] = GDLCONF1::e_itd1; ee_itd[2] = GDLCONF1::e_itd2;
    ee_psn[0] = GDLCONF1::e_psn0; ee_psn[1] = GDLCONF1::e_psn1; ee_psn[2] = GDLCONF1::e_psn2;
    ee_ftd[0] = GDLCONF1::e_ftd0; ee_ftd[1] = GDLCONF1::e_ftd1; ee_ftd[2] = GDLCONF1::e_ftd2;
    _e_timtype = GDLCONF1::e_timtype;
    setPointer = GDLCONF1::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF1::e_gdll1rvc;
  } else {
    n_clocks = GDLCONF0::nClks;
    n_leafs = GDLCONF0::nLeafs;
    n_leafsExtra = GDLCONF0::nLeafsExtra;
    ee_itd[0] = GDLCONF0::e_itd0; ee_itd[1] = GDLCONF0::e_itd1; ee_itd[2] = GDLCONF0::e_itd2;
    ee_psn[0] = GDLCONF0::e_psn0; ee_psn[1] = GDLCONF0::e_psn1; ee_psn[2] = GDLCONF0::e_psn2;
    ee_ftd[0] = GDLCONF0::e_ftd0; ee_ftd[1] = GDLCONF0::e_ftd1; ee_ftd[2] = GDLCONF0::e_ftd2;
    _e_timtype = GDLCONF0::e_timtype;
    setPointer = GDLCONF0::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF0::e_gdll1rvc;
  }


  const double clkTo2ns = 1. / .508877;
  const double clkTo1ns = 0.5 / .508877;
  TH2I* h_0;

  dirDQM->cd();
  evtno = entAry[0]->m_evt;
  h_0 = new TH2I(Form("hgdl%08d", evtno), "", n_clocks, 0, n_clocks, n_leafs + n_leafsExtra, 0,
                 n_leafs + n_leafsExtra);
  h_p = new TH2I(Form("hpsn%08d", evtno), "", n_clocks, 0, n_clocks, tb.getNumOfOutputs(), 0, tb.getNumOfOutputs());
  h_f = new TH2I(Form("hftd%08d", evtno), "", n_clocks, 0, n_clocks, tb.getNumOfOutputs(), 0, tb.getNumOfOutputs());
  h_i = new TH2I(Form("hitd%08d", evtno), "", n_clocks, 0, n_clocks, tb.getNumOfInputs(), 0, tb.getNumOfInputs());
  for (unsigned i = 0; i < tb.getNumOfInputs(); i++) {
    h_i->GetYaxis()->SetBinLabel(i + 1, tb.getInputBitName(i));
  }
  for (unsigned i = 0; i < tb.getNumOfOutputs(); i++) {
    h_f->GetYaxis()->SetBinLabel(i + 1, tb.getOutputBitName(i));
    h_p->GetYaxis()->SetBinLabel(i + 1, tb.getOutputBitName(i));
  }

  oldDir->cd();

  // fill "bit vs clk" for the event
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    int* Bits[n_leafs + n_leafsExtra];
    setPointer(entAry[ii], Bits);
    for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
      h_0->SetBinContent(entAry[ii]->m_clk + 1, leaf + 1, *Bits[leaf]);
    }
  }
  for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
    if (6 == nconf) {
      h_0->GetYaxis()->SetBinLabel(leaf + 1, GDLCONF6::LeafNames[leaf]);
    } else if (5 == nconf) {
      h_0->GetYaxis()->SetBinLabel(leaf + 1, GDLCONF5::LeafNames[leaf]);
    } else if (4 == nconf) {
      h_0->GetYaxis()->SetBinLabel(leaf + 1, GDLCONF4::LeafNames[leaf]);
    } else if (3 == nconf) {
      h_0->GetYaxis()->SetBinLabel(leaf + 1, GDLCONF3::LeafNames[leaf]);
    } else if (2 == nconf) {
      h_0->GetYaxis()->SetBinLabel(leaf + 1, GDLCONF2::LeafNames[leaf]);
    } else if (1 == nconf) {
      h_0->GetYaxis()->SetBinLabel(leaf + 1, GDLCONF1::LeafNames[leaf]);
    } else if (0 == nconf) {
      h_0->GetYaxis()->SetBinLabel(leaf + 1, GDLCONF0::LeafNames[leaf]);
    }
  }


  int coml1rvc    = 0;
  int c1_ecl_timing = 0;
  int c2_cdc_timing = 0;
  int eclrvc  = 0;
  int c1_top_timing = 0;
  if (6 == nconf) {
    coml1rvc    = h_0->GetBinContent(1, 1 + GDLCONF6::e_coml1rvc);
    c1_ecl_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF6::e_ecltiming);
    c2_cdc_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF6::e_cdctiming) / 2;
    eclrvc  = h_0->GetBinContent(1, 1 + GDLCONF6::e_eclrvc);
    c1_top_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF6::e_toptiming);
  } else if (5 == nconf) {
    coml1rvc    = h_0->GetBinContent(1, 1 + GDLCONF5::e_coml1rvc);
    c1_ecl_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF5::e_ecltiming);
    c2_cdc_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF5::e_cdctiming) / 2;
    eclrvc  = h_0->GetBinContent(1, 1 + GDLCONF5::e_eclrvc);
    c1_top_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF5::e_toptiming);
  } else if (4 == nconf) {
    coml1rvc    = h_0->GetBinContent(1, 1 + GDLCONF4::e_coml1rvc);
    c1_ecl_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF4::e_ecltiming);
    c2_cdc_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF4::e_cdctiming) / 2;
    eclrvc  = h_0->GetBinContent(1, 1 + GDLCONF4::e_eclrvc);
    c1_top_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF4::e_toptiming);
  } else if (3 == nconf) {
    coml1rvc    = h_0->GetBinContent(1, 1 + GDLCONF3::e_coml1rvc);
    c1_ecl_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF3::e_ecltiming);
    c2_cdc_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF3::e_cdctiming) / 2;
    eclrvc  = h_0->GetBinContent(1, 1 + GDLCONF3::e_eclrvc);
    c1_top_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF3::e_toptiming);
  } else if (2 == nconf) {
    coml1rvc    = h_0->GetBinContent(1, 1 + GDLCONF2::e_coml1rvc);
    c1_ecl_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF2::e_ecltiming);
    c2_cdc_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF2::e_cdctiming) / 2;
    eclrvc  = h_0->GetBinContent(1, 1 + GDLCONF2::e_eclrvc);
    c1_top_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF2::e_toptiming);
  } else if (1 <= nconf && nconf <= 2) {
    coml1rvc    = h_0->GetBinContent(1, 1 + GDLCONF1::e_coml1rvc);
    c1_ecl_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF1::e_ecltiming);
    c2_cdc_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF1::e_cdctiming) / 2;
    eclrvc  = h_0->GetBinContent(1, 1 + GDLCONF1::e_eclrvc);
    c1_top_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF1::e_toptiming);
  } else if (0 == nconf) {
    coml1rvc    = h_0->GetBinContent(1, 1 + GDLCONF0::e_coml1rvc);
    c1_ecl_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF0::e_cdctiming);
    c2_cdc_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF0::e_cdctiming);
    eclrvc  = h_0->GetBinContent(1, 1 + GDLCONF0::e_eclrvc);
    c1_top_timing = h_0->GetBinContent(n_clocks, 1 + GDLCONF0::e_toptiming);
  }

  if (begin_run) {
    B2DEBUG(20, "nconf(" << nconf
            << "), n_clocks(" << n_clocks
            << "), n_leafs(" << n_leafs
            << "), n_leafsExtra(" << n_leafsExtra
            << ")");
    begin_run = false;
  }

  int psn[3] = {0};
  int ftd[3] = {0};
  int itd[5] = {0};
  int timtype  = 0;
  int c8_ecl_timing   = (c1_ecl_timing >> 3);
  int c2_ecl_timing   = (c1_ecl_timing >> 1);

  int c2_top_timing = c1_top_timing >> 1;

  int gdll1_rvc = h_0->GetBinContent(h_0->GetXaxis()->FindBin(n_clocks - 0.5), 1 + _e_gdll1rvc);

  // fill event by event timing histogram and get time integrated bit info
  for (unsigned clk = 1; clk <= n_clocks; clk++) {
    int psn_tmp[3] = {0};
    int ftd_tmp[3] = {0};
    int itd_tmp[5] = {0};
    for (unsigned j = 0; j < nword_input; j++) {
      itd_tmp[j] = h_0->GetBinContent(clk, 1 + ee_itd[j]);
      itd[j] |= itd_tmp[j];
      for (int i = 0; i < 32; i++) {
        if (itd_tmp[j] & (1 << i)) h_i->SetBinContent(clk, i + 1 +  j * 32, 1);
      }
    }
    if (nconf == 0) {
      psn_tmp[0] = h_0->GetBinContent(clk, 1 + ee_psn[0]);
      ftd_tmp[0] = h_0->GetBinContent(clk, 1 + ee_ftd[0]);
      psn[0] |= psn_tmp[0];
      ftd[0] |= ftd_tmp[0];
      for (int i = 0; i < 32; i++) {
        if (psn_tmp[0] & (1 << i)) h_p->SetBinContent(clk, i + 1, 1);
        if (ftd_tmp[0] & (1 << i)) h_f->SetBinContent(clk, i + 1, 1);
      }
      psn_tmp[1] = h_0->GetBinContent(clk, 1 + ee_psn[2]) * (1 << 16) + h_0->GetBinContent(clk, 1 + ee_psn[1]);
      ftd_tmp[1] = h_0->GetBinContent(clk, 1 + ee_ftd[2]) * (1 << 16) + h_0->GetBinContent(clk, 1 + ee_ftd[1]);
      psn[1] |= psn_tmp[1];
      ftd[1] |= ftd_tmp[1];
      for (int i = 0; i < 32; i++) {
        if (psn_tmp[1] & (1 << i)) h_p->SetBinContent(clk, i + 1 + 32, 1);
        if (ftd_tmp[1] & (1 << i)) h_f->SetBinContent(clk, i + 1 + 32, 1);
      }
    } else {
      for (unsigned j = 0; j < nword_output; j++) {
        psn_tmp[j] = h_0->GetBinContent(clk, 1 + ee_psn[j]);
        ftd_tmp[j] = h_0->GetBinContent(clk, 1 + ee_ftd[j]);
        psn[j] |= psn_tmp[j];
        ftd[j] |= ftd_tmp[j];
        for (int i = 0; i < 32; i++) {
          if (psn_tmp[j] & (1 << i)) h_p->SetBinContent(clk, i + 1 +  j * 32, 1);
          if (ftd_tmp[j] & (1 << i)) h_f->SetBinContent(clk, i + 1 +  j * 32, 1);
        }
      }
    }
    int timtype_tmp = h_0->GetBinContent(clk, 1 + _e_timtype);
    timtype = (timtype_tmp == 0) ? timtype : timtype_tmp;

  } // clk

  // fill rising and falling edges
  fillRiseFallTimings();
  // fill Output_extra for efficiency study
  fillOutputExtra();

  // fill summary histograms
  h_timtype->Fill(timtype);
  h_itd->Fill(-0.5);
  h_ftd->Fill(-0.5);
  h_psn->Fill(-0.5);
  for (int i = 0; i < 32; i++) {
    for (unsigned j = 0; j < nword_input; j++) {
      if (itd[j] & (1 << i)) h_itd->Fill(i + 0.5 + 32 * j);
    }
    for (unsigned j = 0; j < nword_output; j++) {
      if (ftd[j] & (1 << i)) h_ftd->Fill(i + 0.5 + 32 * j);
      if (psn[j] & (1 << i)) h_psn->Fill(i + 0.5 + 32 * j);
    }
  }

  // fill timestamp values stored in header
  int gdlL1TocomL1 = gdll1_rvc < coml1rvc ? coml1rvc - gdll1_rvc : (coml1rvc + 1280) - gdll1_rvc;
  h_c8_gdlL1TocomL1->Fill(gdlL1TocomL1);

  int eclTogdlL1 = gdll1_rvc < c8_ecl_timing ? (gdll1_rvc + 1280) - c8_ecl_timing : gdll1_rvc - c8_ecl_timing;
  h_c8_eclTogdlL1->Fill(eclTogdlL1);

  int c127_ecl_timing = c8_ecl_timing & (((1 << 7) - 1) << 4);
  int fit8mToGDL = c127_ecl_timing < eclrvc ? eclrvc - c127_ecl_timing : (eclrvc + 1280) - c127_ecl_timing;
  h_c8_ecl8mToGDL->Fill(fit8mToGDL);

  int eclToGDL = c8_ecl_timing < eclrvc ? eclrvc - c8_ecl_timing : (eclrvc + 1280) - c8_ecl_timing;
  h_c8_eclToGDL->Fill(eclToGDL);

  int c2_comL1 = coml1rvc << 2;
  int c2_gdlL1 = gdll1_rvc << 2;
  int c2_diff_cdcTogdlL1 = c2_gdlL1 > c2_cdc_timing ?
                           c2_gdlL1 - c2_cdc_timing :
                           c2_gdlL1 - c2_cdc_timing + (1280 << 2) ;
  h_ns_cdcTogdlL1->Fill(c2_diff_cdcTogdlL1 * clkTo2ns);

  int c2_diff_cdcTocomL1 = c2_comL1 > c2_cdc_timing ?
                           c2_comL1 - c2_cdc_timing :
                           c2_comL1 - c2_cdc_timing + (1280 << 2) ;
  h_c2_cdcTocomL1->Fill(c2_diff_cdcTocomL1);
  h_ns_cdcTocomL1->Fill(c2_diff_cdcTocomL1 * clkTo2ns);

  int c2_diff_cdcToecl = c2_ecl_timing > c2_cdc_timing ?
                         c2_ecl_timing - c2_cdc_timing :
                         c2_ecl_timing - c2_cdc_timing + (1280 << 2);
  h_c2_cdcToecl->Fill(c2_diff_cdcToecl);
  h_ns_cdcToecl->Fill(c2_diff_cdcToecl * clkTo2ns);

  int c1_diff_topToecl = c1_ecl_timing > c1_top_timing ?
                         c1_ecl_timing - c1_top_timing :
                         c1_ecl_timing - c1_top_timing + (1280 << 3);
  h_ns_topToecl->Fill(c1_diff_topToecl *  clkTo1ns);

  int c2_diff_topTocdc = c2_cdc_timing > c2_top_timing ?
                         c2_cdc_timing - c2_top_timing :
                         c2_cdc_timing - c2_top_timing + (1280 << 2);
  h_ns_topTocdc->Fill(c2_diff_topTocdc *  clkTo2ns);

  // vcd dump
  if (m_dumpVcdFile) {
    if (anaBitCondition()) {
      nvcd++;
      B2DEBUG(20, "anaBitCondition fired, evt(" << evtno << ")");
      if (m_vcdEventStart <= nvcd && nvcd < m_vcdEventStart + m_vcdNumberOfEvents) {
        genVcd();
      }
    }
  }

  // discard event by event histograms
  if (! m_eventByEventTimingHistRecord) {
    h_0->Delete();
    h_p->Delete();
    h_f->Delete();
    h_i->Delete();
  }
}

bool TRGGDLDQMModule::anaBitCondition(void)
{
  if (m_bitConditionToDumpVcd.length() == 0) return true;
  TrgBit tb;
  const char* cst = m_bitConditionToDumpVcd.c_str();
  bool reading_word = false;
  bool result_the_term = true; // init value must be true
  bool not_flag = false;
  unsigned begin_word = 0;
  unsigned word_length = 0;
  // notation steeing side must follow
  //  no blank between '!' and word
  for (unsigned i = 0; i < m_bitConditionToDumpVcd.length(); i++) {
    if (('a' <= cst[i] && cst[i] <= 'z') ||
        ('A' <= cst[i] && cst[i] <= 'Z') ||
        ('_' == cst[i]) || ('!' == cst[i]) ||
        ('0' <= cst[i] && cst[i] <= '9')) {
      if (reading_word) { // must not be '!'
        word_length++;
        if (i == m_bitConditionToDumpVcd.length() - 1) {
          bool fired = isFired(m_bitConditionToDumpVcd.substr(begin_word, word_length));
          B2DEBUG(20,
                  m_bitConditionToDumpVcd.substr(begin_word, word_length).c_str()
                  << "(" << fired << ")");
          if (((!not_flag && fired) || (not_flag && !fired)) && result_the_term) {
            return true;
          }
        }
      } else {
        // start of new word
        reading_word = true;
        if ('!' == cst[i]) {
          begin_word = i + 1;
          not_flag = true;
          word_length = 0;
        } else {
          begin_word = i;
          not_flag = false;
          word_length = 1;
          if (i == m_bitConditionToDumpVcd.length() - 1) {
            // one char bit ('f',...) comes end of conditions, 'xxx+f'
            bool fired = isFired(m_bitConditionToDumpVcd.substr(begin_word, word_length));
            B2DEBUG(20,
                    m_bitConditionToDumpVcd.substr(begin_word, word_length).c_str()
                    << "(" << fired << ")");
            if (((!not_flag && fired) || (not_flag && !fired)) && result_the_term) {
              return true;
            }
          }
        }
      }
    } else if ('+' == cst[i] || i == m_bitConditionToDumpVcd.length() - 1) {
      // End of the term.
      if (reading_word) { // 'xxx+'
        if (result_the_term) {
          bool fired = isFired(m_bitConditionToDumpVcd.substr(begin_word, word_length));
          B2DEBUG(20,
                  m_bitConditionToDumpVcd.substr(begin_word, word_length).c_str()
                  << "(" << fired << ")");
          if ((!not_flag && fired) || (not_flag && !fired)) {
            return true;
          } else {
            // this term is denied by the latest bit
          }
        } else {
          // already false.
        }
        reading_word = false;
      } else {
        // prior char is blank, 'xxx  +'
        if (result_the_term) {
          return true;
        } else {
          // already false
        }
      }
      result_the_term = true; //  go to next term
    } else {
      // can be blank (white space) or any delimiter.
      if (reading_word) {
        // end of a word, 'xxxx '
        reading_word = false;
        if (result_the_term) {
          // worth to try
          bool fired = isFired(m_bitConditionToDumpVcd.substr(begin_word, word_length));
          B2DEBUG(20,
                  m_bitConditionToDumpVcd.substr(begin_word, word_length).c_str()
                  << "(" << fired << ")");
          if ((!not_flag && fired) || (not_flag && !fired)) {
            // go to next word
          } else {
            result_the_term = false;
          }
        } else {
          // already false
        }
        reading_word = false;
      } else {
        // 2nd blank 'xx  ' or leading blanck '^ '
      }
    }
  }
  return false;
}

void TRGGDLDQMModule::genVcd(void)
{
  TrgBit tb;
  int prev_i[400] = {0};
  int prev_f[400] = {0};
  int prev_p[400] = {0};
  //int prev_g[400]={0}; // Future Plan
  ofstream outf(Form("vcd/e%02dr%08de%08d.vcd", _exp, _run, evtno));
  outf << "$date" << endl;
  outf << "   Aug 20, 2018 17:53:52" << endl;
  outf << "$end" << endl;
  outf << "$version" << endl;
  outf << "   ChipScope Pro Analyzer  14.7 P.20131013 (Build 14700.13.286.464)" << endl;
  outf << "$end" << endl;
  outf << "$timescale" << endl;
  if (n_clocks == 32) {
    outf << "    32ns" << endl;
  } else if (n_clocks == 48) {
    outf << "    8ns" << endl;
  } else {
    outf << "    1ns" << endl;
  }
  outf << "$end" << endl;
  outf << "" << endl;
  outf << "$scope module gdl0067d_icn $end" << endl;
  int seqnum = 0;
  for (unsigned j = 0; j < tb.getNumOfInputs(); j++) {
    outf << "$var wire  1  n" << seqnum++ << " " << tb.getInputBitName(j) << " $end" << endl;
  }
  for (unsigned j = 0; j < tb.getNumOfOutputs(); j++) {
    outf << "$var wire  1  n" << seqnum++ << " ftd." << tb.getOutputBitName(j) << " $end" << endl;
  }
  for (unsigned j = 0; j < tb.getNumOfOutputs(); j++) {
    outf << "$var wire  1  n" << seqnum++ << " psn." << tb.getOutputBitName(j) << " $end" << endl;
  }

  outf << "$upscope $end" << endl;
  outf << "$enddefinitions $end" << endl << endl;

  for (unsigned clk = 1; clk <= n_clocks; clk++) {
    seqnum = 0;
    outf << "#" << clk - 1 << endl;
    for (unsigned k = 1; k <= tb.getNumOfInputs(); k++) {
      if (clk == 1 || prev_i[k - 1] != h_i->GetBinContent(clk, k)) {
        prev_i[k - 1] = h_i->GetBinContent(clk, k);
        outf << h_i->GetBinContent(clk, k) << "n" << seqnum << endl;
      }
      seqnum++;
    }
    for (unsigned k = 1; k <= tb.getNumOfOutputs(); k++) {
      if (clk == 1 || prev_f[k - 1] != h_f->GetBinContent(clk, k)) {
        prev_f[k - 1] = h_f->GetBinContent(clk, k);
        outf << h_f->GetBinContent(clk, k) << "n" << seqnum << endl;
      }
      seqnum++;
    }
    for (unsigned k = 1; k <= tb.getNumOfOutputs(); k++) {
      if (clk == 1 || prev_p[k - 1] != h_p->GetBinContent(clk, k)) {
        prev_p[k - 1] = h_p->GetBinContent(clk, k);
        outf << h_p->GetBinContent(clk, k) << "n" << seqnum << endl;
      }
      seqnum++;
    }
    /* Future Plan
    for(unsigned k=1; k<=n_gdlbits; k++){
      if(clk == 1 || prev_g[k-1] != hg->GetBinContent(clk, k)){
    prev_g[k-1] = hg->GetBinContent(clk, k);
    if(EBitWidth[k-1] != 1){
    char ans[33];
    if(k-1 == e_rvc){
      dec2binstring(hg->GetBinContent(clk, k), ans, true);
    }else{
      dec2binstring(hg->GetBinContent(clk, k), ans);
    }
    outf << "b" << ans << " n" << seqnum << endl;
    }else{
    outf << hg->GetBinContent(clk, k) << "n" << seqnum << endl;
    }
      }
      seqnum++;
    }
    */
  }
  outf.close();
}

bool
TRGGDLDQMModule::isFired(std::string bitname)
{
  TrgBit tb;
  bool isPsnm = false;
  for (unsigned i = 0; i < bitname.length(); i++) {
    if ('A' <= bitname[i] && bitname[i] <= 'Z') {
      isPsnm = true;
    }
  }
  boost::algorithm::to_lower(bitname);
  int bn = tb.getBitNum(bitname.c_str());
  for (unsigned clk = 0; clk < n_clocks; clk++) {
    if (bn < 0) {
      if (isPsnm) {
        if (h_p->GetBinContent(clk + 1, abs(bn)) > 0)
          return true;
      } else {
        if (h_f->GetBinContent(clk + 1, abs(bn)) > 0)
          return true;
      }
    } else {
      if (h_i->GetBinContent(clk + 1, bn + 1) > 0)
        return true;
    }
  }
  return false;
}

void
TRGGDLDQMModule::fillRiseFallTimings(void)
{
  TrgBit tb;

  for (unsigned i = 0; i < tb.getNumOfInputs(); i++) {
    if (n_clocks == 32) {
      h_itd_rise[i]->GetXaxis()->SetTitle("clk32ns");
      h_itd_fall[i]->GetXaxis()->SetTitle("clk32ns");
      h_itd_rise[i]->GetXaxis()->SetRange(1, 32);
      h_itd_fall[i]->GetXaxis()->SetRange(1, 32);
    } else {
      h_itd_rise[i]->GetXaxis()->SetTitle("clk8ns");
      h_itd_fall[i]->GetXaxis()->SetTitle("clk8ns");
    }
    bool rising_done = false;
    bool falling_done = false;
    for (unsigned clk = 0; clk < n_clocks; clk++) {
      if (h_i->GetBinContent(clk + 1, i + 1) > 0) {
        if (! rising_done) {
          h_itd_rise[i]->Fill(clk + 0.5);
          rising_done = true;
        } else if (rising_done && !falling_done && clk == n_clocks - 1) {
          h_itd_fall[i]->Fill(clk + 0.5);
        }
      } else if (h_i->GetBinContent(clk + 1, i + 1) == 0) {
        if (rising_done && ! falling_done) {
          h_itd_fall[i]->Fill(clk + 0.5);
          falling_done = true;
        }
      }
    }
  }
  for (unsigned i = 0; i < tb.getNumOfOutputs(); i++) {
    if (n_clocks == 32) {
      h_ftd_rise[i]->GetXaxis()->SetTitle("clk32ns");
      h_psn_rise[i]->GetXaxis()->SetTitle("clk32ns");
      h_ftd_fall[i]->GetXaxis()->SetTitle("clk32ns");
      h_psn_fall[i]->GetXaxis()->SetTitle("clk32ns");
      h_ftd_rise[i]->GetXaxis()->SetRange(1, 32);
      h_psn_rise[i]->GetXaxis()->SetRange(1, 32);
      h_ftd_fall[i]->GetXaxis()->SetRange(1, 32);
      h_psn_fall[i]->GetXaxis()->SetRange(1, 32);
    } else {
      h_ftd_rise[i]->GetXaxis()->SetTitle("clk8ns");
      h_psn_rise[i]->GetXaxis()->SetTitle("clk8ns");
      h_ftd_fall[i]->GetXaxis()->SetTitle("clk8ns");
      h_psn_fall[i]->GetXaxis()->SetTitle("clk8ns");
    }
    bool rising_done = false;
    bool falling_done = false;
    for (unsigned clk = 0; clk < n_clocks; clk++) {
      if (h_f->GetBinContent(clk + 1, i + 1) > 0) {
        if (! rising_done) {
          h_ftd_rise[i]->Fill(clk + 0.5);
          rising_done = true;
        } else if (rising_done && !falling_done && clk == n_clocks - 1) {
          h_ftd_fall[i]->Fill(clk + 0.5);
        }
      } else if (h_f->GetBinContent(clk + 1, i + 1) == 0) {
        if (rising_done && ! falling_done) {
          h_ftd_fall[i]->Fill(clk + 0.5);
          falling_done = true;
        }
      }
    }
    rising_done = false;
    falling_done = false;
    for (unsigned clk = 0; clk < n_clocks; clk++) {
      if (h_p->GetBinContent(clk + 1, i + 1) > 0) {
        if (! rising_done) {
          h_psn_rise[i]->Fill(clk + 0.5);
          rising_done = true;
        } else if (rising_done && !falling_done && clk == n_clocks - 1) {
          h_psn_fall[i]->Fill(clk + 0.5);
        }
      } else if (h_p->GetBinContent(clk + 1, i + 1) == 0) {
        if (rising_done && ! falling_done) {
          h_psn_fall[i]->Fill(clk + 0.5);
          falling_done = true;
        }
      }
    }
  }
}

void
TRGGDLDQMModule::fillOutputExtra(void)
{
  bool c4_fired = isFired("C4");
  bool hie_fired = isFired("HIE");
  bool fff_fired = isFired("FFF");
  if (fff_fired && (c4_fired || hie_fired)) {
    h_psn_extra->Fill(0.5);
  }
  if (c4_fired || hie_fired) {
    h_psn_extra->Fill(1.5);
  }
  if (fff_fired && hie_fired) {
    h_psn_extra->Fill(2.5);
  }
  if (fff_fired && c4_fired) {
    h_psn_extra->Fill(3.5);
  }
}

const int TRGGDLDQMModule::n_output_extra = 4;
const char* TRGGDLDQMModule::output_extra[4] = {
  "fff&(c4|hie)", "c4|hie", "fff&hie", "fff&c4"
};
