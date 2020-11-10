//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGTOPDQMModule.cc
// Section  : TRG TOP
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : DQM module for TRGTOP
//---------------------------------------------------------------
// 1.00 : 2020/09/20 : First version
//
// Modeled after / heavily borrowing from GDL and GRL DQM modules
//---------------------------------------------------------------
#include <trg/top/modules/trgtopDQM/TRGTOPDQMModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include "trg/top/dataobjects/TRGTOPUnpackerStore.h"

#include <TDirectory.h>
#include <TPostScript.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <fstream>
#include <framework/logging/Logger.h>
#include <boost/algorithm/string.hpp>

#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(TRGTOPDQM);

TRGTOPDQMModule::TRGTOPDQMModule() : HistoModule()
{

  setDescription("DQM for TOP Trigger subsystem");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("doGDLCorrelations", m_doGDLCorrelations,
           "Do GDL-TOP correlations (true)",
           true);
  addParam("doGRLCorrelations", m_doGRLCorrelations,
           "Do GRL-TOP correlations (true)",
           true);
  addParam("generatePostscriptFile", m_generatePostscriptFile,
           "Generate postscript file (false)",
           false);
  addParam("postScriptFileName", m_postScriptFileName,
           "postscript file name",
           string("topl1dqm.ps"));
  addParam("skim", m_skim,
           "use skim information (-1)",
           int(-1));

}

void TRGTOPDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = gDirectory;
  oldDir->mkdir("TRGTOP");
  dirDQM->cd("TRGTOP");

  for (int slot = 0; slot <= 15; slot++) {
    h_topSlotSegment[slot]  = new TH1I(Form("h_segment_slot_%s", (to_string(slot + 1)).c_str()), "segment", 10, 1, 11);
    h_topSlotNHits[slot]  = new TH1I(Form("h_nhit_slot_%s", (to_string(slot + 1)).c_str()), "nhit", 200, 0, 200);
    h_topSlotLogL[slot]  = new TH1I(Form("h_logl_slot_%s", (to_string(slot + 1)).c_str()), "log L", 100, 0, 100000);
    h_topSlotT0[slot]  = new TH1I(Form("h_t0_slot_%s", (to_string(slot + 1)).c_str()), "t0", 100, 0, 100000);
  }

  h_topSlotVsSegment  = new TH2I("h_segment_vs_slot", "segment", 10, 1, 11, 16, 1, 17);
  h_topSlotVsNHits  = new TH2I("h_nhit_vs_slot", "nhit", 200, 0, 200, 16, 1, 17);
  h_topSlotVsLogL  = new TH2I("h_logl_vs_slot", "log L", 100, 0, 100000, 16, 1, 17);
  h_topSlotVsT0  = new TH2I("h_t0_vs_slot", "t0", 100, 0, 100000, 16, 1, 17);

  for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {

    // max N of combined t0 decisions (over past ~us) = N windows in circular buffer
    h_N_decision[iskim]  = new TH1I(Form("h_N_decisions_%s",  skim_smap[iskim].c_str()), "N t0 decisions in 1 us before GDL L1", 25, 0,
                                    25);
    h_N_decision[iskim]->GetXaxis()->SetTitle("TOPTRG N combined t0 decisions");

    h_topCombinedTimingTop[iskim]  = new TH1I(Form("h_TOP_t0_%s",  skim_smap[iskim].c_str()),  "TOP combined t0 decision", 100, 0,
                                              100000);
    h_topCombinedTimingTop[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions");

    h_topNSlotsCombinedTimingTop[iskim]  = new TH1I(Form("h_TOP_N_slots_%s",  skim_smap[iskim].c_str()),
                                                    "TOP combined t0 decision: N slots", 16, 0, 16);
    h_topNSlotsCombinedTimingTop[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N slots");

    h_topNHitSum[iskim]  = new TH1I(Form("h_TOP_N_hit_per_slot_%s",  skim_smap[iskim].c_str()),
                                    "TOP combined t0 decision: N hit per slot", 300, 0, 300);
    h_topNHitSum[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N hits per slot");

    h_topLogLSum[iskim]  = new TH1I(Form("h_TOP_logL_per_slot_%s",  skim_smap[iskim].c_str()),
                                    "TOP combined t0 decision: logL per slot", 100, 0, 100000);
    h_topLogLSum[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: logL per slot");

    h_topRvcDiff1[iskim]  = new TH1I(Form("h_TOP_rvc_diff_1_%s",  skim_smap[iskim].c_str()),  "TOP combined t0 decision: rvc diff 1",
                                     400, -100, 300);
    h_topRvcDiff1[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: rvc diff 1");

    h_topRvcDiff2[iskim]  = new TH1I(Form("h_TOP_rvc_diff_2_%s",  skim_smap[iskim].c_str()),  "TOP combined t0 decision: rvc diff 2",
                                     200, -1000, 1000);
    h_topRvcDiff2[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: rvc diff 2");

    h_topRvcDiff3[iskim]  = new TH1I(Form("h_TOP_rvc_diff_3_%s",  skim_smap[iskim].c_str()),  "TOP combined t0 decision: rvc diff 3",
                                     200, -1000, 1000);
    h_topRvcDiff3[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: rvc diff 3");

    h_topRvcDiff4[iskim]  = new TH1I(Form("h_TOP_rvc_diff_4_%s",  skim_smap[iskim].c_str()),  "TOP combined t0 decision: rvc diff 4",
                                     200, -1000, 1000);
    h_topRvcDiff4[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: rvc diff 4");

    h_top_gdl_timing_diff[iskim]  = new TH1I(Form("h_top_gdl_timing_diff_%s",  skim_smap[iskim].c_str()),  "TOP TOP-GDL t0 difference",
                                             25, 0, 25);
    h_top_gdl_timing_diff[iskim]->GetXaxis()->SetTitle("TOPTRG TOP-GDL t0 difference (5ns)");

    h_gdl_ecltop_timing_diff_5ns[iskim] = new TH1I(Form("h_gdl_ecltop_timing_diff_5ns_%s",  skim_smap[iskim].c_str()),
                                                   "GDL ECL-TOP t0 difference", 248, 0, 10240);
    h_gdl_ecltop_timing_diff_5ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference (5ns)");

    h_gdl_cdctop_timing_diff_5ns[iskim] = new TH1I(Form("h_gdl_cdctop_timing_diff_5ns_%s",  skim_smap[iskim].c_str()),
                                                   "GDL CDC-TOP t0 difference", 248, 0, 10240);
    h_gdl_cdctop_timing_diff_5ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL CDC-TOP t0 difference (5ns)");

    h_gdl_ecltop_timing_diff_1ns[iskim] = new TH1I(Form("h_gdl_ecltop_timing_diff_1ns_%s",  skim_smap[iskim].c_str()),
                                                   "GDL ECL-TOP t0 difference", 800, 1400, 2200);
    h_gdl_ecltop_timing_diff_1ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference (1ns)");

    h_gdl_cdctop_timing_diff_2ns[iskim] = new TH1I(Form("h_gdl_cdctop_timing_diff_2ns_%s",  skim_smap[iskim].c_str()),
                                                   "GDL CDC-TOP t0 difference", 250, 0, 500);
    h_gdl_cdctop_timing_diff_2ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL CDC-TOP t0 difference (2ns)");

    h_gdl_gdltop_rvc_diff[iskim] = new TH1I(Form("h_gdl_gdltop_rvc_diff_%s",  skim_smap[iskim].c_str()),  "GDL GDL-TOP rvc difference",
                                            20, 0, 20);
    h_gdl_gdltop_rvc_diff[iskim]->GetXaxis()->SetTitle("TOPTRG GDL GDL-TOP rvc difference (2ns)");

    h_gdl_comtop_rvc_diff[iskim] = new TH1I(Form("h_gdl_comtop_rvc_diff_%s",  skim_smap[iskim].c_str()),  "GDL COM-TOP rvc difference",
                                            30, 20, 50);
    h_gdl_comtop_rvc_diff[iskim]->GetXaxis()->SetTitle("TOPTRG GDL COM-TOP rvc difference (2ns)");

    h_gdl_ecltop_rvc_diff[iskim] = new TH1I(Form("h_gdl_ecltop_rvc_diff_%s",  skim_smap[iskim].c_str()),  "GDL ECL-TOP rvc difference",
                                            10, 0, 10);
    h_gdl_ecltop_rvc_diff[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP rvc difference (2ns)");

    h_gdl_cdctop_rvc_diff[iskim] = new TH1I(Form("h_gdl_cdctop_rvc_diff_%s",  skim_smap[iskim].c_str()),  "GDL CDC-TOP rvc difference",
                                            10, 0, 10);
    h_gdl_cdctop_rvc_diff[iskim]->GetXaxis()->SetTitle("TOPTRG GDL CDC-TOP rvc difference (2ns)");

  }

  oldDir->cd();

}

void TRGTOPDQMModule::beginRun()
{

  dirDQM->cd();

  for (int slot = 0; slot <= 15; slot++) {
    h_topSlotSegment[slot]->Reset();
    h_topSlotNHits[slot]->Reset();
    h_topSlotLogL[slot]->Reset();
    h_topSlotT0[slot]->Reset();
  }

  h_topSlotVsSegment->Reset();
  h_topSlotVsNHits->Reset();
  h_topSlotVsLogL->Reset();
  h_topSlotVsT0->Reset();

  for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {

    h_N_decision[iskim]->Reset();

    h_topCombinedTimingTop[iskim]->Reset();
    h_topNSlotsCombinedTimingTop[iskim]->Reset();
    h_topNHitSum[iskim]->Reset();
    h_topLogLSum[iskim]->Reset();

    h_topRvcDiff1[iskim]->Reset();
    h_topRvcDiff2[iskim]->Reset();
    h_topRvcDiff3[iskim]->Reset();
    h_topRvcDiff4[iskim]->Reset();

    h_top_gdl_timing_diff[iskim]->Reset();

    h_gdl_ecltop_timing_diff_5ns[iskim]->Reset();
    h_gdl_cdctop_timing_diff_5ns[iskim]->Reset();
    h_gdl_ecltop_timing_diff_1ns[iskim]->Reset();
    h_gdl_cdctop_timing_diff_2ns[iskim]->Reset();
    h_gdl_gdltop_rvc_diff[iskim]->Reset();
    h_gdl_comtop_rvc_diff[iskim]->Reset();
    h_gdl_ecltop_rvc_diff[iskim]->Reset();
    h_gdl_cdctop_rvc_diff[iskim]->Reset();
  }

  oldDir->cd();
}

void TRGTOPDQMModule::initialize()
{

  if (m_skim == 0) { //no skims
    start_skim_topdqm = 0;
    end_skim_topdqm = 1;
  } else if (m_skim == 1) { //skims
    start_skim_topdqm = 1;
    end_skim_topdqm = nskim_topdqm;
  } else { //no skims + skims
    start_skim_topdqm = 0;
    end_skim_topdqm = nskim_topdqm;
  }

  StoreObjPtr<EventMetaData> bevt;

  m_exp = -1;
  m_run = -1;

  if (bevt) {
    m_exp = bevt->getExperiment();
    m_run = bevt->getRun();
  }

  //  cout << "TOP L1 DQM: begin run / exp = " << m_exp << endl;
  //  cout << "TOP L1 DQM: begin run / run = " << m_run << endl;

  //  m_ECLCalDigitData.registerInDataStore();
  //  m_ECLDigitData.registerInDataStore();
  //  trgeclmap = new TrgEclMapping();

  // calls back the defineHisto() function, but the HistoManager module has to be in the path
  REG_HISTOGRAM

  if (m_doGDLCorrelations) {

    for (int i = 0; i < 320; i++) {
      gdlLeafBitMap[i] = m_gdlUnpacker->getLeafMap(i);
    }
    for (int i = 0; i < 320; i++) {
      strcpy(gdlLeafNames[i], m_gdlUnpacker->getLeafnames(i));
    }

    gdl_e_timtype = 0;
    gdl_e_gdll1rvc = 0;
    gdl_e_coml1rvc = 0;
    gdl_e_toptiming = 0;
    gdl_e_ecltiming = 0;
    gdl_e_cdctiming = 0;
    gdl_e_toprvc  = 0;
    gdl_e_eclrvc  = 0;
    gdl_e_cdcrvc  = 0;

    for (int i = 0; i < 320; i++) {
      if (strcmp(gdlLeafNames[i], "timtype") == 0)   gdl_e_timtype  = gdlLeafBitMap[i];
      if (strcmp(gdlLeafNames[i], "gdll1rvc") == 0)  gdl_e_gdll1rvc = gdlLeafBitMap[i];
      if (strcmp(gdlLeafNames[i], "coml1rvc") == 0)  gdl_e_coml1rvc = gdlLeafBitMap[i];
      if (strcmp(gdlLeafNames[i], "toptiming") == 0) gdl_e_toptiming = gdlLeafBitMap[i];
      if (strcmp(gdlLeafNames[i], "ecltiming") == 0) gdl_e_ecltiming = gdlLeafBitMap[i];
      if (strcmp(gdlLeafNames[i], "cdctiming") == 0) gdl_e_cdctiming = gdlLeafBitMap[i];
      if (strcmp(gdlLeafNames[i], "toprvc") == 0)    gdl_e_toprvc   = gdlLeafBitMap[i];
      if (strcmp(gdlLeafNames[i], "eclrvc") == 0)    gdl_e_eclrvc   = gdlLeafBitMap[i];
      if (strcmp(gdlLeafNames[i], "cdcrvc") == 0)    gdl_e_cdcrvc   = gdlLeafBitMap[i];
    }
  }

}

void TRGTOPDQMModule::endRun()
{
}

void TRGTOPDQMModule::event()
{

  /* cppcheck-suppress variableScope */
  //  static bool begin_run = true;

  StoreArray<TRGTOPUnpackerStore> trgtopCombinedTimingArray("TRGTOPUnpackerStores");

  if (!trgtopCombinedTimingArray) return;
  if (!trgtopCombinedTimingArray.getEntries()) return;

  // prepare histograms according to HLT decisions
  skim.clear();

  //Get skim type from SoftwareTriggerResult
  for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {
    if (iskim == 0) skim.push_back(iskim);
  }

  StoreObjPtr<SoftwareTriggerResult> result_soft;
  if (result_soft.isValid()) {
    const std::map<std::string, int>& skim_map = result_soft->getResults();
    for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {
      if (iskim == 0);
      else if (skim_map.find(skim_menu[iskim]) != skim_map.end()) {
        const bool accepted = (result_soft->getResult(skim_menu[iskim]) == SoftwareTriggerCutResult::c_accept);
        if (accepted) skim.push_back(iskim);
      }
    }
  }

  bool gdlInfoAvailable = false;
  bool grlInfoAvailable = false;

  // investigate GDL-TOP correlations if asked to do so

  int gdl_gdll1rvc   = -1;
  int gdl_coml1rvc   = -1;
  int gdl_toprvc     = -1;
  int gdl_eclrvc     = -1;
  int gdl_cdcrvc     = -1;
  int gdl_top_timing = -1;
  int gdl_ecl_timing = -1;
  int gdl_cdc_timing = -1;


  if (m_doGDLCorrelations) {

    // make sure first that GDL information could be retrieved
    if (m_gdlUnpacker) {

      /*
      int N_track = evtinfo->getEventIdL1();
      int bin = h_N_track->GetBinContent(N_track + 1);
      h_N_track->SetBinContent(N_track + 1, bin + 1);
      */

      int n_leafs  = m_gdlUnpacker->getnLeafs();
      int n_leafsExtra = m_gdlUnpacker->getnLeafsExtra();
      int n_clocks = m_gdlUnpacker->getnClks();
      //      int nconf = m_gdlUnpacker->getconf();

      //  int nword_input  = m_gdlUnpacker->get_nword_input();
      //  int nword_output = m_gdlUnpacker->get_nword_output();

      StoreArray<TRGGDLUnpackerStore> entAry;
      if (entAry && entAry.getEntries()) {

        m_evtno = 0;

        //prepare entAry address
        int clk_map = 0;
        for (int i = 0; i < 320; i++) {
          if (strcmp(entAry[0]->m_unpackername[i], "evt") == 0) m_evtno = entAry[0]->m_unpacker[i];
          if (strcmp(entAry[0]->m_unpackername[i], "clk") == 0) clk_map = i;
        }

        //  cout << "TOP L1 DQM GDL event number = " << m_evtno << endl;

        std::vector<std::vector<int> > _data(n_leafs + n_leafsExtra);
        for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
          std::vector<int> _v(n_clocks);
          _data[leaf] = _v;
        }

        // fill "bit vs clk" for the event
        for (int ii = 0; ii < entAry.getEntries(); ii++) {
          std::vector<int*> Bits(n_leafs + n_leafsExtra);
          //set pointer
          for (int i = 0; i < 320; i++) {
            if (gdlLeafBitMap[i] != -1) {
              Bits[gdlLeafBitMap[i]] = &(entAry[ii]->m_unpacker[i]);
            }
          }
          for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
            _data[leaf][entAry[ii]->m_unpacker[clk_map]] =  *Bits[leaf];
          }

        }

        gdl_gdll1rvc   = _data[gdl_e_gdll1rvc][n_clocks - 1];
        gdl_coml1rvc   = _data[gdl_e_coml1rvc][n_clocks - 1];
        gdl_toprvc     = _data[gdl_e_toprvc][n_clocks - 1];
        gdl_eclrvc     = _data[gdl_e_eclrvc][n_clocks - 1];
        gdl_cdcrvc     = _data[gdl_e_cdcrvc][n_clocks - 1];
        gdl_top_timing = _data[gdl_e_toptiming][n_clocks - 1];
        gdl_ecl_timing = _data[gdl_e_ecltiming][n_clocks - 1];
        gdl_cdc_timing = _data[gdl_e_cdctiming][n_clocks - 1];

        gdlInfoAvailable = true;

        /*
            if (begin_run) {
            B2DEBUG(20, "nconf(" << nconf
            << "), n_clocks(" << n_clocks
            << "), n_leafs(" << n_leafs
            << "), n_leafsExtra(" << n_leafsExtra
            << ")");
            begin_run = false;
            }
        */

        /*
        cout << " " << endl;
        cout << "TOP L1 DQM gdl L1 rvc = " << gdl_gdll1rvc << endl;
        cout << "TOP L1 DQM com L1 rvc = " << gdl_coml1rvc << endl;
        cout << "TOP L1 DQM top L1 rvc = " << gdl_toprvc << endl;
        cout << "TOP L1 DQM ecl L1 rvc = " << gdl_eclrvc << endl;
        cout << "TOP L1 DQM cdc L1 rvc = " << gdl_cdcrvc << endl;
        */

      }
    }
  }

  // info from GDL

  if (gdlInfoAvailable) {

    int ecl_top_timing_diff = gdl_ecl_timing >= gdl_top_timing ? gdl_ecl_timing - gdl_top_timing : gdl_ecl_timing - gdl_top_timing +
                              10240;
    int cdc_top_timing_diff = gdl_cdc_timing >= gdl_top_timing ? gdl_cdc_timing - gdl_top_timing : gdl_cdc_timing - gdl_top_timing +
                              10240;

    int gdl_top_rvc_diff = gdl_gdll1rvc >= gdl_toprvc ? gdl_gdll1rvc - gdl_toprvc : gdl_gdll1rvc - gdl_toprvc + 10;
    int com_top_rvc_diff = gdl_coml1rvc >= gdl_toprvc ? gdl_coml1rvc - gdl_toprvc : gdl_coml1rvc - gdl_toprvc + 1280;
    int ecl_top_rvc_diff = gdl_eclrvc >= gdl_toprvc ? gdl_eclrvc - gdl_toprvc : gdl_eclrvc - gdl_toprvc + 1280;
    int cdc_top_rvc_diff = gdl_cdcrvc >= gdl_toprvc ? gdl_cdcrvc - gdl_toprvc : gdl_cdcrvc - gdl_toprvc + 1280;

    //  const double clkTo1ns = 0.5 / .508877;
    // Feb. 23, 2020  ecltopdiff = (int) ( (double) ecltopdiff * clkTo1ns);

    for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

      h_gdl_ecltop_timing_diff_5ns[skim[ifill]]->Fill(ecl_top_timing_diff);
      h_gdl_cdctop_timing_diff_5ns[skim[ifill]]->Fill(cdc_top_timing_diff);

      h_gdl_ecltop_timing_diff_1ns[skim[ifill]]->Fill(ecl_top_timing_diff);
      h_gdl_cdctop_timing_diff_2ns[skim[ifill]]->Fill(cdc_top_timing_diff);

      h_gdl_gdltop_rvc_diff[skim[ifill]]->Fill(gdl_top_rvc_diff);
      h_gdl_comtop_rvc_diff[skim[ifill]]->Fill(com_top_rvc_diff);
      h_gdl_ecltop_rvc_diff[skim[ifill]]->Fill(ecl_top_rvc_diff);
      h_gdl_cdctop_rvc_diff[skim[ifill]]->Fill(cdc_top_rvc_diff);

    }

  }

  // investigate GRL-TOP correlations if asked to do so

  int grlTimeL1 = -1;
  int grlTOPL1 = -1;

  bool grlCDCSlots[16];
  bool grlTOPSlots[16];

  if (m_doGRLCorrelations) {

    StoreObjPtr<TRGGRLUnpackerStore> grlEventInfo("TRGGRLUnpackerStore");

    // make sure first that GDL information could be retrieved
    if (grlEventInfo) {

      grlTimeL1 = grlEventInfo->m_coml1 - grlEventInfo->m_revoclk;

      grlTOPL1 = (grlEventInfo->m_TOPL1_count + grlTimeL1 - 0.5) * (-clk127To1ns);

      // Slot number -> phi (11.25 + i * 22.5)
      // 16 slots @ 22.5 deg = 360 deg
      for (int i = 0; i < 16; i++) {
        grlCDCSlots[i] = false;
        if (grlEventInfo->m_slot_CDC[i]) {
          grlCDCSlots[i] = true;
        }
        grlTOPSlots[i] = false;
        if (grlEventInfo->m_slot_TOP[i]) {
          grlTOPSlots[i] = true;
        }
      }
      grlInfoAvailable = true;
    }
  }

  // retrieve TOP L1 information

  int nT0Decisions = trgtopCombinedTimingArray.getEntries();

  // prepare histograms for TOP alone
  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    h_N_decision[skim[ifill]]->Fill(nT0Decisions);
  }

  //  int t0 = -1;
  int t0Index = 0;
  int nInTime = 0;

  for (const auto& t0Decision : trgtopCombinedTimingArray) {

    // not possible    const vector<Belle2::TRGTOPSlotTiming> t0DecisionSlots = t0Decision.getSlotTimingDecisions();
    const vector<Belle2::TRGTOPSlotTiming> t0DecisionSlots = trgtopCombinedTimingArray[t0Index]->getSlotTimingDecisions();

    for (const auto& t0DecisionSlot : t0DecisionSlots) {

      int slot = t0DecisionSlot.getSlotId();
      int segment = t0DecisionSlot.getSlotSegment();
      int nHits = t0DecisionSlot.getSlotNHits();
      int logL = t0DecisionSlot.getSlotLogL();
      int t0 = t0DecisionSlot.getSlotTiming();

      if (slot >= 1 && slot <= 16) {
        if (segment >= 1 && segment <= 10) {
          h_topSlotSegment[slot - 1]->Fill(segment);
          h_topSlotNHits[slot - 1]->Fill(nHits);
          h_topSlotLogL[slot - 1]->Fill(logL);
          h_topSlotT0[slot - 1]->Fill(t0);

          h_topSlotVsSegment->Fill(segment, slot);
          h_topSlotVsNHits->Fill(nHits, slot);
          h_topSlotVsLogL->Fill(logL, slot);
          h_topSlotVsT0->Fill(t0, slot);
        }
      }
    }

    //    int topEventIdL1 = t0Decision.getEventIdL1();
    //    int topEventIdTOP = t0Decision.getEventIdTOP();
    //    int topWindowIdTOP = t0Decision.getWindowIdTOP();

    int topRvcB2L = t0Decision.getRvcB2L();
    int topRvcWindow = t0Decision.getRvcWindow();
    int topRvcTopTimingDecisionPrev = t0Decision.getRvcTopTimingDecisionPrev();
    int topRvcTopTimingDecisionNow = t0Decision.getRvcTopTimingDecisionNow();
    int topRvcTopTimingDecisionNowGdl = t0Decision.getRvcTopTimingDecisionNowGdl();

    int topCombinedTimingTop = t0Decision.getCombinedTimingTop();
    int topNSlotsCombinedTimingTop = t0Decision.getNSlotsCombinedTimingTop();
    //    int topCombinedTimingTopResidual = t0Decision.getCombinedTimingTopResidual();
    //    int topNErrorsMinor = t0Decision.getNErrorsMinor();
    //    int topNErrorsMajor = t0Decision.getNErrorsMajor();
    //    int topTrigType = t0Decision.getTrigType();
    int topNHitSum = t0Decision.getNHitSum();
    int topLogLSum = t0Decision.getLogLSum();
    //    int topLogLVar = t0Decision.getLogLVar();
    //    int topTimingVar = t0Decision.getTimingVar();

    float hitsPerSlot = topNHitSum / max(1, topNSlotsCombinedTimingTop);
    float logLPerSlot = topLogLSum / max(1, topNSlotsCombinedTimingTop);

    int topRvcDiff1 = topRvcTopTimingDecisionNow - topRvcTopTimingDecisionNowGdl;
    int topRvcDiff2 = topRvcTopTimingDecisionNow - topRvcTopTimingDecisionPrev;
    int topRvcDiff3 = topRvcTopTimingDecisionNow - topRvcWindow;
    int topRvcDiff4 = topRvcTopTimingDecisionNow - topRvcB2L;

    for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
      h_topCombinedTimingTop[skim[ifill]]->Fill(topCombinedTimingTop);
      h_topNSlotsCombinedTimingTop[skim[ifill]]->Fill(topNSlotsCombinedTimingTop);
      h_topNHitSum[skim[ifill]]->Fill((int) hitsPerSlot);
      h_topLogLSum[skim[ifill]]->Fill((int) logLPerSlot);
      h_topRvcDiff1[skim[ifill]]->Fill(topRvcDiff1);
      h_topRvcDiff2[skim[ifill]]->Fill(topRvcDiff2);
      h_topRvcDiff3[skim[ifill]]->Fill(topRvcDiff3);
      h_topRvcDiff4[skim[ifill]]->Fill(topRvcDiff4);
    }

    t0Index++;
    if (gdlInfoAvailable) {
      int top_top_timing = (topCombinedTimingTop % 10240);
      int top_gdl_timing_diff = gdl_top_timing - top_top_timing;
      if (top_gdl_timing_diff == 0) {
        nInTime++;
        for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
          h_top_gdl_timing_diff[skim[ifill]]->Fill(top_gdl_timing_diff + t0Index);
        }
      }
    }
  }

  // ALL TOP t0 decisions made within the last us before GDL L1 decision are out of time with GDL L1!
  if (gdlInfoAvailable && t0Index != 0 && nInTime == 0) {
    for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
      h_top_gdl_timing_diff[skim[ifill]]->Fill(0);
    }
  }

  // prepare histograms for GDL-TOP

  // prepare histograms for GRL-TOP

}


