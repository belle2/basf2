#ifndef TRCCDCTSFDQMMODULE_h
#define TRCCDCTSFDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <trg/cdc/dataobjects/TRGCDCTSFUnpackerStore.h>
#include <trg/cdc/modules/trgcdctsfUnpacker/trgcdctsfUnpackerModule.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <TH2I.h>
#include <TH1I.h>

namespace Belle2 {

  class TRGCDCTSFDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGCDCTSFDQMModule();
    //! Destrunctor
    virtual ~TRGCDCTSFDQMModule() {}

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
    //Total number of TSF hits per event in each superlayer
    TH1I* h_nhit;

    //Total number of hits in each TSF
    TH1I* h_nhit_tsf;

    //Valid type of TSF hits in each superlayer
    TH1I* h_valid;

    //Timing of TSF hits in each superlayer
    TH1I* h_timing;

    //TDirectories
    TDirectory* oldDir;
    TDirectory* dirDQM;

    //flag to save ps file
    bool m_generatePostscript;

    //name of ps file
    std::string m_postScriptName;

    //experiment number
    unsigned _exp;

    //run number
    unsigned _run;

    //TSF module number
    int m_TSFMOD;

    //TSF data store
    StoreArray<TRGCDCTSFUnpackerStore> entAry;

  };

}

#endif
