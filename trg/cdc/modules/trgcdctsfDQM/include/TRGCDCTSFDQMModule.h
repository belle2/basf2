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
    //! nhit
    TH1I* h_nhit;

    TDirectory* oldDir;
    TDirectory* dirDQM;

    bool m_generatePostscript;
    std::string m_postScriptName;
    unsigned _exp;
    unsigned _run;

  };

}

#endif
