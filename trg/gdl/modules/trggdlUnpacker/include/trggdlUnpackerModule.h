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
    const int nLeafs0 = 39;
    const int nLeafs1 = 28;
    /** num of leafs for others **/
    const int nLeafsExtra0 = 7;
    const int nLeafsExtra1 = 10;
    /** num of clk time window **/
    const int nClks0 = 48;
    const int nClks1 = 48;
    const int nClks2 = 32;
    /** num of b2l bits **/
    const int nBits = 640;
    /** leaf names **/

    namespace GDLCONF0 {
      const char* LeafNames[nLeafs0 + nLeafsExtra0] = {
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
    }

    /** bus bit map. (a downto a-b) **/
    const int BitMap0[nLeafs0][2] = {
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
      339, 31, // psn1
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
    setLeafPointersArray0(TRGGDLUnpackerStore* store, int** bitArray)
    {
      bitArray[GDLCONF0::e_etffmv    ] = &(store->m_etffmv);
      bitArray[GDLCONF0::e_l1rvc     ] = &(store->m_l1rvc);
      bitArray[GDLCONF0::e_timtype   ] = &(store->m_timtype);
      bitArray[GDLCONF0::e_etyp      ] = &(store->m_etyp);
      bitArray[GDLCONF0::e_final     ] = &(store->m_final);
      bitArray[GDLCONF0::e_gdll1     ] = &(store->m_gdll1);
      bitArray[GDLCONF0::e_etfout    ] = &(store->m_etfout);
      bitArray[GDLCONF0::e_etfvd     ] = &(store->m_etfvd);
      bitArray[GDLCONF0::e_toprvc    ] = &(store->m_toprvc);
      bitArray[GDLCONF0::e_topvd     ] = &(store->m_topvd);
      bitArray[GDLCONF0::e_topt0     ] = &(store->m_topt0);
      bitArray[GDLCONF0::e_eclmsb7   ] = &(store->m_eclmsb7);
      bitArray[GDLCONF0::e_ecllsb7   ] = &(store->m_ecllsb7);
      bitArray[GDLCONF0::e_cdc_timing] = &(store->m_cdc_timing);
      bitArray[GDLCONF0::e_rvcout    ] = &(store->m_rvcout);
      bitArray[GDLCONF0::e_rvcout0   ] = &(store->m_rvcout0);
      bitArray[GDLCONF0::e_comrvc    ] = &(store->m_comrvc);
      bitArray[GDLCONF0::e_etnrvc    ] = &(store->m_etnrvc);
      bitArray[GDLCONF0::e_nim0rvc   ] = &(store->m_nim0rvc);
      bitArray[GDLCONF0::e_etm0rvc   ] = &(store->m_etm0rvc);
      bitArray[GDLCONF0::e_rvc       ] = &(store->m_rvc);
      bitArray[GDLCONF0::e_drvc      ] = &(store->m_drvc);
      bitArray[GDLCONF0::e_dtoprvc   ] = &(store->m_dtoprvc);
      bitArray[GDLCONF0::e_declrvc   ] = &(store->m_declrvc);
      bitArray[GDLCONF0::e_dcdcrvc   ] = &(store->m_dcdcrvc);
      bitArray[GDLCONF0::e_psn1      ] = &(store->m_psn1);
      bitArray[GDLCONF0::e_etfth2    ] = &(store->m_etfth2);
      bitArray[GDLCONF0::e_etfhdt    ] = &(store->m_etfhdt);
      bitArray[GDLCONF0::e_etfth     ] = &(store->m_etfth);
      bitArray[GDLCONF0::e_psn0      ] = &(store->m_psn0);
      bitArray[GDLCONF0::e_ftd1      ] = &(store->m_ftd1);
      bitArray[GDLCONF0::e_etfvdrvc  ] = &(store->m_etfvdrvc);
      bitArray[GDLCONF0::e_ftd0      ] = &(store->m_ftd0);
      bitArray[GDLCONF0::e_itd2      ] = &(store->m_itd2);
      bitArray[GDLCONF0::e_itd1      ] = &(store->m_itd1);
      bitArray[GDLCONF0::e_itd0      ] = &(store->m_itd0);
      bitArray[GDLCONF0::e_inp2      ] = &(store->m_inp2);
      bitArray[GDLCONF0::e_inp1      ] = &(store->m_inp1);
      bitArray[GDLCONF0::e_inp0      ] = &(store->m_inp0);
      bitArray[GDLCONF0::e_evt       ] = &(store->m_evt);
      bitArray[GDLCONF0::e_clk       ] = &(store->m_clk);
      bitArray[GDLCONF0::e_coml1     ] = &(store->m_coml1);
      bitArray[GDLCONF0::e_firmid    ] = &(store->m_firmid);
      bitArray[GDLCONF0::e_firmver   ] = &(store->m_firmver);
      bitArray[GDLCONF0::e_b2ldly    ] = &(store->m_b2ldly);
      bitArray[GDLCONF0::e_maxrvc    ] = &(store->m_maxrvc);
    }


    namespace GDLCONF1 {

      const char* LeafNames[nLeafs1 + nLeafsExtra1] = {
        "rvc",
        "timtype",
        "etyp",
        "tttmdl",
        "timingp",
        "rvcout",
        "toprvc",
        "eclrvc",
        "cdcrvc",
        "toptiming",
        "ecltiming",
        "cdctiming",
        "nim0rvc",
        "dtoprvc",
        "declrvc",
        "dcdcrvc",
        "psn2",
        "psn1",
        "psn0",
        "topslot1",
        "topslot0",
        "ntopslot",
        "ftd2",
        "ftd1",
        "ftd0",
        "itd2",
        "itd1",
        "itd0",
        "evt", "clk", "firmid", "firmver", "finalrvc", "drvc", "gdll1rvc", "coml1rvc", "b2ldly", "maxrvc"
      };

      /** enum of leafs **/
      enum EBits {
        e_rvc,
        e_timtype,
        e_etyp,
        e_tttmdl,
        e_timingp,
        e_rvcout,
        e_toprvc,
        e_eclrvc,
        e_cdcrvc,
        e_toptiming,
        e_ecltiming,
        e_cdctiming,
        e_nim0rvc,
        e_dtoprvc,
        e_declrvc,
        e_dcdcrvc,
        e_psn2,
        e_psn1,
        e_psn0,
        e_topslot1,
        e_topslot0,
        e_ntopslot,
        e_ftd2,
        e_ftd1,
        e_ftd0,
        e_itd2,
        e_itd1,
        e_itd0,
        e_evt, e_clk, e_firmid, e_firmver, e_finalrvc, e_drvc, e_gdll1rvc, e_coml1rvc, e_b2ldly, e_maxrvc
      };
    }

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

    /** bus bit map. (a downto a-b) **/
    const int BitMap1[nLeafs1][2] = {
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 12, // timingp
      583, 14, // rvcout
      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 14, // toptiming
      509, 14, // ecltiming
      494, 14, // cdctiming
      479, 10, // nim0rvc
      468, 11, // dtoprvc
      457, 11, // declrvc
      446, 11, // dcdcrvc
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0
      287, 31, // topslot1
      255, 31, // topslot0
      223,  4, // ntopslot
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    };

    void
    setLeafPointersArray1(TRGGDLUnpackerStore* store, int** bitArray)
    {
      bitArray[GDLCONF1::e_rvc      ] = &(store->m_rvc);
      bitArray[GDLCONF1::e_timtype  ] = &(store->m_timtype);
      bitArray[GDLCONF1::e_etyp     ] = &(store->m_etyp);
      bitArray[GDLCONF1::e_tttmdl   ] = &(store->m_tttmdl);
      bitArray[GDLCONF1::e_timingp  ] = &(store->m_timingp);
      bitArray[GDLCONF1::e_rvcout   ] = &(store->m_rvcout);
      bitArray[GDLCONF1::e_toprvc   ] = &(store->m_toprvc);
      bitArray[GDLCONF1::e_eclrvc   ] = &(store->m_eclrvc);
      bitArray[GDLCONF1::e_cdcrvc   ] = &(store->m_cdcrvc);
      bitArray[GDLCONF1::e_toptiming] = &(store->m_toptiming);
      bitArray[GDLCONF1::e_ecltiming] = &(store->m_ecltiming);
      bitArray[GDLCONF1::e_cdctiming] = &(store->m_cdctiming);
      bitArray[GDLCONF1::e_nim0rvc  ] = &(store->m_nim0rvc);
      bitArray[GDLCONF1::e_dtoprvc  ] = &(store->m_dtoprvc);
      bitArray[GDLCONF1::e_declrvc  ] = &(store->m_declrvc);
      bitArray[GDLCONF1::e_dcdcrvc  ] = &(store->m_dcdcrvc);
      bitArray[GDLCONF1::e_psn2     ] = &(store->m_psn2);
      bitArray[GDLCONF1::e_psn1     ] = &(store->m_psn1);
      bitArray[GDLCONF1::e_psn0     ] = &(store->m_psn0);
      bitArray[GDLCONF1::e_topslot1 ] = &(store->m_topslot1);
      bitArray[GDLCONF1::e_topslot0 ] = &(store->m_topslot0);
      bitArray[GDLCONF1::e_ntopslot ] = &(store->m_ntopslot);
      bitArray[GDLCONF1::e_ftd2     ] = &(store->m_ftd2);
      bitArray[GDLCONF1::e_ftd1     ] = &(store->m_ftd1);
      bitArray[GDLCONF1::e_ftd0     ] = &(store->m_ftd0);
      bitArray[GDLCONF1::e_itd2     ] = &(store->m_itd2);
      bitArray[GDLCONF1::e_itd1     ] = &(store->m_itd1);
      bitArray[GDLCONF1::e_itd0     ] = &(store->m_itd0);
      bitArray[GDLCONF1::e_evt      ] = &(store->m_evt);
      bitArray[GDLCONF1::e_clk      ] = &(store->m_clk);
      bitArray[GDLCONF1::e_firmid   ] = &(store->m_firmid);
      bitArray[GDLCONF1::e_firmver  ] = &(store->m_firmver);
      bitArray[GDLCONF1::e_finalrvc ] = &(store->m_finalrvc);
      bitArray[GDLCONF1::e_drvc     ] = &(store->m_drvc);
      bitArray[GDLCONF1::e_gdll1rvc ] = &(store->m_gdll1rvc);
      bitArray[GDLCONF1::e_coml1rvc ] = &(store->m_coml1rvc);
      bitArray[GDLCONF1::e_b2ldly   ] = &(store->m_b2ldly);
      bitArray[GDLCONF1::e_maxrvc   ] = &(store->m_maxrvc);
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
      virtual void fillTreeGDL0(int* buf, int evt);

      /** Unpacker main function.*/
      virtual void fillTreeGDL1(int* buf, int evt);

      /** Unpacker main function.*/
      virtual void fillTreeGDL2(int* buf, int evt);

    private:

      StoreArray<TRGGDLUnpackerStore> store;

    };
  }
} // end namespace Belle2

#endif // TRGGDLUnpackerModule_H
