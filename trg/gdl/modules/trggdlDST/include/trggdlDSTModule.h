/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef TRCGDLDSTModule_h
#define TRCGDLDSTModule_h

#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/gdl/dataobjects/TRGGDLDST.h>
#include <framework/database/DBObjPtr.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>

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

    //condition database for unpacer
    DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
    int LeafBitMap[320] = {0};
    char LeafNames[320][100] = {{""}};
    int _e_timtype = 0 ;
    int _e_gdll1rvc = 0 ;
    int _e_coml1rvc = 0 ;
    int _e_toprvc = 0  ;
    int _e_eclrvc = 0  ;
    int _e_cdcrvc = 0  ;
    int _e_toptiming = 0;
    int _e_ecltiming = 0;
    int _e_cdctiming = 0;

  };

}

#endif
