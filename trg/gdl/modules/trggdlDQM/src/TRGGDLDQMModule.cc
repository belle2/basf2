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
#include <trg/gdl/TrgBit.h>
//#include <framework/dbobjects/RunInfo.h>


#include <TDirectory.h>
#include <TRandom3.h>
#include <unistd.h>

using namespace Belle2;
using namespace GDL;

REG_MODULE(TRGGDLDQM);


TRGGDLDQMModule::TRGGDLDQMModule() : HistoModule()
{

  setDescription("DQM for GDL Trigger system");
  setPropertyFlags(c_ParallelProcessingCertified);

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
  h_itd = new TH1I("hGDL_itd", "itd", 161, -1, 160);
  h_ftd = new TH1I("hGDL_ftd", "ftd", 97, -1, 96);
  h_psn = new TH1I("hGDL_psn", "psn", 97, -1, 96);
  h_timtype = new TH1I("hGDL_timtype", "timtype", 7, 0, 7);

  oldDir->cd();
}

void TRGGDLDQMModule::beginRun()
{

  TrgBit tb;
  tb.printPreScaleValues();

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

  REG_HISTOGRAM
  defineHisto();

}

void TRGGDLDQMModule::event()
{

  int n_clocks = 0;
  int n_leafs = 0;
  int n_leafsExtra = 0;
  int ee_psn[10] = {0};
  int ee_ftd[10] = {0};
  int ee_itd[10] = {0};
  int nword_input = 3;
  int nword_output = 3;
  int _e_timtype = 0;
  void (*setPointer)(TRGGDLUnpackerStore * store, int** bitArray);
  int _e_gdll1rvc = 0;
  StoreArray<TRGGDLUnpackerStore> entAry;
  if (!entAry || !entAry.getEntries()) return;

  int nconf = entAry[0]->m_conf;

  if (nconf == 6) {
    nword_input = 5;
    n_clocks = GDLCONF6::nClks;
    n_leafs = GDLCONF6::nLeafs;
    n_leafsExtra = GDLCONF6::nLeafsExtra;
    ee_itd[0] = GDLCONF6::e_psn0; ee_itd[1] = GDLCONF6::e_itd1; ee_itd[2] = GDLCONF6::e_itd2;
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
    ee_itd[0] = GDLCONF5::e_psn0; ee_itd[1] = GDLCONF5::e_itd1; ee_itd[2] = GDLCONF5::e_itd2;
    ee_psn[0] = GDLCONF5::e_psn0; ee_psn[1] = GDLCONF5::e_psn1; ee_psn[2] = GDLCONF5::e_psn2;
    ee_ftd[0] = GDLCONF5::e_ftd0; ee_ftd[1] = GDLCONF5::e_ftd1; ee_ftd[2] = GDLCONF5::e_ftd2;
    _e_timtype = GDLCONF5::e_timtype;
    setPointer = GDLCONF5::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF5::e_gdll1rvc;
  } else if (nconf == 4) {
    n_clocks = GDLCONF4::nClks;
    n_leafs = GDLCONF4::nLeafs;
    n_leafsExtra = GDLCONF4::nLeafsExtra;
    ee_itd[0] = GDLCONF4::e_psn0; ee_itd[1] = GDLCONF4::e_itd1; ee_itd[2] = GDLCONF4::e_itd2;
    ee_psn[0] = GDLCONF4::e_psn0; ee_psn[1] = GDLCONF4::e_psn1; ee_psn[2] = GDLCONF4::e_psn2;
    ee_ftd[0] = GDLCONF4::e_ftd0; ee_ftd[1] = GDLCONF4::e_ftd1; ee_ftd[2] = GDLCONF4::e_ftd2;
    _e_timtype = GDLCONF4::e_timtype;
    setPointer = GDLCONF4::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF4::e_gdll1rvc;
  } else if (nconf == 3) {
    n_clocks = GDLCONF3::nClks;
    n_leafs = GDLCONF3::nLeafs;
    n_leafsExtra = GDLCONF3::nLeafsExtra;
    ee_itd[0] = GDLCONF3::e_psn0; ee_itd[1] = GDLCONF3::e_itd1; ee_itd[2] = GDLCONF3::e_itd2;
    ee_psn[0] = GDLCONF3::e_psn0; ee_psn[1] = GDLCONF3::e_psn1; ee_psn[2] = GDLCONF3::e_psn2;
    ee_ftd[0] = GDLCONF3::e_ftd0; ee_ftd[1] = GDLCONF3::e_ftd1; ee_ftd[2] = GDLCONF3::e_ftd2;
    _e_timtype = GDLCONF3::e_timtype;
    setPointer = GDLCONF3::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF3::e_gdll1rvc;
  } else if (nconf == 2) {
    n_clocks = GDLCONF2::nClks;
    n_leafs = GDLCONF2::nLeafs;
    n_leafsExtra = GDLCONF2::nLeafsExtra;
    ee_itd[0] = GDLCONF2::e_psn0; ee_itd[1] = GDLCONF2::e_itd1; ee_itd[2] = GDLCONF2::e_itd2;
    ee_psn[0] = GDLCONF2::e_psn0; ee_psn[1] = GDLCONF2::e_psn1; ee_psn[2] = GDLCONF2::e_psn2;
    ee_ftd[0] = GDLCONF2::e_ftd0; ee_ftd[1] = GDLCONF2::e_ftd1; ee_ftd[2] = GDLCONF2::e_ftd2;
    _e_timtype = GDLCONF2::e_timtype;
    setPointer = GDLCONF2::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF2::e_gdll1rvc;
  } else if (nconf == 1) {
    n_clocks = GDLCONF1::nClks;
    n_leafs = GDLCONF1::nLeafs;
    n_leafsExtra = GDLCONF1::nLeafsExtra;
    ee_itd[0] = GDLCONF1::e_psn0; ee_itd[1] = GDLCONF1::e_itd1; ee_itd[2] = GDLCONF1::e_itd2;
    ee_psn[0] = GDLCONF1::e_psn0; ee_psn[1] = GDLCONF1::e_psn1; ee_psn[2] = GDLCONF1::e_psn2;
    ee_ftd[0] = GDLCONF1::e_ftd0; ee_ftd[1] = GDLCONF1::e_ftd1; ee_ftd[2] = GDLCONF1::e_ftd2;
    _e_timtype = GDLCONF1::e_timtype;
    setPointer = GDLCONF1::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF1::e_gdll1rvc;
  } else {
    n_clocks = GDLCONF0::nClks;
    n_leafs = GDLCONF0::nLeafs;
    n_leafsExtra = GDLCONF0::nLeafsExtra;
    ee_itd[0] = GDLCONF0::e_psn0; ee_itd[1] = GDLCONF0::e_itd1; ee_itd[2] = GDLCONF0::e_itd2;
    ee_psn[0] = GDLCONF0::e_psn0; ee_psn[1] = GDLCONF0::e_psn1; ee_psn[2] = GDLCONF0::e_psn2;
    ee_ftd[0] = GDLCONF0::e_ftd0; ee_ftd[1] = GDLCONF0::e_ftd1; ee_ftd[2] = GDLCONF0::e_ftd2;
    _e_timtype = GDLCONF0::e_timtype;
    setPointer = GDLCONF0::setLeafPointersArray;
    _e_gdll1rvc = GDLCONF0::e_gdll1rvc;
  }


  const double clkTo2ns = 1. / .508877;
  const double clkTo1ns = 0.5 / .508877;
  TH2I* h_0;
  TH2I* h_p;
  TH2I* h_f;
  TH2I* h_i;

  dirDQM->cd();
  h_0 = new TH2I(Form("hgdl%08d", entAry[0]->m_evt), "", n_clocks, 0, n_clocks, n_leafs + n_leafsExtra, 0,
                 n_leafs + n_leafsExtra);
  h_p = new TH2I(Form("hpsn%08d", entAry[0]->m_evt), "", n_clocks, 0, n_clocks, 96, 0, 96);
  h_f = new TH2I(Form("hftd%08d", entAry[0]->m_evt), "", n_clocks, 0, n_clocks, 96, 0, 96);
  h_i = new TH2I(Form("hitd%08d", entAry[0]->m_evt), "", n_clocks, 0, n_clocks, 160, 0, 160);

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
  int psn[3] = {0};
  int ftd[3] = {0};
  int itd[5] = {0};
  int timtype  = 0;
  int c8_ecl_timing   = (c1_ecl_timing >> 3);
  int c2_ecl_timing   = (c1_ecl_timing >> 1);

  int c2_top_timing = c1_top_timing >> 1;

  int gdll1_rvc = h_0->GetBinContent(h_0->GetXaxis()->FindBin(n_clocks - 0.5), 1 + _e_gdll1rvc);
  for (int clk = 1; clk <= n_clocks; clk++) {
    int psn_tmp[3] = {0};
    int ftd_tmp[3] = {0};
    int itd_tmp[5] = {0};
    for (int j = 0; j < nword_input; j++) {
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
      for (int j = 0; j < nword_output; j++) {
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

  h_timtype->Fill(timtype);

  h_itd->Fill(-0.5);
  h_ftd->Fill(-0.5);
  h_psn->Fill(-0.5);
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < nword_input; j++) {
      if (itd[j] & (1 << i)) h_itd->Fill(i + 0.5 + 32 * j);
    }
    for (int j = 0; j < nword_output; j++) {
      if (ftd[j] & (1 << i)) h_ftd->Fill(i + 0.5 + 32 * j);
      if (psn[j] & (1 << i)) h_psn->Fill(i + 0.5 + 32 * j);
    }
  }

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
}
