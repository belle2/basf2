#ifndef TRCGDLDQMMODULE_h
#define TRCGDLDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <stdlib.h>
#include <iostream>

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
    virtual void initialize();
    //! begin Run
    virtual void beginRun() {}
    //! Event
    virtual void event();
    //! End Run
    virtual void endRun() {}
    //! terminate
    virtual void terminate() {}
    //! Define Histogram
    virtual void defineHisto();

  private:
    //! revoclk diff from gdlL1 to comL1 in LSB8nsec
    TH1I* h_c8_gdlL1TocomL1;
    //! revoclk diff from rvcout to gdlL1 in LSB8nsec
    TH1I* h_c8_rvcoutTogdlL1;
    //! revoclk diff from fit on fam to gdlL1 in LSB8nsec
    TH1I* h_c8_eclTogdlL1;
    //! revoclk diff from ecl_timing to gdlL1 in LSB8nsec
    TH1I* h_c8_ecl8mToGDL;
    //! revoclk diff from fit on fam to GDL in LSB8nsec
    TH1I* h_c8_eclToGDL;
    //! revoclk diff from cdc_timing to comL1 in LSB2nsec
    TH1I* h_c2_cdcTocomL1;
    //! revoclk diff from cdc_timing to comL1 in nsec
    TH1D* h_ns_cdcTocomL1;
    //! revoclk diff from cdc_timing to gdlL1 in nsec
    TH1D* h_ns_cdcTogdlL1;
    //! revoclk diff from top_timing to ecl_timing in nsec
    TH1D* h_ns_topToecl;
    //! revoclk diff from top_timing to cdc_timing in nsec
    TH1D* h_ns_topTocdc;
    //! rvc from cdc_timing to ecl_timing to L1 in LSB2nsec
    TH1I* h_c2_cdcToecl;
    //! revoclk diff from cdc_timing to ecl_timing in nsec
    TH1D* h_ns_cdcToecl;

    //! input bits
    TH1I* h_inp;
    //! itd bits
    TH1I* h_itd;
    //! ftd bits
    TH1I* h_ftd;
    //! psn bits
    TH1I* h_psn;
    //! timtype
    TH1I* h_timtype;

    TDirectory* oldDir;
    TDirectory* dirDQM;

    //private:
    //StoreArray<TRGGDLUnpackerStore> store;
  };

}

#endif
