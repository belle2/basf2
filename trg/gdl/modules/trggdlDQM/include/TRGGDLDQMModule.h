#ifndef TRCGDLDQMMODULE_h
#define TRCGDLDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <trg/gdl/TrgBit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
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
    //! timestamp diff from gdlL1 to comL1 in LSB8nsec
    TH1I* h_c8_gdlL1TocomL1;
    //! timestamp diff from t0(top) to gdlL1 in LSB8nsec
    TH1I* h_c8_topTogdlL1;
    //! timestamp diff from t0(ecl) to gdlL1 in LSB8nsec
    TH1I* h_c8_eclTogdlL1;
    //! timestamp diff from t0(cdc) to gdlL1 in LSB8nsec
    TH1I* h_c8_cdcTogdlL1;
    //! timestamp diff from fit on fam to GDL in LSB8nsec
    TH1I* h_c8_ecl8mToGDL;
    //! timestamp diff from t0(top) to GDL in LSB8nsec
    TH1I* h_c8_topToGDL;
    //! timestamp diff from t0(ecl) to GDL in LSB8nsec
    TH1I* h_c8_eclToGDL;
    //! timestamp diff from t0(cdc) to GDL in LSB8nsec
    TH1I* h_c8_cdcToGDL;
    //! timestamp diff from cdc_timing to comL1 in LSB2nsec
    TH1I* h_c2_cdcTocomL1;
    //! timestamp diff from cdc_timing to comL1 in nsec
    TH1D* h_ns_cdcTocomL1;
    //! timestamp diff from cdc_timing to gdlL1 in nsec
    TH1D* h_ns_cdcTogdlL1;
    //! timestamp diff from top_timing to ecl_timing in nsec
    TH1D* h_ns_topToecl;
    //! timestamp diff from top_timing to cdc_timing in nsec
    TH1D* h_ns_topTocdc;
    //! timestamp from cdc_timing to ecl_timing to L1 in LSB2nsec
    TH1I* h_c2_cdcToecl;
    //! timestamp diff from cdc_timing to ecl_timing in nsec
    TH1D* h_ns_cdcToecl;

    //! input bits
    // TH1I* h_inp;
    // TH1I* h_inp_rise[N_BITS_RESERVED];
    // TH1I* h_inp_fall[N_BITS_RESERVED];
    //! itd bits
    TH1I* h_itd;
    TH1I* h_itd_rise[N_BITS_RESERVED];
    TH1I* h_itd_fall[N_BITS_RESERVED];
    //! ftd bits
    TH1I* h_ftd;
    TH1I* h_ftd_rise[N_BITS_RESERVED];
    TH1I* h_ftd_fall[N_BITS_RESERVED];
    //! psn bits
    TH1I* h_psn;
    TH1I* h_psn_rise[N_BITS_RESERVED];
    TH1I* h_psn_fall[N_BITS_RESERVED];
    TH1I* h_psn_extra;
    //! timtype
    TH1I* h_timtype;
    //! event by event psnm timing distribution
    TH2I* h_p;
    //! event by event ftdl timing distribution
    TH2I* h_f;
    //! event by event input timing distribution
    TH2I* h_i;

    TDirectory* oldDir;
    TDirectory* dirDQM;

    bool m_eventByEventTimingHistRecord;
    bool m_dumpVcdFile;
    bool m_bitNameOnBinLabel;
    bool m_generatePostscript;
    unsigned m_vcdEventStart;
    unsigned m_vcdNumberOfEvents;
    std::string m_bitConditionToDumpVcd;
    std::string m_postScriptName;
    void fillRiseFallTimings(void);
    void fillOutputExtra(void);

    void genVcd(void);
    bool anaBitCondition(void);
    bool isFired(std::string bitname);
    unsigned n_clocks;
    unsigned evtno;
    unsigned _exp;
    unsigned _run;
    //private:
    //StoreArray<TRGGDLUnpackerStore> store;

    static const int n_output_extra;
    static const char* output_extra[4];

    //condition database for unpacer
    DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
    int LeafBitMap[320];
    char LeafNames[320][100];
    int _e_timtype;
    int _e_gdll1rvc;
    int _e_coml1rvc;
    int _e_toptiming;
    int _e_ecltiming;
    int _e_cdctiming;
    int _e_toprvc;
    int _e_eclrvc;
    int _e_cdcrvc;
    int ee_psn[10];
    int ee_ftd[10];
    int ee_itd[10];


  };

}

#endif
