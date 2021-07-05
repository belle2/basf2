/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

  for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {

    //
    //  combined decisions information
    //

    h_topCombinedTimingTopAll[iskim]  = new TH1I(Form("h_t0_comb_%s", skim_smap[iskim].c_str()), "ALL TOP combined t0 decisions", 100,
                                                 0,
                                                 100000);
    h_topCombinedTimingTopAll[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decision time (us)");

    h_topNSlotsCombinedTimingTopAll[iskim]  = new TH1I(Form("h_n_slots_comb_%s", skim_smap[iskim].c_str()),
                                                       "ALL TOP combined t0 decisions: N slots", 17, 0, 17);
    h_topNSlotsCombinedTimingTopAll[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N slots");

    h_topNHitSumAll[iskim]  = new TH1I(Form("h_nhit_per_slot_comb_%s", skim_smap[iskim].c_str()),
                                       "ALL TOP combined t0 decisions: N hits per slot", 200, 0, 200);
    h_topNHitSumAll[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N hits per slot");

    h_topLogLSumAll[iskim]  = new TH1I(Form("h_logl_per_slot_comb_%s", skim_smap[iskim].c_str()),
                                       "ALL TOP combined t0 decisions: log L per slot", 100, 0, 60000);
    h_topLogLSumAll[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: log L per slot");

    h_topNSlotsCombinedTimingVsNHitsTopAll[iskim] = new TH2I(Form("h_n_slots_vs_nhit_total_comb_%s", skim_smap[iskim].c_str()),
                                                             "ALL TOP combined t0 decisions: N slots vs N hits (total)", 300, 0, 300, 17, 0, 17);
    h_topNSlotsCombinedTimingVsNHitsTopAll[iskim]->GetXaxis()->SetTitle("N hits (total)");
    h_topNSlotsCombinedTimingVsNHitsTopAll[iskim]->GetYaxis()->SetTitle("N slots");

    h_topTrigType[iskim]  = new TH1I(Form("h_trig_type_%s", skim_smap[iskim].c_str()),
                                     "ALL TOP combined decisions: trig type", 10, 0, 10);
    h_topTrigType[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: trig type");

    h_topTimingResiduals[iskim]  = new TH1I(Form("h_timing_residuals_%s", skim_smap[iskim].c_str()),
                                            "ALL TOP combined decisions: timing residual", 100, 0, 100);
    h_topTimingResiduals[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: timing residuals");

    h_topTimingVariance[iskim]  = new TH1I(Form("h_timing_variance_%s", skim_smap[iskim].c_str()),
                                           "ALL TOP combined decisions: timing variance", 100, 0, 100);
    h_topTimingVariance[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: timing variance");

    //-------------------------------------------------------------------------------------------------------------------------------------------
    //
    //  slot-level information for combined decisions
    //

    h_topSlotAll[iskim]  = new TH1I(Form("h_slot_number_%s", skim_smap[iskim].c_str()), "slot", 16, 1, 17);
    h_topSlotAll[iskim]->GetXaxis()->SetTitle("slot number in combined decisions (over 1us)");

    h_topSegmentAll[iskim]  = new TH1I(Form("h_segment_slot_%s", skim_smap[iskim].c_str()), "segment", 10, 1, 11);
    h_topSegmentAll[iskim]->GetXaxis()->SetTitle("segment for each slot in combined decisions (over 1us)");

    h_topNHitsAll[iskim]  = new TH1I(Form("h_nhit_slot_%s", skim_smap[iskim].c_str()), "nhit", 200, 0, 200);
    h_topNHitsAll[iskim]->GetXaxis()->SetTitle("N hits (for slots in combined decisions (over 1us))");

    h_topLogLAll[iskim]  = new TH1I(Form("h_logl_slot_%s", skim_smap[iskim].c_str()), "log L", 100, 0, 60000);
    h_topLogLAll[iskim]->GetXaxis()->SetTitle("log L (for slots in combined decisions (over 1us))");

    h_topT0All[iskim]  = new TH1I(Form("h_t0_slot_%s", skim_smap[iskim].c_str()), "t0", 100, 0, 100000);
    h_topT0All[iskim]->GetXaxis()->SetTitle("t0 for slots in combined decisions (over 1us)");

    //-------------------------------------------------------------------------------------------------------------------------------------------
    //
    //  correlations in slot-level information for combined decisions
    //

    h_topSlotVsSegment[iskim]  = new TH2I(Form("h_slot_vs_segment_%s", skim_smap[iskim].c_str()), "slot # vs slot segment", 10, 1, 11,
                                          16, 1, 17);
    h_topSlotVsSegment[iskim]->GetXaxis()->SetTitle("segment");
    h_topSlotVsSegment[iskim]->GetYaxis()->SetTitle("slot");

    h_topSlotVsNHits[iskim]  = new TH2I(Form("h_slot_vs_nhit_%s", skim_smap[iskim].c_str()), "slot # vs slot nhit", 200, 0, 200, 16, 1,
                                        17);
    h_topSlotVsNHits[iskim]->GetXaxis()->SetTitle("N hits");
    h_topSlotVsNHits[iskim]->GetYaxis()->SetTitle("slot");

    h_topSlotVsLogL[iskim]  = new TH2I(Form("h_slot_vs_logl_%s", skim_smap[iskim].c_str()), "slot # vs slot log L", 100, 0, 60000, 16,
                                       1, 17);
    h_topSlotVsLogL[iskim]->GetXaxis()->SetTitle("logL");
    h_topSlotVsLogL[iskim]->GetYaxis()->SetTitle("slot");

    h_topSlotVsT0[iskim]  = new TH2I(Form("h_slot_vs_t0_%s", skim_smap[iskim].c_str()), "slot # vs slot t0", 100, 0, 100000, 16, 1, 17);
    h_topSlotVsT0[iskim]->GetXaxis()->SetTitle("t0");
    h_topSlotVsT0[iskim]->GetYaxis()->SetTitle("slot");

    h_topSegmentVsNHits[iskim]  = new TH2I(Form("h_segment_vs_nhit_%s", skim_smap[iskim].c_str()), "slot segment vs slot nhit", 200, 0,
                                           200, 10, 1, 11);
    h_topSegmentVsNHits[iskim]->GetXaxis()->SetTitle("N hits");
    h_topSegmentVsNHits[iskim]->GetYaxis()->SetTitle("segment");

    h_topSegmentVsLogL[iskim]  = new TH2I(Form("h_segment_vs_logl_%s", skim_smap[iskim].c_str()), "slot segment vs slot log L", 100, 0,
                                          60000, 10, 1, 11);
    h_topSegmentVsLogL[iskim]->GetXaxis()->SetTitle("logL");
    h_topSegmentVsLogL[iskim]->GetYaxis()->SetTitle("segment");

    h_topSegmentVsT0[iskim]  = new TH2I(Form("h_segment_vs_t0_%s", skim_smap[iskim].c_str()), "slot segment vs slot t0", 100, 0, 100000,
                                        10, 1, 11);
    h_topSegmentVsT0[iskim]->GetXaxis()->SetTitle("t0");
    h_topSegmentVsT0[iskim]->GetYaxis()->SetTitle("segment");

    h_topNHitsVsLogL[iskim]  = new TH2I(Form("h_nhit_vs_logl_%s", skim_smap[iskim].c_str()), "slot nhit vs slot log L", 100, 0, 100000,
                                        200, 0, 200);
    h_topNHitsVsLogL[iskim]->GetXaxis()->SetTitle("logL");
    h_topNHitsVsLogL[iskim]->GetYaxis()->SetTitle("N hits");

    h_topNHitsVsT0[iskim]  = new TH2I(Form("h_nhit_vs_t0_%s", skim_smap[iskim].c_str()), "slot nhit vs slot t0", 100, 0, 100000, 200, 0,
                                      200);
    h_topNHitsVsT0[iskim]->GetXaxis()->SetTitle("t0");
    h_topNHitsVsT0[iskim]->GetYaxis()->SetTitle("N hits");

    h_topLogLVsT0[iskim]  = new TH2I(Form("h_logl_vs_t0_%s", skim_smap[iskim].c_str()), "slot log L vs slot t0", 100, 0, 100000, 100, 0,
                                     60000);
    h_topLogLVsT0[iskim]->GetXaxis()->SetTitle("t0");
    h_topLogLVsT0[iskim]->GetYaxis()->SetTitle("logL");

    //-------------------------------------------------------------------------------------------------------------------------------------------
    //
    //  more slot-level information (per slot) for combined decisions
    //

    for (int slot = 0; slot <= 15; slot++) {

      h_topSlotSegment[iskim][slot]  = new TH1I(Form("h_segment_slot_%s_%s", (to_string(slot + 1)).c_str(), skim_smap[iskim].c_str()),
                                                "segment", 10, 1, 11);
      h_topSlotSegment[iskim][slot]->GetXaxis()->SetTitle("segment");

      h_topSlotNHits[iskim][slot]  = new TH1I(Form("h_nhit_slot_%s_%s", (to_string(slot + 1)).c_str(), skim_smap[iskim].c_str()), "nhit",
                                              200, 0, 200);
      h_topSlotNHits[iskim][slot]->GetXaxis()->SetTitle("N hits");

      h_topSlotLogL[iskim][slot]  = new TH1I(Form("h_logl_slot_%s_%s", (to_string(slot + 1)).c_str(), skim_smap[iskim].c_str()), "log L",
                                             100, 0, 60000);
      h_topSlotLogL[iskim][slot]->GetXaxis()->SetTitle("log L");

      h_topSlotT0[iskim][slot]  = new TH1I(Form("h_t0_slot_%s_%s", (to_string(slot + 1)).c_str(), skim_smap[iskim].c_str()), "t0", 100, 0,
                                           100000);
      h_topSlotT0[iskim][slot]->GetXaxis()->SetTitle("t0");

    }

    //-------------------------------------------------------------------------------------------------------------------------------------------
    //
    //  GDL-TOP comparisons for slots in combined decisions
    //

    h_gdl_ecltop_timing_diff_vs_slot[iskim] = new TH2I(Form("h_gdl_slot_vs_ecltop_timing_diff_%s", skim_smap[iskim].c_str()),
                                                       "slot vs GDL ECL-TOP slot t0 difference", 400, 1400,
                                                       2200, 16, 1, 17);
    h_gdl_ecltop_timing_diff_vs_slot[iskim]->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
    h_gdl_ecltop_timing_diff_vs_slot[iskim]->GetYaxis()->SetTitle("slot");

    h_gdl_ecltop_timing_diff_vs_segment[iskim] = new TH2I(Form("h_gdl_segment_vs_ecltop_timing_diff_%s", skim_smap[iskim].c_str()),
                                                          "segment vs GDL ECL-TOP slot t0 difference",
                                                          400, 1400, 2200, 10, 1, 11);
    h_gdl_ecltop_timing_diff_vs_segment[iskim]->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
    h_gdl_ecltop_timing_diff_vs_segment[iskim]->GetYaxis()->SetTitle("segment");

    h_gdl_ecltop_timing_diff_vs_nhits[iskim] = new TH2I(Form("h_gdl_nhits_vs_ecltop_timing_diff_%s", skim_smap[iskim].c_str()),
                                                        "N hits (for slots) vs GDL ECL-TOP slot t0 difference", 400, 1400, 2200, 200, 0, 200);
    h_gdl_ecltop_timing_diff_vs_nhits[iskim]->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
    h_gdl_ecltop_timing_diff_vs_nhits[iskim]->GetYaxis()->SetTitle("N hits");

    h_gdl_ecltop_timing_diff_vs_logl[iskim] = new TH2I(Form("h_gdl_logl_vs_ecltop_timing_diff_%s", skim_smap[iskim].c_str()),
                                                       "log L (for slots) vs GDL ECL-TOP slot t0 difference", 400, 1400, 2200, 100, 0, 60000);
    h_gdl_ecltop_timing_diff_vs_logl[iskim]->GetXaxis()->SetTitle("GDL ECL - TOP slot t0 time difference (2ns)");
    h_gdl_ecltop_timing_diff_vs_logl[iskim]->GetYaxis()->SetTitle("log L");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_ecl_gdl_top_timing_diff_both[iskim] = new TH1I(Form("h_ecl_gdl_top_timing_diff_both_%s", skim_smap[iskim].c_str()),  "", 400,
                                                     1400, 2200);
    h_ecl_gdl_top_timing_diff_both[iskim]->GetXaxis()->SetTitle("Both ECL - GDL_TOP timing difference (2ns)");

    h_ecl_top_top_timing_diff_both[iskim] = new TH1I(Form("h_ecl_top_top_timing_diff_both_%s", skim_smap[iskim].c_str()),  "", 400,
                                                     1400, 2200);
    h_ecl_top_top_timing_diff_both[iskim]->GetXaxis()->SetTitle("Both ECL - TOP_TOP timing difference (2ns)");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    if (m_histLevel > 2) {

      //
      //  various rvc-related distributions for GDL/TOP
      //

      h_gdl_top_rvc_vs_top_timing[iskim] = new TH2I(Form("h_gdl_top_rvc_vs_top_timing_%s", skim_smap[iskim].c_str()),
                                                    "ALL GDL TOP rvc vs GDL TOP timing (7.8ns)",
                                                    128, 0, 1280, 128, 0, 1280);
      h_gdl_top_rvc_vs_top_timing[iskim]->GetXaxis()->SetTitle("TOP timing according to GDL (7.8ns)");
      h_gdl_top_rvc_vs_top_timing[iskim]->GetYaxis()->SetTitle("GDL rvc((TOP decision (received))");

      h_gdl_ecltop_rvc_diff[iskim] = new TH1I(Form("h_gdl_ecltop_rvc_diff_%s", skim_smap[iskim].c_str()),
                                              "ALL GDL ECL-TOP rvc difference",
                                              10, 0, 10);
      h_gdl_ecltop_rvc_diff[iskim]->GetXaxis()->SetTitle("TOPTRG GDL ECL-TOP rvc difference (clks)");

      h_gdl_cdctop_rvc_diff[iskim] = new TH1I(Form("h_gdl_cdctop_rvc_diff_%s", skim_smap[iskim].c_str()),
                                              "ALL GDL CDC-TOP rvc difference",
                                              10, 0, 10);
      h_gdl_cdctop_rvc_diff[iskim]->GetXaxis()->SetTitle("TOPTRG GDL CDC-TOP rvc difference (clks)");

      //-------------------------------------------------------------------------------------------------------------------------------------------

      h_gdl_gdltop_rvc_diff_all[iskim] = new TH1I(Form("h_gdl_gdltop_rvc_diff_%s", skim_smap[iskim].c_str()),
                                                  "ALL GDL GDL-TOP rvc difference",
                                                  10, 1270, 1280);
      h_gdl_gdltop_rvc_diff_all[iskim]->GetXaxis()->SetTitle("TOPTRG GDL GDL-TOP rvc difference (clks)");

      h_gdl_comtop_rvc_diff_all[iskim] = new TH1I(Form("h_gdl_comtop_rvc_diff_%s", skim_smap[iskim].c_str()),
                                                  "ALL GDL COM-TOP rvc difference",
                                                  30, 20, 50);
      h_gdl_comtop_rvc_diff_all[iskim]->GetXaxis()->SetTitle("TOPTRG GDL COM-TOP rvc difference (clks)");

      //-------------------------------------------------------------------------------------------------------------------------------------------

      h_topRvcDiff1All[iskim]  = new TH1I(Form("h_top_rvc_diff_1_%s", skim_smap[iskim].c_str()),  "ALL rvc(posted to GDL)-rvc(TOP(this))",
                                          250, -100, 150);
      h_topRvcDiff1All[iskim]->GetXaxis()->SetTitle("rvc(posted to GDL)-rvc(TOP(this))");

      h_topRvcDiff2All[iskim]  = new TH1I(Form("h_top_rvc_diff_2_%s", skim_smap[iskim].c_str()),  "ALL rvc(TOP(this))-rvc(TOP(prev))",
                                          128, 0, 1280);
      h_topRvcDiff2All[iskim]->GetXaxis()->SetTitle("rvc(TOP(this))-rvc(TOP(prev))");

      h_topRvcDiff3All[iskim]  = new TH1I(Form("h_top_rvc_diff_3_%s", skim_smap[iskim].c_str()),  "ALL rvc(CB(window))-rvc(TOP(this))",
                                          150, 0, 150);
      h_topRvcDiff3All[iskim]->GetXaxis()->SetTitle("rvc(CB(window))-rvc(TOP(this))");

      h_topRvcDiff4All[iskim]  = new TH1I(Form("h_top_rvc_diff_4_%s", skim_smap[iskim].c_str()),
                                          "ALL rvc(received L1 from GDL)-rvc(TOP(this))",
                                          250, 0, 250);
      h_topRvcDiff4All[iskim]->GetXaxis()->SetTitle("rvc(L1)-rvc(TOP(this))");

      //-------------------------------------------------------------------------------------------------------------------------------------------

      h_topGdlRvcDiff1All[iskim]  = new TH1I(Form("h_gdl_top_l1_rvc_diff_%s", skim_smap[iskim].c_str()),
                                             "ALL rvc(L1(GDL))-rvc(L1(as reported to TOP))",
                                             10, 1235, 1245);
      h_topGdlRvcDiff1All[iskim]->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(L1(as reported to TOP))");

      h_topGdlRvcDiff2All[iskim]  = new TH1I(Form("h_gdl_l1_top_t0_this_rvc_diff_%s", skim_smap[iskim].c_str()),
                                             "ALL rvc(TOP(received by GDL))-rvc(TOP(this))",
                                             50, 0, 200);
      h_topGdlRvcDiff2All[iskim]->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(this))");

      h_topGdlRvcDiff3All[iskim]  = new TH1I(Form("h_gdl_l1_top_t0_prev_rvc_diff_%s", skim_smap[iskim].c_str()),
                                             "ALL rvc(TOP(received by GDL))-rvc(TOP(prev))",
                                             128, 0, 1280);
      h_topGdlRvcDiff3All[iskim]->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(prev))");

      //-------------------------------------------------------------------------------------------------------------------------------------------

      h_decisionNumberVsNumberDecisionsGood[iskim] = new TH2I(Form("h_decision_vs_n_decisions_good_%s", skim_smap[iskim].c_str()),
                                                              "Good decision number vs N of decisions", 5, 0,
                                                              5, 5, 0, 5);
      h_decisionNumberVsNumberDecisionsGood[iskim]->GetXaxis()->SetTitle("N decisions");
      h_decisionNumberVsNumberDecisionsGood[iskim]->GetYaxis()->SetTitle("Decision number");

      //------------------

      h_ecl_gdl_top_timing_diff_good[iskim] = new TH1I(Form("h_ecl_gdl_top_timing_diff_good_%s", skim_smap[iskim].c_str()),  "", 400,
                                                       1400, 2200);
      h_ecl_gdl_top_timing_diff_good[iskim]->GetXaxis()->SetTitle("Good ECL - GDL_TOP timing difference (2ns)");

      h_ecl_top_top_timing_diff_good[iskim] = new TH1I(Form("h_ecl_top_top_timing_diff_good_%s", skim_smap[iskim].c_str()),  "", 400,
                                                       1400, 2200);
      h_ecl_top_top_timing_diff_good[iskim]->GetXaxis()->SetTitle("Good ECL - TOP_TOP timing difference (2ns)");

      //------------------

      h_gdl_ecltop_timing_diff_vs_toptop_good[iskim] = new TH2I(Form("h_gdl_ecltop_timing_diff_vs_toptop_good_%s",
                                                                skim_smap[iskim].c_str()),
                                                                "GOOD TOP-TOP vs (GDL ECL)-(GDL TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
      h_gdl_ecltop_timing_diff_vs_toptop_good[iskim]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) t0 difference (ns, 100ns bins)");
      h_gdl_ecltop_timing_diff_vs_toptop_good[iskim]->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

      h_top_ecltop_timing_diff_vs_toptop_good[iskim] = new TH2I(Form("h_top_ecltop_timing_diff_vs_toptop_good_%s",
                                                                skim_smap[iskim].c_str()),
                                                                "GOOD TOP-TOP vs (GDL ECL)-(TOP TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
      h_top_ecltop_timing_diff_vs_toptop_good[iskim]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");
      h_top_ecltop_timing_diff_vs_toptop_good[iskim]->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

      h_gdl_ecltop_timing_diff_vs_ecltop_good[iskim] = new TH2I(Form("h_gdl_ecltop_timing_diff_vs_ecltop_good_%s",
                                                                skim_smap[iskim].c_str()),
                                                                "GOOD (GDL ECL)-(TOP TOP) vs (GDL ECL)-(GDL TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
      h_gdl_ecltop_timing_diff_vs_ecltop_good[iskim]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) t0 difference (ns, 100ns bins)");
      h_gdl_ecltop_timing_diff_vs_ecltop_good[iskim]->GetYaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");

      //-------------------------------------------------------------------------------------------------------------------------------------------

      h_gdl_gdltop_rvc_diff_good[iskim] = new TH1I(Form("h_gdl_gdltop_rvc_diff_good_%s", skim_smap[iskim].c_str()),
                                                   "GOOD GDL GDL-TOP rvc difference",
                                                   10, 1270, 1280);
      h_gdl_gdltop_rvc_diff_good[iskim]->GetXaxis()->SetTitle("TOPTRG GDL GDL-TOP rvc difference (clks)");

      h_gdl_comtop_rvc_diff_good[iskim] = new TH1I(Form("h_gdl_comtop_rvc_diff_good_%s", skim_smap[iskim].c_str()),
                                                   "GOOD GDL COM-TOP rvc difference",
                                                   30, 20, 50);
      h_gdl_comtop_rvc_diff_good[iskim]->GetXaxis()->SetTitle("TOPTRG GDL COM-TOP rvc difference (clks)");

      //-------------

      h_topCombinedTimingTopGood[iskim]  = new TH1I(Form("h_t0_good_%s", skim_smap[iskim].c_str()), "GOOD TOP combined t0 decision", 100,
                                                    0,
                                                    100000);
      h_topCombinedTimingTopGood[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions");

      h_topNSlotsCombinedTimingTopGood[iskim]  = new TH1I(Form("h_n_slots_good_%s", skim_smap[iskim].c_str()),
                                                          "GOOD TOP combined t0 decision: N slots", 17, 0, 17);
      h_topNSlotsCombinedTimingTopGood[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N slots");

      h_topNHitSumGood[iskim]  = new TH1I(Form("h_n_hit_per_slot_good_%s", skim_smap[iskim].c_str()),
                                          "GOOD TOP combined t0 decision: N hit per slot", 200, 0, 200);
      h_topNHitSumGood[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N hits per slot");

      h_topLogLSumGood[iskim]  = new TH1I(Form("h_logl_per_slot_good_%s", skim_smap[iskim].c_str()),
                                          "GOOD TOP combined t0 decision: log L per slot", 100, 0, 60000);
      h_topLogLSumGood[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: log L per slot");

      //-------------

      h_topRvcDiff1Good[iskim]  = new TH1I(Form("h_rvc_diff_1_good_%s", skim_smap[iskim].c_str()),
                                           "GOOD rvc(posted to GDL)-rvc(TOP(this))",
                                           250, -100, 150);
      h_topRvcDiff1Good[iskim]->GetXaxis()->SetTitle("rvc(posted to GDL)-rvc(TOP(this))");

      h_topRvcDiff2Good[iskim]  = new TH1I(Form("h_rvc_diff_2_good_%s", skim_smap[iskim].c_str()), "GOOD rvc(TOP(this))-rvc(TOP(prev))",
                                           128, 0, 1280);
      h_topRvcDiff2Good[iskim]->GetXaxis()->SetTitle("rvc(TOP(this))-rvc(TOP(prev))");

      h_topRvcDiff3Good[iskim]  = new TH1I(Form("h_rvc_diff_3_good_%s", skim_smap[iskim].c_str()),  "GOOD rvc(CB(window))-rvc(TOP(this))",
                                           150, 0, 150);
      h_topRvcDiff3Good[iskim]->GetXaxis()->SetTitle("rvc(CB(window))-rvc(TOP(this))");

      h_topRvcDiff4Good[iskim]  = new TH1I(Form("h_rvc_diff_4_good_%s", skim_smap[iskim].c_str()),
                                           "GOOD rvc(received L1 from GDL)-rvc(TOP(this))",
                                           250, 0, 250);
      h_topRvcDiff4Good[iskim]->GetXaxis()->SetTitle("rvc(L1)-rvc(TOP(this))");

      //-------------------------------------------------------------------------------------------------------------------------------------------

      h_topGdlRvcDiff1Good[iskim]  = new TH1I(Form("h_gdl_top_l1_rvc_diff_good_%s", skim_smap[iskim].c_str()),
                                              "GOOD rvc(L1(GDL))-rvc(L1(as reported to TOP))",
                                              10, 1235, 1245);
      h_topGdlRvcDiff1Good[iskim]->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(L1(as reported to TOP))");

      h_topGdlRvcDiff2Good[iskim]  = new TH1I(Form("h_gdl_l1_top_t0_this_rvc_diff_good_%s", skim_smap[iskim].c_str()),
                                              "GOOD rvc(TOP(received by GDL))-rvc(TOP(this))",
                                              100, 0, 200);
      h_topGdlRvcDiff2Good[iskim]->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(this))");

      h_topGdlRvcDiff3Good[iskim]  = new TH1I(Form("h_gdl_l1_top_t0_prev_rvc_diff_good_%s", skim_smap[iskim].c_str()),
                                              "GOOD rvc(TOP(received by GDL))-rvc(TOP(prev))",
                                              128, 0, 1280);
      h_topGdlRvcDiff3Good[iskim]->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(prev))");

      //-------------------------------------------------------------------------------------------------------------------------------------------

      h_decisionNumberVsNumberDecisionsBad[iskim] = new TH2I(Form("h_decision_vs_n_decisions_bad_%s", skim_smap[iskim].c_str()),
                                                             "Bad decision number vs N of decisions", 5, 0, 5,
                                                             5, 0, 5);
      h_decisionNumberVsNumberDecisionsBad[iskim]->GetXaxis()->SetTitle("N decisions");
      h_decisionNumberVsNumberDecisionsBad[iskim]->GetYaxis()->SetTitle("Decision number");

      //------------------

      h_ecl_gdl_top_timing_diff_bad[iskim] = new TH1I(Form("h_ecl_gdl_top_timing_diff_bad_%s", skim_smap[iskim].c_str()),  "", 400, 1400,
                                                      2200);
      h_ecl_gdl_top_timing_diff_bad[iskim]->GetXaxis()->SetTitle("Bad ECL - GDL_TOP timing difference (2ns)");

      h_ecl_top_top_timing_diff_bad[iskim] = new TH1I(Form("h_ecl_top_top_timing_diff_bad_%s", skim_smap[iskim].c_str()),  "", 400, 1400,
                                                      2200);
      h_ecl_top_top_timing_diff_bad[iskim]->GetXaxis()->SetTitle("Bad ECL - TOP_TOP timing difference (2ns)");

      //------------------

      h_gdl_ecltop_timing_diff_vs_toptop_bad[iskim] = new TH2I(Form("h_gdl_ecltop_timing_diff_vs_toptop_bad_%s",
                                                               skim_smap[iskim].c_str()),
                                                               "BAD TOP-TOP vs (GDL ECL)-(GDL TOP) t0 differences", 110, -760, 10240, 110, -760, 10240);
      h_gdl_ecltop_timing_diff_vs_toptop_bad[iskim]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) (combined) t0 difference (ns, 100ns bins)");
      h_gdl_ecltop_timing_diff_vs_toptop_bad[iskim]->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

      h_top_ecltop_timing_diff_vs_toptop_bad[iskim] = new TH2I(Form("h_top_ecltop_timing_diff_vs_toptop_bad_%s",
                                                               skim_smap[iskim].c_str()),
                                                               "BAD TOP-TOP vs (GDL ECL)-(TOP TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
      h_top_ecltop_timing_diff_vs_toptop_bad[iskim]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");
      h_top_ecltop_timing_diff_vs_toptop_bad[iskim]->GetYaxis()->SetTitle("TOP timing (TOP) - TOP timing (GDL) (ns, 100ns bins)");

      h_gdl_ecltop_timing_diff_vs_ecltop_bad[iskim] = new TH2I(Form("h_gdl_ecltop_timing_diff_vs_ecltop_bad_%s",
                                                               skim_smap[iskim].c_str()),
                                                               "BAD (GDL ECL)-(TOP TOP) vs (GDL ECL)-(GDL TOP) (combined) t0 differences", 110, -760, 10240, 110, -760, 10240);
      h_gdl_ecltop_timing_diff_vs_ecltop_bad[iskim]->GetXaxis()->SetTitle("TOPTRG (GDL ECL)-(GDL TOP) t0 difference (ns, 100ns bins)");
      h_gdl_ecltop_timing_diff_vs_ecltop_bad[iskim]->GetYaxis()->SetTitle("TOPTRG (GDL ECL)-(TOP TOP) t0 difference (ns, 100ns bins)");

      //-------------------------------------------------------------------------------------------------------------------------------------------

      h_gdl_gdltop_rvc_diff_bad[iskim] = new TH1I(Form("h_gdl_gdltop_rvc_diff_bad_%s", skim_smap[iskim].c_str()),
                                                  "BAD GDL GDL-TOP rvc difference",
                                                  10, 1270, 1280);
      h_gdl_gdltop_rvc_diff_bad[iskim]->GetXaxis()->SetTitle("TOPTRG GDL GDL-TOP rvc difference (clks)");

      h_gdl_comtop_rvc_diff_bad[iskim] = new TH1I(Form("h_gdl_comtop_rvc_diff_bad_%s", skim_smap[iskim].c_str()),
                                                  "BAD GDL COM-TOP rvc difference",
                                                  30, 20, 50);
      h_gdl_comtop_rvc_diff_bad[iskim]->GetXaxis()->SetTitle("TOPTRG GDL COM-TOP rvc difference (clks)");

      //-------------

      h_topCombinedTimingTopBad[iskim]  = new TH1I(Form("h_t0_bad_%s", skim_smap[iskim].c_str()), "BAD TOP combined t0 decision", 100, 0,
                                                   100000);
      h_topCombinedTimingTopBad[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions");

      h_topNSlotsCombinedTimingTopBad[iskim]  = new TH1I(Form("h_N_slots_bad_%s", skim_smap[iskim].c_str()),
                                                         "BAD TOP combined t0 decision: N slots", 17, 0, 17);
      h_topNSlotsCombinedTimingTopBad[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N slots");

      h_topNHitSumBad[iskim]  = new TH1I(Form("h_n_hit_per_slot_bad_%s", skim_smap[iskim].c_str()),
                                         "BAD TOP combined t0 decision: N hit per slot", 200, 0, 200);
      h_topNHitSumBad[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: N hits per slot");

      h_topLogLSumBad[iskim]  = new TH1I(Form("h_logl_per_slot_bad_%s", skim_smap[iskim].c_str()),
                                         "BAD TOP combined t0 decision: log L per slot", 100, 0, 60000);
      h_topLogLSumBad[iskim]->GetXaxis()->SetTitle("TOPTRG combined t0 decisions: log L per slot");

      //-------------

      h_topRvcDiff1Bad[iskim]  = new TH1I(Form("h_rvc_diff_1_bad_%s", skim_smap[iskim].c_str()), "BAD rvc(posted to GDL)-rvc(TOP(this))",
                                          250, -100, 150);

      h_topRvcDiff1Bad[iskim]->GetXaxis()->SetTitle("rvc(posted to GDL)-rvc(TOP(this))");

      h_topRvcDiff2Bad[iskim]  = new TH1I(Form("h_rvc_diff_2_bad_%s", skim_smap[iskim].c_str()), "BAD rvc(TOP(this))-rvc(TOP(prev))",
                                          128, 0, 1280);
      h_topRvcDiff2Bad[iskim]->GetXaxis()->SetTitle("rvc(TOP(this))-rvc(TOP(prev))");

      h_topRvcDiff3Bad[iskim]  = new TH1I(Form("h_rvc_diff_3_bad_%s", skim_smap[iskim].c_str()),  "BAD rvc(CB(window))-rvc(TOP(this))",
                                          150, 0, 150);
      h_topRvcDiff3Bad[iskim]->GetXaxis()->SetTitle("rvc(CB(window))-rvc(TOP(this))");

      h_topRvcDiff4Bad[iskim]  = new TH1I(Form("h_rvc_diff_4_bad_%s", skim_smap[iskim].c_str()),
                                          "BAD rvc(received L1 from GDL)-rvc(TOP(this))",
                                          250, 0, 250);
      h_topRvcDiff4Bad[iskim]->GetXaxis()->SetTitle("rvc(L1)-rvc(TOP(this))");

      //-------------------------------------------------------------------------------------------------------------------------------------------

      h_topGdlRvcDiff1Bad[iskim]  = new TH1I(Form("h_gdl_top_l1_rvc_diff_bad_%s", skim_smap[iskim].c_str()),
                                             "BAD rvc(L1(GDL))-rvc(L1(as reported to TOP))",
                                             10, 1235, 1245);
      h_topGdlRvcDiff1Bad[iskim]->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(L1(as reported to TOP))");

      h_topGdlRvcDiff2Bad[iskim]  = new TH1I(Form("h_gdl_l1_top_t0_this_rvc_diff_bad_%s", skim_smap[iskim].c_str()),
                                             "BAD rvc(TOP(received by GDL))-rvc(TOP(this))",
                                             100, 0, 200);
      h_topGdlRvcDiff2Bad[iskim]->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(this))");

      h_topGdlRvcDiff3Bad[iskim]  = new TH1I(Form("h_gdl_l1_top_t0_prev_rvc_diff_bad_%s", skim_smap[iskim].c_str()),
                                             "BAD rvc(TOP(received by GDL))-rvc(TOP(prev))",
                                             128, 0, 1280);
      h_topGdlRvcDiff3Bad[iskim]->GetXaxis()->SetTitle("rvc(L1(GDL))-rvc(TOP(prev))");

    }
    //-------------------------------------------------------------------------------------------------------------------------------------------

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

    }

    //------------------------

    if (m_histLevel > 1) {

      // these two for loops are nested in this particular way to simplify browsing through the histograms

      for (int histClass = 0; histClass < m_nHistClassesActual; histClass++) {

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

    //-----

    h_topTC2IdVsTC1IdAll[iskim] = new TH2I(Form("h_top_tc2id_vs_tc1id_%s", skim_smap[iskim].c_str()),
                                           "Sorted (by energy) TC2 Id vs TC1 Id", 100, 0, 600, 100, 0, 600);
    h_topTC2IdVsTC1IdAll[iskim]->GetXaxis()->SetTitle("Most energetic TC id");
    h_topTC2IdVsTC1IdAll[iskim]->GetYaxis()->SetTitle("Next to most energetic TC id");

    h_topTC2EnergyVsTC1EnergyAll[iskim] = new TH2I(Form("h_top_tc2en_vs_tc1en_%s", skim_smap[iskim].c_str()),
                                                   "Sorted TC2 energy vs TC1 energy", 100, 0, 1000, 100, 0, 1000);
    h_topTC2EnergyVsTC1EnergyAll[iskim]->GetXaxis()->SetTitle("Most energetic TC energy");
    h_topTC2EnergyVsTC1EnergyAll[iskim]->GetYaxis()->SetTitle("Next to most energetic TC energy");

    h_topTC2ThetaIdVsTC1ThetaIdAll[iskim] = new TH2I(Form("h_top_tc2thetaid_vs_tc1thetaid_%s", skim_smap[iskim].c_str()),
                                                     "Sorted by energy TC2 vs TC1 theta ids", 20, 0, 20,
                                                     20, 0, 20);
    h_topTC2ThetaIdVsTC1ThetaIdAll[iskim]->GetXaxis()->SetTitle("Most energetic TC theta id");
    h_topTC2ThetaIdVsTC1ThetaIdAll[iskim]->GetYaxis()->SetTitle("Next to most energetic TC theta id");

    h_topTC2PhiIdVsTC1PhiIdAll[iskim] = new TH2I(Form("h_top_tc2phiid_vs_tc1phiid_%s", skim_smap[iskim].c_str()),
                                                 "Sorted by energy TC2 vs TC1 phi ids", 36, 1, 37, 36, 1,
                                                 37);
    h_topTC2PhiIdVsTC1PhiIdAll[iskim]->GetXaxis()->SetTitle("Most energetic TC phi id");
    h_topTC2PhiIdVsTC1PhiIdAll[iskim]->GetYaxis()->SetTitle("Next to most energetic TC phi id");

    h_topTCPhiIdVsTCThetaIdAll[iskim] = new TH3I(Form("h_top_tcphiid_vs_tcthetaid_%s", skim_smap[iskim].c_str()),
                                                 "Sorted by energy TC2 vs TC1 theta vs phi ids", 20, 0, 20,
                                                 36, 1, 37, 2, 0, 2);
    h_topTCPhiIdVsTCThetaIdAll[iskim]->GetXaxis()->SetTitle("Most and next to most energetic TCs theta ids");
    h_topTCPhiIdVsTCThetaIdAll[iskim]->GetYaxis()->SetTitle("Most and next to most energetic TCs phi ids");
    h_topTCPhiIdVsTCThetaIdAll[iskim]->GetZaxis()->SetTitle("Most and next to most energetic TCs");

    //-----

    h_topTC2IdVsTC1IdGRLAll[iskim] = new TH2I(Form("h_top_tc2id_vs_tc1id_grl_%s", skim_smap[iskim].c_str()),
                                              "Sorted (by energy) TC2 Id vs TC1 Id", 100, 0, 600, 100, 0, 600);
    h_topTC2IdVsTC1IdGRLAll[iskim]->GetXaxis()->SetTitle("Most energetic TC id");
    h_topTC2IdVsTC1IdGRLAll[iskim]->GetYaxis()->SetTitle("Next to most energetic TC id");

    h_topTC2EnergyVsTC1EnergyGRLAll[iskim] = new TH2I(Form("h_top_tc2en_vs_tc1en_grl_%s", skim_smap[iskim].c_str()),
                                                      "Sorted TC2 energy vs TC1 energy", 100, 0, 1000, 100, 0,
                                                      1000);
    h_topTC2EnergyVsTC1EnergyGRLAll[iskim]->GetXaxis()->SetTitle("Most energetic TC energy");
    h_topTC2EnergyVsTC1EnergyGRLAll[iskim]->GetYaxis()->SetTitle("Next to most energetic TC energy");

    h_topTC2ThetaIdVsTC1ThetaIdGRLAll[iskim] = new TH2I(Form("h_top_tc2thetaid_vs_tc1thetaid_grl_%s", skim_smap[iskim].c_str()),
                                                        "Sorted by energy TC2 vs TC1 theta ids", 20,
                                                        0, 20, 20, 0, 20);
    h_topTC2ThetaIdVsTC1ThetaIdGRLAll[iskim]->GetXaxis()->SetTitle("Most energetic TC theta id");
    h_topTC2ThetaIdVsTC1ThetaIdGRLAll[iskim]->GetYaxis()->SetTitle("Next to most energetic TC theta id");

    h_topTC2PhiIdVsTC1PhiIdGRLAll[iskim] = new TH2I(Form("h_top_tc2phiid_vs_tc1phiid_grl_%s", skim_smap[iskim].c_str()),
                                                    "Sorted by energy TC2 vs TC1 phi ids", 36, 1, 37, 36,
                                                    1, 37);
    h_topTC2PhiIdVsTC1PhiIdGRLAll[iskim]->GetXaxis()->SetTitle("Most energetic TC phi id");
    h_topTC2PhiIdVsTC1PhiIdGRLAll[iskim]->GetYaxis()->SetTitle("Next to most energetic TC phi id");

    h_topTCPhiIdVsTCThetaIdGRLAll[iskim] = new TH3I(Form("h_top_tcphiid_vs_tcthetaid_grl_%s", skim_smap[iskim].c_str()),
                                                    "Sorted by energy TC2 vs TC1 theta vs phi ids", 20,
                                                    0, 20, 36, 1, 37, 2, 0, 2);
    h_topTCPhiIdVsTCThetaIdGRLAll[iskim]->GetXaxis()->SetTitle("Most and next to most energetic TCs theta ids");
    h_topTCPhiIdVsTCThetaIdGRLAll[iskim]->GetYaxis()->SetTitle("Most and next to most energetic TCs phi ids");
    h_topTCPhiIdVsTCThetaIdGRLAll[iskim]->GetZaxis()->SetTitle("Most and next to most energetic TCs");

    //-----

    h_topTC2IdVsTC1Id[iskim] = new TH2I(Form("h_top_tc2id_vs_tc1id_b2b_%s", skim_smap[iskim].c_str()),
                                        "Sorted (by energy) TC2 Id vs TC1 Id", 100, 0, 600, 100, 0, 600);
    h_topTC2IdVsTC1Id[iskim]->GetXaxis()->SetTitle("Most energetic TC id");
    h_topTC2IdVsTC1Id[iskim]->GetYaxis()->SetTitle("Next to most energetic TC id");

    h_topTC2EnergyVsTC1Energy[iskim] = new TH2I(Form("h_top_tc2en_vs_tc1en_b2b_%s", skim_smap[iskim].c_str()),
                                                "Sorted TC2 energy vs TC1 energy", 100, 0, 1000, 100, 0, 1000);
    h_topTC2EnergyVsTC1Energy[iskim]->GetXaxis()->SetTitle("Most energetic TC energy");
    h_topTC2EnergyVsTC1Energy[iskim]->GetYaxis()->SetTitle("Next to most energetic TC energy");

    h_topTC2ThetaIdVsTC1ThetaId[iskim] = new TH2I(Form("h_top_tc2thetaid_vs_tc1thetaid_b2b_%s", skim_smap[iskim].c_str()),
                                                  "Sorted by energy TC2 vs TC1 theta ids", 20, 0, 20, 20, 0,
                                                  20);
    h_topTC2ThetaIdVsTC1ThetaId[iskim]->GetXaxis()->SetTitle("Most energetic TC theta id");
    h_topTC2ThetaIdVsTC1ThetaId[iskim]->GetYaxis()->SetTitle("Next to most energetic TC theta id");

    h_topTC2PhiIdVsTC1PhiId[iskim] = new TH2I(Form("h_top_tc2phiid_vs_tc1phiid_b2b_%s", skim_smap[iskim].c_str()),
                                              "Sorted by energy TC2 vs TC1 phi ids", 36, 1, 37, 36, 1, 37);
    h_topTC2PhiIdVsTC1PhiId[iskim]->GetXaxis()->SetTitle("Most energetic TC phi id");
    h_topTC2PhiIdVsTC1PhiId[iskim]->GetYaxis()->SetTitle("Next to most energetic TC phi id");

    h_topTCPhiIdVsTCThetaId[iskim] = new TH3I(Form("h_top_tcphiid_vs_tcthetaid_b2b_%s", skim_smap[iskim].c_str()),
                                              "Sorted by energy TC2 vs TC1 theta vs phi ids", 20, 0, 20, 36, 1,
                                              37, 2, 0, 2);
    h_topTCPhiIdVsTCThetaId[iskim]->GetXaxis()->SetTitle("Most and next to most energetic TCs theta ids");
    h_topTCPhiIdVsTCThetaId[iskim]->GetYaxis()->SetTitle("Most and next to most energetic TCs phi ids");
    h_topTCPhiIdVsTCThetaId[iskim]->GetZaxis()->SetTitle("Most and next to most energetic TCs");

    //-----

    h_topTC2IdVsTC1IdGRL[iskim] = new TH2I(Form("h_top_tc2id_vs_tc1id_grl_b2b_%s", skim_smap[iskim].c_str()),
                                           "Sorted (by energy) TC2 Id vs TC1 Id", 100, 0, 600, 100, 0, 600);
    h_topTC2IdVsTC1IdGRL[iskim]->GetXaxis()->SetTitle("Most energetic TC id");
    h_topTC2IdVsTC1IdGRL[iskim]->GetYaxis()->SetTitle("Next to most energetic TC id");

    h_topTC2EnergyVsTC1EnergyGRL[iskim] = new TH2I(Form("h_top_tc2en_vs_tc1en_grl_b2b_%s", skim_smap[iskim].c_str()),
                                                   "Sorted TC2 energy vs TC1 energy", 100, 0, 1000, 100, 0, 1000);
    h_topTC2EnergyVsTC1EnergyGRL[iskim]->GetXaxis()->SetTitle("Most energetic TC energy");
    h_topTC2EnergyVsTC1EnergyGRL[iskim]->GetYaxis()->SetTitle("Next to most energetic TC energy");

    h_topTC2ThetaIdVsTC1ThetaIdGRL[iskim] = new TH2I(Form("h_top_tc2thetaid_vs_tc1thetaid_grl_b2b_%s", skim_smap[iskim].c_str()),
                                                     "Sorted by energy TC2 vs TC1 theta ids", 20, 0, 20,
                                                     20, 0, 20);
    h_topTC2ThetaIdVsTC1ThetaIdGRL[iskim]->GetXaxis()->SetTitle("Most energetic TC theta id");
    h_topTC2ThetaIdVsTC1ThetaIdGRL[iskim]->GetYaxis()->SetTitle("Next to most energetic TC theta id");

    h_topTC2PhiIdVsTC1PhiIdGRL[iskim] = new TH2I(Form("h_top_tc2phiid_vs_tc1phiid_grl_b2b_%s", skim_smap[iskim].c_str()),
                                                 "Sorted by energy TC2 vs TC1 phi ids", 36, 1, 37, 36, 1,
                                                 37);
    h_topTC2PhiIdVsTC1PhiIdGRL[iskim]->GetXaxis()->SetTitle("Most energetic TC phi id");
    h_topTC2PhiIdVsTC1PhiIdGRL[iskim]->GetYaxis()->SetTitle("Next to most energetic TC phi id");

    h_topTCPhiIdVsTCThetaIdGRL[iskim] = new TH3I(Form("h_top_tcphiid_vs_tcthetaid_grl_b2b_%s", skim_smap[iskim].c_str()),
                                                 "Sorted by energy TC2 vs TC1 theta vs phi ids", 20, 0, 20,
                                                 36, 1, 37, 2, 0, 2);
    h_topTCPhiIdVsTCThetaIdGRL[iskim]->GetXaxis()->SetTitle("Most and next to most energetic TCs theta ids");
    h_topTCPhiIdVsTCThetaIdGRL[iskim]->GetYaxis()->SetTitle("Most and next to most energetic TCs phi ids");
    h_topTCPhiIdVsTCThetaIdGRL[iskim]->GetZaxis()->SetTitle("Most and next to most energetic TCs");

    //-------------------------------------------------------------------------------------------------------------------------------------------

    h_gdl_ecltop_timing_diff_vs_grl_top_l1[iskim] = new TH2I(Form("h_gdl_ecltop_timing_diff_vs_grl_top_l1_%s",
                                                             skim_smap[iskim].c_str()),
                                                             "(GDL ECL)-(GDL TOP) vs GRL TOP rvc (relative to GDL L1)", 100, -2000, 0, 110, -760, 10240);
    h_gdl_ecltop_timing_diff_vs_grl_top_l1[iskim]->GetXaxis()->SetTitle("GRL TOP rvc relative to GDL L1 (rvc)");
    h_gdl_ecltop_timing_diff_vs_grl_top_l1[iskim]->GetYaxis()->SetTitle("(GDL ECL)-(GDL TOP) t0 difference (ns, 100ns bins)");

    h_gdl_top_l1_vs_grl_top_l1[iskim] = new TH2I(Form("h_gdl_top_l1_vs_grl_top_l1_%s", skim_smap[iskim].c_str()),
                                                 "GDL TOP rvc vs GRL TOP rvc (relative to GDL L1)", 100, -2000,
                                                 0, 128, 0, 1280);
    h_gdl_top_l1_vs_grl_top_l1[iskim]->GetXaxis()->SetTitle("GRL TOP rvc relative to GDL L1 (rvc)");
    h_gdl_top_l1_vs_grl_top_l1[iskim]->GetYaxis()->SetTitle("GDL TOP rvc relative to GDL L1 (rvc)");

    h_grl_ntopslots_vs_ncdcslots[iskim] = new TH2I(Form("h_grl_ntopslots_vs_ncdcslots_%s", skim_smap[iskim].c_str()),
                                                   "GRL TOP slots vs CDC slots", 10, 0, 10, 10, 0, 10);
    h_grl_ntopslots_vs_ncdcslots[iskim]->GetXaxis()->SetTitle("GRL CDC slots");
    h_grl_ntopslots_vs_ncdcslots[iskim]->GetYaxis()->SetTitle("GRL TOP slots");

    h_grl_ncdctopslots_matched[iskim] = new TH1I(Form("h_grl_ncdctopslots_matched_%s", skim_smap[iskim].c_str()),
                                                 "GRL Number of matched TOP and CDC slots", 17, 0, 17);
    h_grl_ncdctopslots_matched[iskim]->GetXaxis()->SetTitle("Number of matched TOP and CDC slots");

    h_grl_topslots_vs_cdcslots_match[iskim] = new TH2I(Form("h_grl_topslots_vs_cdcslots_match_%s", skim_smap[iskim].c_str()),
                                                       "GRL TOP slots vs CDC slots matches", 17, 0, 17, 17,
                                                       0, 17);
    h_grl_topslots_vs_cdcslots_match[iskim]->GetXaxis()->SetTitle("GRL CDC slot");
    h_grl_topslots_vs_cdcslots_match[iskim]->GetYaxis()->SetTitle("GRL TOP slot");

  }
  //-------------------------------------------------------------------------------------------------------------------------------------------

  oldDir->cd();

}

void TRGTOPDQMModule::beginRun()
{

  dirDQM->cd();

  for (int iskim = start_skim_topdqm; iskim < end_skim_topdqm; iskim++) {

    h_topSlotAll[iskim]->Reset();
    h_topSegmentAll[iskim]->Reset();
    h_topNHitsAll[iskim]->Reset();
    h_topLogLAll[iskim]->Reset();
    h_topT0All[iskim]->Reset();

    for (int slot = 0; slot <= 15; slot++) {
      h_topSlotSegment[iskim][slot]->Reset();
      h_topSlotNHits[iskim][slot]->Reset();
      h_topSlotLogL[iskim][slot]->Reset();
      h_topSlotT0[iskim][slot]->Reset();
    }

    h_topSlotVsSegment[iskim]->Reset();

    h_topSlotVsNHits[iskim]->Reset();
    h_topSlotVsLogL[iskim]->Reset();
    h_topSlotVsT0[iskim]->Reset();

    h_topSegmentVsNHits[iskim]->Reset();
    h_topSegmentVsLogL[iskim]->Reset();
    h_topSegmentVsT0[iskim]->Reset();

    h_topNHitsVsLogL[iskim]->Reset();
    h_topNHitsVsT0[iskim]->Reset();
    h_topLogLVsT0[iskim]->Reset();

    h_ecl_gdl_top_timing_diff_both[iskim]->Reset();
    h_ecl_top_top_timing_diff_both[iskim]->Reset();

    if (m_histLevel > 2) {

      h_gdl_top_rvc_vs_top_timing[iskim]->Reset();

      h_gdl_ecltop_rvc_diff[iskim]->Reset();
      h_gdl_cdctop_rvc_diff[iskim]->Reset();

      h_gdl_gdltop_rvc_diff_all[iskim]->Reset();
      h_gdl_comtop_rvc_diff_all[iskim]->Reset();

      h_topRvcDiff1All[iskim]->Reset();
      h_topRvcDiff2All[iskim]->Reset();
      h_topRvcDiff3All[iskim]->Reset();
      h_topRvcDiff4All[iskim]->Reset();

      h_topGdlRvcDiff1All[iskim]->Reset();
      h_topGdlRvcDiff2All[iskim]->Reset();
      h_topGdlRvcDiff3All[iskim]->Reset();

      h_decisionNumberVsNumberDecisionsGood[iskim]->Reset();

      h_ecl_gdl_top_timing_diff_good[iskim]->Reset();
      h_ecl_top_top_timing_diff_good[iskim]->Reset();

      h_gdl_ecltop_timing_diff_vs_toptop_good[iskim]->Reset();
      h_top_ecltop_timing_diff_vs_toptop_good[iskim]->Reset();
      h_gdl_ecltop_timing_diff_vs_ecltop_good[iskim]->Reset();

      h_gdl_gdltop_rvc_diff_good[iskim]->Reset();
      h_gdl_comtop_rvc_diff_good[iskim]->Reset();

      h_topCombinedTimingTopGood[iskim]->Reset();
      h_topNSlotsCombinedTimingTopGood[iskim]->Reset();
      h_topNHitSumGood[iskim]->Reset();
      h_topLogLSumGood[iskim]->Reset();

      h_topRvcDiff1Good[iskim]->Reset();
      h_topRvcDiff2Good[iskim]->Reset();
      h_topRvcDiff3Good[iskim]->Reset();
      h_topRvcDiff4Good[iskim]->Reset();

      h_topGdlRvcDiff1Good[iskim]->Reset();
      h_topGdlRvcDiff2Good[iskim]->Reset();
      h_topGdlRvcDiff3Good[iskim]->Reset();

      h_decisionNumberVsNumberDecisionsBad[iskim]->Reset();

      h_ecl_gdl_top_timing_diff_bad[iskim]->Reset();
      h_ecl_top_top_timing_diff_bad[iskim]->Reset();

      h_gdl_ecltop_timing_diff_vs_toptop_bad[iskim]->Reset();
      h_top_ecltop_timing_diff_vs_toptop_bad[iskim]->Reset();
      h_gdl_ecltop_timing_diff_vs_ecltop_bad[iskim]->Reset();

      h_gdl_gdltop_rvc_diff_bad[iskim]->Reset();
      h_gdl_comtop_rvc_diff_bad[iskim]->Reset();

      h_topCombinedTimingTopBad[iskim]->Reset();
      h_topNSlotsCombinedTimingTopBad[iskim]->Reset();
      h_topNHitSumBad[iskim]->Reset();
      h_topLogLSumBad[iskim]->Reset();

      h_topRvcDiff1Bad[iskim]->Reset();
      h_topRvcDiff2Bad[iskim]->Reset();
      h_topRvcDiff3Bad[iskim]->Reset();
      h_topRvcDiff4Bad[iskim]->Reset();

      h_topGdlRvcDiff1Bad[iskim]->Reset();
      h_topGdlRvcDiff2Bad[iskim]->Reset();
      h_topGdlRvcDiff3Bad[iskim]->Reset();

    }

    h_topCombinedTimingTopAll[iskim]->Reset();
    h_topNSlotsCombinedTimingTopAll[iskim]->Reset();
    h_topNHitSumAll[iskim]->Reset();
    h_topLogLSumAll[iskim]->Reset();

    h_topNSlotsCombinedTimingVsNHitsTopAll[iskim]->Reset();

    h_topTrigType[iskim]->Reset();
    h_topTimingResiduals[iskim]->Reset();
    h_topTimingVariance[iskim]->Reset();

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

    for (int histClass = 0; histClass < m_nHistClassesActual; histClass++) {
      h_N_decision[iskim][histClass]->Reset();

      h_topNHitVsNhit[iskim][histClass]->Reset();
      h_topSlotVsSlot[iskim][histClass]->Reset();
      h_topT0VsT0[iskim][histClass]->Reset();
      h_topSegmentVsSegment[iskim][histClass]->Reset();
      h_topLogLVsLogL[iskim][histClass]->Reset();
    }

    if (m_histLevel > 1) {
      for (int histClass = 0; histClass < m_nHistClassesActual; histClass++) {
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

    h_gdl_ecltop_timing_diff_vs_slot[iskim]->Reset();
    h_gdl_ecltop_timing_diff_vs_segment[iskim]->Reset();
    h_gdl_ecltop_timing_diff_vs_nhits[iskim]->Reset();
    h_gdl_ecltop_timing_diff_vs_logl[iskim]->Reset();

    h_topTC2IdVsTC1IdAll[iskim]->Reset();
    h_topTC2EnergyVsTC1EnergyAll[iskim]->Reset();
    h_topTC2ThetaIdVsTC1ThetaIdAll[iskim]->Reset();
    h_topTC2PhiIdVsTC1PhiIdAll[iskim]->Reset();
    h_topTCPhiIdVsTCThetaIdAll[iskim]->Reset();

    h_topTC2IdVsTC1IdGRLAll[iskim]->Reset();
    h_topTC2EnergyVsTC1EnergyGRLAll[iskim]->Reset();
    h_topTC2ThetaIdVsTC1ThetaIdGRLAll[iskim]->Reset();
    h_topTC2PhiIdVsTC1PhiIdGRLAll[iskim]->Reset();
    h_topTCPhiIdVsTCThetaIdGRLAll[iskim]->Reset();

    h_topTC2IdVsTC1Id[iskim]->Reset();
    h_topTC2EnergyVsTC1Energy[iskim]->Reset();
    h_topTC2ThetaIdVsTC1ThetaId[iskim]->Reset();
    h_topTC2PhiIdVsTC1PhiId[iskim]->Reset();
    h_topTCPhiIdVsTCThetaId[iskim]->Reset();

    h_topTC2IdVsTC1IdGRL[iskim]->Reset();
    h_topTC2EnergyVsTC1EnergyGRL[iskim]->Reset();
    h_topTC2ThetaIdVsTC1ThetaIdGRL[iskim]->Reset();
    h_topTC2PhiIdVsTC1PhiIdGRL[iskim]->Reset();
    h_topTCPhiIdVsTCThetaIdGRL[iskim]->Reset();

    h_gdl_ecltop_timing_diff_vs_grl_top_l1[iskim]->Reset();
    h_gdl_top_l1_vs_grl_top_l1[iskim]->Reset();

    h_grl_ntopslots_vs_ncdcslots[iskim]->Reset();
    h_grl_ncdctopslots_matched[iskim]->Reset();
    h_grl_topslots_vs_cdcslots_match[iskim]->Reset();

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

        //  poor physicist's barrel b2b
        //      if (tcId1 >= 81 && tcId1 <= 512) {
        //  if (tcId2 >= 81 && tcId2 <= 512) {
        //    if (abs(tcId2-tcId1-220) <= 50) {

        if (tcThetaId1 >= 4 && tcThetaId1 <= 15) {
          barrelEcl = true;
          if (tcThetaId2 >= 4 && tcThetaId2 <= 15) {
            if (abs(tcPhiId1 - tcPhiId2) >= 12 && abs(tcPhiId1 - tcPhiId2) <= 24) {
              barrelEclB2B = true;
            }
          }
        }

        for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

          h_topTC2IdVsTC1IdAll[skim[ifill]]->Fill(tcId1, tcId2);
          h_topTC2EnergyVsTC1EnergyAll[skim[ifill]]->Fill(tc1.tcEnergy, tc2.tcEnergy);
          h_topTC2ThetaIdVsTC1ThetaIdAll[skim[ifill]]->Fill(tcThetaId1, tcThetaId2);
          h_topTC2PhiIdVsTC1PhiIdAll[skim[ifill]]->Fill(tcPhiId1, tcPhiId2);
          h_topTCPhiIdVsTCThetaIdAll[skim[ifill]]->Fill(tcThetaId1, tcPhiId1, 0);
          h_topTCPhiIdVsTCThetaIdAll[skim[ifill]]->Fill(tcThetaId2, tcPhiId2, 1);

          if (nCDCSlotsGRL > 0) {
            h_topTC2IdVsTC1IdGRLAll[skim[ifill]]->Fill(tcId1, tcId2);
            h_topTC2EnergyVsTC1EnergyGRLAll[skim[ifill]]->Fill(tc1.tcEnergy, tc2.tcEnergy);
            h_topTC2ThetaIdVsTC1ThetaIdGRLAll[skim[ifill]]->Fill(tcThetaId1, tcThetaId2);
            h_topTC2PhiIdVsTC1PhiIdGRLAll[skim[ifill]]->Fill(tcPhiId1, tcPhiId2);
            h_topTCPhiIdVsTCThetaIdGRLAll[skim[ifill]]->Fill(tcThetaId1, tcPhiId1, 0);
            h_topTCPhiIdVsTCThetaIdGRLAll[skim[ifill]]->Fill(tcThetaId2, tcPhiId2, 1);
          }

          if (barrelEclB2B) {

            h_topTC2IdVsTC1Id[skim[ifill]]->Fill(tcId1, tcId2);
            h_topTC2EnergyVsTC1Energy[skim[ifill]]->Fill(tc1.tcEnergy, tc2.tcEnergy);
            h_topTC2ThetaIdVsTC1ThetaId[skim[ifill]]->Fill(tcThetaId1, tcThetaId2);
            h_topTC2PhiIdVsTC1PhiId[skim[ifill]]->Fill(tcPhiId1, tcPhiId2);
            h_topTCPhiIdVsTCThetaId[skim[ifill]]->Fill(tcThetaId1, tcPhiId1, 0);
            h_topTCPhiIdVsTCThetaId[skim[ifill]]->Fill(tcThetaId2, tcPhiId2, 1);
            if (nCDCSlotsGRL > 0) {
              h_topTC2IdVsTC1IdGRL[skim[ifill]]->Fill(tcId1, tcId2);
              h_topTC2EnergyVsTC1EnergyGRL[skim[ifill]]->Fill(tc1.tcEnergy, tc2.tcEnergy);
              h_topTC2ThetaIdVsTC1ThetaIdGRL[skim[ifill]]->Fill(tcThetaId1, tcThetaId2);
              h_topTC2PhiIdVsTC1PhiIdGRL[skim[ifill]]->Fill(tcPhiId1, tcPhiId2);
              h_topTCPhiIdVsTCThetaIdGRL[skim[ifill]]->Fill(tcThetaId1, tcPhiId1, 0);
              h_topTCPhiIdVsTCThetaIdGRL[skim[ifill]]->Fill(tcThetaId2, tcPhiId2, 1);
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

    for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

      h_grl_ntopslots_vs_ncdcslots[skim[ifill]]->Fill(nCDCSlotsGRL, nTOPSlotsGRL);

      h_grl_ncdctopslots_matched[skim[ifill]]->Fill(nCDCTOPSlotsMatchedGRL);

      for (int i = 0; i < 16; i++) {
        if (grlCDCTOPSlotsMatch[i]) {
          h_grl_topslots_vs_cdcslots_match[skim[ifill]]->Fill(i + 1, i + 1);
        } else if (grlCDCSlots[i]) {
          h_grl_topslots_vs_cdcslots_match[skim[ifill]]->Fill(i + 1, 0.);
        } else if (grlTOPSlots[i]) {
          h_grl_topslots_vs_cdcslots_match[skim[ifill]]->Fill(0., i + 1);
        }
      }
    }
  }

  // normally this should not be happening
  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    if (nCDCSlotsGRL == 0 && nTOPSlotsGRL == 0) h_grl_topslots_vs_cdcslots_match[skim[ifill]]->Fill(0., 0.);
  }

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
  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    h_N_decision[skim[ifill]][histClass]->Fill(nT0Decisions);
  }

  // info from GDL

  if (gdlInfoAvailable) {

    //  const double clkTo1ns = 0.5 / .508877;
    // Feb. 23, 2020  ecltopdiff = (int) ( (double) ecltopdiff * clkTo1ns);

    for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

      if (m_histLevel > 2) {

        int gdl_top_timing_1280 = gdl_top_timing >> 3;
        h_gdl_top_rvc_vs_top_timing[skim[ifill]]->Fill(gdl_top_timing_1280, gdl_toprvc);

        h_gdl_ecltop_rvc_diff[skim[ifill]]->Fill(ecl_top_rvc_diff);
        h_gdl_cdctop_rvc_diff[skim[ifill]]->Fill(cdc_top_rvc_diff);

        h_gdl_gdltop_rvc_diff_all[skim[ifill]]->Fill(gdl_top_rvc_diff);
        h_gdl_comtop_rvc_diff_all[skim[ifill]]->Fill(com_top_rvc_diff);
      }

      if (grlInfoAvailable) {
        h_gdl_ecltop_timing_diff_vs_grl_top_l1[skim[ifill]]->Fill(grlTOPL1, ecl_top_timing_diff);
      }

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

        for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

          h_topSlotAll[skim[ifill]]->Fill(slot);
          h_topSegmentAll[skim[ifill]]->Fill(segment);
          h_topNHitsAll[skim[ifill]]->Fill(nHits);
          h_topLogLAll[skim[ifill]]->Fill(logL);
          h_topT0All[skim[ifill]]->Fill(t0);

          // note that the histograms in this section are for ALL combined decisions from 1us circular buffer
          if (gdlInfoAvailable) {
            int top_top_timing_slot = (t0 % 10240);
            int top_gdl_timing_diff_slot = gdl_ecl_timing >= top_top_timing_slot ? gdl_ecl_timing - top_top_timing_slot : gdl_ecl_timing -
                                           top_top_timing_slot  + 10240;

            h_gdl_ecltop_timing_diff_vs_slot[skim[ifill]]->Fill(top_gdl_timing_diff_slot, slot);
            h_gdl_ecltop_timing_diff_vs_segment[skim[ifill]]->Fill(top_gdl_timing_diff_slot, segment);
            h_gdl_ecltop_timing_diff_vs_nhits[skim[ifill]]->Fill(top_gdl_timing_diff_slot, nHits);
            h_gdl_ecltop_timing_diff_vs_logl[skim[ifill]]->Fill(top_gdl_timing_diff_slot, logL);

            if (m_histLevel > 1) {
              h_gdl_ecltop_timing_diff_vs_slot_2ns[skim[ifill]][histClass]->Fill(top_gdl_timing_diff_slot, slot);
              h_gdl_ecltop_timing_diff_vs_segment_2ns[skim[ifill]][histClass]->Fill(top_gdl_timing_diff_slot, segment);
              h_gdl_ecltop_timing_diff_vs_nhits_2ns[skim[ifill]][histClass]->Fill(top_gdl_timing_diff_slot, nHits);
              h_gdl_ecltop_timing_diff_vs_logl_2ns[skim[ifill]][histClass]->Fill(top_gdl_timing_diff_slot, logL);
            }
          }

          if (slot >= 1 && slot <= 16) {
            if (segment >= 1 && segment <= 10) {
              h_topSlotSegment[skim[ifill]][slot - 1]->Fill(segment);
              h_topSlotNHits[skim[ifill]][slot - 1]->Fill(nHits);
              h_topSlotLogL[skim[ifill]][slot - 1]->Fill(logL);
              h_topSlotT0[skim[ifill]][slot - 1]->Fill(t0);

              h_topSlotVsSegment[skim[ifill]]->Fill(segment, slot);

              h_topSlotVsNHits[skim[ifill]]->Fill(nHits, slot);
              h_topSlotVsLogL[skim[ifill]]->Fill(logL, slot);
              h_topSlotVsT0[skim[ifill]]->Fill(t0, slot);

              h_topSegmentVsNHits[skim[ifill]]->Fill(nHits, segment);
              h_topSegmentVsLogL[skim[ifill]]->Fill(logL, segment);
              h_topSegmentVsT0[skim[ifill]]->Fill(t0, segment);

              h_topNHitsVsLogL[skim[ifill]]->Fill(logL, nHits);
              h_topNHitsVsT0[skim[ifill]]->Fill(t0, nHits);
              h_topLogLVsT0[skim[ifill]]->Fill(t0, logL);

            }
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
        for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
          h_gdl_top_l1_vs_grl_top_l1[skim[ifill]]->Fill(grlTOPL1, topRvcDiff4);
        }
      }

      for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

        h_topCombinedTimingTopAll[skim[ifill]]->Fill(topCombinedTimingTop);
        h_topNSlotsCombinedTimingTopAll[skim[ifill]]->Fill(topNSlotsCombinedTimingTop);
        h_topNHitSumAll[skim[ifill]]->Fill((int) hitsPerSlot);
        h_topLogLSumAll[skim[ifill]]->Fill((int) logLPerSlot);

        h_topNSlotsCombinedTimingVsNHitsTopAll[skim[ifill]]->Fill(topNHitSum, topNSlotsCombinedTimingTop);

        h_topTrigType[skim[ifill]]->Fill(topTrigType);
        h_topTimingResiduals[skim[ifill]]->Fill(topCombinedTimingTopResidual);
        h_topTimingVariance[skim[ifill]]->Fill(topTimingVar);

        if (m_histLevel > 2) {
          h_topRvcDiff1All[skim[ifill]]->Fill(topRvcDiff1);
          h_topRvcDiff2All[skim[ifill]]->Fill(topRvcDiff2);
          h_topRvcDiff3All[skim[ifill]]->Fill(topRvcDiff3);
          h_topRvcDiff4All[skim[ifill]]->Fill(topRvcDiff4);
        }
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
        for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
          if (m_histLevel > 2) {
            h_topGdlRvcDiff1All[skim[ifill]]->Fill(topGdlRvcDiff1);
            h_topGdlRvcDiff2All[skim[ifill]]->Fill(topGdlRvcDiff2);
            h_topGdlRvcDiff3All[skim[ifill]]->Fill(topGdlRvcDiff3);
          }

          h_ecl_gdl_top_timing_diff_both[skim[ifill]]->Fill(ecl_gdl_top_timing_combined_diff);
          h_ecl_top_top_timing_diff_both[skim[ifill]]->Fill(ecl_top_top_timing_combined_diff);
        }
      }

      if (m_histLevel > 2) {

        // something odd is going on...
        if (top_top_timing_combined_diff == 0) {

          for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

            h_decisionNumberVsNumberDecisionsGood[skim[ifill]]->Fill(nT0Decisions, t0DecisionNumber + 1);

            if (t0DecisionNumber == 0) {

              h_ecl_gdl_top_timing_diff_good[skim[ifill]]->Fill(ecl_gdl_top_timing_combined_diff);
              h_ecl_top_top_timing_diff_good[skim[ifill]]->Fill(ecl_top_top_timing_combined_diff);

              h_gdl_ecltop_timing_diff_vs_toptop_good[skim[ifill]]->Fill(ecl_gdl_top_timing_combined_diff, top_top_timing_combined_diff);
              h_top_ecltop_timing_diff_vs_toptop_good[skim[ifill]]->Fill(ecl_top_top_timing_combined_diff, top_top_timing_combined_diff);
              h_gdl_ecltop_timing_diff_vs_ecltop_good[skim[ifill]]->Fill(ecl_gdl_top_timing_combined_diff, ecl_top_top_timing_combined_diff);

              h_topCombinedTimingTopGood[skim[ifill]]->Fill(topCombinedTimingTop);
              h_topNSlotsCombinedTimingTopGood[skim[ifill]]->Fill(topNSlotsCombinedTimingTop);
              h_topNHitSumGood[skim[ifill]]->Fill((int) hitsPerSlot);
              h_topLogLSumGood[skim[ifill]]->Fill((int) logLPerSlot);

              h_topRvcDiff1Good[skim[ifill]]->Fill(topRvcDiff1);
              h_topRvcDiff2Good[skim[ifill]]->Fill(topRvcDiff2);
              h_topRvcDiff3Good[skim[ifill]]->Fill(topRvcDiff3);
              h_topRvcDiff4Good[skim[ifill]]->Fill(topRvcDiff4);

              h_topGdlRvcDiff1Good[skim[ifill]]->Fill(topGdlRvcDiff1);
              h_topGdlRvcDiff2Good[skim[ifill]]->Fill(topGdlRvcDiff2);
              h_topGdlRvcDiff3Good[skim[ifill]]->Fill(topGdlRvcDiff3);

              h_gdl_gdltop_rvc_diff_good[skim[ifill]]->Fill(gdl_top_rvc_diff);
              h_gdl_comtop_rvc_diff_good[skim[ifill]]->Fill(com_top_rvc_diff);
            }
          }
        } else {

          for (unsigned ifill = 0; ifill < skim.size(); ifill++) {

            h_decisionNumberVsNumberDecisionsBad[skim[ifill]]->Fill(nT0Decisions, t0DecisionNumber + 1);

            if (t0DecisionNumber == 0) {
              h_ecl_gdl_top_timing_diff_bad[skim[ifill]]->Fill(ecl_gdl_top_timing_combined_diff);
              h_ecl_top_top_timing_diff_bad[skim[ifill]]->Fill(ecl_top_top_timing_combined_diff);

              h_gdl_ecltop_timing_diff_vs_toptop_bad[skim[ifill]]->Fill(ecl_gdl_top_timing_combined_diff, top_top_timing_combined_diff);
              h_top_ecltop_timing_diff_vs_toptop_bad[skim[ifill]]->Fill(ecl_top_top_timing_combined_diff, top_top_timing_combined_diff);
              h_gdl_ecltop_timing_diff_vs_ecltop_bad[skim[ifill]]->Fill(ecl_gdl_top_timing_combined_diff, ecl_top_top_timing_combined_diff);

              h_topCombinedTimingTopBad[skim[ifill]]->Fill(topCombinedTimingTop);
              h_topNSlotsCombinedTimingTopBad[skim[ifill]]->Fill(topNSlotsCombinedTimingTop);
              h_topNHitSumBad[skim[ifill]]->Fill((int) hitsPerSlot);
              h_topLogLSumBad[skim[ifill]]->Fill((int) logLPerSlot);

              h_topRvcDiff1Bad[skim[ifill]]->Fill(topRvcDiff1);
              h_topRvcDiff2Bad[skim[ifill]]->Fill(topRvcDiff2);
              h_topRvcDiff3Bad[skim[ifill]]->Fill(topRvcDiff3);
              h_topRvcDiff4Bad[skim[ifill]]->Fill(topRvcDiff4);

              h_topGdlRvcDiff1Bad[skim[ifill]]->Fill(topGdlRvcDiff1);
              h_topGdlRvcDiff2Bad[skim[ifill]]->Fill(topGdlRvcDiff2);
              h_topGdlRvcDiff3Bad[skim[ifill]]->Fill(topGdlRvcDiff3);

              h_gdl_gdltop_rvc_diff_bad[skim[ifill]]->Fill(gdl_top_rvc_diff);
              h_gdl_comtop_rvc_diff_bad[skim[ifill]]->Fill(com_top_rvc_diff);
            }
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

