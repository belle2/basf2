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
// Modeled after / heavily borrowing from GDL, GRL and ECL DQM modules
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

  addParam("doECLCorrelations", m_doECLCorrelations,
           "Do ECL-TOP correlations (true)",
           true);

  addParam("requireECLBarrel", m_requireEclBarrel,
           "Require the most energetic ECL TC in the barrel if doing correlations with ECL (false)",
           true);

  addParam("requireECLBarrelB2B", m_requireEclBarrelB2B,
           "Require ECL barrel back-to-back if doing correlations with ECL (false)",
           false);

  addParam("doGDLCorrelations", m_doGDLCorrelations,
           "Do GDL-TOP correlations (true)",
           true);

  addParam("doGRLCorrelations", m_doGRLCorrelations,
           "Do GRL-TOP correlations (true)",
           true);

  addParam("requireCDC2DTrack", m_requireCDC2DTrack,
           "Require at least one CDC 2D track at TRG level",
           false);

  addParam("generatePostscriptFile", m_generatePostscriptFile,
           "Generate postscript file (false)",
           false);

  addParam("postScriptFileName", m_postScriptFileName,
           "postscript file name",
           string("topl1dqm.ps"));

  addParam("skim", m_skim,
           "use skim information (-1)",
           int(-1));

  addParam("nHistClasses", m_nHistClassesActual,
           "The number of histogram classes: 1, 2, 3 (1); use 2 and 3 for detailed studies only",
           int(1));

  addParam("m_histLevel", m_histLevel,
           "Histogramming level: 1, 2, 3 (1); 3 most detailed",
           int(1));

}

void TRGTOPDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = gDirectory;
  oldDir->mkdir("TRGTOP");
  dirDQM->cd("TRGTOP");

  if (m_nHistClassesActual > nHistClasses) m_nHistClassesActual = nHistClasses;
  if (m_nHistClassesActual < 1) m_nHistClassesActual = 1;

  //-------------------------------------------------------------------------------------------------------------------------------------------
  //
  //  combined decisions information
  //

  h_topCombinedTimingTopAll  = new TH1I("h_t0_comb", "ALL TOP combined t0 decisions", 100, 0,
                                        100000);
  h_topCombinedTimingTopAll->GetXaxis()->SetTitle("TOPTRG combined t0 decision time (us)");

  h_topNSlotsCombinedTimingTopAll  = new TH1I("h_n_slots_comb",
                                              "ALL TOP combined t0 decisions: N slots", 17, 0, 17);
  h_topNSlotsCombinedTimingTopAll->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N slots");

  h_topNHitSumAll  = new TH1I("h_nhit_per_slot_comb",
                              "ALL TOP combined t0 decisions: N hits per slot", 200, 0, 200);
  h_topNHitSumAll->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N hits per slot");

  h_topLogLSumAll  = new TH1I("h_logl_per_slot_comb",
                              "ALL TOP combined t0 decisions: log L per slot", 100, 0, 60000);
  h_topLogLSumAll->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: log L per slot");

  h_topNSlotsCombinedTimingVsNHitsTopAll = new TH2I("h_n_slots_vs_nhit_total_comb",
                                                    "ALL TOP combined t0 decisions: N slots vs N hits (total)", 300, 0, 300, 17, 0, 17);
  h_topNSlotsCombinedTimingVsNHitsTopAll->GetXaxis()->SetTitle("N hits (total)");
  h_topNSlotsCombinedTimingVsNHitsTopAll->GetYaxis()->SetTitle("N slots");

  h_topTrigType  = new TH1I("h_trig_type",
                            "ALL TOP combined decisions: trig type", 10, 0, 10);
  h_topTrigType->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: trig type");

  h_topTimingResiduals  = new TH1I("h_timing_residuals",
                                   "ALL TOP combined decisions: timing residual", 100, 0, 100);
  h_topTimingResiduals->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: timing residuals");

  h_topTimingVariance  = new TH1I("h_timing_variance",
                                  "ALL TOP combined decisions: timing variance", 100, 0, 100);
  h_topTimingVariance->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: timing variance");

  //-------------------------------------------------------------------------------------------------------------------------------------------
  //
  //  slot-level information for combined decisions
  //

  h_topSlotAll  = new TH1I("h_slot_number", "slot", 16, 1, 17);
  h_topSlotAll->GetXaxis()->SetTitle("slot number in combined decisions (over 1us)");

  h_topSegmentAll  = new TH1I("h_segment_slot", "segment", 10, 1, 11);
  h_topSegmentAll->GetXaxis()->SetTitle("segment for each slot in combined decisions (over 1us)");

  h_topNHitsAll  = new TH1I("h_nhit_slot", "nhit", 200, 0, 200);
  h_topNHitsAll->GetXaxis()->SetTitle("N hits (for slots in combined decisions (over 1us))");

  h_topLogLAll  = new TH1I("h_logl_slot", "log L", 100, 0, 60000);
  h_topLogLAll->GetXaxis()->SetTitle("log L (for slots in combined decisions (over 1us))");

  h_topT0All  = new TH1I("h_t0_slot", "t0", 100, 0, 100000);
  h_topT0All->GetXaxis()->SetTitle("t0 for slots in combined decisions (over 1us)");

  //-------------------------------------------------------------------------------------------------------------------------------------------
  //
  //  correlations in slot-level information for combined decisions
  //

  h_topSlotVsSegment  = new TH2I("h_slot_vs_segment", "slot # vs slot segment", 10, 1, 11, 16, 1, 17);
  h_topSlotVsSegment->GetXaxis()->SetTitle("segment");
  h_topSlotVsSegment->GetYaxis()->SetTitle("slot");

  h_topSlotVsNHits  = new TH2I("h_slot_vs_nhit", "slot # vs slot nhit", 200, 0, 200, 16, 1, 17);
  h_topSlotVsNHits->GetXaxis()->SetTitle("N hits");
  h_topSlotVsNHits->GetYaxis()->SetTitle("slot");

  h_topSlotVsLogL  = new TH2I("h_slot_vs_logl", "slot # vs slot log L", 100, 0, 60000, 16, 1, 17);
  h_topSlotVsLogL->GetXaxis()->SetTitle("logL");
  h_topSlotVsLogL->GetYaxis()->SetTitle("slot");

  h_topSlotVsT0  = new TH2I("h_slot_vs_t0", "slot # vs slot t0", 100, 0, 100000, 16, 1, 17);
  h_topSlotVsT0->GetXaxis()->SetTitle("t0");
  h_topSlotVsT0->GetYaxis()->SetTitle("slot");

  h_topSegmentVsNHits  = new TH2I("h_segment_vs_nhit", "slot segment vs slot nhit", 200, 0, 200, 10, 1, 11);
  h_topSegmentVsNHits->GetXaxis()->SetTitle("N hits");
  h_topSegmentVsNHits->GetYaxis()->SetTitle("segment");

  h_topSegmentVsLogL  = new TH2I("h_segment_vs_logl", "slot segment vs slot log L", 100, 0, 60000, 10, 1, 11);
  h_topSegmentVsLogL->GetXaxis()->SetTitle("logL");
  h_topSegmentVsLogL->GetYaxis()->SetTitle("segment");

  h_topSegmentVsT0  = new TH2I("h_segment_vs_t0", "slot segment vs slot t0", 100, 0, 100000, 10, 1, 11);
  h_topSegmentVsT0->GetXaxis()->SetTitle("t0");
  h_topSegmentVsT0->GetYaxis()->SetTitle("segment");

  h_topNHitsVsLogL  = new TH2I("h_nhit_vs_logl", "slot nhit vs slot log L", 100, 0, 100000, 200, 0, 200);
  h_topNHitsVsLogL->GetXaxis()->SetTitle("logL");
  h_topNHitsVsLogL->GetYaxis()->SetTitle("N hits");

  h_topNHitsVsT0  = new TH2I("h_nhit_vs_t0", "slot nhit vs slot t0", 100, 0, 100000, 200, 0, 200);
  h_topNHitsVsT0->GetXaxis()->SetTitle("t0");
  h_topNHitsVsT0->GetYaxis()->SetTitle("N hits");

  h_topLogLVsT0  = new TH2I("h_logl_vs_t0", "slot log L vs slot t0", 100, 0, 100000, 100, 0, 60000);
  h_topLogLVsT0->GetXaxis()->SetTitle("t0");
  h_topLogLVsT0->GetYaxis()->SetTitle("logL");

  //-------------------------------------------------------------------------------------------------------------------------------------------
  //
  //  more slot-level information (per slot) for combined decisions
  //

  for (int slot = 0; slot <= 15; slot++) {

    h_topSlotSegment[slot]  = new TH1I(Form("h_segment_slot_%s", (to_string(slot + 1)).c_str()), "segment", 10, 1, 11);
    h_topSlotSegment[slot]->GetXaxis()->SetTitle("segment");

    h_topSlotNHits[slot]  = new TH1I(Form("h_nhit_slot_%s", (to_string(slot + 1)).c_str()), "nhit", 200, 0, 200);
    h_topSlotNHits[slot]->GetXaxis()->SetTitle("N hits");

    h_topSlotLogL[slot]  = new TH1I(Form("h_logl_slot_%s", (to_string(slot + 1)).c_str()), "log L", 100, 0, 60000);
    h_topSlotLogL[slot]->GetXaxis()->SetTitle("log L");

    h_topSlotT0[slot]  = new TH1I(Form("h_t0_slot_%s", (to_string(slot + 1)).c_str()), "t0", 100, 0, 100000);
    h_topSlotT0[slot]->GetXaxis()->SetTitle("t0");

  }

  //-------------------------------------------------------------------------------------------------------------------------------------------
  //
  //  GDL-TOP comparisons for slots in combined decisions
  //

  h_gdl_ecltop_timing_diff_vs_slot = new TH2I("h_gdl_slot_vs_ecltop_timing_diff", "slot vs GDL ECL-TOP slot t0 difference", 400, 1400,
                                              2200, 16, 1, 17);
  h_gdl_ecltop_timing_diff_vs_slot->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
  h_gdl_ecltop_timing_diff_vs_slot->GetYaxis()->SetTitle("slot");

  h_gdl_ecltop_timing_diff_vs_segment = new TH2I("h_gdl_segment_vs_ecltop_timing_diff", "segment vs GDL ECL-TOP slot t0 difference",
                                                 400, 1400, 2200, 10, 1, 11);
  h_gdl_ecltop_timing_diff_vs_segment->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
  h_gdl_ecltop_timing_diff_vs_segment->GetYaxis()->SetTitle("segment");

  h_gdl_ecltop_timing_diff_vs_nhits = new TH2I("h_gdl_nhits_vs_ecltop_timing_diff",
                                               "N hits (for slots) vs GDL ECL-TOP slot t0 difference", 400, 1400, 2200, 200, 0, 200);
  h_gdl_ecltop_timing_diff_vs_nhits->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
  h_gdl_ecltop_timing_diff_vs_nhits->GetYaxis()->SetTitle("N hits");

  h_gdl_ecltop_timing_diff_vs_logl = new TH2I("h_gdl_logl_vs_ecltop_timing_diff",
                                              "log L (for slots) vs GDL ECL-TOP slot t0 difference", 400, 1400, 2200, 100, 0, 60000);
  h_gdl_ecltop_timing_diff_vs_logl->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
  h_gdl_ecltop_timing_diff_vs_logl->GetYaxis()->SetTitle("log L");

  //-------------------------------------------------------------------------------------------------------------------------------------------

  h_ecl_gdl_top_timing_diff_both = new TH1I("h_ecl_gdl_top_timing_diff_both",  "", 400, 1400, 2200);
  h_ecl_gdl_top_timing_diff_both->GetXaxis()->SetTitle("Both ECL - GDL_TOP timing difference (2ns)");

  h_ecl_top_top_timing_diff_both = new TH1I("h_ecl_top_top_timing_diff_both",  "", 400, 1400, 2200);
  h_ecl_top_top_timing_diff_both->GetXaxis()->SetTitle("Both ECL - TOP_TOP timing difference (2ns)");

  //-------------------------------------------------------------------------------------------------------------------------------------------

  if (m_histLevel > 2) {

    //
    //  various rvc-related distributions for GDL/TOP
    //

    h_gdl_top_rvc_vs_top_timing = new TH2I("h_gdl_top_rvc_vs_top_timing",  "ALL GDL TOP rvc vs GDL TOP timing (7.8ns)",
                                           128, 0, 1280, 128, 0, 1280);
    h_gdl_top_rvc_vs_top_timing->GetXaxis()->SetTitle("TOP timing according to GDL (7.8ns)");
    h_gdl_top_rvc_vs_top_timing->GetYaxis()->SetTitle("GDL rvc((TOP decision (received))");

    h_gdl_ecltop_rvc_diff = new TH1I("h_gdl_ecltop_rvc_diff",  "ALL GDL ECL-TOP rvc difference",
                                     10, 0, 10);
    h_gdl_ecltop_rvc_diff->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP rvc difference (clks)");

    h_gdl_cdctop_rvc_diff = new TH1I("h_gdl_cdctop_rvc_diff",  "ALL GDL CDC-TOP rvc difference",
                                     10, 0, 10);
    h_gdl_cdctop_rvc_diff->GetXaxis()->SetTitle("TOPTRG GDL CDC-TOP rvc difference (clks)");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_gdl_gdltop_rvc_diff_all = new TH1I("h_gdl_gdltop_rvc_diff_all",  "ALL GDL GDL-TOP rvc difference",
                                         10, 1270, 1280);
    h_gdl_gdltop_rvc_diff_all->GetXaxis()->SetTitle("TOPTRG GDL GDL-TOP rvc difference (clks)");

    h_gdl_comtop_rvc_diff_all = new TH1I("h_gdl_comtop_rvc_diff_all",  "ALL GDL COM-TOP rvc difference",
                                         30, 20, 50);
    h_gdl_comtop_rvc_diff_all->GetXaxis()->SetTitle("TOPTRG GDL COM-TOP rvc difference (clks)");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_topRvcDiff1All  = new TH1I("h_top_rvc_diff_1_all",  "ALL rvc(posted to GDL)-rvc(TOP(this))",
                                 250, -100, 150);
    h_topRvcDiff1All->GetXaxis()->SetTitle("rvc(posted to GDL)-rvc(TOP(this))");

    h_topRvcDiff2All  = new TH1I("h_top_rvc_diff_2_all",  "ALL rvc(TOP(this))-rvc(TOP(prev))",
                                 128, 0, 1280);
    h_topRvcDiff2All->GetXaxis()->SetTitle("rvc(TOP(this))-rvc(TOP(prev))");

    h_topRvcDiff3All  = new TH1I("h_top_rvc_diff_3_all",  "ALL rvc(CB(window))-rvc(TOP(this))",
                                 150, 0, 150);
    h_topRvcDiff3All->GetXaxis()->SetTitle("rvc(CB(window))-rvc(TOP(this))");

    h_topRvcDiff4All  = new TH1I("h_top_rvc_diff_4_all",  "ALL rvc(received L1 from GDL)-rvc(TOP(this))",
                                 250, 0, 250);
    h_topRvcDiff4All->GetXaxis()->SetTitle("rvc(L1)-rvc(TOP(this))");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_topGdlRvcDiff1All  = new TH1I("h_gdl_top_l1_rvc_diff_all",  "ALL rvc(L1(GDL))-rvc(L1(as reported to TOP))",
                                    10, 1235, 1245);
    h_topGdlRvcDiff1All->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(L1(as reported to TOP))");

    h_topGdlRvcDiff2All  = new TH1I("h_gdl_l1_top_t0_this_rvc_diff_all",  "ALL rvc(TOP(received by GDL))-rvc(TOP(this))",
                                    50, 0, 200);
    h_topGdlRvcDiff2All->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(this))");

    h_topGdlRvcDiff3All  = new TH1I("h_gdl_l1_top_t0_prev_rvc_diff_all",  "ALL rvc(TOP(received by GDL))-rvc(TOP(prev))",
                                    128, 0, 1280);
    h_topGdlRvcDiff3All->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(prev))");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_decisionNumberVsNumberDecisionsGood = new TH2I("h_decision_vs_n_decisions_good", "Good decision number vs N of decisions", 5, 0,
                                                     5, 5, 0, 5);
    h_decisionNumberVsNumberDecisionsGood->GetXaxis()->SetTitle("N decisions");
    h_decisionNumberVsNumberDecisionsGood->GetYaxis()->SetTitle("Decision number");

    //------------------

    h_ecl_gdl_top_timing_diff_good = new TH1I("h_ecl_gdl_top_timing_diff_good",  "", 400, 1400, 2200);
    h_ecl_gdl_top_timing_diff_good->GetXaxis()->SetTitle("Good ECL - GDL_TOP timing difference (2ns)");

    h_ecl_top_top_timing_diff_good = new TH1I("h_ecl_top_top_timing_diff_good",  "", 400, 1400, 2200);
    h_ecl_top_top_timing_diff_good->GetXaxis()->SetTitle("Good ECL - TOP_TOP timing difference (2ns)");

    //------------------

    h_gdl_ecltop_timing_diff_vs_toptop_good = new TH2I("h_gdl_ecltop_timing_diff_vs_toptop_good",
                                                       "GOOD TOP-TOP vs (GDL ECL)-(GDL TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
    h_gdl_ecltop_timing_diff_vs_toptop_good->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) t0 difference (ns, 100ns bins)");
    h_gdl_ecltop_timing_diff_vs_toptop_good->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

    h_top_ecltop_timing_diff_vs_toptop_good = new TH2I("h_top_ecltop_timing_diff_vs_toptop_good",
                                                       "GOOD TOP-TOP vs (GDL ECL)-(TOP TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
    h_top_ecltop_timing_diff_vs_toptop_good->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");
    h_top_ecltop_timing_diff_vs_toptop_good->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

    h_gdl_ecltop_timing_diff_vs_ecltop_good = new TH2I("h_gdl_ecltop_timing_diff_vs_ecltop_good",
                                                       "GOOD (GDL ECL)-(TOP TOP) vs (GDL ECL)-(GDL TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
    h_gdl_ecltop_timing_diff_vs_ecltop_good->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) t0 difference (ns, 100ns bins)");
    h_gdl_ecltop_timing_diff_vs_ecltop_good->GetYaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_gdl_gdltop_rvc_diff_good = new TH1I("h_gdl_gdltop_rvc_diff_good",  "GOOD GDL GDL-TOP rvc difference",
                                          10, 1270, 1280);
    h_gdl_gdltop_rvc_diff_good->GetXaxis()->SetTitle("TOPTRG GDL GDL-TOP rvc difference (clks)");

    h_gdl_comtop_rvc_diff_good = new TH1I("h_gdl_comtop_rvc_diff_good",  "GOOD GDL COM-TOP rvc difference",
                                          30, 20, 50);
    h_gdl_comtop_rvc_diff_good->GetXaxis()->SetTitle("TOPTRG GDL COM-TOP rvc difference (clks)");

    //-------------

    h_topCombinedTimingTopGood  = new TH1I("h_t0_good", "GOOD TOP combined t0 decision", 100, 0,
                                           100000);
    h_topCombinedTimingTopGood->GetXaxis()->SetTitle("TOPTRG combined t0 decisions");

    h_topNSlotsCombinedTimingTopGood  = new TH1I("h_n_slots_good",
                                                 "GOOD TOP combined t0 decision: N slots", 17, 0, 17);
    h_topNSlotsCombinedTimingTopGood->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N slots");

    h_topNHitSumGood  = new TH1I("h_n_hit_per_slot_good",
                                 "GOOD TOP combined t0 decision: N hit per slot", 200, 0, 200);
    h_topNHitSumGood->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N hits per slot");

    h_topLogLSumGood  = new TH1I("h_logl_per_slot_good",
                                 "GOOD TOP combined t0 decision: log L per slot", 100, 0, 60000);
    h_topLogLSumGood->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: log L per slot");

    //-------------

    h_topRvcDiff1Good  = new TH1I("h_rvc_diff_1_good", "GOOD rvc(posted to GDL)-rvc(TOP(this))",
                                  250, -100, 150);
    h_topRvcDiff1Good->GetXaxis()->SetTitle("rvc(posted to GDL)-rvc(TOP(this))");

    h_topRvcDiff2Good  = new TH1I("h_rvc_diff_2_good", "GOOD rvc(TOP(this))-rvc(TOP(prev))",
                                  128, 0, 1280);
    h_topRvcDiff2Good->GetXaxis()->SetTitle("rvc(TOP(this))-rvc(TOP(prev))");

    h_topRvcDiff3Good  = new TH1I("h_rvc_diff_3_good",  "GOOD rvc(CB(window))-rvc(TOP(this))",
                                  150, 0, 150);
    h_topRvcDiff3Good->GetXaxis()->SetTitle("rvc(CB(window))-rvc(TOP(this))");

    h_topRvcDiff4Good  = new TH1I("h_rvc_diff_4_good",  "GOOD rvc(received L1 from GDL)-rvc(TOP(this))",
                                  250, 0, 250);
    h_topRvcDiff4Good->GetXaxis()->SetTitle("rvc(L1)-rvc(TOP(this))");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_topGdlRvcDiff1Good  = new TH1I("h_gdl_top_l1_rvc_diff_good",  "GOOD rvc(L1(GDL))-rvc(L1(as reported to TOP))",
                                     10, 1235, 1245);
    h_topGdlRvcDiff1Good->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(L1(as reported to TOP))");

    h_topGdlRvcDiff2Good  = new TH1I("h_gdl_l1_top_t0_this_rvc_diff_good",  "GOOD rvc(TOP(received by GDL))-rvc(TOP(this))",
                                     100, 0, 200);
    h_topGdlRvcDiff2Good->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(this))");

    h_topGdlRvcDiff3Good  = new TH1I("h_gdl_l1_top_t0_prev_rvc_diff_good",  "GOOD rvc(TOP(received by GDL))-rvc(TOP(prev))",
                                     128, 0, 1280);
    h_topGdlRvcDiff3Good->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(prev))");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_decisionNumberVsNumberDecisionsBad = new TH2I("h_decision_vs_n_decisions_bad", "Bad decision number vs N of decisions", 5, 0, 5,
                                                    5, 0, 5);
    h_decisionNumberVsNumberDecisionsBad->GetXaxis()->SetTitle("N decisions");
    h_decisionNumberVsNumberDecisionsBad->GetYaxis()->SetTitle("Decision number");

    //------------------

    h_ecl_gdl_top_timing_diff_bad = new TH1I("h_ecl_gdl_top_timing_diff_bad",  "", 400, 1400, 2200);
    h_ecl_gdl_top_timing_diff_bad->GetXaxis()->SetTitle("Bad ECL - GDL_TOP timing difference (2ns)");

    h_ecl_top_top_timing_diff_bad = new TH1I("h_ecl_top_top_timing_diff_bad",  "", 400, 1400, 2200);
    h_ecl_top_top_timing_diff_bad->GetXaxis()->SetTitle("Bad ECL - TOP_TOP timing difference (2ns)");

    //------------------

    h_gdl_ecltop_timing_diff_vs_toptop_bad = new TH2I("h_gdl_ecltop_timing_diff_vs_toptop_bad",
                                                      "BAD TOP-TOP vs (GDL ECL)-(GDL TOP) t0 differences", 110, -760, 10240, 110, -760, 10240);
    h_gdl_ecltop_timing_diff_vs_toptop_bad->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) (combined) t0 difference (ns, 100ns bins)");
    h_gdl_ecltop_timing_diff_vs_toptop_bad->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

    h_top_ecltop_timing_diff_vs_toptop_bad = new TH2I("h_top_ecltop_timing_diff_vs_toptop_bad",
                                                      "BAD TOP-TOP vs (GDL ECL)-(TOP TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
    h_top_ecltop_timing_diff_vs_toptop_bad->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");
    h_top_ecltop_timing_diff_vs_toptop_bad->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

    h_gdl_ecltop_timing_diff_vs_ecltop_bad = new TH2I("h_gdl_ecltop_timing_diff_vs_ecltop_bad",
                                                      "BAD (GDL ECL)-(TOP TOP) vs (GDL ECL)-(GDL TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
    h_gdl_ecltop_timing_diff_vs_ecltop_bad->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) t0 difference (ns, 100ns bins)");
    h_gdl_ecltop_timing_diff_vs_ecltop_bad->GetYaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_gdl_gdltop_rvc_diff_bad = new TH1I("h_gdl_gdltop_rvc_diff_bad",  "BAD GDL GDL-TOP rvc difference",
                                         10, 1270, 1280);
    h_gdl_gdltop_rvc_diff_bad->GetXaxis()->SetTitle("TOPTRG GDL GDL-TOP rvc difference (clks)");

    h_gdl_comtop_rvc_diff_bad = new TH1I("h_gdl_comtop_rvc_diff_bad",  "BAD GDL COM-TOP rvc difference",
                                         30, 20, 50);
    h_gdl_comtop_rvc_diff_bad->GetXaxis()->SetTitle("TOPTRG GDL COM-TOP rvc difference (clks)");

    //-------------

    h_topCombinedTimingTopBad  = new TH1I("h_t0_bad", "BAD TOP combined t0 decision", 100, 0,
                                          100000);
    h_topCombinedTimingTopBad->GetXaxis()->SetTitle("TOPTRG combined t0 decisions");

    h_topNSlotsCombinedTimingTopBad  = new TH1I("h_N_slots_bad",
                                                "BAD TOP combined t0 decision: N slots", 17, 0, 17);
    h_topNSlotsCombinedTimingTopBad->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N slots");

    h_topNHitSumBad  = new TH1I("h_n_hit_per_slot_bad",
                                "BAD TOP combined t0 decision: N hit per slot", 200, 0, 200);
    h_topNHitSumBad->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N hits per slot");

    h_topLogLSumBad  = new TH1I("h_logl_per_slot_bad",
                                "BAD TOP combined t0 decision: log L per slot", 100, 0, 60000);
    h_topLogLSumBad->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: log L per slot");

    //-------------

    h_topRvcDiff1Bad  = new TH1I("h_rvc_diff_1_bad", "BAD rvc(posted to GDL)-rvc(TOP(this))",
                                 250, -100, 150);
    h_topRvcDiff1Bad->GetXaxis()->SetTitle("rvc(posted to GDL)-rvc(TOP(this))");

    h_topRvcDiff2Bad  = new TH1I("h_rvc_diff_2_bad", "BAD rvc(TOP(this))-rvc(TOP(prev))",
                                 128, 0, 1280);
    h_topRvcDiff2Bad->GetXaxis()->SetTitle("rvc(TOP(this))-rvc(TOP(prev))");

    h_topRvcDiff3Bad  = new TH1I("h_rvc_diff_3_bad",  "BAD rvc(CB(window))-rvc(TOP(this))",
                                 150, 0, 150);
    h_topRvcDiff3Bad->GetXaxis()->SetTitle("rvc(CB(window))-rvc(TOP(this))");

    h_topRvcDiff4Bad  = new TH1I("h_rvc_diff_4_bad",  "BAD rvc(received L1 from GDL)-rvc(TOP(this))",
                                 250, 0, 250);
    h_topRvcDiff4Bad->GetXaxis()->SetTitle("rvc(L1)-rvc(TOP(this))");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_topGdlRvcDiff1Bad  = new TH1I("h_gdl_top_l1_rvc_diff_bad",  "BAD rvc(L1(GDL))-rvc(L1(as reported to TOP))",
                                    10, 1235, 1245);
    h_topGdlRvcDiff1Bad->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(L1(as reported to TOP))");

    h_topGdlRvcDiff2Bad  = new TH1I("h_gdl_l1_top_t0_this_rvc_diff_bad",  "BAD rvc(TOP(received by GDL))-rvc(TOP(this))",
                                    100, 0, 200);
    h_topGdlRvcDiff2Bad->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(this))");

    h_topGdlRvcDiff3Bad  = new TH1I("h_gdl_l1_top_t0_prev_rvc_diff_bad",  "BAD rvc(TOP(received by GDL))-rvc(TOP(prev))",
                                    128, 0, 1280);
    h_topGdlRvcDiff3Bad->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(prev))");

  }
  //-------------------------------------------------------------------------------------------------------------------------------------------

  for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {

    h_top_nt0decisions_vs_hist_class[iskim] = new TH2I(Form("h_top_nt0decisions_vs_hist_class_%s",  skim_smap[iskim].c_str()),
                                                       "N t0 decisions vs event class according to TOP info matching between GDL and TOP",
                                                       3, 0, 3, 5, 1, 6);
    h_top_nt0decisions_vs_hist_class[iskim]->GetXaxis()->SetTitle("TOP info match (GDL vs TOP): 0/1/2 : first decision match/no match/match in some other decision");
    h_top_nt0decisions_vs_hist_class[iskim]->GetYaxis()->SetTitle("Number of t0 decisions");

    h_top_gdl_match_decision_number[iskim]  = new TH1I(Form("h_top_gdl_match_decision_number_%s",  skim_smap[iskim].c_str()),
                                                       "Decision number where TOP info is the same at TOP and GDL",
                                                       6, 0, 6);
    h_top_gdl_match_decision_number[iskim]->GetXaxis()->SetTitle("TOPTRG combined decision # where TOP info is the same at TOP and GDL");

    h_gdl_ecltop_timing_diff_5ns[iskim] = new TH1I(Form("h_gdl_ecltop_timing_diff_5ns_%s", skim_smap[iskim].c_str()),
                                                   "GDL ECL-TOP t0 difference", 248, 0, 10240);
    h_gdl_ecltop_timing_diff_5ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference (5ns)");

    h_gdl_cdctop_timing_diff_5ns[iskim] = new TH1I(Form("h_gdl_cdctop_timing_diff_5ns_%s", skim_smap[iskim].c_str()),
                                                   "GDL CDC-TOP t0 difference", 248, 0, 10240);
    h_gdl_cdctop_timing_diff_5ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL CDC-TOP t0 difference (5ns)");

    //--- resolution plots ---

    h_gdl_ecltop_timing_diff_2ns[iskim] = new TH1I(Form("h_gdl_ecltop_timing_diff_2ns_%s", skim_smap[iskim].c_str()),
                                                   "GDL ECL-TOP t0 difference", 400, 1400, 2200);
    h_gdl_ecltop_timing_diff_2ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference (2ns)");

    h_gdl_ecltop_timing_diff_grl_matched_2ns[iskim] = new TH1I(Form("h_gdl_ecltop_timing_diff_grl_matched_2ns_%s",
                                                               skim_smap[iskim].c_str()),
                                                               "GDL ECL-TOP t0 difference", 400, 1400, 2200);
    h_gdl_ecltop_timing_diff_grl_matched_2ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference when TOP and GRL match (2ns)");

    h_gdl_ecltop_timing_diff_no_grl_matched_2ns[iskim] = new TH1I(Form("h_gdl_ecltop_timing_diff_no_grl_matched_2ns_%s",
        skim_smap[iskim].c_str()),
        "GDL ECL-TOP t0 difference", 400, 1400, 2200);
    h_gdl_ecltop_timing_diff_no_grl_matched_2ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference when TOP and GRL do not match (2ns)");

    h_gdl_ecltop_timing_diff_no_grl_at_all_2ns[iskim] = new TH1I(Form("h_gdl_ecltop_timing_diff_no_grl_at_all_2ns_%s",
        skim_smap[iskim].c_str()),
        "GDL ECL-TOP t0 difference", 400, 1400, 2200);
    h_gdl_ecltop_timing_diff_no_grl_at_all_2ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference when GRL has no 2D tracks (2ns)");

    h_gdl_ecltop_timing_diff_vs_nslots_2ns[iskim] = new TH2I(Form("h_gdl_ecltop_timing_diff_vs_nslots_2ns_%s",
                                                             skim_smap[iskim].c_str()),
                                                             "GDL N slots vs ECL-TOP t0 difference", 400, 1400, 2200, 16, 1, 17);
    h_gdl_ecltop_timing_diff_vs_nslots_2ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference (2ns)");
    h_gdl_ecltop_timing_diff_vs_nslots_2ns[iskim]->GetYaxis()->SetTitle("N slots in combined decision");

    h_top_ecltop_timing_diff_combined_2ns[iskim] = new TH1I(Form("h_gdl_ecltop_timing_diff_combined_2ns_%s", skim_smap[iskim].c_str()),
                                                            "GDL ECL-TOP TOP t0 difference", 400, 1400, 2200);
    h_top_ecltop_timing_diff_combined_2ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference (2ns)");

    h_top_ecltop_timing_diff_best_slot_2ns[iskim] = new TH1I(Form("h_gdl_ecltop_timing_diff_best_slot_2ns_%s",
                                                             skim_smap[iskim].c_str()),
                                                             "GDL ECL-TOP TOP t0 difference using slot with max N hits", 400, 1400, 2200);
    h_top_ecltop_timing_diff_best_slot_2ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP t0 difference (2ns)");

    h_gdl_cdctop_timing_diff_2ns[iskim] = new TH1I(Form("h_gdl_cdctop_timing_diff_2ns_%s", skim_smap[iskim].c_str()),
                                                   "GDL CDC-TOP t0 difference", 250, 0, 500);
    h_gdl_cdctop_timing_diff_2ns[iskim]->GetXaxis()->SetTitle("TOPTRG GDL CDC-TOP t0 difference (2ns)");

    //------------------------

  }

  if (m_histLevel > 1) {

    // these two for loops are nested in this particular way to simplify browsing through the histograms

    for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {

      for (int histClass = 0; histClass < m_nHistClassesActual; histClass++) {

        // max N of combined t0 decisions (over past ~us) = N windows in circular buffer
        h_N_decision[iskim][histClass]  = new TH1I(Form("h_N_decisions_%d_%s", histClass, skim_smap[iskim].c_str()),
                                                   "N t0 decisions in 1 us before GDL L1", 5, 1,
                                                   6);
        h_N_decision[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG N combined t0 decisions");

        //-------------

        h_topNHitVsNhit[iskim][histClass] = new TH2I(Form("h_nhit_vs_nhit_first_decision_%d_%s", histClass, skim_smap[iskim].c_str()),
                                                     "First decision: N hits vs N hits", 20, 0, 100, 20, 0, 100);
        h_topNHitVsNhit[iskim][histClass]->GetXaxis()->SetTitle("N hits for slot with largest N hits");
        h_topNHitVsNhit[iskim][histClass]->GetYaxis()->SetTitle("N hits for slot with next to the largest N hits");

        h_topSlotVsSlot[iskim][histClass] = new TH2I(Form("h_slot_vs_slot_first_decision_%d_%s", histClass, skim_smap[iskim].c_str()),
                                                     "First decision: slot # vs slot #", 16, 1, 17, 16, 1, 17);
        h_topSlotVsSlot[iskim][histClass]->GetXaxis()->SetTitle("Slot # for slot with largest N hits");
        h_topSlotVsSlot[iskim][histClass]->GetYaxis()->SetTitle("Slot # for slot with next to the largest N hits");

        h_topT0VsT0[iskim][histClass] = new TH2I(Form("h_t0_vs_t0_first_decision_%d_%s", histClass, skim_smap[iskim].c_str()),
                                                 "First decision: T0 vs T0", 100, 0, 100000, 100, 0, 100000);
        h_topT0VsT0[iskim][histClass]->GetXaxis()->SetTitle("T0 for slot with largest N hits");
        h_topT0VsT0[iskim][histClass]->GetYaxis()->SetTitle("T0 for slot with next to the largest N hits");

        h_topSegmentVsSegment[iskim][histClass] = new TH2I(Form("h_segment_vs_segment_first_decision_%d_%s", histClass,
                                                                skim_smap[iskim].c_str()),
                                                           "First decision: segment # vs segment #", 10, 1, 11, 10, 1, 11);
        h_topSegmentVsSegment[iskim][histClass]->GetXaxis()->SetTitle("Segment # for slot with largest N hits");
        h_topSegmentVsSegment[iskim][histClass]->GetYaxis()->SetTitle("Segment # for slot with next to the largest N hits");

        h_topLogLVsLogL[iskim][histClass] = new TH2I(Form("h_logl_vs_logl_first_decision_%d_%s", histClass, skim_smap[iskim].c_str()),
                                                     "First decision: log L vs log L", 100, 0, 60000, 100, 0, 60000);
        h_topLogLVsLogL[iskim][histClass]->GetXaxis()->SetTitle("log L for slot with largest N hits");
        h_topLogLVsLogL[iskim][histClass]->GetYaxis()->SetTitle("log L for slot with next to the largest N hits");

        //-------------

        h_topCombinedTimingTop[iskim][histClass]  = new TH1I(Form("h_t0_%d_%s",  histClass, skim_smap[iskim].c_str()),
                                                             "TOP combined t0 decision", 100, 0,
                                                             100000);
        h_topCombinedTimingTop[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions");

        h_topNSlotsCombinedTimingTop[iskim][histClass]  = new TH1I(Form("h_n_slots_%d_%s",  histClass, skim_smap[iskim].c_str()),
                                                                   "TOP combined t0 decision: N slots", 17, 0, 17);
        h_topNSlotsCombinedTimingTop[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N slots");

        h_topNHitSum[iskim][histClass]  = new TH1I(Form("h_n_hit_per_slot_%d_%s",  histClass, skim_smap[iskim].c_str()),
                                                   "TOP combined t0 decision: N hits per slot", 200, 0, 200);
        h_topNHitSum[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N hits per slot");

        h_topNHitBestSlot[iskim][histClass]  = new TH1I(Form("h_n_hit_best_slot_%d_%s",  histClass, skim_smap[iskim].c_str()),
                                                        "TOP combined t0 decision: N hits best slot", 200, 0, 200);
        h_topNHitBestSlot[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N hits best slot");

        //        h_topT0DecisionNumberBestSlot[iskim][histClass]  = new TH1I(Form("h_t0_decision_number_best_slot_%d_%s",  histClass,
        //                                                                    skim_smap[iskim].c_str()),
        //                                                                    "TOP combined t0 decision: combined t0 decision number best slot", 5, 1, 6);
        //        h_topT0DecisionNumberBestSlot[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: combined t0 decision number best slot");

        h_topLogLSum[iskim][histClass]  = new TH1I(Form("h_logl_per_slot_%d_%s",  histClass, skim_smap[iskim].c_str()),
                                                   "TOP combined t0 decision: log L per slot", 100, 0, 60000);
        h_topLogLSum[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: log L per slot");

        //-------------------------------------------------------------------------------------------------------------------------------------------

        h_gdl_ecltop_timing_diff_vs_slot_2ns[iskim][histClass] = new TH2I(Form("h_gdl_slot_vs_ecltop_timing_diff_%d_%s", histClass,
            skim_smap[iskim].c_str()), "slot vs GDL ECL-TOP slot t0 difference", 400, 1400, 2200, 16, 1, 17);
        h_gdl_ecltop_timing_diff_vs_slot_2ns[iskim][histClass]->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
        h_gdl_ecltop_timing_diff_vs_slot_2ns[iskim][histClass]->GetYaxis()->SetTitle("slot");

        h_gdl_ecltop_timing_diff_vs_segment_2ns[iskim][histClass] = new TH2I(Form("h_gdl_segment_vs_ecltop_timing_diff_%d_%s", histClass,
            skim_smap[iskim].c_str()), "segment vs GDL ECL-TOP slot t0 difference", 400, 1400, 2200, 10, 1, 11);
        h_gdl_ecltop_timing_diff_vs_segment_2ns[iskim][histClass]->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
        h_gdl_ecltop_timing_diff_vs_segment_2ns[iskim][histClass]->GetYaxis()->SetTitle("segment");

        h_gdl_ecltop_timing_diff_vs_nhits_2ns[iskim][histClass] = new TH2I(Form("h_gdl_nhits_vs_ecltop_timing_diff_%d_%s", histClass,
            skim_smap[iskim].c_str()), "N hits (for slots) vs GDL ECL-TOP slot t0 difference", 400, 1400, 2200, 200, 0, 200);
        h_gdl_ecltop_timing_diff_vs_nhits_2ns[iskim][histClass]->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
        h_gdl_ecltop_timing_diff_vs_nhits_2ns[iskim][histClass]->GetYaxis()->SetTitle("N hits");

        h_gdl_ecltop_timing_diff_vs_logl_2ns[iskim][histClass] = new TH2I(Form("h_gdl_logl_vs_ecltop_timing_diff_%d_%s", histClass,
            skim_smap[iskim].c_str()), "log L (for slots) vs GDL ECL-TOP slot t0 difference", 400, 1400, 2200, 100, 0, 60000);
        h_gdl_ecltop_timing_diff_vs_logl_2ns[iskim][histClass]->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
        h_gdl_ecltop_timing_diff_vs_logl_2ns[iskim][histClass]->GetYaxis()->SetTitle("log L");

        //-------------

        h_top_ecltop_timing_combined_diff_2ns[iskim][histClass] = new TH1I(Form("h_top_ecltop_timing_combined_diff_2ns_%d_%s",  histClass,
            skim_smap[iskim].c_str()),
            "(GDL ECL)-(TOP TOP) t0 difference (combined decision)", 400, 1400, 2200);
        h_top_ecltop_timing_combined_diff_2ns[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (2ns)");

        h_top_ecltop_timing_best_slot_diff_2ns[iskim][histClass] = new TH1I(Form("h_top_ecltop_timing_best_slot_diff_2ns_%d_%s",  histClass,
            skim_smap[iskim].c_str()),
            "(GDL ECL)-(TOP TOP) t0 difference (best slot decision)", 400, 1400, 2200);
        h_top_ecltop_timing_best_slot_diff_2ns[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (2ns)");

        h_gdl_ecltop_timing_diff_vs_toptop[iskim][histClass] = new TH2I(Form("h_gdl_ecltop_timing_diff_vs_toptop_%d_%s",  histClass,
            skim_smap[iskim].c_str()),
            "TOP-TOP vs (GDL ECL)-(GDL TOP) t0 differences", 110, -760, 10240, 110, -760, 10240);
        h_gdl_ecltop_timing_diff_vs_toptop[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) (combined) t0 difference (ns, 100ns bins)");
        h_gdl_ecltop_timing_diff_vs_toptop[iskim][histClass]->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

        h_top_ecltop_timing_diff_vs_toptop[iskim][histClass] = new TH2I(Form("h_top_ecltop_timing_diff_vs_toptop_%d_%s",  histClass,
            skim_smap[iskim].c_str()),
            "TOP-TOP vs (GDL ECL)-(TOP TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
        h_top_ecltop_timing_diff_vs_toptop[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");
        h_top_ecltop_timing_diff_vs_toptop[iskim][histClass]->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

        h_gdl_ecltop_timing_diff_vs_ecltop[iskim][histClass] = new TH2I(Form("h_gdl_ecltop_timing_diff_vs_ecltop_%d_%s",  histClass,
            skim_smap[iskim].c_str()),
            "(GDL ECL)-(TOP TOP) vs (GDL ECL)-(GDL TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
        h_gdl_ecltop_timing_diff_vs_ecltop[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) t0 difference (ns, 100ns bins)");
        h_gdl_ecltop_timing_diff_vs_ecltop[iskim][histClass]->GetYaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");

        h_gdl_ecltop_timing_diff_vs_ecltop_best_slot[iskim][histClass] = new TH2I(Form("h_gdl_ecltop_timing_diff_vs_ecltop_best_slot_%d_%s",
            histClass, skim_smap[iskim].c_str()),
            "(GDL ECL)-(TOP TOP) vs (GDL ECL)-(GDL TOP) (best slot) t0 differences", 110, -760, 10240, 110, -760, 10240);
        h_gdl_ecltop_timing_diff_vs_ecltop_best_slot[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP() best slot) t0 difference (ns, 100ns bins)");
        h_gdl_ecltop_timing_diff_vs_ecltop_best_slot[iskim][histClass]->GetYaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");

        h_top_ecltop_timing_diff_2ns[iskim][histClass] = new TH1I(Form("h_top_ecltop_timing_diff_2ns_%d_%s", histClass,
                                                                  skim_smap[iskim].c_str()),
                                                                  "(GDL ECL)-(TOP TOP) t0 difference (all decisions)", 400, 1400, 2200);
        h_top_ecltop_timing_diff_2ns[iskim][histClass]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (2ns)");

      }
    }
  }

  //-----

  h_topTC2IdVsTC1IdAll = new TH2I("h_top_tc2id_vs_tc1id_all", "Sorted (by energy) TC2 Id vs TC1 Id", 100, 0, 600, 100, 0, 600);
  h_topTC2IdVsTC1IdAll->GetXaxis()->SetTitle("Most energetic TC id");
  h_topTC2IdVsTC1IdAll->GetYaxis()->SetTitle("Next to most energetic TC id");

  h_topTC2EnergyVsTC1EnergyAll = new TH2I("h_top_tc2en_vs_tc1en_all", "Sorted TC2 energy vs TC1 energy", 100, 0, 1000, 100, 0, 1000);
  h_topTC2EnergyVsTC1EnergyAll->GetXaxis()->SetTitle("Most energetic TC energy");
  h_topTC2EnergyVsTC1EnergyAll->GetYaxis()->SetTitle("Next to most energetic TC energy");

  h_topTC2ThetaIdVsTC1ThetaIdAll = new TH2I("h_top_tc2thetaid_vs_tc1thetaid_all", "Sorted by energy TC2 vs TC1 theta ids", 20, 0, 20,
                                            20, 0, 20);
  h_topTC2ThetaIdVsTC1ThetaIdAll->GetXaxis()->SetTitle("Most energetic TC theta id");
  h_topTC2ThetaIdVsTC1ThetaIdAll->GetYaxis()->SetTitle("Next to most energetic TC theta id");

  h_topTC2PhiIdVsTC1PhiIdAll = new TH2I("h_top_tc2phiid_vs_tc1phiid_all", "Sorted by energy TC2 vs TC1 phi ids", 36, 1, 37, 36, 1,
                                        37);
  h_topTC2PhiIdVsTC1PhiIdAll->GetXaxis()->SetTitle("Most energetic TC phi id");
  h_topTC2PhiIdVsTC1PhiIdAll->GetYaxis()->SetTitle("Next to most energetic TC phi id");

  h_topTCPhiIdVsTCThetaIdAll = new TH3I("h_top_tcphiid_vs_tcthetaid_all", "Sorted by energy TC2 vs TC1 theta vs phi ids", 20, 0, 20,
                                        36, 1, 37, 2, 0, 2);
  h_topTCPhiIdVsTCThetaIdAll->GetXaxis()->SetTitle("Most and next to most energetic TCs theta ids");
  h_topTCPhiIdVsTCThetaIdAll->GetYaxis()->SetTitle("Most and next to most energetic TCs phi ids");
  h_topTCPhiIdVsTCThetaIdAll->GetZaxis()->SetTitle("Most and next to most energetic TCs");

  //-----

  h_topTC2IdVsTC1IdGRLAll = new TH2I("h_top_tc2id_vs_tc1id_grl_all", "Sorted (by energy) TC2 Id vs TC1 Id", 100, 0, 600, 100, 0, 600);
  h_topTC2IdVsTC1IdGRLAll->GetXaxis()->SetTitle("Most energetic TC id");
  h_topTC2IdVsTC1IdGRLAll->GetYaxis()->SetTitle("Next to most energetic TC id");

  h_topTC2EnergyVsTC1EnergyGRLAll = new TH2I("h_top_tc2en_vs_tc1en_grl_all", "Sorted TC2 energy vs TC1 energy", 100, 0, 1000, 100, 0,
                                             1000);
  h_topTC2EnergyVsTC1EnergyGRLAll->GetXaxis()->SetTitle("Most energetic TC energy");
  h_topTC2EnergyVsTC1EnergyGRLAll->GetYaxis()->SetTitle("Next to most energetic TC energy");

  h_topTC2ThetaIdVsTC1ThetaIdGRLAll = new TH2I("h_top_tc2thetaid_vs_tc1thetaid_grl_all", "Sorted by energy TC2 vs TC1 theta ids", 20,
                                               0, 20, 20, 0, 20);
  h_topTC2ThetaIdVsTC1ThetaIdGRLAll->GetXaxis()->SetTitle("Most energetic TC theta id");
  h_topTC2ThetaIdVsTC1ThetaIdGRLAll->GetYaxis()->SetTitle("Next to most energetic TC theta id");

  h_topTC2PhiIdVsTC1PhiIdGRLAll = new TH2I("h_top_tc2phiid_vs_tc1phiid_grl_all", "Sorted by energy TC2 vs TC1 phi ids", 36, 1, 37, 36,
                                           1, 37);
  h_topTC2PhiIdVsTC1PhiIdGRLAll->GetXaxis()->SetTitle("Most energetic TC phi id");
  h_topTC2PhiIdVsTC1PhiIdGRLAll->GetYaxis()->SetTitle("Next to most energetic TC phi id");

  h_topTCPhiIdVsTCThetaIdGRLAll = new TH3I("h_top_tcphiid_vs_tcthetaid_grl_all", "Sorted by energy TC2 vs TC1 theta vs phi ids", 20,
                                           0, 20, 36, 1, 37, 2, 0, 2);
  h_topTCPhiIdVsTCThetaIdGRLAll->GetXaxis()->SetTitle("Most and next to most energetic TCs theta ids");
  h_topTCPhiIdVsTCThetaIdGRLAll->GetYaxis()->SetTitle("Most and next to most energetic TCs phi ids");
  h_topTCPhiIdVsTCThetaIdGRLAll->GetZaxis()->SetTitle("Most and next to most energetic TCs");

  //-----

  h_topTC2IdVsTC1Id = new TH2I("h_top_tc2id_vs_tc1id", "Sorted (by energy) TC2 Id vs TC1 Id", 100, 0, 600, 100, 0, 600);
  h_topTC2IdVsTC1Id->GetXaxis()->SetTitle("Most energetic TC id");
  h_topTC2IdVsTC1Id->GetYaxis()->SetTitle("Next to most energetic TC id");

  h_topTC2EnergyVsTC1Energy = new TH2I("h_top_tc2en_vs_tc1en", "Sorted TC2 energy vs TC1 energy", 100, 0, 1000, 100, 0, 1000);
  h_topTC2EnergyVsTC1Energy->GetXaxis()->SetTitle("Most energetic TC energy");
  h_topTC2EnergyVsTC1Energy->GetYaxis()->SetTitle("Next to most energetic TC energy");

  h_topTC2ThetaIdVsTC1ThetaId = new TH2I("h_top_tc2thetaid_vs_tc1thetaid", "Sorted by energy TC2 vs TC1 theta ids", 20, 0, 20, 20, 0,
                                         20);
  h_topTC2ThetaIdVsTC1ThetaId->GetXaxis()->SetTitle("Most energetic TC theta id");
  h_topTC2ThetaIdVsTC1ThetaId->GetYaxis()->SetTitle("Next to most energetic TC theta id");

  h_topTC2PhiIdVsTC1PhiId = new TH2I("h_top_tc2phiid_vs_tc1phiid", "Sorted by energy TC2 vs TC1 phi ids", 36, 1, 37, 36, 1, 37);
  h_topTC2PhiIdVsTC1PhiId->GetXaxis()->SetTitle("Most energetic TC phi id");
  h_topTC2PhiIdVsTC1PhiId->GetYaxis()->SetTitle("Next to most energetic TC phi id");

  h_topTCPhiIdVsTCThetaId = new TH3I("h_top_tcphiid_vs_tcthetaid", "Sorted by energy TC2 vs TC1 theta vs phi ids", 20, 0, 20, 36, 1,
                                     37, 2, 0, 2);
  h_topTCPhiIdVsTCThetaId->GetXaxis()->SetTitle("Most and next to most energetic TCs theta ids");
  h_topTCPhiIdVsTCThetaId->GetYaxis()->SetTitle("Most and next to most energetic TCs phi ids");
  h_topTCPhiIdVsTCThetaId->GetZaxis()->SetTitle("Most and next to most energetic TCs");

  //-----

  h_topTC2IdVsTC1IdGRL = new TH2I("h_top_tc2id_vs_tc1id_grl", "Sorted (by energy) TC2 Id vs TC1 Id", 100, 0, 600, 100, 0, 600);
  h_topTC2IdVsTC1IdGRL->GetXaxis()->SetTitle("Most energetic TC id");
  h_topTC2IdVsTC1IdGRL->GetYaxis()->SetTitle("Next to most energetic TC id");

  h_topTC2EnergyVsTC1EnergyGRL = new TH2I("h_top_tc2en_vs_tc1en_grl", "Sorted TC2 energy vs TC1 energy", 100, 0, 1000, 100, 0, 1000);
  h_topTC2EnergyVsTC1EnergyGRL->GetXaxis()->SetTitle("Most energetic TC energy");
  h_topTC2EnergyVsTC1EnergyGRL->GetYaxis()->SetTitle("Next to most energetic TC energy");

  h_topTC2ThetaIdVsTC1ThetaIdGRL = new TH2I("h_top_tc2thetaid_vs_tc1thetaid_grl", "Sorted by energy TC2 vs TC1 theta ids", 20, 0, 20,
                                            20, 0, 20);
  h_topTC2ThetaIdVsTC1ThetaIdGRL->GetXaxis()->SetTitle("Most energetic TC theta id");
  h_topTC2ThetaIdVsTC1ThetaIdGRL->GetYaxis()->SetTitle("Next to most energetic TC theta id");

  h_topTC2PhiIdVsTC1PhiIdGRL = new TH2I("h_top_tc2phiid_vs_tc1phiid_grl", "Sorted by energy TC2 vs TC1 phi ids", 36, 1, 37, 36, 1,
                                        37);
  h_topTC2PhiIdVsTC1PhiIdGRL->GetXaxis()->SetTitle("Most energetic TC phi id");
  h_topTC2PhiIdVsTC1PhiIdGRL->GetYaxis()->SetTitle("Next to most energetic TC phi id");

  h_topTCPhiIdVsTCThetaIdGRL = new TH3I("h_top_tcphiid_vs_tcthetaid_grl", "Sorted by energy TC2 vs TC1 theta vs phi ids", 20, 0, 20,
                                        36, 1, 37, 2, 0, 2);
  h_topTCPhiIdVsTCThetaIdGRL->GetXaxis()->SetTitle("Most and next to most energetic TCs theta ids");
  h_topTCPhiIdVsTCThetaIdGRL->GetYaxis()->SetTitle("Most and next to most energetic TCs phi ids");
  h_topTCPhiIdVsTCThetaIdGRL->GetZaxis()->SetTitle("Most and next to most energetic TCs");

  //-------------------------------------------------------------------------------------------------------------------------------------------

  h_gdl_ecltop_timing_diff_vs_grl_top_l1 = new TH2I("h_gdl_ecltop_timing_diff_vs_grl_top_l1",
                                                    "(GDL ECL)-(GDL TOP) vs GRL TOP rvc (relative to GDL L1)", 100, -2000, 0, 110, -760, 10240);
  h_gdl_ecltop_timing_diff_vs_grl_top_l1->GetXaxis()->SetTitle("GRL TOP rvc relative to GDL L1 (rvc)");
  h_gdl_ecltop_timing_diff_vs_grl_top_l1->GetYaxis()->SetTitle("(GDL ECL)-(GDL TOP) t0 difference (ns, 100ns bins)");

  h_gdl_top_l1_vs_grl_top_l1 = new TH2I("h_gdl_top_l1_vs_grl_top_l1", "GDL TOP rvc vs GRL TOP rvc (relative to GDL L1)", 100, -2000,
                                        0, 128, 0, 1280);
  h_gdl_top_l1_vs_grl_top_l1->GetXaxis()->SetTitle("GRL TOP rvc relative to GDL L1 (rvc)");
  h_gdl_top_l1_vs_grl_top_l1->GetYaxis()->SetTitle("GDL TOP rvc relative to GDL L1 (rvc)");

  h_grl_ntopslots_vs_ncdcslots = new TH2I("h_grl_ntopslots_vs_ncdcslots", "GRL TOP slots vs CDC slots", 10, 0, 10, 10, 0, 10);
  h_grl_ntopslots_vs_ncdcslots->GetXaxis()->SetTitle("GRL CDC slots");
  h_grl_ntopslots_vs_ncdcslots->GetYaxis()->SetTitle("GRL TOP slots");

  h_grl_ncdctopslots_matched = new TH1I("h_grl_ncdctopslots_matched", "GRL Number of matched TOP and CDC slots", 17, 0, 17);
  h_grl_ncdctopslots_matched->GetXaxis()->SetTitle("Number of matched TOP and CDC slots");

  h_grl_topslots_vs_cdcslots_match = new TH2I("h_grl_topslots_vs_cdcslots_match", "GRL TOP slots vs CDC slots matches", 17, 0, 17, 17,
                                              0, 17);
  h_grl_topslots_vs_cdcslots_match->GetXaxis()->SetTitle("GRL CDC slot");
  h_grl_topslots_vs_cdcslots_match->GetYaxis()->SetTitle("GRL TOP slot");


  //-------------------------------------------------------------------------------------------------------------------------------------------

  oldDir->cd();

}

void TRGTOPDQMModule::beginRun()
{

  dirDQM->cd();

  h_topSlotAll->Reset();
  h_topSegmentAll->Reset();
  h_topNHitsAll->Reset();
  h_topLogLAll->Reset();
  h_topT0All->Reset();

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

  h_topSegmentVsNHits->Reset();
  h_topSegmentVsLogL->Reset();
  h_topSegmentVsT0->Reset();

  h_topNHitsVsLogL->Reset();
  h_topNHitsVsT0->Reset();
  h_topLogLVsT0->Reset();

  h_ecl_gdl_top_timing_diff_both->Reset();
  h_ecl_top_top_timing_diff_both->Reset();

  if (m_histLevel > 2) {

    h_gdl_top_rvc_vs_top_timing->Reset();

    h_gdl_ecltop_rvc_diff->Reset();
    h_gdl_cdctop_rvc_diff->Reset();

    h_gdl_gdltop_rvc_diff_all->Reset();
    h_gdl_comtop_rvc_diff_all->Reset();

    h_topRvcDiff1All->Reset();
    h_topRvcDiff2All->Reset();
    h_topRvcDiff3All->Reset();
    h_topRvcDiff4All->Reset();

    h_topGdlRvcDiff1All->Reset();
    h_topGdlRvcDiff2All->Reset();
    h_topGdlRvcDiff3All->Reset();

    h_decisionNumberVsNumberDecisionsGood->Reset();

    h_ecl_gdl_top_timing_diff_good->Reset();
    h_ecl_top_top_timing_diff_good->Reset();

    h_gdl_ecltop_timing_diff_vs_toptop_good->Reset();
    h_top_ecltop_timing_diff_vs_toptop_good->Reset();
    h_gdl_ecltop_timing_diff_vs_ecltop_good->Reset();

    h_gdl_gdltop_rvc_diff_good->Reset();
    h_gdl_comtop_rvc_diff_good->Reset();

    h_topCombinedTimingTopGood->Reset();
    h_topNSlotsCombinedTimingTopGood->Reset();
    h_topNHitSumGood->Reset();
    h_topLogLSumGood->Reset();

    h_topRvcDiff1Good->Reset();
    h_topRvcDiff2Good->Reset();
    h_topRvcDiff3Good->Reset();
    h_topRvcDiff4Good->Reset();

    h_topGdlRvcDiff1Good->Reset();
    h_topGdlRvcDiff2Good->Reset();
    h_topGdlRvcDiff3Good->Reset();

    h_decisionNumberVsNumberDecisionsBad->Reset();

    h_ecl_gdl_top_timing_diff_bad->Reset();
    h_ecl_top_top_timing_diff_bad->Reset();

    h_gdl_ecltop_timing_diff_vs_toptop_bad->Reset();
    h_top_ecltop_timing_diff_vs_toptop_bad->Reset();
    h_gdl_ecltop_timing_diff_vs_ecltop_bad->Reset();

    h_gdl_gdltop_rvc_diff_bad->Reset();
    h_gdl_comtop_rvc_diff_bad->Reset();

    h_topCombinedTimingTopBad->Reset();
    h_topNSlotsCombinedTimingTopBad->Reset();
    h_topNHitSumBad->Reset();
    h_topLogLSumBad->Reset();

    h_topRvcDiff1Bad->Reset();
    h_topRvcDiff2Bad->Reset();
    h_topRvcDiff3Bad->Reset();
    h_topRvcDiff4Bad->Reset();

    h_topGdlRvcDiff1Bad->Reset();
    h_topGdlRvcDiff2Bad->Reset();
    h_topGdlRvcDiff3Bad->Reset();

  }

  h_topCombinedTimingTopAll->Reset();
  h_topNSlotsCombinedTimingTopAll->Reset();
  h_topNHitSumAll->Reset();
  h_topLogLSumAll->Reset();

  h_topNSlotsCombinedTimingVsNHitsTopAll->Reset();

  h_topTrigType->Reset();
  h_topTimingResiduals->Reset();
  h_topTimingVariance->Reset();

  for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {

    h_top_nt0decisions_vs_hist_class[iskim]->Reset();

    h_top_gdl_match_decision_number[iskim]->Reset();

    h_gdl_ecltop_timing_diff_5ns[iskim]->Reset();
    h_gdl_cdctop_timing_diff_5ns[iskim]->Reset();

    h_gdl_ecltop_timing_diff_2ns[iskim]->Reset();
    h_gdl_ecltop_timing_diff_grl_matched_2ns[iskim]->Reset();
    h_gdl_ecltop_timing_diff_no_grl_matched_2ns[iskim]->Reset();
    h_gdl_ecltop_timing_diff_no_grl_at_all_2ns[iskim]->Reset();
    h_gdl_ecltop_timing_diff_vs_nslots_2ns[iskim]->Reset();
    h_top_ecltop_timing_diff_combined_2ns[iskim]->Reset();
    h_top_ecltop_timing_diff_best_slot_2ns[iskim]->Reset();
    h_gdl_cdctop_timing_diff_2ns[iskim]->Reset();

  }

  if (m_histLevel > 1) {
    for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {
      for (int histClass = 0; histClass < m_nHistClassesActual; histClass++) {
        h_N_decision[iskim][histClass]->Reset();

        h_topNHitVsNhit[iskim][histClass]->Reset();
        h_topSlotVsSlot[iskim][histClass]->Reset();
        h_topT0VsT0[iskim][histClass]->Reset();
        h_topSegmentVsSegment[iskim][histClass]->Reset();
        h_topLogLVsLogL[iskim][histClass]->Reset();

        h_topCombinedTimingTop[iskim][histClass]->Reset();
        h_topNSlotsCombinedTimingTop[iskim][histClass]->Reset();
        h_topNHitSum[iskim][histClass]->Reset();
        h_topNHitBestSlot[iskim][histClass]->Reset();
        //        h_topT0DecisionNumberBestSlot[iskim][histClass]->Reset();
        h_topLogLSum[iskim][histClass]->Reset();

        h_gdl_ecltop_timing_diff_vs_slot_2ns[iskim][histClass]->Reset();
        h_gdl_ecltop_timing_diff_vs_segment_2ns[iskim][histClass]->Reset();
        h_gdl_ecltop_timing_diff_vs_nhits_2ns[iskim][histClass]->Reset();
        h_gdl_ecltop_timing_diff_vs_logl_2ns[iskim][histClass]->Reset();

        h_top_ecltop_timing_diff_2ns[iskim][histClass]->Reset();
        h_top_ecltop_timing_combined_diff_2ns[iskim][histClass]->Reset();
        h_top_ecltop_timing_best_slot_diff_2ns[iskim][histClass]->Reset();

        h_gdl_ecltop_timing_diff_vs_toptop[iskim][histClass]->Reset();
        h_top_ecltop_timing_diff_vs_toptop[iskim][histClass]->Reset();

        h_gdl_ecltop_timing_diff_vs_ecltop[iskim][histClass]->Reset();
        h_gdl_ecltop_timing_diff_vs_ecltop_best_slot[iskim][histClass]->Reset();
      }
    }
  }

  h_gdl_ecltop_timing_diff_vs_slot->Reset();
  h_gdl_ecltop_timing_diff_vs_segment->Reset();
  h_gdl_ecltop_timing_diff_vs_nhits->Reset();
  h_gdl_ecltop_timing_diff_vs_logl->Reset();

  h_topTC2IdVsTC1IdAll->Reset();
  h_topTC2EnergyVsTC1EnergyAll->Reset();
  h_topTC2ThetaIdVsTC1ThetaIdAll->Reset();
  h_topTC2PhiIdVsTC1PhiIdAll->Reset();
  h_topTCPhiIdVsTCThetaIdAll->Reset();

  h_topTC2IdVsTC1IdGRLAll->Reset();
  h_topTC2EnergyVsTC1EnergyGRLAll->Reset();
  h_topTC2ThetaIdVsTC1ThetaIdGRLAll->Reset();
  h_topTC2PhiIdVsTC1PhiIdGRLAll->Reset();
  h_topTCPhiIdVsTCThetaIdGRLAll->Reset();

  h_topTC2IdVsTC1Id->Reset();
  h_topTC2EnergyVsTC1Energy->Reset();
  h_topTC2ThetaIdVsTC1ThetaId->Reset();
  h_topTC2PhiIdVsTC1PhiId->Reset();
  h_topTCPhiIdVsTCThetaId->Reset();

  h_topTC2IdVsTC1IdGRL->Reset();
  h_topTC2EnergyVsTC1EnergyGRL->Reset();
  h_topTC2ThetaIdVsTC1ThetaIdGRL->Reset();
  h_topTC2PhiIdVsTC1PhiIdGRL->Reset();
  h_topTCPhiIdVsTCThetaIdGRL->Reset();

  h_gdl_ecltop_timing_diff_vs_grl_top_l1->Reset();
  h_gdl_top_l1_vs_grl_top_l1->Reset();

  h_grl_ntopslots_vs_ncdcslots->Reset();
  h_grl_ncdctopslots_matched->Reset();
  h_grl_topslots_vs_cdcslots_match->Reset();

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

  //  trgeclHitArray.registerInDataStore();

}

void TRGTOPDQMModule::endRun()
{
  // implement saving a postscript file (for some histograms) here
  // use ROOT scripts as a template
}

void TRGTOPDQMModule::event()
{

  StoreArray<TRGTOPUnpackerStore> trgtopCombinedTimingArray("TRGTOPUnpackerStores");

  if (!trgtopCombinedTimingArray) return;
  if (!trgtopCombinedTimingArray.getEntries()) return;

  bool gdlInfoAvailable = false;
  bool grlInfoAvailable = false;

  // investigate GRL-TOP correlations if asked to do so

  int grlTOPL1 = -1;

  bool grlCDCSlots[16];
  bool grlTOPSlots[16];

  bool grlCDCTOPSlotsMatch[16];

  if (m_doGRLCorrelations) {

    StoreObjPtr<TRGGRLUnpackerStore> grlEventInfo("TRGGRLUnpackerStore");

    // make sure first that GDL information could be retrieved
    if (grlEventInfo) {

      grlInfoAvailable = true;

      int grlTimeL1 = grlEventInfo->m_coml1 - grlEventInfo->m_revoclk;

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
    }
  }

  // investigate GRL-TOP
  int nCDCSlotsGRL = 0;
  int nTOPSlotsGRL = 0;
  int nCDCTOPSlotsMatchedGRL = 0;

  if (grlInfoAvailable) {
    for (int i = 0; i < 16; i++) {
      if (grlCDCSlots[i]) nCDCSlotsGRL++;
      if (grlTOPSlots[i]) nTOPSlotsGRL++;
      grlCDCTOPSlotsMatch[i] = false;
      if (grlCDCSlots[i] && grlTOPSlots[i]) {
        nCDCTOPSlotsMatchedGRL++;
        grlCDCTOPSlotsMatch[i] = true;
      }
    }
  }

  //  StoreArray<TRGECLUnpackerStore> trgeclHitArray;

  if (trgeclHitArray) {

    if (m_doECLCorrelations) {

      bool barrelEcl = false;
      bool barrelEclB2B = false;

      tcEclList.clear();

      //    TrgEclMapping* trgEclMap = new TrgEclMapping();
      TrgEclMapping trgEclMap;

      for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {

        TRGECLUnpackerStore* aTRGECLUnpackerStore = trgeclHitArray[ii];
        int TCID = (aTRGECLUnpackerStore->getTCId());

        int hit_win =  aTRGECLUnpackerStore -> getHitWin();
        double HitEnergy =  aTRGECLUnpackerStore -> getTCEnergy();
        double HitTiming    = aTRGECLUnpackerStore ->getTCTime();

        if (TCID < 1 || TCID > 576 || HitEnergy == 0) {continue;}
        if (!(hit_win == 3 || hit_win == 4)) {continue;}

        tcEcl tcEclThis;
        tcEclThis.tcId = TCID;
        tcEclThis.tcEnergy = HitEnergy;
        tcEclThis.tcTiming = HitTiming;
        tcEclThis.tcThetaId = trgEclMap.getTCThetaIdFromTCId(TCID);
        tcEclThis.tcPhiId = trgEclMap.getTCPhiIdFromTCId(TCID);
        tcEclList.push_back(tcEclThis);
      }

      // sort TCs according to their energies
      sort(tcEclList.begin(), tcEclList.end(), largestEnergy());

      // Barrel TCID map:
      // https://confluence.desy.de/display/BI/TRGECL+Meeting?preview=%2F43899281%2F58924075%2F170630yjkim.pdf
      if (tcEclList.size() >= 2) {
        vector<tcEcl>::const_iterator it = tcEclList.begin();
        const tcEcl& tc1 = *it;
        ++it;
        const tcEcl& tc2 = *it;
        int tcId1 = tc1.tcId;
        int tcId2 = tc2.tcId;

        int tcThetaId1 = tc1.tcThetaId;
        int tcPhiId1 = tc1.tcPhiId;

        int tcThetaId2 = tc2.tcThetaId;
        int tcPhiId2 = tc2.tcPhiId;

        h_topTC2IdVsTC1IdAll->Fill(tcId1, tcId2);
        h_topTC2EnergyVsTC1EnergyAll->Fill(tc1.tcEnergy, tc2.tcEnergy);
        h_topTC2ThetaIdVsTC1ThetaIdAll->Fill(tcThetaId1, tcThetaId2);
        h_topTC2PhiIdVsTC1PhiIdAll->Fill(tcPhiId1, tcPhiId2);
        h_topTCPhiIdVsTCThetaIdAll->Fill(tcThetaId1, tcPhiId1, 0);
        h_topTCPhiIdVsTCThetaIdAll->Fill(tcThetaId2, tcPhiId2, 1);

        if (nCDCSlotsGRL > 0) {
          h_topTC2IdVsTC1IdGRLAll->Fill(tcId1, tcId2);
          h_topTC2EnergyVsTC1EnergyGRLAll->Fill(tc1.tcEnergy, tc2.tcEnergy);
          h_topTC2ThetaIdVsTC1ThetaIdGRLAll->Fill(tcThetaId1, tcThetaId2);
          h_topTC2PhiIdVsTC1PhiIdGRLAll->Fill(tcPhiId1, tcPhiId2);
          h_topTCPhiIdVsTCThetaIdGRLAll->Fill(tcThetaId1, tcPhiId1, 0);
          h_topTCPhiIdVsTCThetaIdGRLAll->Fill(tcThetaId2, tcPhiId2, 1);
        }

        //  poor physicist's barrel b2b
        //      if (tcId1 >= 81 && tcId1 <= 512) {
        //  if (tcId2 >= 81 && tcId2 <= 512) {
        //    if (abs(tcId2-tcId1-220) <= 50) {

        if (tcThetaId1 >= 4 && tcThetaId1 <= 15) {
          barrelEcl = true;
          if (tcThetaId2 >= 4 && tcThetaId2 <= 15) {
            if (abs(tcPhiId1 - tcPhiId2) >= 12 && abs(tcPhiId1 - tcPhiId2) <= 24) {
              barrelEclB2B = true;
              h_topTC2IdVsTC1Id->Fill(tcId1, tcId2);
              h_topTC2EnergyVsTC1Energy->Fill(tc1.tcEnergy, tc2.tcEnergy);
              h_topTC2ThetaIdVsTC1ThetaId->Fill(tcThetaId1, tcThetaId2);
              h_topTC2PhiIdVsTC1PhiId->Fill(tcPhiId1, tcPhiId2);
              h_topTCPhiIdVsTCThetaId->Fill(tcThetaId1, tcPhiId1, 0);
              h_topTCPhiIdVsTCThetaId->Fill(tcThetaId2, tcPhiId2, 1);
              if (nCDCSlotsGRL > 0) {
                h_topTC2IdVsTC1IdGRL->Fill(tcId1, tcId2);
                h_topTC2EnergyVsTC1EnergyGRL->Fill(tc1.tcEnergy, tc2.tcEnergy);
                h_topTC2ThetaIdVsTC1ThetaIdGRL->Fill(tcThetaId1, tcThetaId2);
                h_topTC2PhiIdVsTC1PhiIdGRL->Fill(tcPhiId1, tcPhiId2);
                h_topTCPhiIdVsTCThetaIdGRL->Fill(tcThetaId1, tcPhiId1, 0);
                h_topTCPhiIdVsTCThetaIdGRL->Fill(tcThetaId2, tcPhiId2, 1);
              }
            }
          }
        }
      }

      if (m_requireEclBarrel) {
        if (!barrelEcl) return;
      }
      if (m_requireEclBarrelB2B) {
        if (!barrelEclB2B) return;
      }
    }

  }

  if (m_doGRLCorrelations && grlInfoAvailable) {
    if (m_requireCDC2DTrack) {
      if (nCDCSlotsGRL <= 0) return;
    }
  }

  if (grlInfoAvailable) {

    h_grl_ntopslots_vs_ncdcslots->Fill(nCDCSlotsGRL, nTOPSlotsGRL);

    h_grl_ncdctopslots_matched->Fill(nCDCTOPSlotsMatchedGRL);

    for (int i = 0; i < 16; i++) {
      if (grlCDCTOPSlotsMatch[i]) {
        h_grl_topslots_vs_cdcslots_match->Fill(i + 1, i + 1);
      } else if (grlCDCSlots[i]) {
        h_grl_topslots_vs_cdcslots_match->Fill(i + 1, 0.);
      } else if (grlTOPSlots[i]) {
        h_grl_topslots_vs_cdcslots_match->Fill(0., i + 1);
      }
    }
  }

  // normally this should not be happening
  if (nCDCSlotsGRL == 0 && nTOPSlotsGRL == 0) h_grl_topslots_vs_cdcslots_match->Fill(0., 0.);

  /*
  cout << "DEBUG---------------------------------------------------------------------------------------------------------------------" << endl;
  int j=0;
  for ( vector<tcEcl>::const_iterator it = tcEclList.begin(); it != tcEclList.end(); ++it) {
    j++;
    const tcEcl& tc = *it;
    cout << "DEBUG index, tcId, tcEnergy, tcTiming = " << j << ", " << tc.tcId << ", " << tc.tcEnergy << ", " << tc.tcTiming << endl;
  }
  cout << "DEBUG---------------------------------------------------------------------------------------------------------------------" << endl;
  */

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

  // investigate GDL-TOP correlations if asked to do so

  // There are 5 values of revolution counter which could be obtained from GDL L1 data packet
  // Most important (from TOP L1 perspective) are gdll1rvc and toprvc
  int gdl_gdll1rvc   = -1;
  int gdl_coml1rvc   = -1;
  int gdl_toprvc     = -1;
  int gdl_eclrvc     = -1;
  int gdl_cdcrvc     = -1;

  int gdl_top_timing = -1;
  int gdl_ecl_timing = -1;
  int gdl_cdc_timing = -1;

  // obtain various important information about GDL decision from the database
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

  //
  //--------------------------------------------- actual histogramming starts here ----------------------------------------------------
  //
  //  Very generally, many (but not all) TOP L1 DQM histograms could be separated into three mutually exclusive classes according to
  //  the agreement between TOP information in GDL data packet and TOP information in TOP L1 data packet (i.e. circular buffer (CB)):
  //
  //  1) class 0:  TOP information at GDL and at TOP (in the FIRST decision stored in TOP L1 CB) is the same
  //  2) class 1:  There are NO TOP combined decision in TOP L1 CB that agree with TOP information at GDL
  //  3) class 2:  TOP information at GDL and at TOP is different for all but one combined decision (but not first),
  //               there could be more than one such combined t0 decision in TOP L1 CB
  //
  //  When GDL information is unavailable (is NEVER supposed to happen!), all events are labeled as class 1 (just to be safe)
  //
  //  Histograms for different types of events (according to software trigger) are prepared for each of the three classes.
  //  In addition to these histograms there are also a number of other histograms which are prepared for all combined decisions in TOP L1 CB
  //  or/and for GDL TOP information (and for TOP-ECL, TOP-GRL and TOP-GDL correlations).
  //

  // info from GDL

  int ecl_top_timing_diff = -1;
  int cdc_top_timing_diff = -1;

  int gdl_top_rvc_diff = -1;
  int com_top_rvc_diff = -1;
  int ecl_top_rvc_diff = -1;
  int cdc_top_rvc_diff = -1;

  if (gdlInfoAvailable) {

    ecl_top_timing_diff = gdl_ecl_timing >= gdl_top_timing ? gdl_ecl_timing - gdl_top_timing : gdl_ecl_timing - gdl_top_timing +
                          10240;
    cdc_top_timing_diff = gdl_cdc_timing >= gdl_top_timing ? gdl_cdc_timing - gdl_top_timing : gdl_cdc_timing - gdl_top_timing +
                          10240;

    gdl_top_rvc_diff = gdl_gdll1rvc >= gdl_toprvc ? gdl_gdll1rvc - gdl_toprvc : gdl_gdll1rvc - gdl_toprvc + 1280;
    com_top_rvc_diff = gdl_coml1rvc >= gdl_toprvc ? gdl_coml1rvc - gdl_toprvc : gdl_coml1rvc - gdl_toprvc + 1280;
    ecl_top_rvc_diff = gdl_eclrvc >= gdl_toprvc ? gdl_eclrvc - gdl_toprvc : gdl_eclrvc - gdl_toprvc + 1280;
    cdc_top_rvc_diff = gdl_cdcrvc >= gdl_toprvc ? gdl_cdcrvc - gdl_toprvc : gdl_cdcrvc - gdl_toprvc + 1280;

  }

  int histClass = 1;

  // retrieve TOP L1 information

  int nT0Decisions = trgtopCombinedTimingArray.getEntries();

  int t0DecisionNumber = 0;

  if (gdlInfoAvailable) {

    int nInTime = 0;

    for (const auto& t0Decision : trgtopCombinedTimingArray) {
      int topCombinedTimingTop = t0Decision.getCombinedTimingTop();
      int top_top_timing = (topCombinedTimingTop % 10240);

      if (gdl_top_timing == top_top_timing) {
        nInTime++;
        if (t0DecisionNumber == 0) {
          histClass = 0;
        } else if (histClass != 0) {
          histClass = 2;
        }
        for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
          h_top_gdl_match_decision_number[skim[ifill]]->Fill(t0DecisionNumber + 1);
        }
      }
      t0DecisionNumber++;
    }

    // classification of events according to the three classes described previously
    for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
      h_top_nt0decisions_vs_hist_class[skim[ifill]]->Fill(histClass, nT0Decisions);
    }

    // ALL TOP t0 decisions made within the last us before GDL L1 decision are out of time with GDL L1!
    if (nInTime == 0) {
      for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
        h_top_gdl_match_decision_number[skim[ifill]]->Fill(0);
      }
    }
  }

  // in such case we do NOT distinguish among the three classes
  if (m_nHistClassesActual == 1) histClass = 0;

  // when histClass==2 is not requested, combined it with histClass==1
  if (m_nHistClassesActual == 2 && histClass == 2) histClass = 1;

  // prepare histograms for TOP alone
  if (m_histLevel > 1) {
    for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
      h_N_decision[skim[ifill]][histClass]->Fill(nT0Decisions);
    }
  }

  // info from GDL

  if (gdlInfoAvailable) {

    //  const double clkTo1ns = 0.5 / .508877;
    // Feb. 23, 2020  ecltopdiff = (int) ( (double) ecltopdiff * clkTo1ns);

    if (m_histLevel > 2) {
      int gdl_top_timing_1280 = gdl_top_timing >> 3;
      h_gdl_top_rvc_vs_top_timing->Fill(gdl_top_timing_1280, gdl_toprvc);

      h_gdl_ecltop_rvc_diff->Fill(ecl_top_rvc_diff);
      h_gdl_cdctop_rvc_diff->Fill(cdc_top_rvc_diff);

      h_gdl_gdltop_rvc_diff_all->Fill(gdl_top_rvc_diff);
      h_gdl_comtop_rvc_diff_all->Fill(com_top_rvc_diff);
    }

    if (grlInfoAvailable) {
      h_gdl_ecltop_timing_diff_vs_grl_top_l1->Fill(grlTOPL1, ecl_top_timing_diff);
    }

    for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

      h_gdl_ecltop_timing_diff_5ns[skim[ifill]]->Fill(ecl_top_timing_diff);
      h_gdl_cdctop_timing_diff_5ns[skim[ifill]]->Fill(cdc_top_timing_diff);

      h_gdl_ecltop_timing_diff_2ns[skim[ifill]]->Fill(ecl_top_timing_diff);

      if (nCDCTOPSlotsMatchedGRL > 0) {
        h_gdl_ecltop_timing_diff_grl_matched_2ns[skim[ifill]]->Fill(ecl_top_timing_diff);
      } else if (nCDCSlotsGRL > 0) {
        h_gdl_ecltop_timing_diff_no_grl_matched_2ns[skim[ifill]]->Fill(ecl_top_timing_diff);
      } else {
        h_gdl_ecltop_timing_diff_no_grl_at_all_2ns[skim[ifill]]->Fill(ecl_top_timing_diff);
      }
      h_gdl_cdctop_timing_diff_2ns[skim[ifill]]->Fill(cdc_top_timing_diff);

    }
  }

  // note that now we are going to inspect all decisions in 1us TOP CB, however, most of the time we will be interested in the first decision only

  t0DecisionNumber = 0;

  for (const auto& t0Decision : trgtopCombinedTimingArray) {

    bool top_timing_same = false;
    if (gdlInfoAvailable) {
      int topCombinedTimingTop = t0Decision.getCombinedTimingTop();
      int top_top_timing = (topCombinedTimingTop % 10240);
      if (top_top_timing == gdl_top_timing) {
        top_timing_same = true;
      }
    }

    // not possible    const vector<Belle2::TRGTOPSlotTiming> t0DecisionSlots = t0Decision.getSlotTimingDecisions();
    const vector<Belle2::TRGTOPSlotTiming> t0DecisionSlots = trgtopCombinedTimingArray[t0DecisionNumber]->getSlotTimingDecisions();

    slotDecisionList.clear();

    if (t0DecisionNumber == 0) {

      for (const auto& t0DecisionSlot : t0DecisionSlots) {

        int slot = t0DecisionSlot.getSlotId();
        int segment = t0DecisionSlot.getSlotSegment();
        int nHits = t0DecisionSlot.getSlotNHits();
        int logL = t0DecisionSlot.getSlotLogL();
        int t0 = t0DecisionSlot.getSlotTiming();

        // Note that slotDecisionList is prepared for all TOP combined t0 decisions
        slotDecision slotDecisionThis;
        slotDecisionThis.slot = slot;
        slotDecisionThis.segment = segment;
        slotDecisionThis.nHits = nHits;
        slotDecisionThis.logL = logL;
        slotDecisionThis.t0 = t0;
        slotDecisionList.push_back(slotDecisionThis);

        h_topSlotAll->Fill(slot);
        h_topSegmentAll->Fill(segment);
        h_topNHitsAll->Fill(nHits);
        h_topLogLAll->Fill(logL);
        h_topT0All->Fill(t0);

        // note that the histograms in this section are for ALL combined decisions from 1us circular buffer
        if (gdlInfoAvailable) {
          int top_top_timing_slot = (t0 % 10240);
          int top_gdl_timing_diff_slot = gdl_ecl_timing >= top_top_timing_slot ? gdl_ecl_timing - top_top_timing_slot : gdl_ecl_timing -
                                         top_top_timing_slot  + 10240;

          h_gdl_ecltop_timing_diff_vs_slot->Fill(top_gdl_timing_diff_slot, slot);
          h_gdl_ecltop_timing_diff_vs_segment->Fill(top_gdl_timing_diff_slot, segment);
          h_gdl_ecltop_timing_diff_vs_nhits->Fill(top_gdl_timing_diff_slot, nHits);
          h_gdl_ecltop_timing_diff_vs_logl->Fill(top_gdl_timing_diff_slot, logL);

          if (m_histLevel > 1) {
            for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
              h_gdl_ecltop_timing_diff_vs_slot_2ns[skim[ifill]][histClass]->Fill(top_gdl_timing_diff_slot, slot);
              h_gdl_ecltop_timing_diff_vs_segment_2ns[skim[ifill]][histClass]->Fill(top_gdl_timing_diff_slot, segment);
              h_gdl_ecltop_timing_diff_vs_nhits_2ns[skim[ifill]][histClass]->Fill(top_gdl_timing_diff_slot, nHits);
              h_gdl_ecltop_timing_diff_vs_logl_2ns[skim[ifill]][histClass]->Fill(top_gdl_timing_diff_slot, logL);
            }
          }
        }

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

            h_topSegmentVsNHits->Fill(nHits, segment);
            h_topSegmentVsLogL->Fill(logL, segment);
            h_topSegmentVsT0->Fill(t0, segment);

            h_topNHitsVsLogL->Fill(logL, nHits);
            h_topNHitsVsT0->Fill(t0, nHits);
            h_topLogLVsT0->Fill(t0, logL);

          }
        }
      }
    }

    //
    //  This is the complete information from the slot with the largest N hits
    //  associated with the TOP combined t0 decision within 1us before GDL L1
    //  where TOP timing in TOP data packet is the same as TOP timing in GDL data packet
    //

    int nHitsSlotBest = 0;
    int t0SlotBest = -1;

    // slotDecisionList should always be available
    if (slotDecisionList.size() > 0) {

      int slotSlotBest = 0;
      int segmentSlotBest = 0;
      int logLSlotBest = 0;

      // sort slots according to their nHits
      sort(slotDecisionList.begin(), slotDecisionList.end(), largestNHits());

      vector<slotDecision>::const_iterator it = slotDecisionList.begin();
      const slotDecision& slotDecisionBest = *it;

      slotSlotBest = slotDecisionBest.slot;
      segmentSlotBest = slotDecisionBest.segment;
      logLSlotBest = slotDecisionBest.logL;
      nHitsSlotBest = slotDecisionBest.nHits;
      t0SlotBest = slotDecisionBest.t0;

      if (slotDecisionList.size() > 1) {
        ++it;
        const slotDecision& slotDecisionBest2 = *it;

        int slotSlotBest2 = slotDecisionBest2.slot;
        int segmentSlotBest2 = slotDecisionBest2.segment;
        int logLSlotBest2 = slotDecisionBest2.logL;
        int nHitsSlotBest2 = slotDecisionBest2.nHits;
        int t0SlotBest2 = slotDecisionBest2.t0;

        if (m_histLevel > 1) {
          if (t0DecisionNumber == 0) {
            for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
              h_topNHitVsNhit[skim[ifill]][histClass]->Fill(nHitsSlotBest, nHitsSlotBest2);
              h_topSlotVsSlot[skim[ifill]][histClass]->Fill(slotSlotBest, slotSlotBest2);
              h_topT0VsT0[skim[ifill]][histClass]->Fill(t0SlotBest, t0SlotBest2);
              h_topSegmentVsSegment[skim[ifill]][histClass]->Fill(segmentSlotBest, segmentSlotBest2);
              h_topLogLVsLogL[skim[ifill]][histClass]->Fill(logLSlotBest, logLSlotBest2);
            }
          }
        }
      }

    }

    //    if (slotDecisionList.size() > 1) {
    //
    //      cout << "DEBUG: more than one slot: " << slotDecisionList.size() <<endl;
    //      cout << "DEBUG: nHitsSlotBest: " << nHitsSlotBest << endl;
    //
    //      for ( vector<slotDecision>::const_iterator it = slotDecisionList.begin(); it != slotDecisionList.end(); ++it) {
    //  const slotDecision& slotDecisionThis = *it;
    //  cout << "DEBUG slotDecisionThis nHits = " << slotDecisionThis.nHits << endl;
    //      }
    //
    //    }

    //    int topEventIdL1 = t0Decision.getEventIdL1();
    //    int topEventIdTOP = t0Decision.getEventIdTOP();
    //    int topWindowIdTOP = t0Decision.getWindowIdTOP();

    // There are 5 values of revolution counter available for each TOP L1 combined t0 decision
    // timestamp when received L1 from GDL
    int topRvcL1 = t0Decision.getRvcB2L();
    // timestamp of the circular buffer window containing given combined t0 decision (this is the "NOW" time)
    int topRvcWindow = t0Decision.getRvcWindow();
    // timestamp of PREVIOUS combined t0 decision
    int topRvcTopTimingDecisionPrev = t0Decision.getRvcTopTimingDecisionPrev();
    // timestamp of the current combined t0 decision (i.e. decision in the current window)
    int topRvcTopTimingDecisionNow = t0Decision.getRvcTopTimingDecisionNow();
    // timestamp of when the current combined t0 decision was posted to GDL
    int topRvcTopTimingDecisionNowGdl = t0Decision.getRvcTopTimingDecisionNowGdl();

    //    int trigType = t0Decision.getTrigType();
    //    cout << "DEBUG trigType = " << trigType << endl;

    int topCombinedTimingTop = t0Decision.getCombinedTimingTop();

    int topNSlotsCombinedTimingTop = t0Decision.getNSlotsCombinedTimingTop();
    int topCombinedTimingTopResidual = t0Decision.getCombinedTimingTopResidual();
    //    int topNErrorsMinor = t0Decision.getNErrorsMinor();
    //    int topNErrorsMajor = t0Decision.getNErrorsMajor();
    int topTrigType = t0Decision.getTrigType();
    int topNHitSum = t0Decision.getNHitSum();
    int topLogLSum = t0Decision.getLogLSum();
    //    int topLogLVar = t0Decision.getLogLVar();
    int topTimingVar = t0Decision.getTimingVar();

    float hitsPerSlot = topNHitSum / max(1, topNSlotsCombinedTimingTop);
    float logLPerSlot = topLogLSum / max(1, topNSlotsCombinedTimingTop);

    int topRvcDiff1 = topRvcTopTimingDecisionNowGdl - topRvcTopTimingDecisionNow;
    int topRvcDiff2 = topRvcTopTimingDecisionNow >= topRvcTopTimingDecisionPrev ? topRvcTopTimingDecisionNow -
                      topRvcTopTimingDecisionPrev : topRvcTopTimingDecisionNow - topRvcTopTimingDecisionPrev + 1280;
    int topRvcDiff3 = topRvcWindow >= topRvcTopTimingDecisionNow ? topRvcWindow - topRvcTopTimingDecisionNow : topRvcWindow -
                      topRvcTopTimingDecisionNow + 1280;
    int topRvcDiff4 = topRvcL1 >= topRvcTopTimingDecisionNow ? topRvcL1 - topRvcTopTimingDecisionNow : topRvcL1 -
                      topRvcTopTimingDecisionNow + 1280;

    if (t0DecisionNumber == 0) {

      if (grlInfoAvailable) {
        h_gdl_top_l1_vs_grl_top_l1->Fill(grlTOPL1, topRvcDiff4);
      }

      h_topCombinedTimingTopAll->Fill(topCombinedTimingTop);
      h_topNSlotsCombinedTimingTopAll->Fill(topNSlotsCombinedTimingTop);
      h_topNHitSumAll->Fill((int) hitsPerSlot);
      h_topLogLSumAll->Fill((int) logLPerSlot);

      h_topNSlotsCombinedTimingVsNHitsTopAll->Fill(topNHitSum, topNSlotsCombinedTimingTop);

      h_topTrigType->Fill(topTrigType);
      h_topTimingResiduals->Fill(topCombinedTimingTopResidual);
      h_topTimingVariance->Fill(topTimingVar);

      if (m_histLevel > 2) {
        h_topRvcDiff1All->Fill(topRvcDiff1);
        h_topRvcDiff2All->Fill(topRvcDiff2);
        h_topRvcDiff3All->Fill(topRvcDiff3);
        h_topRvcDiff4All->Fill(topRvcDiff4);
      }
    }

    int topGdlRvcDiff1 = gdl_gdll1rvc >= topRvcL1 ? gdl_gdll1rvc - topRvcL1 : gdl_gdll1rvc - topRvcL1 + 1280;
    int topGdlRvcDiff2 = gdl_toprvc >= topRvcTopTimingDecisionNow ? gdl_toprvc - topRvcTopTimingDecisionNow : gdl_toprvc -
                         topRvcTopTimingDecisionNow + 1280;
    int topGdlRvcDiff3 = gdl_toprvc >= topRvcTopTimingDecisionPrev ? gdl_toprvc - topRvcTopTimingDecisionPrev : gdl_toprvc -
                         topRvcTopTimingDecisionPrev + 1280;

    int ecl_gdl_top_timing_combined_diff = -1;

    int top_top_timing_combined_diff = -1;

    int ecl_top_top_timing_combined_diff = -1;
    int ecl_top_top_timing_best_slot_diff = -1;

    if (gdlInfoAvailable) {

      int top_top_timing_combined = -1;
      int top_top_timing_best_slot = -1;

      ecl_gdl_top_timing_combined_diff = gdl_ecl_timing >= gdl_top_timing ? gdl_ecl_timing - gdl_top_timing : gdl_ecl_timing -
                                         gdl_top_timing + 10240;

      top_top_timing_combined = (topCombinedTimingTop % 10240);
      top_top_timing_best_slot = (t0SlotBest % 10240);

      top_top_timing_combined_diff = top_top_timing_combined >= gdl_top_timing ? top_top_timing_combined - gdl_top_timing :
                                     top_top_timing_combined - gdl_top_timing + 10240;

      ecl_top_top_timing_combined_diff = gdl_ecl_timing >= top_top_timing_combined ? gdl_ecl_timing - top_top_timing_combined :
                                         gdl_ecl_timing - top_top_timing_combined + 10240;
      ecl_top_top_timing_best_slot_diff = gdl_ecl_timing >= top_top_timing_best_slot ? gdl_ecl_timing - top_top_timing_best_slot :
                                          gdl_ecl_timing - top_top_timing_best_slot + 10240;

      if (t0DecisionNumber == 0) {
        if (m_histLevel > 2) {
          h_topGdlRvcDiff1All->Fill(topGdlRvcDiff1);
          h_topGdlRvcDiff2All->Fill(topGdlRvcDiff2);
          h_topGdlRvcDiff3All->Fill(topGdlRvcDiff3);
        }

        h_ecl_gdl_top_timing_diff_both->Fill(ecl_gdl_top_timing_combined_diff);
        h_ecl_top_top_timing_diff_both->Fill(ecl_top_top_timing_combined_diff);
      }

      if (m_histLevel > 2) {

        // something odd is going on...
        if (top_top_timing_combined_diff == 0) {

          h_decisionNumberVsNumberDecisionsGood->Fill(nT0Decisions, t0DecisionNumber + 1);

          if (t0DecisionNumber == 0) {
            h_ecl_gdl_top_timing_diff_good->Fill(ecl_gdl_top_timing_combined_diff);
            h_ecl_top_top_timing_diff_good->Fill(ecl_top_top_timing_combined_diff);

            h_gdl_ecltop_timing_diff_vs_toptop_good->Fill(ecl_gdl_top_timing_combined_diff, top_top_timing_combined_diff);
            h_top_ecltop_timing_diff_vs_toptop_good->Fill(ecl_top_top_timing_combined_diff, top_top_timing_combined_diff);
            h_gdl_ecltop_timing_diff_vs_ecltop_good->Fill(ecl_gdl_top_timing_combined_diff, ecl_top_top_timing_combined_diff);

            h_topCombinedTimingTopGood->Fill(topCombinedTimingTop);
            h_topNSlotsCombinedTimingTopGood->Fill(topNSlotsCombinedTimingTop);
            h_topNHitSumGood->Fill((int) hitsPerSlot);
            h_topLogLSumGood->Fill((int) logLPerSlot);

            h_topRvcDiff1Good->Fill(topRvcDiff1);
            h_topRvcDiff2Good->Fill(topRvcDiff2);
            h_topRvcDiff3Good->Fill(topRvcDiff3);
            h_topRvcDiff4Good->Fill(topRvcDiff4);

            h_topGdlRvcDiff1Good->Fill(topGdlRvcDiff1);
            h_topGdlRvcDiff2Good->Fill(topGdlRvcDiff2);
            h_topGdlRvcDiff3Good->Fill(topGdlRvcDiff3);

            h_gdl_gdltop_rvc_diff_good->Fill(gdl_top_rvc_diff);
            h_gdl_comtop_rvc_diff_good->Fill(com_top_rvc_diff);
          }
        } else {

          h_decisionNumberVsNumberDecisionsBad->Fill(nT0Decisions, t0DecisionNumber + 1);

          if (t0DecisionNumber == 0) {
            h_ecl_gdl_top_timing_diff_bad->Fill(ecl_gdl_top_timing_combined_diff);
            h_ecl_top_top_timing_diff_bad->Fill(ecl_top_top_timing_combined_diff);

            h_gdl_ecltop_timing_diff_vs_toptop_bad->Fill(ecl_gdl_top_timing_combined_diff, top_top_timing_combined_diff);
            h_top_ecltop_timing_diff_vs_toptop_bad->Fill(ecl_top_top_timing_combined_diff, top_top_timing_combined_diff);
            h_gdl_ecltop_timing_diff_vs_ecltop_bad->Fill(ecl_gdl_top_timing_combined_diff, ecl_top_top_timing_combined_diff);

            h_topCombinedTimingTopBad->Fill(topCombinedTimingTop);
            h_topNSlotsCombinedTimingTopBad->Fill(topNSlotsCombinedTimingTop);
            h_topNHitSumBad->Fill((int) hitsPerSlot);
            h_topLogLSumBad->Fill((int) logLPerSlot);

            h_topRvcDiff1Bad->Fill(topRvcDiff1);
            h_topRvcDiff2Bad->Fill(topRvcDiff2);
            h_topRvcDiff3Bad->Fill(topRvcDiff3);
            h_topRvcDiff4Bad->Fill(topRvcDiff4);

            h_topGdlRvcDiff1Bad->Fill(topGdlRvcDiff1);
            h_topGdlRvcDiff2Bad->Fill(topGdlRvcDiff2);
            h_topGdlRvcDiff3Bad->Fill(topGdlRvcDiff3);

            h_gdl_gdltop_rvc_diff_bad->Fill(gdl_top_rvc_diff);
            h_gdl_comtop_rvc_diff_bad->Fill(com_top_rvc_diff);
          }
        }
      }
    }

    if (t0DecisionNumber == 0) {
      for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

        if (m_histLevel > 1) {
          if (gdlInfoAvailable) {
            h_gdl_ecltop_timing_diff_vs_toptop[skim[ifill]][histClass]->Fill(ecl_gdl_top_timing_combined_diff, top_top_timing_combined_diff);
            h_top_ecltop_timing_diff_vs_toptop[skim[ifill]][histClass]->Fill(ecl_top_top_timing_combined_diff, top_top_timing_combined_diff);
            h_gdl_ecltop_timing_diff_vs_ecltop[skim[ifill]][histClass]->Fill(ecl_gdl_top_timing_combined_diff,
                ecl_top_top_timing_combined_diff);
            h_gdl_ecltop_timing_diff_vs_ecltop_best_slot[skim[ifill]][histClass]->Fill(ecl_gdl_top_timing_combined_diff,
                ecl_top_top_timing_best_slot_diff);

            h_top_ecltop_timing_diff_2ns[skim[ifill]][histClass]->Fill(ecl_top_top_timing_combined_diff);

            // these histograms are (presumably) for just ONE decision from circular buffer (past 1us) unless there are more than one TOP t0 decision with exactly same timing
            if (top_timing_same) {
              h_top_ecltop_timing_combined_diff_2ns[skim[ifill]][histClass]->Fill(ecl_top_top_timing_combined_diff);
              h_top_ecltop_timing_best_slot_diff_2ns[skim[ifill]][histClass]->Fill(ecl_top_top_timing_best_slot_diff);
              h_topNHitBestSlot[skim[ifill]][histClass]->Fill(nHitsSlotBest);
              //              h_topT0DecisionNumberBestSlot[skim[ifill]][histClass]->Fill(t0DecisionNumber + 1);
            }
          }

          h_topCombinedTimingTop[skim[ifill]][histClass]->Fill(topCombinedTimingTop);
          h_topNSlotsCombinedTimingTop[skim[ifill]][histClass]->Fill(topNSlotsCombinedTimingTop);
          h_topNHitSum[skim[ifill]][histClass]->Fill((int) hitsPerSlot);
          h_topLogLSum[skim[ifill]][histClass]->Fill((int) logLPerSlot);
        }

        h_gdl_ecltop_timing_diff_vs_nslots_2ns[skim[ifill]]->Fill(ecl_top_timing_diff, topNSlotsCombinedTimingTop);
        h_top_ecltop_timing_diff_combined_2ns[skim[ifill]]->Fill(ecl_top_top_timing_combined_diff);
        h_top_ecltop_timing_diff_best_slot_2ns[skim[ifill]]->Fill(ecl_top_top_timing_best_slot_diff);

      }
    }

    t0DecisionNumber++;
  }

}
