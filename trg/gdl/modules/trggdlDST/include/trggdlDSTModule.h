#ifndef TRCGDLDSTModule_h
#define TRCGDLDSTModule_h

#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <trg/gdl/dataobjects/TRGGDLDST.h>
#include <framework/database/DBObjPtr.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
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

    //condition database for unpacer
    DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
    int LeafBitMap[320];
    char LeafNames[320][100];
    int _e_timtype  ;
    int _e_gdll1rvc ;
    int _e_coml1rvc ;
    int _e_toprvc   ;
    int _e_eclrvc   ;
    int _e_cdcrvc   ;
    int _e_toptiming;
    int _e_ecltiming;
    int _e_cdctiming;

  };

}

#endif
