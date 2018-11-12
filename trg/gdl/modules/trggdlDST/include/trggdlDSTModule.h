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
    virtual void initialize() override;
    //! begin Run
    virtual void beginRun() override {}
    //! Event
    virtual void event() override;
    //! End Run
    virtual void endRun() override {}
    //! terminate
    virtual void terminate() override {}

  private:

    StoreObjPtr<TRGGDLDST> GDLResult; /**< output for TRGDST */

  };

}

#endif
