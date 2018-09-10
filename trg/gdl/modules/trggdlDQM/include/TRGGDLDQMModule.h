#ifndef TRCGDLDQMMODULE_h
#define TRCGDLDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <TH2I.h>
#include <TH1I.h>

#define N_BITS_RESERVED 200

namespace Belle2 {

  class TRGGDLDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGGDLDQMModule();
    //! Destrunctor
    virtual ~TRGGDLDQMModule() {}

  public:
    //! initialize
    virtual void initialize();
    //! begin Run
    virtual void beginRun();
    //! Event
    virtual void event();
    //! End Run
    virtual void endRun();
    //! terminate
    virtual void terminate() {}
    //! Define Histogram
    virtual void defineHisto();

  protected:
    //! timestamp diff from gdlL1 to comL1 in LSB8nsec
    TH1I* h_c8_gdlL1TocomL1;
    //! timestamp diff from fit on fam to gdlL1 in LSB8nsec
    TH1I* h_c8_eclTogdlL1;
    //! timestamp diff from ecl_timing to gdlL1 in LSB8nsec
    TH1I* h_c8_ecl8mToGDL;
    //! timestamp diff from fit on fam to GDL in LSB8nsec
    TH1I* h_c8_eclToGDL;
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

  };

}

#endif
