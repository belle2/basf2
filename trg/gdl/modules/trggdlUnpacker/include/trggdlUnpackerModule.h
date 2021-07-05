/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

#include <string>
#include <vector>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/gdl/dataobjects/TRGGDLUnpackerStore.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/TRGGDLDBBadrun.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>

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

      /** flag to dump data base map **/
      bool m_print_dbmap;

      /** flag to select board search mode **/
      bool m_trgReadoutBoardSearch;

      StoreArray<TRGGDLUnpackerStore> store;

      //condition database for unpacer
      DBObjPtr<TRGGDLDBUnpacker> m_dbunpacker;
      //condition database for badrun
      DBObjPtr<TRGGDLDBBadrun> m_dbbadrun;

      //variables to store database
      int LeafBitMap[320];
      char LeafNames[320][100];
      int nword_header;
      int n_clocks;
      int nBits;
      int n_leafs;
      int n_leafsExtra;
      int conf;
      int bad_flag;
      int unpack_flag;
      std::vector<std::vector<int>> BitMap;
      std::vector<std::vector<int>> BitMap_extra;
      int conf_map;
      int evt_map;
      int clk_map;
    };
  }
} // end namespace Belle2

#endif // TRGGDLUnpackerModule_H
