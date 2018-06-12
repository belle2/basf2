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

namespace Belle2 {

  namespace GDL {

    /** num of leafs in data_b2l **/
    const int nLeafs = 39;
    /** num of leafs for others **/
    const int nLeafsExtra = 7;
    /** num of clk time window **/
    const int nClks = 48;
    /** num of b2l bits **/
    const int nBits = 640;
    /** leaf names **/
    const char* LeafNames[nLeafs + nLeafsExtra] = {
      "etffmv",
      "l1rvc",
      "timtype",
      "etyp",
      "final",
      "gdll1",
      "etfout",
      "etfvd",
      "toprvc",
      "topvd",
      "topt0",
      "eclmsb7",
      "ecllsb7",
      "cdc_timing",
      "rvcout",
      "rvcout0",
      "comrvc",
      "etnrvc",
      "nim0rvc",
      "etm0rvc",
      "rvc",
      "drvc",
      "dtoprvc",
      "declrvc",
      "dcdcrvc",
      "psn1",
      "etfth2",
      "etfhdt",
      "etfth",
      "psn0",
      "ftd1",
      "etfvdrvc",
      "ftd0",
      "itd2",
      "itd1",
      "itd0",
      "inp2",
      "inp1",
      "inp0",
      "evt", "clk", "coml1", "firmid", "firmver", "b2ldly", "maxrvc"
    };

    /** enum of leafs **/
    enum EBits {
      e_etffmv,
      e_l1rvc,
      e_timtype,
      e_etyp,
      e_final,
      e_gdll1,
      e_etfout,
      e_etfvd,
      e_toprvc,
      e_topvd,
      e_topt0,
      e_eclmsb7,
      e_ecllsb7,
      e_cdc_timing,
      e_rvcout,
      e_rvcout0,
      e_comrvc,
      e_etnrvc,
      e_nim0rvc,
      e_etm0rvc,
      e_rvc,
      e_drvc,
      e_dtoprvc,
      e_declrvc,
      e_dcdcrvc,
      e_psn1,
      e_etfth2,
      e_etfhdt,
      e_etfth,
      e_psn0,
      e_ftd1,
      e_etfvdrvc,
      e_ftd0,
      e_itd2,
      e_itd1,
      e_itd0,
      e_inp2,
      e_inp1,
      e_inp0,
      e_evt, e_clk, e_coml1, e_firmid, e_firmver, e_b2ldly, e_maxrvc
    };

    /** enum of GDLTimingType **/
    enum EGDLTimingType {
      e_tt_non,
      e_tt_cdc,
      e_tt_ecl,
      e_tt_top,
      e_tt_psnm,
      e_tt_rand
    };

    /** bus bit map. (a downto a-b) **/
    const int BitMap[nLeafs][2] = {
      639, 31, // etffmv
      575, 10, // l1rvc
      564, 2,  // timtype
      561, 2,  // etyp
      558, 0,  // final
      557, 0,  // gdll1
      538, 12, // etfout
      525, 0,  // etfvd
      524, 14, // toprvc
      480, 0,  // topvd
      498, 17, // topt0
      479, 6,  // eclmsb7
      472, 6,  // ecllsb7
      465, 12, // cdc_timing
      447, 14, // rvcout
      432,  0, // rvcout0
      431, 11, // comrvc
      419, 11, // etnrvc
      407, 11, // nim0rvc
      395, 11, // etm0rvc
      383, 11, // rvc
      371, 11, // drvc
      359, 11, // dtoprvc
      347, 11, // declrvc
      335, 11, // dcdcrvc
      323, 15, // psn1
      307, 7,  // etfth2
      299, 7,  // etfhdt
      291, 3,  // etfth
      287, 31, // psn0
      255, 15, // ftd1
      234, 10, // etfvdrvc
      223, 31, // ftd0
      191, 31, // itd2
      159, 31, // itd1
      127, 31, // itd0
      95, 31,  // inp2
      63, 31,  // inp1
      31, 31,  // inp0
    };

    void
    setLeafPointersArray(TRGGDLUnpackerStore* store, int** bitArray)
    {
      bitArray[e_etffmv    ] = &(store->m_etffmv);
      bitArray[e_l1rvc     ] = &(store->m_l1rvc);
      bitArray[e_timtype   ] = &(store->m_timtype);
      bitArray[e_etyp      ] = &(store->m_etyp);
      bitArray[e_final     ] = &(store->m_final);
      bitArray[e_gdll1     ] = &(store->m_gdll1);
      bitArray[e_etfout    ] = &(store->m_etfout);
      bitArray[e_etfvd     ] = &(store->m_etfvd);
      bitArray[e_toprvc    ] = &(store->m_toprvc);
      bitArray[e_topvd     ] = &(store->m_topvd);
      bitArray[e_topt0     ] = &(store->m_topt0);
      bitArray[e_eclmsb7   ] = &(store->m_eclmsb7);
      bitArray[e_ecllsb7   ] = &(store->m_ecllsb7);
      bitArray[e_cdc_timing] = &(store->m_cdc_timing);
      bitArray[e_rvcout    ] = &(store->m_rvcout);
      bitArray[e_rvcout0   ] = &(store->m_rvcout0);
      bitArray[e_comrvc    ] = &(store->m_comrvc);
      bitArray[e_etnrvc    ] = &(store->m_etnrvc);
      bitArray[e_nim0rvc   ] = &(store->m_nim0rvc);
      bitArray[e_etm0rvc   ] = &(store->m_etm0rvc);
      bitArray[e_rvc       ] = &(store->m_rvc);
      bitArray[e_drvc      ] = &(store->m_drvc);
      bitArray[e_dtoprvc   ] = &(store->m_dtoprvc);
      bitArray[e_declrvc   ] = &(store->m_declrvc);
      bitArray[e_dcdcrvc   ] = &(store->m_dcdcrvc);
      bitArray[e_psn1      ] = &(store->m_psn1);
      bitArray[e_etfth2    ] = &(store->m_etfth2);
      bitArray[e_etfhdt    ] = &(store->m_etfhdt);
      bitArray[e_etfth     ] = &(store->m_etfth);
      bitArray[e_psn0      ] = &(store->m_psn0);
      bitArray[e_ftd1      ] = &(store->m_ftd1);
      bitArray[e_etfvdrvc  ] = &(store->m_etfvdrvc);
      bitArray[e_ftd0      ] = &(store->m_ftd0);
      bitArray[e_itd2      ] = &(store->m_itd2);
      bitArray[e_itd1      ] = &(store->m_itd1);
      bitArray[e_itd0      ] = &(store->m_itd0);
      bitArray[e_inp2      ] = &(store->m_inp2);
      bitArray[e_inp1      ] = &(store->m_inp1);
      bitArray[e_inp0      ] = &(store->m_inp0);
      bitArray[e_evt       ] = &(store->m_evt);
      bitArray[e_clk       ] = &(store->m_clk);
      bitArray[e_coml1     ] = &(store->m_coml1);
      bitArray[e_firmid    ] = &(store->m_firmid);
      bitArray[e_firmver   ] = &(store->m_firmver);
      bitArray[e_b2ldly    ] = &(store->m_b2ldly);
      bitArray[e_maxrvc    ] = &(store->m_maxrvc);
    }

    /*! A module of TRG TRG Unpacker */
    class TRGGDLUnpackerModule : public Module {

    public:

      /** Constructor */
      TRGGDLUnpackerModule();

      /** Destructor */
      virtual ~TRGGDLUnpackerModule() {}

      /** Initilizes TRGGDLUnpackerModuel.*/
      virtual void initialize();

      /** Called event by event.*/
      virtual void event();

      /** Called when processing ended.*/
      virtual void terminate() {}

      /** Called when new run started.*/
      virtual void beginRun() {}

      /** Called when run ended*/
      virtual void endRun() {}

      /** returns version of TRGGDLUnpackerModule.*/
      std::string version() const;

      /** Unpacker main function.*/
      virtual void fillTreeGDL1(int* buf, int evt);

    private:

      StoreArray<TRGGDLUnpackerStore> store;

    };
  }
} // end namespace Belle2

#endif // TRGGDLUnpackerModule_H
