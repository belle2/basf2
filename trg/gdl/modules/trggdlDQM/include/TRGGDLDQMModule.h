#ifndef TRCGDLDQMMODULE_h
#define TRCGDLDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <trg/gdl/TrgBit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <TH2I.h>
#include <TH1I.h>

namespace Belle2 {


  class TRGGDLDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGGDLDQMModule();
    //! Destrunctor
    virtual ~TRGGDLDQMModule() {}

  public:
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
    //number of skims
    static const int nskim_gdldqm = 7;

    //! timestamp diff from gdlL1 to comL1 in LSB8nsec
    TH1I* h_c8_gdlL1TocomL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(top) to gdlL1 in LSB8nsec
    TH1I* h_c8_topTogdlL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(ecl) to gdlL1 in LSB8nsec
    TH1I* h_c8_eclTogdlL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(cdc) to gdlL1 in LSB8nsec
    TH1I* h_c8_cdcTogdlL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from fit on fam to GDL in LSB8nsec
    TH1I* h_c8_ecl8mToGDL[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(top) to GDL in LSB8nsec
    TH1I* h_c8_topToGDL[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(ecl) to GDL in LSB8nsec
    TH1I* h_c8_eclToGDL[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(cdc) to GDL in LSB8nsec
    TH1I* h_c8_cdcToGDL[nskim_gdldqm] = {nullptr};
    //! timestamp diff from cdc_timing to comL1 in LSB2nsec
    TH1I* h_c2_cdcTocomL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from cdc_timing to comL1 in nsec
    TH1D* h_ns_cdcTocomL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from cdc_timing to gdlL1 in nsec
    TH1D* h_ns_cdcTogdlL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from top_timing to ecl_timing in nsec
    TH1D* h_ns_topToecl[nskim_gdldqm] = {nullptr};
    //! timestamp diff from top_timing to cdc_timing in nsec
    TH1D* h_ns_topTocdc[nskim_gdldqm] = {nullptr};
    //! timestamp from cdc_timing to ecl_timing to L1 in LSB2nsec
    TH1I* h_c2_cdcToecl[nskim_gdldqm] = {nullptr};
    //! timestamp diff from cdc_timing to ecl_timing in nsec
    TH1D* h_ns_cdcToecl[nskim_gdldqm] = {nullptr};

    //! input bits
    // TH1I* h_inp;
    // TH1I* h_inp_rise[N_BITS_RESERVED][nskim_gdldqm];
    // TH1I* h_inp_fall[N_BITS_RESERVED][nskim_gdldqm];
    //! itd bits
    TH1I* h_itd[nskim_gdldqm] = {nullptr};
    TH1I* h_itd_rise[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    TH1I* h_itd_fall[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    //! ftd bits
    TH1I* h_ftd[nskim_gdldqm] = {nullptr};
    TH1I* h_ftd_rise[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    TH1I* h_ftd_fall[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    //! psn bits
    TH1I* h_psn[nskim_gdldqm] = {nullptr};
    TH1I* h_psn_rise[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    TH1I* h_psn_fall[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    TH1I* h_psn_extra[nskim_gdldqm] = {nullptr};
    //! timtype
    TH1I* h_timtype[nskim_gdldqm] = {nullptr};
    //! event by event psnm timing distribution
    TH2I* h_p = nullptr;
    //! event by event ftdl timing distribution
    TH2I* h_f = nullptr;
    //! event by event input timing distribution
    TH2I* h_i = nullptr;

    TDirectory* oldDir = nullptr;
    TDirectory* dirDQM = nullptr;

    bool m_eventByEventTimingHistRecord;
    bool m_dumpVcdFile;
    bool m_bitNameOnBinLabel;
    bool m_generatePostscript;
    unsigned m_vcdEventStart = 0;
    unsigned m_vcdNumberOfEvents = 0;
    std::string m_bitConditionToDumpVcd;
    std::string m_postScriptName;
    void fillRiseFallTimings(void);
    void fillOutputExtra(void);

    void genVcd(void);
    bool anaBitCondition(void);
    bool isFired(std::string bitname);
    unsigned n_clocks = 0;
    unsigned evtno = 0;
    unsigned _exp = 0;
    unsigned _run = 0;
    std::vector<int> skim;
    //private:
    //StoreArray<TRGGDLUnpackerStore> store;

    static const int n_output_extra;
    static const char* output_extra[4];

    //condition database for unpacker
    DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
    int LeafBitMap[320] = {0};
    char LeafNames[320][100] = {0};
    int _e_timtype = 0;
    int _e_gdll1rvc = 0;
    int _e_coml1rvc = 0;
    int _e_toptiming = 0;
    int _e_ecltiming = 0;
    int _e_cdctiming = 0;
    int _e_toprvc = 0;
    int _e_eclrvc = 0;
    int _e_cdcrvc = 0;
    int ee_psn[10] = {0};
    int ee_ftd[10] = {0};
    int ee_itd[10] = {0};

    //condition database for input bits
    DBObjPtr<TRGGDLDBInputBits>m_dbinput;
    unsigned  n_inbit = 0;
    char inbitname[320][100] = {""};

    //condition database for output bits
    DBObjPtr<TRGGDLDBFTDLBits> m_dbftdl;
    unsigned  n_outbit = 0;
    char outbitname[320][100]  = {""};

    //name of skim from hlt/softwaretrigger/scripts/softwaretrigger/db_access.py
    std::string skim_menu[nskim_gdldqm] = {
      "all",
      "software_trigger_cut&skim&accept_hadron",
      "software_trigger_cut&skim&accept_tau_tau",
      "software_trigger_cut&skim&accept_mumu_1trk",
      "software_trigger_cut&skim&accept_mumu_2trk",
      "software_trigger_cut&skim&accept_gamma_gamma",
      "software_trigger_cut&skim&accept_bhabha"
    };

    //name of histgrams
    std::string skim_smap[nskim_gdldqm] = {
      "all",
      "hadron",
      "tautau",
      "mumu1trk",
      "mumu2trk",
      "gammagamma",
      "bhabha"
    };

  };

}

#endif
