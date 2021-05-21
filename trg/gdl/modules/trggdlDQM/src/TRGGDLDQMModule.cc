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

#include <TDirectory.h>
#include <TPostScript.h>
#include <TCanvas.h>
#include <TStyle.h>
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
           true);
  addParam("generatePostscript", m_generatePostscript,
           "Genarete postscript file or not",
           false);
  addParam("postScriptName", m_postScriptName,
           "postscript file name",
           string("gdldqm.ps"));
  addParam("skim", m_skim,
           "use skim information or not",
           int(-1));
  B2DEBUG(20, "eventByEventTimingFlag(" << m_eventByEventTimingHistRecord
          << "), m_dumpVcdFile(" << m_dumpVcdFile
          << "), m_bitConditionToDumpVcd(" << m_bitConditionToDumpVcd
          << "), m_vcdEventStart(" << m_vcdEventStart
          << "), m_vcdNumberOfEvents(" << m_vcdNumberOfEvents);


}

void TRGGDLDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = gDirectory;
  if (!oldDir->Get("TRGGDL"))oldDir->mkdir("TRGGDL");
  dirDQM->cd("TRGGDL");

  for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {
    h_c8_gdlL1TocomL1[iskim]  = new TH1I(Form("hGDL_gdlL1TocomL1_%s", skim_smap[iskim].c_str()),  "comL1 - gdlL1 [clk8ns]", 100, 0,
                                         100);
    h_c8_gdlL1TocomL1[iskim]->GetXaxis()->SetTitle("clk8ns");

    h_c8_topTogdlL1[iskim]    = new TH1I(Form("hGDL_topTogdlL1_%s", skim_smap[iskim].c_str()),    "gdlL1 - top_timing [clk8ns]", 700, 0,
                                         700);
    h_c8_topTogdlL1[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_eclTogdlL1[iskim]    = new TH1I(Form("hGDL_eclTogdlL1_%s", skim_smap[iskim].c_str()),    "gdlL1 - ecl_timing [clk8ns]", 500, 0,
                                         500);
    h_c8_eclTogdlL1[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_cdcTogdlL1[iskim]    = new TH1I(Form("hGDL_cdcTogdlL1_%s", skim_smap[iskim].c_str()),    "gdlL1 - cdc_timing [clk8ns]", 700, 0,
                                         700);
    h_c8_cdcTogdlL1[iskim]->GetXaxis()->SetTitle("clk8ns");

    h_c8_ecl8mToGDL[iskim]    = new TH1I(Form("hGDL_ecl8mToGDL_%s", skim_smap[iskim].c_str()),    "gdlIn^{8MHz} - ecl_timing [clk8ns]",
                                         500, 0, 500);
    h_c8_ecl8mToGDL[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_topToGDL[iskim]      = new TH1I(Form("hGDL_topToGDL_%s", skim_smap[iskim].c_str()),      "gdlIn - top_timing [clk8ns]", 700, 0,
                                         700);
    h_c8_topToGDL[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_eclToGDL[iskim]      = new TH1I(Form("hGDL_eclToGDL_%s", skim_smap[iskim].c_str()),      "gdlIn - ecl_timing [clk8ns]", 500, 0,
                                         500);
    h_c8_eclToGDL[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_cdcToGDL[iskim]      = new TH1I(Form("hGDL_cdcToGDL_%s", skim_smap[iskim].c_str()),      "gdlIn - cdc_timing [clk8ns]", 700, 0,
                                         700);
    h_c8_cdcToGDL[iskim]->GetXaxis()->SetTitle("clk8ns");

    h_c2_cdcTocomL1[iskim] = new TH1I(Form("hGDL_cdcTocomL1_%s", skim_smap[iskim].c_str()), "comL1 - cdc_timing [clk2ns]", 520, 0,
                                      5200);
    h_c2_cdcTocomL1[iskim]->GetXaxis()->SetTitle("clk2ns");
    h_ns_cdcTocomL1[iskim] = new TH1D(Form("hGDL_ns_cdcTocomL1_%s", skim_smap[iskim].c_str()), "comL1 - cdc_timing [ns]", 2600, 0,
                                      10400);
    h_ns_cdcTocomL1[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_cdcTocomL1[iskim]->GetYaxis()->SetTitle("evt / 4ns");
    h_ns_cdcTogdlL1[iskim] = new TH1D(Form("hGDL_ns_cdcTogdlL1_%s", skim_smap[iskim].c_str()), "gdlL1 - cdc_timing [ns]", 2600, 0,
                                      10400);
    h_ns_cdcTogdlL1[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_cdcTogdlL1[iskim]->GetYaxis()->SetTitle("evt / 4ns");

    h_ns_topToecl[iskim] = new TH1D(Form("hGDL_ns_topToecl_%s", skim_smap[iskim].c_str()), "ecl_timing - top_timing [ns]", 800, 0,
                                    4000);
    h_ns_topToecl[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_topToecl[iskim]->GetYaxis()->SetTitle("evt / 5ns");
    h_ns_topTocdc[iskim] = new TH1D(Form("hGDL_ns_topTocdc_%s", skim_smap[iskim].c_str()), "cdc_timing - top_timing [ns]", 800, 0,
                                    4000);
    h_ns_topTocdc[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_topTocdc[iskim]->GetYaxis()->SetTitle("evt / 5ns");
    h_ns_cdcToecl[iskim] = new TH1D(Form("hGDL_ns_cdcToecl_%s", skim_smap[iskim].c_str()), "ecl_timing - cdc_timing [ns]", 2000, 0,
                                    4000);
    h_ns_cdcToecl[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_cdcToecl[iskim]->GetYaxis()->SetTitle("evt / 2ns");

    h_c2_cdcToecl[iskim] = new TH1I(Form("hGDL_cdcToecl_%s", skim_smap[iskim].c_str()), "ecl_timing - cdc_timing [clk2ns]", 1000, 0,
                                    2000);
    h_c2_cdcToecl[iskim]->GetXaxis()->SetTitle("clk2ns");

    h_timtype[iskim] = new TH1I(Form("hGDL_timtype_%s", skim_smap[iskim].c_str()), "timtype", 7, 0, 7);

    h_itd[iskim] = new TH1I(Form("hGDL_itd_%s", skim_smap[iskim].c_str()), "itd", n_inbit + 1, -1, n_inbit);
    h_ftd[iskim] = new TH1I(Form("hGDL_ftd_%s", skim_smap[iskim].c_str()), "ftd", n_outbit + 1, -1, n_outbit);
    h_psn[iskim] = new TH1I(Form("hGDL_psn_%s", skim_smap[iskim].c_str()), "psn", n_outbit + 1, -1, n_outbit);
    // output extra
    h_psn_extra[iskim] = new TH1I(Form("hGDL_psn_extra_%s", skim_smap[iskim].c_str()), "psn extra", n_output_extra, 0, n_output_extra);
    for (int i = 0; i < n_output_extra; i++) {
      h_psn_extra[iskim]->GetXaxis()->SetBinLabel(i + 1, output_extra[i]);
    }
    // output overlap
    h_psn_effect_to_l1[iskim] = new TH1I(Form("hGDL_psn_effect_to_l1_%s", skim_smap[iskim].c_str()), "psn effect to l1",
                                         n_output_overlap, 0,
                                         n_output_overlap);
    h_psn_effect_to_l1[iskim]->LabelsOption("v");
    for (int i = 0; i < n_output_overlap; i++) {
      h_psn_effect_to_l1[iskim]->GetXaxis()->SetBinLabel(i + 1, output_overlap[i]);
    }
    // output no overlap
    h_psn_raw_rate[iskim] = new TH1I(Form("hGDL_psn_raw_rate_%s", skim_smap[iskim].c_str()), "psn raw rate", n_output_overlap, 0,
                                     n_output_overlap);
    h_psn_raw_rate[iskim]->LabelsOption("v");
    for (int i = 0; i < n_output_overlap; i++) {
      h_psn_raw_rate[iskim]->GetXaxis()->SetBinLabel(i + 1, output_overlap[i]);
    }
    // output pure extra
    h_psn_pure_extra[iskim] = new TH1I(Form("hGDL_psn_pure_extra_%s", skim_smap[iskim].c_str()), "psn pure extra", n_output_pure_extra,
                                       0, n_output_pure_extra);
    for (int i = 0; i < n_output_pure_extra; i++) {
      h_psn_pure_extra[iskim]->GetXaxis()->SetBinLabel(i + 1, output_pure_extra[i]);
    }

    h_itd[iskim]->GetXaxis()->SetBinLabel(h_itd[iskim]->GetXaxis()->FindBin(-1 + 0.5), "all");
    h_ftd[iskim]->GetXaxis()->SetBinLabel(h_ftd[iskim]->GetXaxis()->FindBin(-1 + 0.5), "all");
    h_psn[iskim]->GetXaxis()->SetBinLabel(h_psn[iskim]->GetXaxis()->FindBin(-1 + 0.5), "all");
    for (unsigned i = 0; i < n_inbit; i++) {
      if (m_bitNameOnBinLabel) {
        h_itd[iskim]->GetXaxis()->SetBinLabel(h_itd[iskim]->GetXaxis()->FindBin(i + 0.5), inbitname[i]);
      }
    }
    for (unsigned i = 0; i < n_outbit; i++) {
      if (m_bitNameOnBinLabel) {
        h_ftd[iskim]->GetXaxis()->SetBinLabel(h_ftd[iskim]->GetXaxis()->FindBin(i + 0.5), outbitname[i]);
        h_psn[iskim]->GetXaxis()->SetBinLabel(h_psn[iskim]->GetXaxis()->FindBin(i + 0.5), outbitname[i]);
      }
    }

    //reduce #plot
    if (iskim != 0)continue;

    // rise/fall
    for (unsigned i = 0; i < n_inbit; i++) {
      h_itd_rise[i][iskim] = new TH1I(Form("hGDL_itd_%s_rise_%s", inbitname[i], skim_smap[iskim].c_str()),
                                      Form("itd%d(%s) rising", i, inbitname[i]), 48, 0, 48);
      h_itd_rise[i][iskim]->SetLineColor(kRed);
      h_itd_fall[i][iskim] = new TH1I(Form("hGDL_itd_%s_fall_%s", inbitname[i], skim_smap[iskim].c_str()),
                                      Form("itd%d(%s) falling", i, inbitname[i]), 48, 0, 48);
      h_itd_fall[i][iskim]->SetLineColor(kGreen);
    }
    for (unsigned i = 0; i < n_outbit; i++) {
      h_ftd_rise[i][iskim] = new TH1I(Form("hGDL_ftd_%s_rise_%s", outbitname[i], skim_smap[iskim].c_str()),
                                      Form("ftd%d(%s) rising", i, outbitname[i]), 48, 0, 48);
      h_ftd_rise[i][iskim]->SetLineColor(kRed);
      h_ftd_fall[i][iskim] = new TH1I(Form("hGDL_ftd_%s_fall_%s", outbitname[i], skim_smap[iskim].c_str()),
                                      Form("ftd%d(%s) falling", i, outbitname[i]), 48, 0, 48);
      h_ftd_fall[i][iskim]->SetLineColor(kGreen);
      h_psn_rise[i][iskim] = new TH1I(Form("hGDL_psn_%s_rise_%s", outbitname[i], skim_smap[iskim].c_str()),
                                      Form("psn%d(%s) rising", i, outbitname[i]), 48, 0, 48);
      h_psn_rise[i][iskim]->SetLineColor(kRed);
      h_psn_fall[i][iskim] = new TH1I(Form("hGDL_psn_%s_fall_%s", outbitname[i], skim_smap[iskim].c_str()),
                                      Form("psn%d(%s) falling", i, outbitname[i]), 48, 0, 48);
      h_psn_fall[i][iskim]->SetLineColor(kGreen);
    }
  }

  oldDir->cd();
}

void TRGGDLDQMModule::beginRun()
{

  dirDQM->cd();

  for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {
    h_c8_gdlL1TocomL1[iskim]->Reset();
    h_c8_topTogdlL1[iskim]->Reset();
    h_c8_eclTogdlL1[iskim]->Reset();
    h_c8_cdcTogdlL1[iskim]->Reset();
    h_c8_ecl8mToGDL[iskim]->Reset();
    h_c8_topToGDL[iskim]->Reset();
    h_c8_eclToGDL[iskim]->Reset();
    h_c8_cdcToGDL[iskim]->Reset();
    h_c2_cdcTocomL1[iskim]->Reset();
    h_ns_cdcTocomL1[iskim]->Reset();
    h_ns_cdcTogdlL1[iskim]->Reset();
    h_ns_topToecl[iskim]->Reset();
    h_ns_topTocdc[iskim]->Reset();
    h_c2_cdcToecl[iskim]->Reset();
    h_ns_cdcToecl[iskim]->Reset();
    h_itd[iskim]->Reset();
    h_ftd[iskim]->Reset();
    h_psn[iskim]->Reset();
    h_psn_extra[iskim]->Reset();
    h_psn_pure_extra[iskim]->Reset();
    h_timtype[iskim]->Reset();
    h_psn_raw_rate[iskim]->Reset();
    h_psn_effect_to_l1[iskim]->Reset();
  }

  oldDir->cd();
}

void TRGGDLDQMModule::initialize()
{

  if (m_skim == 0) { //no skim
    start_skim_gdldqm = 0;
    end_skim_gdldqm = 1;
  } else if (m_skim == 1) { //skim
    start_skim_gdldqm = 1;
    end_skim_gdldqm = nskim_gdldqm;
  } else { //no skim + skim
    start_skim_gdldqm = 0;
    end_skim_gdldqm = nskim_gdldqm;
  }

  _exp = bevt->getExperiment();
  _run = bevt->getRun();

  trgeclmap = new TrgEclMapping();

  // calls back the defineHisto() function, but the HistoManager module has to be in the path
  REG_HISTOGRAM

  for (int i = 0; i < 320; i++) {
    LeafBitMap[i] = m_unpacker->getLeafMap(i);
  }
  for (int i = 0; i < 320; i++) {
    strcpy(LeafNames[i], m_unpacker->getLeafnames(i));
  }
  _e_timtype = 0;
  _e_gdll1rvc = 0;
  _e_coml1rvc = 0;
  _e_toptiming = 0;
  _e_ecltiming = 0;
  _e_cdctiming = 0;
  _e_toprvc  = 0;
  _e_eclrvc  = 0;
  _e_cdcrvc  = 0;
  for (int i = 0; i < 10; i++) {
    ee_psn[i] = {0};
    ee_ftd[i] = {0};
    ee_itd[i] = {0};
  }
  for (int i = 0; i < 320; i++) {
    if (strcmp(LeafNames[i], "timtype") == 0)  _e_timtype  = LeafBitMap[i];
    if (strcmp(LeafNames[i], "gdll1rvc") == 0) _e_gdll1rvc = LeafBitMap[i];
    if (strcmp(LeafNames[i], "coml1rvc") == 0) _e_coml1rvc = LeafBitMap[i];
    if (strcmp(LeafNames[i], "toptiming") == 0)_e_toptiming = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ecltiming") == 0)_e_ecltiming = LeafBitMap[i];
    if (strcmp(LeafNames[i], "cdctiming") == 0)_e_cdctiming = LeafBitMap[i];
    if (strcmp(LeafNames[i], "toprvc") == 0)   _e_toprvc   = LeafBitMap[i];
    if (strcmp(LeafNames[i], "eclrvc") == 0)   _e_eclrvc   = LeafBitMap[i];
    if (strcmp(LeafNames[i], "cdcrvc") == 0)   _e_cdcrvc   = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn0") == 0)       ee_psn[0] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn1") == 0)       ee_psn[1] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn2") == 0)       ee_psn[2] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn3") == 0)       ee_psn[3] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn4") == 0)       ee_psn[4] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn5") == 0)       ee_psn[5] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn6") == 0)       ee_psn[6] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn7") == 0)       ee_psn[7] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn8") == 0)       ee_psn[8] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn9") == 0)       ee_psn[9] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd0") == 0)       ee_ftd[0] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd1") == 0)       ee_ftd[1] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd2") == 0)       ee_ftd[2] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd3") == 0)       ee_ftd[3] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd4") == 0)       ee_ftd[4] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd5") == 0)       ee_ftd[5] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd6") == 0)       ee_ftd[6] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd7") == 0)       ee_ftd[7] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd8") == 0)       ee_ftd[8] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd9") == 0)       ee_ftd[9] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd0") == 0)       ee_itd[0] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd1") == 0)       ee_itd[1] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd2") == 0)       ee_itd[2] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd3") == 0)       ee_itd[3] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd4") == 0)       ee_itd[4] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd5") == 0)       ee_itd[5] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd6") == 0)       ee_itd[6] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd7") == 0)       ee_itd[7] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd8") == 0)       ee_itd[8] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd9") == 0)       ee_itd[9] = LeafBitMap[i];
  }

  n_inbit = m_dbinput->getninbit();
  n_outbit = m_dbftdl ->getnoutbit();
  for (int i = 0; i < 320; i++) {
    strcpy(inbitname[i], m_dbinput->getinbitname(i));
  }
  for (int i = 0; i < 320; i++) {
    strcpy(outbitname[i], m_dbftdl->getoutbitname(i));
  }
  n_leafs  = m_unpacker->getnLeafs();
  n_leafsExtra = m_unpacker->getnLeafsExtra();
  n_clocks = m_unpacker->getnClks();
  nconf = m_unpacker->getconf();
  nword_input  = m_unpacker->get_nword_input();
  nword_output = m_unpacker->get_nword_output();

  for (unsigned i = 0; i < n_clocks; i++) {
    for (int j = 0; j < n_leafs + n_leafsExtra; j++)h_0_vec.push_back(0);
    for (int j = 0; j < n_outbit; j++)              h_p_vec.push_back(0);
    for (int j = 0; j < n_outbit; j++)              h_f_vec.push_back(0);
    for (int j = 0; j < n_inbit; j++)               h_i_vec.push_back(0);
  }

}

void TRGGDLDQMModule::endRun()
{
  if (m_generatePostscript) {
    TPostScript* ps = new TPostScript(m_postScriptName.c_str(), 112);
    gStyle->SetOptStat(0);
    TCanvas c1("c1", "", 0, 0, 500, 300);
    c1.cd();

    for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {

      h_itd[iskim]->GetXaxis()->SetRange(h_itd[iskim]->GetXaxis()->FindBin(0.5),
                                         h_itd[iskim]->GetXaxis()->FindBin(n_inbit - 0.5));
      h_itd[iskim]->Draw();
      c1.Update();
      h_ftd[iskim]->GetXaxis()->SetRange(h_ftd[iskim]->GetXaxis()->FindBin(0.5),
                                         h_ftd[iskim]->GetXaxis()->FindBin(n_outbit - 0.5));
      h_ftd[iskim]->Draw();
      c1.Update();
      h_psn[iskim]->GetXaxis()->SetRange(h_psn[iskim]->GetXaxis()->FindBin(0.5),
                                         h_psn[iskim]->GetXaxis()->FindBin(n_outbit - 0.5));
      h_psn[iskim]->Draw();
      c1.Update();
      h_ftd[iskim]->SetTitle("ftd(green), psnm(red)");
      h_ftd[iskim]->SetFillColor(kGreen);
      h_ftd[iskim]->SetBarWidth(0.4);
      h_ftd[iskim]->Draw("bar");
      h_psn[iskim]->SetFillColor(kRed);
      h_psn[iskim]->SetBarWidth(0.4);
      h_psn[iskim]->SetBarOffset(0.5);
      h_psn[iskim]->Draw("bar,same");

      c1.Update();
      h_timtype[iskim]->Draw();
      c1.Update();
      h_c8_gdlL1TocomL1[iskim]->Draw();
      c1.Update();
      h_c8_topTogdlL1[iskim]->Draw();
      c1.Update();
      h_c8_eclTogdlL1[iskim]->Draw();
      c1.Update();
      h_c8_cdcTogdlL1[iskim]->Draw();
      c1.Update();
      h_c8_ecl8mToGDL[iskim]->Draw();
      c1.Update();
      h_c8_topToGDL[iskim]->Draw();
      c1.Update();
      h_c8_eclToGDL[iskim]->Draw();
      c1.Update();
      h_c8_cdcToGDL[iskim]->Draw();
      c1.Update();
      h_c2_cdcTocomL1[iskim]->Draw();
      c1.Update();
      h_ns_cdcTocomL1[iskim]->Draw();
      c1.Update();
      h_ns_cdcTogdlL1[iskim]->Draw();
      c1.Update();
      h_ns_topToecl[iskim]->Draw();
      c1.Update();
      h_ns_topTocdc[iskim]->Draw();
      c1.Update();
      h_c2_cdcToecl[iskim]->Draw();
      c1.Update();
      h_ns_cdcToecl[iskim]->Draw();
      c1.Update();

      //reduce #plot
      if (iskim != 0)continue;
      for (unsigned i = 0; i < n_inbit; i++) {

        h_itd_rise[i][iskim]->SetTitle(Form("itd%d(%s) rising(red), falling(green)",
                                            i, inbitname[i]));
        h_itd_rise[i][iskim]->Draw();
        h_itd_fall[i][iskim]->Draw("same");
        c1.Update();
      }

    }

    ps->Close();
  }
}

void TRGGDLDQMModule::event()
{
  /* cppcheck-suppress variableScope */
  static unsigned nvcd = 0;
  static bool begin_run = true;

  skim.clear();

  if (!entAry || !entAry.getEntries()) {
    return;
  }

  //Get skim type from SoftwareTriggerResult
  for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {
    if (iskim == 0) skim.push_back(iskim);
  }
  if (result_soft.isValid()) {
    const std::map<std::string, int>& skim_map = result_soft->getResults();
    for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {
      if (iskim == 0);
      else if (skim_map.find(skim_menu[iskim]) != skim_map.end()) {
        const bool accepted = (result_soft->getResult(skim_menu[iskim]) == SoftwareTriggerCutResult::c_accept);
        if (accepted) skim.push_back(iskim);
      }
    }
  }


  //prepare entAry adress
  int clk_map = 0;
  for (int i = 0; i < 320; i++) {
    if (strcmp(entAry[0]->m_unpackername[i], "evt") == 0) evtno = entAry[0]->m_unpacker[i];
    if (strcmp(entAry[0]->m_unpackername[i], "clk") == 0) clk_map = i;
  }

  const double clkTo2ns = 1. / .508877;
  const double clkTo1ns = 0.5 / .508877;

  dirDQM->cd();

  for (unsigned i = 0; i < n_clocks; i++) {
    for (int j = 0; j < n_leafs + n_leafsExtra; j++) {
      h_0_vec[i * (n_leafs + n_leafsExtra) + j] = 0;
    }
    for (unsigned j = 0; j < n_outbit; j++) {
      h_p_vec[i * n_outbit + j] = 0;
    }
    for (unsigned j = 0; j < n_outbit; j++) {
      h_f_vec[i * n_outbit + j] = 0;
    }
    for (unsigned j = 0; j < n_inbit; j++) {
      h_i_vec[i * n_inbit + j] = 0;
    }
  }

  oldDir->cd();

  // fill "bit vs clk" for the event
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    std::vector<int*> Bits(n_leafs + n_leafsExtra);
    //set pointer
    for (int i = 0; i < 320; i++) {
      if (LeafBitMap[i] != -1) {
        Bits[LeafBitMap[i]] = &(entAry[ii]->m_unpacker[i]);
      }
    }
    for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
      h_0_vec[(entAry[ii]->m_unpacker[clk_map]) * (n_leafs + n_leafsExtra) + leaf] = *Bits[leaf];
    }
  }
  int coml1rvc      = h_0_vec[0 * (n_leafs + n_leafsExtra) + _e_coml1rvc];
  int toprvc        = h_0_vec[0 * (n_leafs + n_leafsExtra) + _e_toprvc];
  int eclrvc        = h_0_vec[0 * (n_leafs + n_leafsExtra) + _e_eclrvc];
  int cdcrvc        = h_0_vec[0 * (n_leafs + n_leafsExtra) + _e_cdcrvc];
  int c1_top_timing = h_0_vec[(n_clocks - 1) * (n_leafs + n_leafsExtra) + _e_toptiming];
  int c1_ecl_timing = h_0_vec[(n_clocks - 1) * (n_leafs + n_leafsExtra) + _e_ecltiming];
  int c1_cdc_timing = h_0_vec[(n_clocks - 1) * (n_leafs + n_leafsExtra) + _e_cdctiming];
  int c8_top_timing = c1_top_timing >> 3;
  int c2_top_timing = c1_top_timing >> 1;
  int c8_ecl_timing = c1_ecl_timing >> 3;
  int c2_ecl_timing = c1_ecl_timing >> 1;
  int c8_cdc_timing = c1_cdc_timing >> 3;
  int c2_cdc_timing = c1_cdc_timing >> 1;

  if (begin_run) {
    B2DEBUG(20, "nconf(" << nconf
            << "), n_clocks(" << n_clocks
            << "), n_leafs(" << n_leafs
            << "), n_leafsExtra(" << n_leafsExtra
            << ")");
    begin_run = false;
  }

  int psn[10] = {0};
  int ftd[10] = {0};
  int itd[10] = {0};
  int timtype  = 0;


  int gdll1_rvc = h_0_vec[(n_clocks - 1) * (n_leafs + n_leafsExtra) + _e_gdll1rvc];

  // fill event by event timing histogram and get time integrated bit info
  for (unsigned clk = 1; clk <= n_clocks; clk++) {
    int psn_tmp[10] = {0};
    int ftd_tmp[10] = {0};
    int itd_tmp[10] = {0};
    for (unsigned j = 0; j < (unsigned)nword_input; j++) {
      itd_tmp[j] = h_0_vec[(clk - 1) * (n_leafs + n_leafsExtra) + ee_itd[j]];
      itd[j] |= itd_tmp[j];
      for (int i = 0; i < 32; i++) {
        if (i + j * 32 >= n_inbit)continue;
        if (itd_tmp[j] & (1 << i)) h_i_vec[(clk - 1)*n_inbit + i +  j * 32] = 1;
      }
    }
    if (nconf == 0) {
      psn_tmp[0] = h_0_vec[(clk - 1) * (n_leafs + n_leafsExtra) + ee_psn[0]];
      ftd_tmp[0] = h_0_vec[(clk - 1) * (n_leafs + n_leafsExtra) + ee_ftd[0]];
      psn[0] |= psn_tmp[0];
      ftd[0] |= ftd_tmp[0];
      for (int i = 0; i < 32; i++) {
        if (i >= n_outbit)continue;
        if (psn_tmp[0] & (1 << i)) h_p_vec[(clk - 1)*n_outbit + i] = 1;
        if (ftd_tmp[0] & (1 << i)) h_f_vec[(clk - 1)*n_outbit + i] = 1;
      }
      psn_tmp[1] = h_0_vec[(clk - 1) * n_outbit + ee_psn[2]] * (1 << 16) + h_0_vec[(clk - 1) * n_outbit + ee_psn[1]];
      ftd_tmp[1] = h_0_vec[(clk - 1) * n_outbit + ee_ftd[2]] * (1 << 16) + h_0_vec[(clk - 1) * n_outbit + ee_ftd[1]];
      psn[1] |= psn_tmp[1];
      ftd[1] |= ftd_tmp[1];
      for (int i = 0; i < 32; i++) {
        if (i + 32 >= n_outbit)continue;
        if (psn_tmp[1] & (1 << i)) h_p_vec[(clk - 1)*n_outbit + i + 32] = 1;
        if (ftd_tmp[1] & (1 << i)) h_f_vec[(clk - 1)*n_outbit + i + 32] = 1;
      }
    } else {
      for (unsigned j = 0; j < (unsigned)nword_output; j++) {
        psn_tmp[j] = h_0_vec[(clk - 1) * (n_leafs + n_leafsExtra) + ee_psn[j]];
        ftd_tmp[j] = h_0_vec[(clk - 1) * (n_leafs + n_leafsExtra) + ee_ftd[j]];
        psn[j] |= psn_tmp[j];
        ftd[j] |= ftd_tmp[j];
        for (int i = 0; i < 32; i++) {
          if (i + j * 32 >= n_outbit)continue;
          if (psn_tmp[j] & (1 << i)) h_p_vec[(clk - 1)*n_outbit + i  +  j * 32] = 1;
          if (ftd_tmp[j] & (1 << i)) h_f_vec[(clk - 1)*n_outbit + i  +  j * 32] = 1;
        }
      }
    }
    int timtype_tmp = h_0_vec[(clk - 1) * (n_leafs + n_leafsExtra) + _e_timtype];
    timtype = (timtype_tmp == 0) ? timtype : timtype_tmp;

  } // clk


  // fill rising and falling edges
  fillRiseFallTimings();
  // fill Output_extra for efficiency study
  fillOutputExtra();
  // fill Output_overlap for trigger rate study
  fillOutputOverlap();

  // fill summary histograms
  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    h_timtype[skim[ifill]]->Fill(timtype);
    h_itd[skim[ifill]]->Fill(-0.5);
    h_ftd[skim[ifill]]->Fill(-0.5);
    h_psn[skim[ifill]]->Fill(-0.5);
    for (int i = 0; i < 32; i++) {
      for (unsigned j = 0; j < (unsigned)nword_input; j++) {
        if (itd[j] & (1 << i)) h_itd[skim[ifill]]->Fill(i + 0.5 + 32 * j);
      }
      for (unsigned j = 0; j < (unsigned)nword_output; j++) {
        if (ftd[j] & (1 << i)) h_ftd[skim[ifill]]->Fill(i + 0.5 + 32 * j);
        if (psn[j] & (1 << i)) h_psn[skim[ifill]]->Fill(i + 0.5 + 32 * j);
      }
    }

    // fill timestamp values stored in header
    int gdlL1TocomL1 = gdll1_rvc < coml1rvc ? coml1rvc - gdll1_rvc : (coml1rvc + 1280) - gdll1_rvc;
    h_c8_gdlL1TocomL1[skim[ifill]]->Fill(gdlL1TocomL1);

    int topTogdlL1 = gdll1_rvc < c8_top_timing ? (gdll1_rvc + 1280) - c8_top_timing : gdll1_rvc - c8_top_timing;
    h_c8_topTogdlL1[skim[ifill]]->Fill(topTogdlL1);

    int eclTogdlL1 = gdll1_rvc < c8_ecl_timing ? (gdll1_rvc + 1280) - c8_ecl_timing : gdll1_rvc - c8_ecl_timing;
    h_c8_eclTogdlL1[skim[ifill]]->Fill(eclTogdlL1);

    int cdcTogdlL1 = gdll1_rvc < c8_cdc_timing ? (gdll1_rvc + 1280) - c8_cdc_timing : gdll1_rvc - c8_cdc_timing;
    h_c8_cdcTogdlL1[skim[ifill]]->Fill(cdcTogdlL1);

    int c127_ecl_timing = c8_ecl_timing & (((1 << 7) - 1) << 4);
    int fit8mToGDL = c127_ecl_timing < eclrvc ? eclrvc - c127_ecl_timing : (eclrvc + 1280) - c127_ecl_timing;
    h_c8_ecl8mToGDL[skim[ifill]]->Fill(fit8mToGDL);

    int topToGDL = c8_top_timing < toprvc ? toprvc - c8_top_timing : (toprvc + 1280) - c8_top_timing;
    h_c8_topToGDL[skim[ifill]]->Fill(topToGDL);
    int eclToGDL = c8_ecl_timing < eclrvc ? eclrvc - c8_ecl_timing : (eclrvc + 1280) - c8_ecl_timing;
    h_c8_eclToGDL[skim[ifill]]->Fill(eclToGDL);
    int cdcToGDL = c8_cdc_timing < cdcrvc ? cdcrvc - c8_cdc_timing : (cdcrvc + 1280) - c8_cdc_timing;
    h_c8_cdcToGDL[skim[ifill]]->Fill(cdcToGDL);

    int c2_comL1 = coml1rvc << 2;
    int c2_gdlL1 = gdll1_rvc << 2;
    int c2_diff_cdcTogdlL1 = c2_gdlL1 > c2_cdc_timing ?
                             c2_gdlL1 - c2_cdc_timing :
                             c2_gdlL1 - c2_cdc_timing + (1280 << 2) ;
    h_ns_cdcTogdlL1[skim[ifill]]->Fill(c2_diff_cdcTogdlL1 * clkTo2ns);

    int c2_diff_cdcTocomL1 = c2_comL1 > c2_cdc_timing ?
                             c2_comL1 - c2_cdc_timing :
                             c2_comL1 - c2_cdc_timing + (1280 << 2) ;
    h_c2_cdcTocomL1[skim[ifill]]->Fill(c2_diff_cdcTocomL1);
    h_ns_cdcTocomL1[skim[ifill]]->Fill(c2_diff_cdcTocomL1 * clkTo2ns);

    int c2_diff_cdcToecl = c2_ecl_timing > c2_cdc_timing ?
                           c2_ecl_timing - c2_cdc_timing :
                           c2_ecl_timing - c2_cdc_timing + (1280 << 2);
    h_c2_cdcToecl[skim[ifill]]->Fill(c2_diff_cdcToecl);
    h_ns_cdcToecl[skim[ifill]]->Fill(c2_diff_cdcToecl * clkTo2ns);

    int c1_diff_topToecl = c1_ecl_timing > c1_top_timing ?
                           c1_ecl_timing - c1_top_timing :
                           c1_ecl_timing - c1_top_timing + (1280 << 3);
    h_ns_topToecl[skim[ifill]]->Fill(c1_diff_topToecl *  clkTo1ns);

    int c2_diff_topTocdc = c2_cdc_timing > c2_top_timing ?
                           c2_cdc_timing - c2_top_timing :
                           c2_cdc_timing - c2_top_timing + (1280 << 2);
    h_ns_topTocdc[skim[ifill]]->Fill(c2_diff_topTocdc *  clkTo2ns);
  }


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

}

bool TRGGDLDQMModule::anaBitCondition(void)
{
  if (m_bitConditionToDumpVcd.length() == 0) return true;
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
            // cppcheck-suppress knownConditionTrueFalse
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
  for (unsigned j = 0; j < n_inbit; j++) {
    outf << "$var wire  1  n" << seqnum++ << " " << inbitname[j] << " $end" << endl;
  }
  for (unsigned j = 0; j < n_outbit; j++) {
    outf << "$var wire  1  n" << seqnum++ << " ftd." << outbitname[j] << " $end" << endl;
  }
  for (unsigned j = 0; j < n_outbit; j++) {
    outf << "$var wire  1  n" << seqnum++ << " psn." << outbitname[j] << " $end" << endl;
  }

  outf << "$upscope $end" << endl;
  outf << "$enddefinitions $end" << endl << endl;

  for (unsigned clk = 1; clk <= n_clocks; clk++) {
    seqnum = 0;
    outf << "#" << clk - 1 << endl;
    for (unsigned k = 1; k <= n_inbit; k++) {
      if (clk == 1 || prev_i[k - 1] != h_i_vec[(clk - 1)*n_inbit + k - 1]) {
        prev_i[k - 1] = h_i_vec[(clk - 1) * n_inbit + k - 1];
        outf << h_i_vec[(clk - 1)*n_inbit + k - 1] << "n" << seqnum << endl;
      }
      seqnum++;
    }
    for (unsigned k = 1; k <= n_outbit; k++) {
      if (clk == 1 || prev_f[k - 1] != h_f_vec[(clk - 1)*n_outbit + k - 1]) {
        prev_f[k - 1] = h_f_vec[(clk - 1) * n_outbit + k - 1];
        outf << h_f_vec[(clk - 1)*n_outbit + k - 1] << "n" << seqnum << endl;
      }
      seqnum++;
    }
    for (unsigned k = 1; k <= n_outbit; k++) {
      if (clk == 1 || prev_p[k - 1] != h_p_vec[(clk - 1)*n_outbit + k - 1]) {
        prev_p[k - 1] = h_p_vec[(clk - 1) * n_outbit + k - 1];
        outf << h_p_vec[(clk - 1)*n_outbit + k - 1] << "n" << seqnum << endl;
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
TRGGDLDQMModule::isFired_quick(const std::string& bitname, const bool& isPsnm = 0)
{
  int bn = getoutbitnum(bitname.c_str());
  for (unsigned clk = 0; clk < n_clocks; clk++) {
    if (bn > -1) {
      if (isPsnm) {
        if (h_p_vec[clk * n_outbit + bn] > 0)
          return true;
      } else {
        if (h_f_vec[clk * n_outbit + bn] > 0)
          return true;
      }
    }
  }
  bn = getinbitnum(bitname.c_str());
  for (unsigned clk = 0; clk < n_clocks; clk++) {
    if (bn > -1) {
      if (h_i_vec[clk * n_inbit + bn] > 0)
        return true;
    }
  }
  return false;
}

int
TRGGDLDQMModule::getinbitnum(const char* c) const
{
  for (int i = 0; i < 320; i++) {
    if (strcmp(c, inbitname[i]) == 0)return i;
  }
  return -1;
}

int
TRGGDLDQMModule::getoutbitnum(const char* c) const
{
  for (int i = 0; i < 320; i++) {
    if (strcmp(c, outbitname[i]) == 0)return i;
  }
  return -1;
}


bool
TRGGDLDQMModule::isFired(std::string bitname)
{
  bool isPsnm = false;
  for (unsigned i = 0; i < bitname.length(); i++) {
    if ('A' <= bitname[i] && bitname[i] <= 'Z') {
      isPsnm = true;
    }
  }
  boost::algorithm::to_lower(bitname);
  int bn = m_dbftdl->getoutbitnum(bitname.c_str());
  for (unsigned clk = 0; clk < n_clocks; clk++) {
    if (bn > -1) {
      if (isPsnm) {
        if (h_p_vec[clk * n_outbit + bn] > 0)
          return true;
      } else {
        if (h_f_vec[clk * n_outbit + bn] > 0)
          return true;
      }
    }
  }
  bn = m_dbinput->getinbitnum(bitname.c_str());
  for (unsigned clk = 0; clk < n_clocks; clk++) {
    if (bn > -1) {
      if (h_i_vec[clk * n_inbit + bn] > 0)
        return true;
    }
  }
  return false;
}

void
TRGGDLDQMModule::fillRiseFallTimings(void)
{


  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    //reduce #plot
    if (skim[ifill] != 0)continue;

    for (unsigned i = 0; i < n_inbit; i++) {
      if (n_clocks == 32) {
        h_itd_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_itd_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_itd_rise[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
        h_itd_fall[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
      } else {
        h_itd_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
        h_itd_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
      }
      bool rising_done = false;
      bool falling_done = false;
      for (unsigned clk = 0; clk < n_clocks; clk++) {
        if (h_i_vec[clk * n_inbit + i] > 0) {
          if (! rising_done) {
            h_itd_rise[i][skim[ifill]]->Fill(clk + 0.5);
            rising_done = true;
            // cppcheck-suppress knownConditionTrueFalse
          } else if (rising_done && !falling_done && clk == n_clocks - 1) {
            h_itd_fall[i][skim[ifill]]->Fill(clk + 0.5);
          }
        } else if (h_i_vec[clk * n_inbit + i] == 0) {
          if (rising_done && ! falling_done) {
            h_itd_fall[i][skim[ifill]]->Fill(clk + 0.5);
            falling_done = true;
          }
        }
      }
    }
    for (unsigned i = 0; i < n_outbit; i++) {
      if (n_clocks == 32) {
        h_ftd_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_psn_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_ftd_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_psn_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_ftd_rise[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
        h_psn_rise[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
        h_ftd_fall[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
        h_psn_fall[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
      } else {
        h_ftd_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
        h_psn_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
        h_ftd_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
        h_psn_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
      }
      bool rising_done = false;
      bool falling_done = false;
      for (unsigned clk = 0; clk < n_clocks; clk++) {
        if (h_f_vec[clk * n_outbit + i] > 0) {
          if (! rising_done) {
            h_ftd_rise[i][skim[ifill]]->Fill(clk + 0.5);
            rising_done = true;
            // cppcheck-suppress knownConditionTrueFalse
          } else if (rising_done && !falling_done && clk == n_clocks - 1) {
            h_ftd_fall[i][skim[ifill]]->Fill(clk + 0.5);
          }
        } else if (h_f_vec[clk * n_outbit + i] == 0) {
          if (rising_done && ! falling_done) {
            h_ftd_fall[i][skim[ifill]]->Fill(clk + 0.5);
            falling_done = true;
          }
        }
      }
      rising_done = false;
      falling_done = false;
      for (unsigned clk = 0; clk < n_clocks; clk++) {
        if (h_p_vec[clk * n_outbit + i] > 0) {
          if (! rising_done) {
            h_psn_rise[i][skim[ifill]]->Fill(clk + 0.5);
            rising_done = true;
            // cppcheck-suppress knownConditionTrueFalse
          } else if (rising_done && !falling_done && clk == n_clocks - 1) {
            h_psn_fall[i][skim[ifill]]->Fill(clk + 0.5);
          }
        } else if (h_p_vec[clk * n_outbit + i] == 0) {
          if (rising_done && ! falling_done) {
            h_psn_fall[i][skim[ifill]]->Fill(clk + 0.5);
            falling_done = true;
          }
        }
      }
    }
  }
}


void
TRGGDLDQMModule::fillOutputOverlap(void)
{
  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    bool ffy_fired = isFired_quick("ffy", true);
    bool fyo_fired = isFired_quick("fyo", true) ||  isFired_quick("fyb", true);
    bool c4_fired = isFired_quick("c4", true);
    bool hie_fired = isFired_quick("hie", true);
    bool klm_fired = isFired_quick("mu_b2b", true) || isFired_quick("mu_eb2b", true) || isFired_quick("eklm2", true)
                     || isFired_quick("beklm", true);
    bool klm_match_fired = isFired_quick("cdcklm1", true) || isFired_quick("cdcklm2", true)
                           || isFired_quick("seklm1", true) || isFired_quick("seklm2", true)
                           || isFired_quick("fwd_seklm", true) || isFired_quick("bwd_seklm", true)
                           || isFired_quick("ieklm1", true) || isFired_quick("ecleklm1", true);
    bool stt_fired = isFired_quick("stt", true) || isFired_quick("sttecl", true);
    bool short_fired = isFired_quick("syo", true) ||  isFired_quick("syb", true) || isFired_quick("yioiecl1", true) ;
    bool ff30_fired = isFired_quick("fy30", true);
    bool inner_fired = isFired_quick("ioiecl2", true);
    bool lml_fired = isFired_quick("lml0", true) || isFired_quick("lml2", true) || isFired_quick("lml6", true)
                     || isFired_quick("lml7", true) || isFired_quick("lml8", true) || isFired_quick("lml9", true)
                     || isFired_quick("lml10", true)
                     || isFired_quick("lml12", true) || isFired_quick("lml13", true);
    bool gg_fired = isFired_quick("ggsel", true);
    bool bhabha_fired = isFired_quick("bhapur", true);
    bool pid_fired =     isFired_quick("ssb", true) || isFired_quick("eed", true) || isFired_quick("fed", true)
                         ||  isFired_quick("fp", true)  || isFired_quick("shem", true) || isFired_quick("ohem", true);
    bool bhamon_fired =  isFired_quick("bffo", true) || isFired_quick("bhie", true);
    bool eclmumu_fired = isFired_quick("eclmumu", true);
    bool lml1_fired = isFired_quick("lml1", true);
    bool lml4_fired = isFired_quick("lml4", true);
    bool veto_fired =  isFired_quick("hiev", true) || isFired_quick("fffv", true);
    bool random_fired =  isFired_quick("bg", true) || isFired_quick("poissonv", true);
    bool ffz_fired = isFired_quick("ffz", true);
    bool fzo_fired = isFired_quick("fzo", true) ||  isFired_quick("fzb", true);
    bool monitor_fired =    isFired_quick("fff", true) || isFired_quick("ffo", true) || isFired_quick("ffb", true)
                            || isFired_quick("fffo", true) || isFired_quick("ffs", true) || isFired_quick("fss", true) || isFired_quick("sss", true)
                            || isFired_quick("ff", true) || isFired_quick("ss", true) || isFired_quick("fso", true)
                            || isFired_quick("sso", true) || isFired_quick("fsb", true) || isFired_quick("ff30", true)
                            || isFired_quick("lume", true) || isFired_quick("c2", true) || isFired_quick("c3", true)
                            || isFired_quick("bha3d", true) || isFired_quick("bhabha", true)
                            || isFired_quick("lml3", true) || isFired_quick("lml5", true)
                            || isFired_quick("g_high", true) || isFired_quick("g_c1", true) || isFired_quick("gg", true)
                            || isFired_quick("eklmhit", true) || isFired_quick("fioiecl1", true) || isFired_quick("ioiecl1", true)
                            || isFired_quick("cdcecl1", true) || isFired_quick("cdcecl2", true) || isFired_quick("cdcecl3", true)
                            || isFired_quick("cdcecl4", true) || isFired_quick("c2gev1", true) || isFired_quick("c2gev2", true)
                            || isFired_quick("c2hie", true) || isFired_quick("f", true) || isFired_quick("s", true)
                            || isFired_quick("revolution", true) || isFired_quick("random", true);

    bool B_CDC_fired = ffy_fired || fyo_fired;
    bool B_ECL_fired = c4_fired  || hie_fired;
    bool LOW_KLM_fired = klm_fired || klm_match_fired;
    bool LOW_CDC_fired = stt_fired || short_fired || ff30_fired || inner_fired;
    bool LOW_ECL_fired = lml_fired;
    bool CALIB_fired = gg_fired || bhabha_fired || pid_fired || bhamon_fired || eclmumu_fired || lml1_fired || lml4_fired || veto_fired
                       || random_fired;
    bool MONITOR_fired = monitor_fired || ffz_fired || fzo_fired;

    //all event
    if (1) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(0.5);
    }
    //main category
    if (B_CDC_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(1.5);
    } else if (B_ECL_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(2.5);
    } else if (LOW_KLM_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(3.5);
    } else if (LOW_CDC_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(4.5);
    } else if (LOW_ECL_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(5.5);
    } else if (CALIB_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(6.5);
    } else if (MONITOR_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(7.5);
    } else {
      h_psn_effect_to_l1[skim[ifill]]->Fill(8.5);
    }
    //detail category
    if (ffy_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(9.5);
    } else if (fyo_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(10.5);
    } else if (c4_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(11.5);
    } else if (hie_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(12.5);
    } else if (klm_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(13.5);
    } else if (klm_match_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(14.5);
    } else if (stt_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(15.5);
    } else if (short_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(16.5);
    } else if (ff30_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(17.5);
    } else if (inner_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(18.5);
    } else if (lml_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(19.5);
    } else if (gg_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(20.5);
    } else if (bhabha_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(21.5);
    } else if (pid_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(22.5);
    } else if (bhamon_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(23.5);
    } else if (eclmumu_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(24.5);
    } else if (lml1_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(25.5);
    } else if (lml4_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(26.5);
    } else if (veto_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(27.5);
    } else if (random_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(28.5);
    } else if (ffz_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(29.5);
    } else if (fzo_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(30.5);
    } else if (monitor_fired) {
      h_psn_effect_to_l1[skim[ifill]]->Fill(31.5);
    } else {
      h_psn_effect_to_l1[skim[ifill]]->Fill(32.5);
    }

    //all event
    if (1) {
      h_psn_raw_rate[skim[ifill]]->Fill(0.5);
    }
    //main category
    if (B_CDC_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(1.5);
    }  if (B_ECL_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(2.5);
    }  if (LOW_KLM_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(3.5);
    }  if (LOW_CDC_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(4.5);
    }  if (LOW_ECL_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(5.5);
    }  if (CALIB_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(6.5);
    }  if (MONITOR_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(7.5);
    }
    //detail category
    if (ffy_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(9.5);
    }  if (fyo_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(10.5);
    }  if (c4_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(11.5);
    }  if (hie_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(12.5);
    }  if (klm_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(13.5);
    }  if (klm_match_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(14.5);
    }  if (stt_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(15.5);
    }  if (short_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(16.5);
    }  if (ff30_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(17.5);
    }  if (inner_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(18.5);
    }  if (lml_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(19.5);
    }  if (gg_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(20.5);
    }  if (bhabha_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(21.5);
    }  if (pid_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(22.5);
    }  if (bhamon_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(23.5);
    }  if (eclmumu_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(24.5);
    }  if (lml1_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(25.5);
    }  if (lml4_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(26.5);
    }  if (veto_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(27.5);
    }  if (random_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(28.5);
    }  if (ffz_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(29.5);
    }  if (fzo_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(30.5);
    }  if (monitor_fired) {
      h_psn_raw_rate[skim[ifill]]->Fill(31.5);
    }
  }
}


void
TRGGDLDQMModule::fillOutputExtra(void)
{
  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    bool c4_fired = isFired_quick("c4");
    bool hie_fired = isFired_quick("hie");
    bool LML_fired = (isFired_quick("lml0", true) || isFired_quick("lml1", true) || isFired_quick("lml2", true)
                      || isFired_quick("lml3", true) || isFired_quick("lml4", true) || isFired_quick("lml5", true)
                      || isFired_quick("lml6", true) || isFired_quick("lml7", true) || isFired_quick("lml8", true)
                      || isFired_quick("lml9", true) || isFired_quick("lml10", true) || isFired_quick("eclmumu", true));
    bool CDC_fired = (isFired_quick("fff", true) || isFired_quick("ffo", true) || isFired_quick("ffb", true)
                      || isFired_quick("ffy", true) || isFired_quick("fyo", true) || isFired_quick("fyb", true));
    bool ECL_fired = (isFired_quick("c4", true) || isFired_quick("hie", true));
    bool fff_fired = isFired_quick("fff");
    bool ff_fired  = isFired_quick("ff");
    bool f_fired   = isFired_quick("f");
    bool ffo_fired = isFired_quick("ffo");
    bool ffb_fired = isFired_quick("ffb");
    bool ffy_fired = isFired_quick("ffy");
    bool fyo_fired = isFired_quick("fyo");
    bool fyb_fired = isFired_quick("fyb");
    bool bha2D_fired = isFired_quick("bhabha");
    bool bha3D_fired = isFired_quick("bha3d");
    bool lml0_fired  = isFired_quick("lml0");
    bool lml1_fired  = isFired_quick("lml1");
    bool lml2_fired  = isFired_quick("lml2");
    bool lml3_fired  = isFired_quick("lml3");
    bool lml4_fired  = isFired_quick("lml4");
    bool lml5_fired  = isFired_quick("lml5");
    bool lml6_fired  = isFired_quick("lml6");
    bool lml7_fired  = isFired_quick("lml7");
    bool lml8_fired  = isFired_quick("lml8");
    bool lml9_fired  = isFired_quick("lml9");
    bool lml10_fired = isFired_quick("lml10");
    bool lml12_fired = isFired_quick("lml12");
    bool lml13_fired = isFired_quick("lml13");
    bool eclmumu_fired = isFired_quick("eclmumu");
    bool mu_b2b_fired = isFired_quick("mu_b2b");
    bool mu_eb2b_fired = isFired_quick("mu_eb2b");
    bool cdcklm1_fired = isFired_quick("cdcklm1");
    bool cdcklm2_fired = isFired_quick("cdcklm2");
    bool klm_hit_fired = isFired_quick("klm_hit");
    bool eklm_hit_fired = isFired_quick("eklm_hit");
    bool cdcecl1_fired = isFired_quick("cdcecl1");
    bool cdcecl2_fired = isFired_quick("cdcecl2");
    bool cdcecl3_fired = isFired_quick("cdcecl3");
    bool cdcecl4_fired = isFired_quick("cdcecl4");
    bool fso_fired = isFired_quick("fso");
    bool fsb_fired = isFired_quick("fsb");
    bool syo_fired = isFired_quick("syo");
    bool syb_fired = isFired_quick("syb");
    bool x_fired = isFired_quick("x");
    bool fioiecl1_fired = isFired_quick("fioiecl1");
    bool ecleklm1_fired = isFired_quick("ecleklm1");
    bool seklm1_fired = isFired_quick("seklm1");
    bool seklm2_fired = isFired_quick("seklm2");
    bool ieklm_fired = isFired_quick("ieklm");
    bool iecl_fired = isFired_quick("iecl");
    bool yioiecl1_fired = isFired_quick("yioiecl1");
    bool stt_fired = isFired_quick("stt");
    bool ffz_fired = isFired_quick("ffz");
    bool fzo_fired = isFired_quick("fzo");
    bool fzb_fired = isFired_quick("fzb");

    if (1) {
      h_psn_extra[skim[ifill]]->Fill(0.5);
    }
    if (fff_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(1.5);
    }
    if (ffo_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(2.5);
    }
    if (ffb_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(3.5);
    }
    if (fff_fired) {
      h_psn_extra[skim[ifill]]->Fill(4.5);
    }
    if (ECL_fired) {
      h_psn_extra[skim[ifill]]->Fill(5.5);
    }
    if (CDC_fired) {
      h_psn_extra[skim[ifill]]->Fill(6.5);
    }
    if ((CDC_fired) && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(7.5);
    }
    if (bha2D_fired) {
      h_psn_extra[skim[ifill]]->Fill(8.5);
    }
    if (bha3D_fired) {
      h_psn_extra[skim[ifill]]->Fill(9.5);
    }
    if (ff_fired) {
      h_psn_extra[skim[ifill]]->Fill(10.5);
    }
    if (ff_fired && (LML_fired)) {
      h_psn_extra[skim[ifill]]->Fill(11.5);
    }
    if (f_fired) {
      h_psn_extra[skim[ifill]]->Fill(12.5);
    }
    if (f_fired && (LML_fired)) {
      h_psn_extra[skim[ifill]]->Fill(13.5);
    }
    if (LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(14.5);
    }
    if (fff_fired && (LML_fired)) {
      h_psn_extra[skim[ifill]]->Fill(15.5);
    }
    if (ffo_fired && (LML_fired)) {
      h_psn_extra[skim[ifill]]->Fill(16.5);
    }
    if (ffb_fired && (LML_fired)) {
      h_psn_extra[skim[ifill]]->Fill(17.5);
    }
    if (ffy_fired) {
      h_psn_extra[skim[ifill]]->Fill(18.5);
    }
    if (ffy_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(19.5);
    }
    if (fyo_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(20.5);
    }
    if (fyb_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(21.5);
    }
    if ((ffy_fired || fyo_fired || fyb_fired) && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(22.5);
    }
    if (ffy_fired && (LML_fired)) {
      h_psn_extra[skim[ifill]]->Fill(23.5);
    }
    if (fyo_fired && (LML_fired)) {
      h_psn_extra[skim[ifill]]->Fill(24.5);
    }
    if (fyb_fired && (LML_fired)) {
      h_psn_extra[skim[ifill]]->Fill(25.5);
    }
    if (c4_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(26.5);
    }
    if (hie_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(27.5);
    }
    if (lml0_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(28.5);
    }
    if (lml1_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(29.5);
    }
    if (lml2_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(30.5);
    }
    if (lml3_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(31.5);
    }
    if (lml4_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(32.5);
    }
    if (lml5_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(33.5);
    }
    if (lml6_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(34.5);
    }
    if (lml7_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(35.5);
    }
    if (lml8_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(36.5);
    }
    if (lml9_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(37.5);
    }
    if (lml10_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(38.5);
    }
    if (lml12_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(39.5);
    }
    if (lml13_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(40.5);
    }
    if (eclmumu_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(41.5);
    }
    if (mu_b2b_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(42.5);
    }
    if (mu_eb2b_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(43.5);
    }
    if (cdcklm1_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(44.5);
    }
    if (cdcklm2_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(45.5);
    }
    if (klm_hit_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(46.5);
    }
    if (eklm_hit_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(47.5);
    }
    if (mu_b2b_fired  && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(48.5);
    }
    if (mu_eb2b_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(49.5);
    }
    if (cdcklm1_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(50.5);
    }
    if (cdcklm2_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(51.5);
    }
    if (klm_hit_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(52.5);
    }
    if (eklm_hit_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(53.5);
    }
    if (cdcecl1_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(54.5);
    }
    if (cdcecl2_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(55.5);
    }
    if (cdcecl3_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(56.5);
    }
    if (cdcecl4_fired && (CDC_fired)) {
      h_psn_extra[skim[ifill]]->Fill(57.5);
    }
    if (cdcecl1_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(58.5);
    }
    if (cdcecl2_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(59.5);
    }
    if (cdcecl3_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(60.5);
    }
    if (cdcecl4_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(61.5);
    }
    if (fso_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(62.5);
    }
    if (fsb_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(63.5);
    }
    if (syo_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(64.5);
    }
    if (syb_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(65.5);
    }
    if (x_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(66.5);
    }
    if (fioiecl1_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(67.5);
    }
    if (ecleklm1_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(68.5);
    }
    if (seklm1_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(69.5);
    }
    if (seklm2_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(70.5);
    }
    if (ieklm_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(71.5);
    }
    if (iecl_fired && LML_fired) {
      h_psn_extra[skim[ifill]]->Fill(72.5);
    }
    if (ecleklm1_fired && CDC_fired) {
      h_psn_extra[skim[ifill]]->Fill(73.5);
    }
    if (syo_fired && ECL_fired) {
      h_psn_extra[skim[ifill]]->Fill(74.5);
    }
    if (yioiecl1_fired && ECL_fired) {
      h_psn_extra[skim[ifill]]->Fill(75.5);
    }
    if (stt_fired && ECL_fired) {
      h_psn_extra[skim[ifill]]->Fill(76.5);
    }
    if (ffz_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(77.5);
    }
    if (fzo_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(78.5);
    }
    if (fzb_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(79.5);
    }
    if (ffy_fired && ffz_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(80.5);
    }
    if (fyo_fired && fzo_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(81.5);
    }
    if (fyb_fired && fzb_fired && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(82.5);
    }
    if ((ffy_fired || ffz_fired) && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(83.5);
    }
    if ((fyo_fired || fzo_fired) && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(84.5);
    }
    if ((fyb_fired || fzb_fired) && (ECL_fired)) {
      h_psn_extra[skim[ifill]]->Fill(85.5);
    }
    if (ffy_fired && ffz_fired) {
      h_psn_extra[skim[ifill]]->Fill(86.5);
    }
    if (fyo_fired && fzo_fired) {
      h_psn_extra[skim[ifill]]->Fill(87.5);
    }
    if (fyb_fired && fzb_fired) {
      h_psn_extra[skim[ifill]]->Fill(88.5);
    }
    if (ffy_fired || ffz_fired) {
      h_psn_extra[skim[ifill]]->Fill(89.5);
    }
    if (fyo_fired || fzo_fired) {
      h_psn_extra[skim[ifill]]->Fill(90.5);
    }
    if (fyb_fired || fzb_fired) {
      h_psn_extra[skim[ifill]]->Fill(91.5);
    }
    if (ffo_fired) {
      h_psn_extra[skim[ifill]]->Fill(92.5);
    }
    if (ffb_fired) {
      h_psn_extra[skim[ifill]]->Fill(93.5);
    }
  }

}

const char* TRGGDLDQMModule::output_extra[n_output_extra] = {
  "all", "fff&(c4|hie)", "ffo&(c4|hie)", "ffb&(c4|hie)", "fff", "c4|hie", "fff|ffo|ffb", "(fff|ffo|ffb)&(c4|hie)", "bha2D", "bha3D",
  "ff", "ff&(lml|eclmumu)", "f", "f&(lml|eclmumu)", "lml|eclmumu", "fff&(lml|eclmumu)", "ffo&(lml|eclmumu)", "ffb&(lml|eclmumu)", "ffy", "ffy&(c4|hie)",
  "fyo&(c4|hie)", "fyb&(c4|hie)", "(ffy|ffo|ffb)&(c4|hie)", "ffy&(lml|eclmumu)", "fyo&(lml|eclmumu)", "fyb&(lml|eclmumu)", "c4&(fff|ffo|ffb)", "hie&(fff|ffo|ffb)", "lml0&(fff|ffo|ffb)", "lml1&(fff|ffo|ffb)",
  "lml2&(fff|ffo|ffb)", "lml3&(fff|ffo|ffb)", "lml4&(fff|ffo|ffb)", "lml5&(fff|ffo|ffb)", "lml6&(fff|ffo|ffb)", "lml7&(fff|ffo|ffb)", "lml8&(fff|ffo|ffb)", "lml9&(fff|ffo|ffb)", "lml10&(fff|ffo|ffb)", "lml12&(fff|ffo|ffb)",
  "lml13&(fff|ffo|ffb)", "eclmumu&(fff|ffo|ffb)", "mu_b2b&(fff|ffo|ffb)", "mu_eb2b&(fff|ffo|ffb)", "cdcklm1&(fff|ffo|ffb)", "cdcklm2&(fff|ffo|ffb)", "klm_hit&(fff|ffo|ffb)", "eklm_hit&(fff|ffo|ffb)", "mu_b2b&(lml|eclmumu)", "mu_eb2b&(lml|eclmumu)",
  "cdcklm1&(lml|eclmumu)", "cdcklm2&(lml|eclmumu)", "klm_hit&(lml|eclmumu)", "eklm_hit&(lml|eclmumu)", "cdcecl1&(fff|ffo|ffb)", "cdcecl2&(fff|ffo|ffb)", "cdcecl3&(fff|ffo|ffb)", "cdcecl4&(fff|ffo|ffb)", "cdcecl1&(lml|eclmumu)", "cdcecl2&(lml|eclmumu)",
  "cdcecl3&(lml|eclmumu)", "cdcecl4&(lml|eclmumu)", "fso&(lml|eclmumu)", "fsb&(lml|eclmumu)", "syo&(lml|eclmumu)", "syb&(lml|eclmumu)", "x&(lml|eclmumu)", "fioiecl1&(lml|eclmumu)", "ecleklm1&(lml|eclmumu)", "seklm1&(lml|eclmumu)",
  "seklm2&(lml|eclmumu)", "ieklm&(lml|eclmumu)", "iecl&(lml|eclmumu)", "ecleklm1&(fff|ffo|ffb)", "syo&(c4|hie)", "yioiecl1&(c4|hie)", "stt&(c4|hie)", "ffz&(c4|hie)", "fzo&(c4|hie)", "fzb&(c4|hie)",
  "ffy&ffz&(c4|hie)", "fyo&fzo&(c4|hie)", "fyb&fzb&(c4|hie)", "(ffy|ffz)&(c4|hie)", "(fyo|fzo)&(c4|hie)", "(fyb|fzb)&(c4|hie)", "ffy&ffz", "fyo&fzo", "fyb&fzb", "(ffy|ffz)",
  "(fyo|fzo)", "(fyb|fzb)", "ffo", "ffb"
};

const char* TRGGDLDQMModule::output_overlap[n_output_overlap] = {
  "all",       "B_CDC",        "B_ECL",      "L_KLM",            "L_CDC",         "L_ECL",      "CALIB",      "MONITOR",      "other",        "B_ffy",
  "B_fyo",     "B_c4",         "B_hie",      "L_klm",            "L_klm_match",   "L_stt",      "L_short",    "L_fy30",       "L_inner",      "L_lml",
  "CALIB_gg",  "CALIB_bhapur", "CALIB_pid",  "CALIB_wo_bhaveto", "CALIB_eclmumu", "CALIB_lml1", "CALIB_lml4", "CALIB_noveto", "CALIB_random", "MON_ffz",
  "MON_fzo",   "monitor",      "other"
};



void
TRGGDLDQMModule::fillOutputPureExtra(void)
{
  //get offline CDC track information
  int n_fulltrack = 0;
  float max_dphi = 0;
  float phi_list[100];
  for (int itrack = 0; itrack < Tracks.getEntries(); itrack++) {
    const TrackFitResult* tfr = Tracks[itrack]->getTrackFitResult(Const::pion);
    if (tfr == nullptr) continue;

    float z0     = tfr->getZ0();
    float d0     = tfr->getD0();
    float phi    = tfr->getPhi();
    //float omega  = tfr->getOmega();
    int   flayer = tfr->getHitPatternCDC().getFirstLayer();
    int   llayer = tfr->getHitPatternCDC().getLastLayer();
    float pt     = tfr->getTransverseMomentum();
    if (z0 > -1 && z0 < 1 && d0 > -1 && d0 < 1 && flayer < 8 && llayer > 50
        && pt > 0.3) { //select track from IP, hit SL0 and SL8, pt>0.3GeV
      phi_list[n_fulltrack] = phi;
      n_fulltrack += 1;
    }
  }
  for (int i = 0; i < n_fulltrack; i++) {
    for (int j = 0; j < n_fulltrack; j++) {
      float dphi = phi_list[i] - phi_list[j];
      if (dphi < 0)   dphi = -dphi;
      if (dphi > 3.14) dphi = 2 * 3.14 - dphi;
      if (dphi > max_dphi) max_dphi = dphi;
    }
  }

  //get offline ECL cluster information
  //double total_energy = 0;
  //for (int iclst = 0; iclst < ECLClusters.getEntries(); iclst++) {
  //  total_energy += ECLClusters[iclst]->getEnergyRaw();
  //}
  //
  //
  int ecl_timing_threshold_low  = -200; // (ns)  xtal timing selection
  int ecl_timing_threshold_high =  200; // (ns)  xtal timing selection
  double ecl_xtcid_energy_sum[576] = {0};
  double total_energy = 0;
  int ncluster = 0;
  for (const auto& eclcalhit : m_ECLCalDigitData) {

    // (ecl) calibation status check and cut
    if (!eclcalhit.isCalibrated()) {continue;}
    if (eclcalhit.isFailedFit()) {continue;}
    if (eclcalhit.isTimeResolutionFailed()) {continue;}

    // (ecl) xtal-id
    int ecl_cid   = (double) eclcalhit.getCellId();

    // (ecl) fitter quality check and cut
    int ecl_quality = -1;
    for (const auto& eclhit : m_ECLDigitData) {
      if (ecl_cid == eclhit.getCellId()) {
        ecl_quality = (int) eclhit.getQuality();
      }
    }
    if (ecl_quality != 0) {continue;}

    // (ecl) xtal energy
    double ecl_xtal_energy  = eclcalhit.getEnergy(); // ECLCalDigit


    // (ecl) timing cut
    int ecl_timing = eclcalhit.getTime();
    if (ecl_timing < ecl_timing_threshold_low ||
        ecl_timing > ecl_timing_threshold_high) {continue;}

    // (ecl) tc-id for xtal-id
    int ecl_tcid  = trgeclmap->getTCIdFromXtalId(ecl_cid);
    int ecl_thetaid = trgeclmap->getTCThetaIdFromTCId(ecl_tcid);

    if (ecl_tcid >= 0 && ecl_tcid < 576 && ecl_thetaid >= 2 && ecl_thetaid <= 15) { //pick up only 2=<thetaid=<15
      ecl_xtcid_energy_sum[ecl_tcid] = ecl_xtcid_energy_sum[ecl_tcid] + ecl_xtal_energy;
      //ecltimingsum[i] = ecl_timing;
    }
  }

  for (int i = 0; i < 576; i++) {
    if (ecl_xtcid_energy_sum[i] > 0.1) {
      total_energy += ecl_xtcid_energy_sum[i];
      ncluster += 1;
    }
  }

  //get offline KLM cluster information


  //fff: require the number of CDC full tracks is more than or equal to 3
  if (n_fulltrack > 2) {
    bool fff_fired = isFired_quick("fff");
    bool ffy_fired = isFired_quick("ffy");
    bool c4_fired  = isFired_quick("c4");
    bool hie_fired = isFired_quick("hie");
    if (c4_fired || hie_fired) {
      h_psn_pure_extra[0]->Fill(0.5);
    }
    if (fff_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(1.5);
    }
    if (ffy_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(2.5);
    }
  }
  //ffo: require the number of CDC full tracks is more than or equal to 2, opening angle > 90deg
  if (n_fulltrack > 1 && max_dphi > 3.14 / 2.) {
    bool ffo_fired = isFired_quick("ffo");
    bool fyo_fired = isFired_quick("fyo");
    bool c4_fired  = isFired_quick("c4");
    bool hie_fired = isFired_quick("hie");
    if (c4_fired || hie_fired) {
      h_psn_pure_extra[0]->Fill(3.5);
    }
    if (ffo_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(4.5);
    }
    if (fyo_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(5.5);
    }
  }
  //ffo: require the number of CDC full tracks is more than or equal to 2, opening angle >150deg
  if (n_fulltrack > 1 && max_dphi > 3.14 * 5 / 6.) {
    bool ffb_fired = isFired_quick("ffb");
    bool fyb_fired = isFired_quick("fyb");
    bool c4_fired  = isFired_quick("c4");
    bool hie_fired = isFired_quick("hie");
    if (c4_fired || hie_fired) {
      h_psn_pure_extra[0]->Fill(6.5);
    }
    if (ffb_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(7.5);
    }
    if (fyb_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(8.5);
    }
  }

  //hie: require the total energy of ECL cluster is more than 1GeV
  if (total_energy > 1) {
    bool fff_fired = isFired_quick("fff");
    bool ffo_fired = isFired_quick("ffo");
    bool ffb_fired = isFired_quick("ffb");
    bool hie_fired = isFired_quick("hie");
    if (fff_fired || ffo_fired || ffb_fired) {
      h_psn_pure_extra[0]->Fill(9.5);
    }
    if (hie_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_pure_extra[0]->Fill(10.5);
    }
  }

  //c4: require the total number of cluster is more than 3
  if (ncluster > 3) {
    bool fff_fired = isFired_quick("fff");
    bool ffo_fired = isFired_quick("ffo");
    bool ffb_fired = isFired_quick("ffb");
    bool c4_fired = isFired_quick("c4");
    if (fff_fired || ffo_fired || ffb_fired) {
      h_psn_pure_extra[0]->Fill(11.5);
    }
    if (c4_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_pure_extra[0]->Fill(12.5);
    }
  }

}

const char* TRGGDLDQMModule::output_pure_extra[n_output_pure_extra] = {
  "c4|hie offline_fff", "fff&(c4|hie) offline_fff", "ffy&(c4|hie) offline_fff",
  "c4|hie offline_ffo", "ffo&(c4|hie) offline_ffo", "fyo&(c4|hie) offline_ffo",
  "c4|hie offline_ffb", "ffb&(c4|hie) offline_ffb", "fyb&(c4|hie) offline_ffb",
  "fff|ffb|ffo offline_hie", "hie&(fff|ffb|ffo) offline_hie",
  "fff|ffb|ffo offline_c4", "c4&(fff|ffb|ffo) offline_c4"
};


