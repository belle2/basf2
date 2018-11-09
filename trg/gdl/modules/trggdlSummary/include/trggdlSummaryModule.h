#ifndef TRCGDLSummaryModule_h
#define TRCGDLSummaryModule_h

#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <stdlib.h>
#include <iostream>
#include <framework/database/DBObjPtr.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
#include <trg/gdl/dbobjects/TRGGDLDBPrescales.h>
#include <trg/gdl/dbobjects/TRGGDLDBFTDLBits.h>

#include <TH2I.h>
#include <TH1I.h>

//using namespace std;

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

    //condition database for unpacer
    DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
    int LeafBitMap[320];
    char LeafNames[320][100];
    int _e_timtype;
    int ee_psn[10];
    int ee_ftd[10];
    int ee_itd[10];
    //condition database for prescale
    DBObjPtr<TRGGDLDBPrescales> m_prescales;
    //condition database for bitname
    DBObjPtr<TRGGDLDBFTDLBits> m_ftdlbits;

  };

}

#endif
