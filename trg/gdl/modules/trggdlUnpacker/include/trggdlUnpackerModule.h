//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : trggdlUnpackerModule.h
// Section  : TRG TRG
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRG TRG Unpacker Module
//---------------------------------------------------------------
// 1.00 : 2017/07/28 : First version
//---------------------------------------------------------------

#ifndef TRGGDLUNPACKER_H
#define TRGGDLUNPACKER_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/gdl/dataobjects/TRGGDLUnpackerStore.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
#include <framework/dbobjects/TRGGDLDBBadrun.h>

namespace Belle2 {

  namespace GDL {

    /** enum of GDLTimingType **/
    enum EGDLTimingType {
      e_tt_non,
      e_tt_cdc,
      e_tt_ecl,
      e_tt_top,
      e_tt_psnm,
      e_tt_rand,
      e_tt_dphy
    };



    /*! A module of TRG TRG Unpacker */
    class TRGGDLUnpackerModule : public Module {

    public:

      /** Constructor */
      TRGGDLUnpackerModule();

      /** Destructor */
      virtual ~TRGGDLUnpackerModule() {}

      /** Initilizes TRGGDLUnpackerModuel.*/
      virtual void initialize() override;

      /** Called event by event.*/
      virtual void event() override;

      /** Called when processing ended.*/
      virtual void terminate() override {}

      /** Called when new run started.*/
      virtual void beginRun() override {}

      /** Called when run ended*/
      virtual void endRun() override {}

      /** returns version of TRGGDLUnpackerModule.*/
      std::string version() const;

      virtual void fillTreeGDLDB(int* buf, int evt);

    private:

      /** flag to select board search mode **/
      bool m_trgReadoutBoardSearch;

      StoreArray<TRGGDLUnpackerStore> store;

      //condition database for unpacer
      DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
      int LeafBitMap[320];
      char LeafNames[320][100];
      //condition database for badrun
      DBObjPtr<TRGGDLDBBadrun> m_badrun;

    };
  }
} // end namespace Belle2

#endif // TRGGDLUnpackerModule_H
