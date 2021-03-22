//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGTOPDQMModule.h
// Section  : TRG GRL
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : DQM module for TRGTOP
//---------------------------------------------------------------
#ifndef TRGGRLDQMMODULE_h
#define TRGGRLDQMMODULE_h

#include <framework/core/HistoModule.h>

#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <trg/gdl/TrgBit.h>
#include <framework/database/DBObjPtr.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>

#include "trg/grl/dataobjects/TRGGRLUnpackerStore.h"

#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include "trg/ecl/TrgEclMapping.h"

#include "trg/top/dataobjects/TRGTOPUnpackerStore.h"
#include "trg/top/dataobjects/TRGTOPSlotTiming.h"

#include <string>
#include <algorithm>

#include <TH1I.h>
#include <TH2I.h>
#include <TH3I.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  class TRGTOPDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGTOPDQMModule();
    //! Destructor
    virtual ~TRGTOPDQMModule() {}

  public:

    static constexpr double clk127To1ns = 7.8;

    //! initialize
    virtual void initialize() override;
    //! begin Run
    virtual void beginRun() override;
    //! Event
    virtual void event() override;
    //! End Run
    virtual void endRun() override;
    //! terminate
    virtual void terminate() override {}
    //! Define Histogram
    virtual void defineHisto() override;

  protected:
    //! TDirectory
    TDirectory* oldDir = nullptr;
    //! TDirectory
    TDirectory* dirDQM = nullptr;

    // number of skims
    static const int nskim_topdqm = 11;

    // number of histogram classes
    static const int nHistClasses = 3;
    int m_nHistClassesActual = 3;

    // histogramming level
    int m_histLevel = 3;

    int start_skim_topdqm = 0;
    int end_skim_topdqm = 0;
    int m_skim = -1;

    TH2I* h_top_nt0decisions_vs_hist_class[nskim_topdqm] = {nullptr};

    TH1I* h_top_gdl_match_decision_number[nskim_topdqm] = {nullptr};

    /** Number of TOP L1 decisions */
    TH1I* h_N_decision[nskim_topdqm][nHistClasses] = {nullptr};

    /** Combined t0 decisions: all, the earliest and second best **/

    TH1I* h_topSlotSegment[nskim_topdqm][16] = {nullptr};
    TH1I* h_topSlotNHits[nskim_topdqm][16] = {nullptr};
    TH1I* h_topSlotLogL[nskim_topdqm][16] = {nullptr};
    TH1I* h_topSlotT0[nskim_topdqm][16] = {nullptr};

    TH1I* h_topSlotAll[nskim_topdqm] = {nullptr};
    TH1I* h_topSegmentAll[nskim_topdqm] = {nullptr};
    TH1I* h_topNHitsAll[nskim_topdqm] = {nullptr};
    TH1I* h_topLogLAll[nskim_topdqm] = {nullptr};
    TH1I* h_topT0All[nskim_topdqm] = {nullptr};

    TH2I* h_topSlotVsSegment[nskim_topdqm] = {nullptr};

    TH2I* h_topSlotVsNHits[nskim_topdqm] = {nullptr};
    TH2I* h_topSlotVsLogL[nskim_topdqm] = {nullptr};
    TH2I* h_topSlotVsT0[nskim_topdqm] = {nullptr};

    TH2I* h_topSegmentVsNHits[nskim_topdqm] = {nullptr};
    TH2I* h_topSegmentVsLogL[nskim_topdqm] = {nullptr};
    TH2I* h_topSegmentVsT0[nskim_topdqm] = {nullptr};

    TH2I* h_topNHitsVsLogL[nskim_topdqm] = {nullptr};
    TH2I* h_topNHitsVsT0[nskim_topdqm] = {nullptr};
    TH2I* h_topLogLVsT0[nskim_topdqm] = {nullptr};

    TH1I* h_ecl_gdl_top_timing_diff_both[nskim_topdqm] = {nullptr};
    TH1I* h_ecl_top_top_timing_diff_both[nskim_topdqm] = {nullptr};

    TH2I* h_decisionNumberVsNumberDecisionsGood[nskim_topdqm] = {nullptr};
    TH2I* h_decisionNumberVsNumberDecisionsBad[nskim_topdqm] = {nullptr};

    TH1I* h_ecl_gdl_top_timing_diff_good[nskim_topdqm] = {nullptr};
    TH1I* h_ecl_top_top_timing_diff_good[nskim_topdqm] = {nullptr};

    TH2I* h_top_ecltop_timing_diff_vs_toptop_good[nskim_topdqm] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_toptop_good[nskim_topdqm] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_ecltop_good[nskim_topdqm] = {nullptr};

    TH1I* h_topCombinedTimingTopGood[nskim_topdqm] = {nullptr};
    TH1I* h_topNSlotsCombinedTimingTopGood[nskim_topdqm] = {nullptr};
    TH1I* h_topNHitSumGood[nskim_topdqm] = {nullptr};
    TH1I* h_topLogLSumGood[nskim_topdqm] = {nullptr};

    TH1I* h_topCombinedTimingTopAll[nskim_topdqm] = {nullptr};
    TH1I* h_topNSlotsCombinedTimingTopAll[nskim_topdqm] = {nullptr};
    TH1I* h_topNHitSumAll[nskim_topdqm] = {nullptr};
    TH1I* h_topLogLSumAll[nskim_topdqm] = {nullptr};

    TH2I* h_topNSlotsCombinedTimingVsNHitsTopAll[nskim_topdqm] = {nullptr};

    TH2I* h_topNHitVsNhit[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_topSlotVsSlot[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_topT0VsT0[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_topSegmentVsSegment[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_topLogLVsLogL[nskim_topdqm][nHistClasses] = {nullptr};

    TH1I* h_topTrigType[nskim_topdqm] = {nullptr};
    TH1I* h_topTimingResiduals[nskim_topdqm] = {nullptr};
    TH1I* h_topTimingVariance[nskim_topdqm] = {nullptr};

    TH1I* h_topRvcDiff1All[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff2All[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff3All[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff4All[nskim_topdqm] = {nullptr};

    TH1I* h_topGdlRvcDiff1All[nskim_topdqm] = {nullptr};
    TH1I* h_topGdlRvcDiff2All[nskim_topdqm] = {nullptr};
    TH1I* h_topGdlRvcDiff3All[nskim_topdqm] = {nullptr};

    TH1I* h_topRvcDiff1Good[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff2Good[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff3Good[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff4Good[nskim_topdqm] = {nullptr};

    TH1I* h_topGdlRvcDiff1Good[nskim_topdqm] = {nullptr};
    TH1I* h_topGdlRvcDiff2Good[nskim_topdqm] = {nullptr};
    TH1I* h_topGdlRvcDiff3Good[nskim_topdqm] = {nullptr};

    TH1I* h_ecl_gdl_top_timing_diff_bad[nskim_topdqm] = {nullptr};
    TH1I* h_ecl_top_top_timing_diff_bad[nskim_topdqm] = {nullptr};

    TH2I* h_top_ecltop_timing_diff_vs_toptop_bad[nskim_topdqm] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_toptop_bad[nskim_topdqm] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_ecltop_bad[nskim_topdqm] = {nullptr};

    TH1I* h_topCombinedTimingTopBad[nskim_topdqm] = {nullptr};
    TH1I* h_topNSlotsCombinedTimingTopBad[nskim_topdqm] = {nullptr};
    TH1I* h_topNHitSumBad[nskim_topdqm] = {nullptr};
    TH1I* h_topLogLSumBad[nskim_topdqm] = {nullptr};

    TH1I* h_topRvcDiff1Bad[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff2Bad[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff3Bad[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff4Bad[nskim_topdqm] = {nullptr};

    TH1I* h_topGdlRvcDiff1Bad[nskim_topdqm] = {nullptr};
    TH1I* h_topGdlRvcDiff2Bad[nskim_topdqm] = {nullptr};
    TH1I* h_topGdlRvcDiff3Bad[nskim_topdqm] = {nullptr};

    TH1I* h_topCombinedTimingTop[nskim_topdqm][nHistClasses] = {nullptr};
    TH1I* h_topNSlotsCombinedTimingTop[nskim_topdqm][nHistClasses] = {nullptr};
    TH1I* h_topNHitSum[nskim_topdqm][nHistClasses] = {nullptr};
    TH1I* h_topNHitBestSlot[nskim_topdqm][nHistClasses] = {nullptr};
    //    TH1I* h_topT0DecisionNumberBestSlot[nskim_topdqm][nHistClasses] = {nullptr};
    TH1I* h_topLogLSum[nskim_topdqm][nHistClasses] = {nullptr};

    TH1I* h_gdl_ecltop_timing_diff_5ns[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_cdctop_timing_diff_5ns[nskim_topdqm] = {nullptr};

    TH1I* h_gdl_ecltop_timing_diff_2ns[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_ecltop_timing_diff_grl_matched_2ns[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_ecltop_timing_diff_no_grl_matched_2ns[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_ecltop_timing_diff_no_grl_at_all_2ns[nskim_topdqm] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_nslots_2ns[nskim_topdqm] = {nullptr};
    TH1I* h_top_ecltop_timing_diff_combined_2ns[nskim_topdqm] = {nullptr};
    TH1I* h_top_ecltop_timing_diff_best_slot_2ns[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_cdctop_timing_diff_2ns[nskim_topdqm] = {nullptr};

    TH1I* h_top_ecltop_timing_diff_2ns[nskim_topdqm][nHistClasses] = {nullptr};
    TH1I* h_top_ecltop_timing_combined_diff_2ns[nskim_topdqm][nHistClasses] = {nullptr};
    TH1I* h_top_ecltop_timing_best_slot_diff_2ns[nskim_topdqm][nHistClasses] = {nullptr};

    TH2I* h_top_ecltop_timing_diff_vs_toptop[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_toptop[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_ecltop[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_ecltop_best_slot[nskim_topdqm][nHistClasses] = {nullptr};

    TH2I* h_gdl_top_rvc_vs_top_timing[nskim_topdqm] = {nullptr};

    TH1I* h_gdl_ecltop_rvc_diff[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_cdctop_rvc_diff[nskim_topdqm] = {nullptr};

    TH1I* h_gdl_gdltop_rvc_diff_all[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_comtop_rvc_diff_all[nskim_topdqm] = {nullptr};

    TH1I* h_gdl_gdltop_rvc_diff_good[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_comtop_rvc_diff_good[nskim_topdqm] = {nullptr};

    TH1I* h_gdl_gdltop_rvc_diff_bad[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_comtop_rvc_diff_bad[nskim_topdqm] = {nullptr};

    TH2I* h_gdl_ecltop_timing_diff_vs_slot[nskim_topdqm] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_segment[nskim_topdqm] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_nhits[nskim_topdqm] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_logl[nskim_topdqm] = {nullptr};

    TH2I* h_gdl_ecltop_timing_diff_vs_slot_2ns[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_segment_2ns[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_nhits_2ns[nskim_topdqm][nHistClasses] = {nullptr};
    TH2I* h_gdl_ecltop_timing_diff_vs_logl_2ns[nskim_topdqm][nHistClasses] = {nullptr};

    TH2I* h_topTC2IdVsTC1IdAll[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2EnergyVsTC1EnergyAll[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2ThetaIdVsTC1ThetaIdAll[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2PhiIdVsTC1PhiIdAll[nskim_topdqm] = {nullptr};
    TH3I* h_topTCPhiIdVsTCThetaIdAll[nskim_topdqm] = {nullptr};

    TH2I* h_topTC2IdVsTC1IdGRLAll[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2EnergyVsTC1EnergyGRLAll[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2ThetaIdVsTC1ThetaIdGRLAll[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2PhiIdVsTC1PhiIdGRLAll[nskim_topdqm] = {nullptr};
    TH3I* h_topTCPhiIdVsTCThetaIdGRLAll[nskim_topdqm] = {nullptr};

    TH2I* h_topTC2IdVsTC1Id[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2EnergyVsTC1Energy[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2ThetaIdVsTC1ThetaId[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2PhiIdVsTC1PhiId[nskim_topdqm] = {nullptr};
    TH3I* h_topTCPhiIdVsTCThetaId[nskim_topdqm] = {nullptr};

    TH2I* h_topTC2IdVsTC1IdGRL[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2EnergyVsTC1EnergyGRL[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2ThetaIdVsTC1ThetaIdGRL[nskim_topdqm] = {nullptr};
    TH2I* h_topTC2PhiIdVsTC1PhiIdGRL[nskim_topdqm] = {nullptr};
    TH3I* h_topTCPhiIdVsTCThetaIdGRL[nskim_topdqm] = {nullptr};

    TH2I* h_gdl_ecltop_timing_diff_vs_grl_top_l1[nskim_topdqm] = {nullptr};
    TH2I* h_gdl_top_l1_vs_grl_top_l1[nskim_topdqm] = {nullptr};

    TH2I* h_grl_ntopslots_vs_ncdcslots[nskim_topdqm] = {nullptr};
    TH1I* h_grl_ncdctopslots_matched[nskim_topdqm] = {nullptr};
    TH2I* h_grl_topslots_vs_cdcslots_match[nskim_topdqm] = {nullptr};

    //condition database for GDL unpacker
    DBObjPtr<TRGGDLDBUnpacker> m_gdlUnpacker;

    int gdlLeafBitMap[320] = {0};
    char gdlLeafNames[320][100] = {0};

    int gdl_e_timtype = 0;
    int gdl_e_gdll1rvc = 0;
    int gdl_e_coml1rvc = 0;
    int gdl_e_toptiming = 0;
    int gdl_e_ecltiming = 0;
    int gdl_e_cdctiming = 0;
    int gdl_e_toprvc = 0;
    int gdl_e_eclrvc = 0;
    int gdl_e_cdcrvc = 0;

    // event information

    //    unsigned n_clocks = 0;
    unsigned m_evtno = 0;
    unsigned m_exp = 0;
    unsigned m_run = 0;

    std::vector<int> skim;

    // ECL TRG
    //! Trg ECL Unpacker TC output
    StoreArray<TRGECLUnpackerStore> trgeclHitArray;

    struct tcEcl {
      int tcId;
      double tcEnergy;
      double tcTiming;
      int tcThetaId;
      int tcPhiId;
    };

    std::vector<tcEcl> tcEclList;

    struct largestEnergy {
      inline bool operator()(tcEcl const& a, tcEcl const& b)
      {
        double i = a.tcEnergy;
        double j = b.tcEnergy;
        return i > j;
      }
    };

    //

    struct slotDecision {
      int slot;
      int segment;
      int nHits;
      int logL;
      int t0;
    };

    std::vector<slotDecision> slotDecisionList;

    struct largestNHits {
      inline bool operator()(slotDecision const& a, slotDecision const& b)
      {
        double i = a.nHits;
        double j = b.nHits;
        return i > j;
      }
    };

    // various parameters and their interpretation

    bool m_generatePostscriptFile;
    std::string m_postScriptFileName;

    bool m_doECLCorrelations;
    bool m_doGDLCorrelations;
    bool m_doGRLCorrelations;

    bool m_requireEclBarrel;
    bool m_requireEclBarrelB2B;

    bool m_requireCDC2DTrack;

    // names of skims from hlt/softwaretrigger/scripts/softwaretrigger/db_access.py
    std::string skim_menu[nskim_topdqm] = {
      "all",
      "software_trigger_cut&skim&accept_hadron",
      "software_trigger_cut&skim&accept_tau_tau",
      "software_trigger_cut&skim&accept_mumu_1trk",
      "software_trigger_cut&skim&accept_mumu_2trk",
      "software_trigger_cut&skim&accept_gamma_gamma",
      "software_trigger_cut&skim&accept_bhabha",
      "software_trigger_cut&skim&accept_hadronb",
      "software_trigger_cut&skim&accept_hadronb1",
      "software_trigger_cut&skim&accept_hadronb2",
      "software_trigger_cut&skim&accept_mumutight"
    };

    // names of histograms
    std::string skim_smap[nskim_topdqm] = {
      "all",
      "hadron",
      "tautau",
      "mumu1trk",
      "mumu2trk",
      "gammagamma",
      "bhabha",
      "hadronb",
      "hadronb1",
      "hadronb2",
      "mumutight"
    };

  };

}

#endif
