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

#include "trg/top/dataobjects/TRGTOPUnpackerStore.h"
#include "trg/top/dataobjects/TRGTOPSlotTiming.h"

#include <string>

#include <TH1I.h>
#include <TH2I.h>
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

    //number of skims
    static const int nskim_topdqm = 11;

    int start_skim_topdqm = 0;
    int end_skim_topdqm = 0;
    int m_skim = -1;

    /** Number of TOP L1 decisions */
    TH1I* h_N_decision[nskim_topdqm] = {nullptr};

    /** Combined t0 decisions: all, the earliest and second best **/

    TH1I* h_topSlotSegment[16] = {nullptr};
    TH1I* h_topSlotNHits[16] = {nullptr};
    TH1I* h_topSlotLogL[16] = {nullptr};
    TH1I* h_topSlotT0[16] = {nullptr};

    TH2I* h_topSlotVsSegment = {nullptr};
    TH2I* h_topSlotVsNHits = {nullptr};
    TH2I* h_topSlotVsLogL = {nullptr};
    TH2I* h_topSlotVsT0 = {nullptr};

    TH1I* h_topCombinedTimingTop[nskim_topdqm] = {nullptr};
    TH1I* h_topNSlotsCombinedTimingTop[nskim_topdqm] = {nullptr};
    TH1I* h_topNHitSum[nskim_topdqm] = {nullptr};
    TH1I* h_topLogLSum[nskim_topdqm] = {nullptr};

    TH1I* h_topRvcDiff1[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff2[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff3[nskim_topdqm] = {nullptr};
    TH1I* h_topRvcDiff4[nskim_topdqm] = {nullptr};

    TH1I* h_top_gdl_timing_diff[nskim_topdqm] = {nullptr};

    TH1I* h_gdl_ecltop_timing_diff_5ns[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_cdctop_timing_diff_5ns[nskim_topdqm] = {nullptr};

    TH1I* h_gdl_ecltop_timing_diff_1ns[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_cdctop_timing_diff_2ns[nskim_topdqm] = {nullptr};

    TH1I* h_gdl_gdltop_rvc_diff[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_comtop_rvc_diff[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_ecltop_rvc_diff[nskim_topdqm] = {nullptr};
    TH1I* h_gdl_cdctop_rvc_diff[nskim_topdqm] = {nullptr};

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

    // various parameters and their interpretation

    bool m_generatePostscriptFile;
    std::string m_postScriptFileName;

    bool m_doGDLCorrelations;
    bool m_doGRLCorrelations;

    //names of skims from hlt/softwaretrigger/scripts/softwaretrigger/db_access.py
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

    //name of histgrams
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
