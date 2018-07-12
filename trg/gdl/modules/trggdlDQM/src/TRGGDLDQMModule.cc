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
  dirDQM = oldDir->mkdir("TRG");
  dirDQM->cd();

  h_c8_gdlL1TocomL1  = new TH1I("hGDL_gdlL1TocomL1",  "comL1 - gdlL1 [clk8ns]", 100, 0, 100);
  h_c8_gdlL1TocomL1->GetXaxis()->SetTitle("clk8ns");
  h_c8_rvcoutTogdlL1 = new TH1I("hGDL_rvcoutTogdlL1", "gdlL1 - rvcout [clk8ns]", 500, 0, 500);
  h_c8_rvcoutTogdlL1->GetXaxis()->SetTitle("clk8ns");
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
  h_inp = new TH1I("hGDL_inp", "inp", 96, 0, 96);
  h_itd = new TH1I("hGDL_itd", "itd", 96, 0, 96);
  h_ftd = new TH1I("hGDL_ftd", "ftd", 48, 0, 48);
  h_psn = new TH1I("hGDL_psn", "psn", 48, 0, 48);
  h_timtype = new TH1I("hGDL_timtype", "timtype", 7, 0, 7);

  oldDir->cd();
}


void TRGGDLDQMModule::initialize()
{

  REG_HISTOGRAM
  defineHisto();

}

void TRGGDLDQMModule::event()
{
  const double clkTo2ns = 1. / .508877;
  const double clkTo1ns = 0.5 / .508877;
//TH2I* h_0 = new TH2I("hTRGGDLDQMModule_event", "", GDL::nClks, 0, GDL::nClks, GDL::nLeafs + GDL::nLeafsExtra, 0,
//                     GDL::nLeafs + GDL::nLeafsExtra);
  TH2I* h_0;

  dirDQM->cd();
  StoreArray<TRGGDLUnpackerStore> entAry;
  h_0 = new TH2I(Form("hgdl%08d", entAry[0]->m_evt), "", GDL::nClks, 0, GDL::nClks, GDL::nLeafs + GDL::nLeafsExtra, 0,
                 GDL::nLeafs + GDL::nLeafsExtra);
  oldDir->cd();

  // fill "bit vs clk" for the event
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    int* Bits[GDL::nLeafs + GDL::nLeafsExtra];
    setLeafPointersArray(entAry[ii], Bits);
    for (int leaf = 0; leaf < GDL::nLeafs + GDL::nLeafsExtra; leaf++) {
      h_0->SetBinContent(entAry[ii]->m_clk + 1, leaf + 1, *Bits[leaf]);
    }
  }
  for (int leaf = 0; leaf < GDL::nLeafs + GDL::nLeafsExtra; leaf++) {
    h_0->GetYaxis()->SetBinLabel(leaf + 1, GDL::LeafNames[leaf]);
  }

  int coml1    = h_0->GetBinContent(1, 1 + e_coml1);
  int eclmsb7  = h_0->GetBinContent(1, 1 + e_eclmsb7);
  int ecllsb7  = h_0->GetBinContent(1, 1 + e_ecllsb7);
  int etfout   = h_0->GetBinContent(1, 1 + GDL::e_etfout);
  int psn0     = 0;
  int psn1     = 0;
  int ftd0     = 0;
  int ftd1     = 0;
  int itd0     = 0;
  int itd1     = 0;
  int itd2     = 0;
  int inp0     = 0;
  int inp1     = 0;
  int inp2     = 0;
  int timtype  = 0;
  int c1_ecl_timing   = (eclmsb7 << 7) +  ecllsb7;
  int c8_ecl_timing   = (eclmsb7 << 4) + (ecllsb7 >> 3);
  int c2_ecl_timing   = (eclmsb7 << 6) + (ecllsb7 >> 1);
  int etm0rvc  = h_0->GetBinContent(1, 1 + e_etm0rvc);

  int c1_top_timing = h_0->GetBinContent(48, 1 + e_topt0);
  int c2_top_timing = c1_top_timing >> 1;

  int gdll1_rvc = -10;
  int rvcout14  = -10;
  bool top_active_found = false;
  bool cdc_active_found = false;
  for (int clk = 1; clk <= GDL::nClks; clk++) {
    if (h_0->GetBinContent(clk, 1 + e_gdll1) > 0) {
      gdll1_rvc = h_0->GetBinContent(clk, 1 + e_rvc);
      rvcout14 = h_0->GetBinContent(clk, 1 + e_rvcout);
    }
    int psn0_tmp = h_0->GetBinContent(clk, 1 + e_psn0);
    int psn1_tmp = h_0->GetBinContent(clk, 1 + e_psn1);
    int ftd0_tmp = h_0->GetBinContent(clk, 1 + e_ftd0);
    int ftd1_tmp = h_0->GetBinContent(clk, 1 + e_ftd1);
    int inp0_tmp = h_0->GetBinContent(clk, 1 + e_inp0);
    int inp1_tmp = h_0->GetBinContent(clk, 1 + e_inp1);
    int inp2_tmp = h_0->GetBinContent(clk, 1 + e_inp2);
    int itd0_tmp = h_0->GetBinContent(clk, 1 + e_itd0);
    int itd1_tmp = h_0->GetBinContent(clk, 1 + e_itd1);
    int itd2_tmp = h_0->GetBinContent(clk, 1 + e_itd2);
    psn0 = (psn0_tmp == 0) ? psn0 : psn0_tmp;
    psn1 = (psn1_tmp == 0) ? psn1 : psn1_tmp;
    ftd0 = (ftd0_tmp == 0) ? ftd0 : ftd0_tmp;
    ftd1 = (ftd1_tmp == 0) ? ftd1 : ftd0_tmp;
    itd0 = (itd0_tmp == 0) ? itd0 : itd0_tmp;
    itd1 = (itd1_tmp == 0) ? itd1 : itd1_tmp;
    itd2 = (itd2_tmp == 0) ? itd2 : itd2_tmp;
    inp0 = (inp0_tmp == 0) ? inp0 : inp0_tmp;
    inp1 = (inp1_tmp == 0) ? inp1 : inp1_tmp;
    inp2 = (inp2_tmp == 0) ? inp2 : inp2_tmp;
    int timtype_tmp = h_0->GetBinContent(clk, 1 + e_timtype);
    timtype = (timtype_tmp == 0) ? timtype : timtype_tmp;

    if (itd1_tmp & (1 << 18)) {
      top_active_found = true;
    }
    if (itd0_tmp & (1 << 14)) {
      cdc_active_found = true;
    }
  }
  h_timtype->Fill(timtype);

  for (int i = 0; i < 32; i++) {
    if (inp0 & (1 << i)) h_inp->Fill(i + 0.5);
    if (inp1 & (1 << i)) h_inp->Fill(i + 0.5 + 32);
    if (inp2 & (1 << i)) h_inp->Fill(i + 0.5 + 64);
    if (itd0 & (1 << i)) h_itd->Fill(i + 0.5);
    if (itd1 & (1 << i)) h_itd->Fill(i + 0.5 + 32);
    if (itd2 & (1 << i)) h_itd->Fill(i + 0.5 + 64);
    if (ftd0 & (1 << i)) h_ftd->Fill(i + 0.5);
    if (ftd1 & (1 << i)) h_ftd->Fill(i + 0.5 + 32);
    if (psn0 & (1 << i)) h_psn->Fill(i + 0.5);
    if (psn1 & (1 << i)) h_psn->Fill(i + 0.5 + 32);
  }

  int gdlL1TocomL1 = gdll1_rvc < coml1 ? coml1 - gdll1_rvc : (coml1 + 1280) - gdll1_rvc;
  h_c8_gdlL1TocomL1->Fill(gdlL1TocomL1);

  int eclTogdlL1 = gdll1_rvc < c8_ecl_timing ? (gdll1_rvc + 1280) - c8_ecl_timing : gdll1_rvc - c8_ecl_timing;
  h_c8_eclTogdlL1->Fill(eclTogdlL1);

  int rvcout = rvcout14 >> 3;
  int rvcoutTogdlL1 = gdll1_rvc < rvcout ? (gdll1_rvc + 1280) - rvcout : gdll1_rvc - rvcout;
  h_c8_rvcoutTogdlL1->Fill(rvcoutTogdlL1);

  int c127_ecl_timing = c8_ecl_timing & (((1 << 7) - 1) << 4);
  int fit8mToGDL = c127_ecl_timing < etm0rvc ? etm0rvc - c127_ecl_timing : (etm0rvc + 1280) - c127_ecl_timing;
  h_c8_ecl8mToGDL->Fill(fit8mToGDL);

  int eclToGDL = c8_ecl_timing < etm0rvc ? etm0rvc - c8_ecl_timing : (etm0rvc + 1280) - c8_ecl_timing;
  h_c8_eclToGDL->Fill(eclToGDL);

  int c2_cdc_timing = etfout;
  int c2_comL1 = coml1 << 2;
  int c2_gdlL1 = gdll1_rvc << 2;
  int c2_diff_cdcTogdlL1 = c2_gdlL1 > c2_cdc_timing ?
                           c2_gdlL1 - c2_cdc_timing :
                           c2_gdlL1 - c2_cdc_timing + (1280 << 2) ;
  h_ns_cdcTogdlL1->Fill(c2_diff_cdcTogdlL1 * clkTo2ns);

  if (cdc_active_found) {
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
  }

  if (top_active_found) {
    int c1_diff_topToecl = c1_ecl_timing > c1_top_timing ?
                           c1_ecl_timing - c1_top_timing :
                           c1_ecl_timing - c1_top_timing + (1280 << 3);
    h_ns_topToecl->Fill(c1_diff_topToecl *  clkTo1ns);
  }
  delete h_0;

  if (cdc_active_found && top_active_found) {
    int c2_diff_topTocdc = c2_cdc_timing > c2_top_timing ?
                           c2_cdc_timing - c2_top_timing :
                           c2_cdc_timing - c2_top_timing + (1280 << 2);
    h_ns_topTocdc->Fill(c2_diff_topTocdc *  clkTo2ns);
  }

}
