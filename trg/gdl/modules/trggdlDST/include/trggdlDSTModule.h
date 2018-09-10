#ifndef TRCGDLDSTModule_h
#define TRCGDLDSTModule_h

#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <trg/gdl/dataobjects/TRGGDLDST.h>
#include <stdlib.h>
#include <iostream>

namespace Belle2 {

  class TRGGDLDSTModule : public Module {

  public:
    //! Costructor
    TRGGDLDSTModule();
    //! Destrunctor
    virtual ~TRGGDLDSTModule() {}

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

    StoreObjPtr<TRGGDLDST> GDLResult; /**< output for TRGDST */

  };

}

#endif
