#ifndef TRCGDLSummaryModule_h
#define TRCGDLSummaryModule_h

#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <stdlib.h>
#include <iostream>

#include <TH2I.h>
#include <TH1I.h>

using namespace std;

namespace Belle2 {

  class TRGGDLSummaryModule : public Module {

  public:
    //! Costructor
    TRGGDLSummaryModule();
    //! Destrunctor
    virtual ~TRGGDLSummaryModule() {}

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

  private:

    StoreObjPtr<TRGSummary> GDLResult; /**< output for TRGSummary */

  };

}

#endif
