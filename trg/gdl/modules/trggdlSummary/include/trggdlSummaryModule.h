#ifndef TRCGDLSummaryModule_h
#define TRCGDLSummaryModule_h

#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/TRGGDLDBPrescales.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>

namespace Belle2 {

  class TRGGDLSummaryModule : public Module {

  public:
    //! Costructor
    TRGGDLSummaryModule();
    //! Destrunctor
    virtual ~TRGGDLSummaryModule() {}

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

    StoreObjPtr<TRGSummary> GDLResult; /**< output for TRGSummary */

    //condition database for unpacer
    DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
    //condition database for prescales
    DBObjPtr<TRGGDLDBPrescales> m_prescales;
    int LeafBitMap[320] = {0};
    char LeafNames[320][100] = {{""}};
    int _e_timtype = 0;
    int _e_rvcout  = 0;
    int ee_psn[10] = {0};
    int ee_ftd[10] = {0};
    int ee_itd[10] = {0};

    /** Debug Level **/
    int _debugLevel;

  };

}

#endif
