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

    /** num of b2l bits **/
    const int nBits = 640;

    namespace GDLCONF0 {

      // gdl0065c, gdl0065e

      /** num of leafs in data_b2l **/
      const int nLeafs = 37;

      /** num of leafs for others **/
      const int nLeafsExtra = 8;

      /** num of clk time window **/
      const int nClks = 48;

      /** leaf names **/
      const char* LeafNames[nLeafs + nLeafsExtra] = {
        "etffmv",
        "l1rvc",
        "timtype",
        "etyp",
        "final",
        "gdll1rvc",
        "etfout",
        "etfvd",
        "toprvc",
        "topvd",
        "toptiming",
        "ecltiming",
        "cdctiming",
        "rvcout",
        "rvcout0",
        "comrvc",
        "etnrvc",
        "nim0rvc",
        "eclrvc",
        "rvc",
        "drvc",
        "ftd2",
        "psn2",
        "psn1",
        "etfth2",
        "etfhdt",
        "etfth",
        "psn0",
        "ftd1",
        "cdcrvc",
        "ftd0",
        "itd2",
        "itd1",
        "itd0",
        "inp2",
        "inp1",
        "inp0",
        "evt", "clk", "firmid", "firmver", "coml1rvc", "b2ldly", "maxrvc",
        "conf"
      };

      /** enum of leafs **/
      enum EBits {
        e_etffmv,
        e_l1rvc,
        e_timtype,
        e_etyp,
        e_final,
        e_gdll1rvc,
        e_etfout,
        e_etfvd,
        e_toprvc,
        e_topvd,
        e_toptiming,
        e_ecltiming,
        e_cdctiming,
        e_rvcout,
        e_rvcout0,
        e_comrvc,
        e_etnrvc,
        e_nim0rvc,
        e_eclrvc,
        e_rvc,
        e_drvc,
        e_ftd2,
        e_psn2,
        e_psn1,
        e_etfth2,
        e_etfhdt,
        e_etfth,
        e_psn0,
        e_ftd1,
        e_cdcrvc,
        e_ftd0,
        e_itd2,
        e_itd1,
        e_itd0,
        e_inp2,
        e_inp1,
        e_inp0,
        e_evt, e_clk, e_firmid, e_firmver, e_coml1rvc, e_b2ldly, e_maxrvc,
        e_conf
      };

      /** bus bit map. (a downto a-b) **/
      /** gdl0065c e3r100 - e3r261 **/
      /** gdl0065e e3r292 - e3r480 **/
      const int BitMap[nLeafs][2] = {
        639, 31, // etffmv
        575, 10, // l1rvc
        564, 2,  // timtype
        561, 2,  // etyp
        558, 0,  // final
        557, 0,  // gdll1rvc
        538, 12, // etfout
        525, 0,  // etfvd
        524, 14, // toprvc
        480, 0,  // topvd
        498, 17, // toptiming
        479, 13, // ecltiming (lsb1ns)
        465, 12, // cdctiming (lsb2ns)
        447, 14, // rvcout
        432,  0, // rvcout0
        431, 11, // comrvc
        419, 11, // etnrvc
        407, 11, // nim0rvc
        395, 11, // eclrvc
        383, 11, // rvc
        371, 11, // drvc
        355, 15, // ftd2
        339, 15, // psn2. Empty for 65c.
        323, 15, // psn1.
        307, 7,  // etfth2
        299, 7,  // etfhdt
        291, 3,  // etfth
        287, 31, // psn0
        255, 15, // ftd1
        234, 10, // cdcrvc
        223, 31, // ftd0
        191, 31, // itd2
        159, 31, // itd1
        127, 31, // itd0
        95,  31, // inp2
        63,  31, // inp1
        31,  31, // inp0
      };

      void setLeafPointersArray(TRGGDLUnpackerStore* store, int** bitArray)
      {
        bitArray[GDLCONF0::e_etffmv    ] = &(store->m_etffmv);
        bitArray[GDLCONF0::e_l1rvc     ] = &(store->m_l1rvc);
        bitArray[GDLCONF0::e_timtype   ] = &(store->m_timtype);
        bitArray[GDLCONF0::e_etyp      ] = &(store->m_etyp);
        bitArray[GDLCONF0::e_final     ] = &(store->m_final);
        bitArray[GDLCONF0::e_gdll1rvc  ] = &(store->m_gdll1rvc);
        bitArray[GDLCONF0::e_etfout    ] = &(store->m_etfout);
        bitArray[GDLCONF0::e_etfvd     ] = &(store->m_etfvd);
        bitArray[GDLCONF0::e_toprvc    ] = &(store->m_toprvc);
        bitArray[GDLCONF0::e_topvd     ] = &(store->m_topvd);
        bitArray[GDLCONF0::e_toptiming ] = &(store->m_toptiming);
        bitArray[GDLCONF0::e_ecltiming ] = &(store->m_ecltiming);
        bitArray[GDLCONF0::e_cdctiming ] = &(store->m_cdctiming);
        bitArray[GDLCONF0::e_rvcout    ] = &(store->m_rvcout);
        bitArray[GDLCONF0::e_rvcout0   ] = &(store->m_rvcout0);
        bitArray[GDLCONF0::e_comrvc    ] = &(store->m_comrvc);
        bitArray[GDLCONF0::e_etnrvc    ] = &(store->m_etnrvc);
        bitArray[GDLCONF0::e_nim0rvc   ] = &(store->m_nim0rvc);
        bitArray[GDLCONF0::e_eclrvc    ] = &(store->m_eclrvc);
        bitArray[GDLCONF0::e_rvc       ] = &(store->m_rvc);
        bitArray[GDLCONF0::e_drvc      ] = &(store->m_drvc);
        bitArray[GDLCONF0::e_ftd2      ] = &(store->m_ftd2);
        bitArray[GDLCONF0::e_psn2      ] = &(store->m_psn2);
        bitArray[GDLCONF0::e_psn1      ] = &(store->m_psn1);
        bitArray[GDLCONF0::e_etfth2    ] = &(store->m_etfth2);
        bitArray[GDLCONF0::e_etfhdt    ] = &(store->m_etfhdt);
        bitArray[GDLCONF0::e_etfth     ] = &(store->m_etfth);
        bitArray[GDLCONF0::e_psn0      ] = &(store->m_psn0);
        bitArray[GDLCONF0::e_ftd1      ] = &(store->m_ftd1);
        bitArray[GDLCONF0::e_cdcrvc    ] = &(store->m_cdcrvc);
        bitArray[GDLCONF0::e_ftd0      ] = &(store->m_ftd0);
        bitArray[GDLCONF0::e_itd2      ] = &(store->m_itd2);
        bitArray[GDLCONF0::e_itd1      ] = &(store->m_itd1);
        bitArray[GDLCONF0::e_itd0      ] = &(store->m_itd0);
        bitArray[GDLCONF0::e_inp2      ] = &(store->m_inp2);
        bitArray[GDLCONF0::e_inp1      ] = &(store->m_inp1);
        bitArray[GDLCONF0::e_inp0      ] = &(store->m_inp0);
        bitArray[GDLCONF0::e_evt       ] = &(store->m_evt);
        bitArray[GDLCONF0::e_clk       ] = &(store->m_clk);
        bitArray[GDLCONF0::e_firmid    ] = &(store->m_firmid);
        bitArray[GDLCONF0::e_firmver   ] = &(store->m_firmver);
        bitArray[GDLCONF0::e_coml1rvc  ] = &(store->m_coml1rvc);
        bitArray[GDLCONF0::e_b2ldly    ] = &(store->m_b2ldly);
        bitArray[GDLCONF0::e_maxrvc    ] = &(store->m_maxrvc);
        bitArray[GDLCONF0::e_conf      ] = &(store->m_conf);
      }

    }

    namespace GDLCONF1 {

      // gdl0065h. 529 <= run < 677

      /** num of leafs in data_b2l **/
      const int nLeafs = 27;

      /** num of leafs for others **/
      const int nLeafsExtra = 9;

      /** num of clk time window **/
      const int nClks = 48;

      /** CONF1 leaf names **/
      const char* LeafNames[nLeafs + nLeafsExtra] = {
        "rvc",
        "timtype",
        "etyp",
        "rvcout",
        "drvc",
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
        "evt", "clk", "firmid", "firmver", "gdll1rvc", "coml1rvc", "b2ldly", "maxrvc",
        "conf"
      };

      /** enum of leafs **/
      enum EBits {
        e_rvc,
        e_timtype,
        e_etyp,
        e_rvcout,
        e_drvc,
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
        e_evt, e_clk, e_firmid, e_firmver, e_gdll1rvc, e_coml1rvc, e_b2ldly, e_maxrvc,
        e_conf
      };

      /** bus bit map. (a downto a-b) **/
      const int BitMap[nLeafs][2] = {
        623, 11, // rvc
        611, 2,  // timtype
        608, 2,  // etyp
        583, 14, // rvcout
        568, 10, // drvc
        557, 10, // toprvc
        546, 10, // eclrvc
        535, 10, // cdcrvc
        524, 13, // toptiming
        509, 13, // ecltiming
        494, 13, // cdctiming
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

      void setLeafPointersArray(TRGGDLUnpackerStore* store, int** bitArray)
      {
        bitArray[GDLCONF1::e_rvc      ] = &(store->m_rvc);
        bitArray[GDLCONF1::e_timtype  ] = &(store->m_timtype);
        bitArray[GDLCONF1::e_etyp     ] = &(store->m_etyp);
        bitArray[GDLCONF1::e_rvcout   ] = &(store->m_rvcout);
        bitArray[GDLCONF1::e_drvc     ] = &(store->m_drvc);
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
        bitArray[GDLCONF1::e_gdll1rvc ] = &(store->m_gdll1rvc);
        bitArray[GDLCONF1::e_coml1rvc ] = &(store->m_coml1rvc);
        bitArray[GDLCONF1::e_b2ldly   ] = &(store->m_b2ldly);
        bitArray[GDLCONF1::e_maxrvc   ] = &(store->m_maxrvc);
        bitArray[GDLCONF1::e_conf     ] = &(store->m_conf);
      }

    }

    namespace GDLCONF2 {

      // gdl0065j. 677 <= run <= 816 < 932
      // ntopslot added (included in CONF1 but not used, empty)
      // header part final shape, 6 words. drvc to header.

      /** num of leafs in data_b2l **/
      const int nLeafs = 26;

      /** num of leafs for others **/
      const int nLeafsExtra = 11;

      /** num of clk time window **/
      const int nClks = 48;

      /** CONF2 leaf names **/
      const char* LeafNames[nLeafs + nLeafsExtra] = {
        "rvc",
        "timtype",
        "etyp",
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
        "evt", "clk", "firmid", "firmver", "drvc", "finalrvc", "gdll1rvc", "coml1rvc", "b2ldly", "maxrvc",
        "conf"
      };

      /** enum of leafs **/
      enum EBits {
        e_rvc,
        e_timtype,
        e_etyp,
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
        e_evt, e_clk, e_firmid, e_firmver, e_drvc, e_finalrvc, e_gdll1rvc, e_coml1rvc, e_b2ldly, e_maxrvc,
        e_conf
      };

      /** bus bit map. (a downto a-b) **/
      /** gdl0065j e3r - **/
      const int BitMap[nLeafs][2] = {
        623, 11, // rvc
        611, 2,  // timtype
        608, 2,  // etyp
        583, 14, // rvcout
        557, 10, // toprvc
        546, 10, // eclrvc
        535, 10, // cdcrvc
        524, 13, // toptiming
        509, 13, // ecltiming
        494, 13, // cdctiming
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

      void setLeafPointersArray(TRGGDLUnpackerStore* store, int** bitArray)
      {
        bitArray[GDLCONF2::e_rvc      ] = &(store->m_rvc);
        bitArray[GDLCONF2::e_timtype  ] = &(store->m_timtype);
        bitArray[GDLCONF2::e_etyp     ] = &(store->m_etyp);
        bitArray[GDLCONF2::e_rvcout   ] = &(store->m_rvcout);
        bitArray[GDLCONF2::e_toprvc   ] = &(store->m_toprvc);
        bitArray[GDLCONF2::e_eclrvc   ] = &(store->m_eclrvc);
        bitArray[GDLCONF2::e_cdcrvc   ] = &(store->m_cdcrvc);
        bitArray[GDLCONF2::e_toptiming] = &(store->m_toptiming);
        bitArray[GDLCONF2::e_ecltiming] = &(store->m_ecltiming);
        bitArray[GDLCONF2::e_cdctiming] = &(store->m_cdctiming);
        bitArray[GDLCONF2::e_nim0rvc  ] = &(store->m_nim0rvc);
        bitArray[GDLCONF2::e_dtoprvc  ] = &(store->m_dtoprvc);
        bitArray[GDLCONF2::e_declrvc  ] = &(store->m_declrvc);
        bitArray[GDLCONF2::e_dcdcrvc  ] = &(store->m_dcdcrvc);
        bitArray[GDLCONF2::e_psn2     ] = &(store->m_psn2);
        bitArray[GDLCONF2::e_psn1     ] = &(store->m_psn1);
        bitArray[GDLCONF2::e_psn0     ] = &(store->m_psn0);
        bitArray[GDLCONF2::e_topslot1 ] = &(store->m_topslot1);
        bitArray[GDLCONF2::e_topslot0 ] = &(store->m_topslot0);
        bitArray[GDLCONF2::e_ntopslot ] = &(store->m_ntopslot);
        bitArray[GDLCONF2::e_ftd2     ] = &(store->m_ftd2);
        bitArray[GDLCONF2::e_ftd1     ] = &(store->m_ftd1);
        bitArray[GDLCONF2::e_ftd0     ] = &(store->m_ftd0);
        bitArray[GDLCONF2::e_itd2     ] = &(store->m_itd2);
        bitArray[GDLCONF2::e_itd1     ] = &(store->m_itd1);
        bitArray[GDLCONF2::e_itd0     ] = &(store->m_itd0);
        bitArray[GDLCONF2::e_evt      ] = &(store->m_evt);
        bitArray[GDLCONF2::e_clk      ] = &(store->m_clk);
        bitArray[GDLCONF2::e_firmid   ] = &(store->m_firmid);
        bitArray[GDLCONF2::e_firmver  ] = &(store->m_firmver);
        bitArray[GDLCONF2::e_drvc     ] = &(store->m_drvc);
        bitArray[GDLCONF2::e_finalrvc ] = &(store->m_finalrvc);
        bitArray[GDLCONF2::e_gdll1rvc ] = &(store->m_gdll1rvc);
        bitArray[GDLCONF2::e_coml1rvc ] = &(store->m_coml1rvc);
        bitArray[GDLCONF2::e_b2ldly   ] = &(store->m_b2ldly);
        bitArray[GDLCONF2::e_maxrvc   ] = &(store->m_maxrvc);
        bitArray[GDLCONF2::e_conf     ] = &(store->m_conf);
      }

    }

    namespace GDLCONF3 {

      // gdl0066a. 932 <= run <= 1355 including gdl0066k.
      // gdl0066k has bug that no clst(0) and clst(1).
      // gdl0066b, gdl0066c(recorded as 66b). 1615<=r<=1512
      // gdl0066e. 1811<=r<=1828.

      /** num of leafs in data_b2l **/
      const int nLeafs = 26;

      /** num of leafs for others **/
      const int nLeafsExtra = 11;

      /** num of clk time window **/
      const int nClks = 48;

      /** CONF3 leaf names **/
      const char* LeafNames[nLeafs + nLeafsExtra] = {
        "rvc",
        "timtype",
        "etyp",
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
        "evt", "clk", "firmid", "firmver", "drvc", "finalrvc", "gdll1rvc", "coml1rvc", "b2ldly", "maxrvc",
        "conf"
      };

      /** enum of leafs **/
      enum EBits {
        e_rvc,
        e_timtype,
        e_etyp,
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
        e_evt, e_clk, e_firmid, e_firmver, e_drvc, e_finalrvc, e_gdll1rvc, e_coml1rvc, e_b2ldly, e_maxrvc,
        e_conf
      };

      /** bus bit map. (a downto a-b) **/
      /** gdl0065h e3r - **/
      const int BitMap[nLeafs][2] = {
        623, 11, // rvc
        611, 2,  // timtype
        608, 2,  // etyp
        583, 14, // rvcout
        557, 10, // toprvc
        546, 10, // eclrvc
        535, 10, // cdcrvc
        524, 13, // toptiming
        509, 13, // ecltiming
        494, 13, // cdctiming
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

      void setLeafPointersArray(TRGGDLUnpackerStore* store, int** bitArray)
      {
        bitArray[GDLCONF3::e_rvc      ] = &(store->m_rvc);
        bitArray[GDLCONF3::e_timtype  ] = &(store->m_timtype);
        bitArray[GDLCONF3::e_etyp     ] = &(store->m_etyp);
        bitArray[GDLCONF3::e_rvcout   ] = &(store->m_rvcout);
        bitArray[GDLCONF3::e_toprvc   ] = &(store->m_toprvc);
        bitArray[GDLCONF3::e_eclrvc   ] = &(store->m_eclrvc);
        bitArray[GDLCONF3::e_cdcrvc   ] = &(store->m_cdcrvc);
        bitArray[GDLCONF3::e_toptiming] = &(store->m_toptiming);
        bitArray[GDLCONF3::e_ecltiming] = &(store->m_ecltiming);
        bitArray[GDLCONF3::e_cdctiming] = &(store->m_cdctiming);
        bitArray[GDLCONF3::e_nim0rvc  ] = &(store->m_nim0rvc);
        bitArray[GDLCONF3::e_dtoprvc  ] = &(store->m_dtoprvc);
        bitArray[GDLCONF3::e_declrvc  ] = &(store->m_declrvc);
        bitArray[GDLCONF3::e_dcdcrvc  ] = &(store->m_dcdcrvc);
        bitArray[GDLCONF3::e_psn2     ] = &(store->m_psn2);
        bitArray[GDLCONF3::e_psn1     ] = &(store->m_psn1);
        bitArray[GDLCONF3::e_psn0     ] = &(store->m_psn0);
        bitArray[GDLCONF3::e_topslot1 ] = &(store->m_topslot1);
        bitArray[GDLCONF3::e_topslot0 ] = &(store->m_topslot0);
        bitArray[GDLCONF3::e_ntopslot ] = &(store->m_ntopslot);
        bitArray[GDLCONF3::e_ftd2     ] = &(store->m_ftd2);
        bitArray[GDLCONF3::e_ftd1     ] = &(store->m_ftd1);
        bitArray[GDLCONF3::e_ftd0     ] = &(store->m_ftd0);
        bitArray[GDLCONF3::e_itd2     ] = &(store->m_itd2);
        bitArray[GDLCONF3::e_itd1     ] = &(store->m_itd1);
        bitArray[GDLCONF3::e_itd0     ] = &(store->m_itd0);
        bitArray[GDLCONF3::e_evt      ] = &(store->m_evt);
        bitArray[GDLCONF3::e_clk      ] = &(store->m_clk);
        bitArray[GDLCONF3::e_firmid   ] = &(store->m_firmid);
        bitArray[GDLCONF3::e_firmver  ] = &(store->m_firmver);
        bitArray[GDLCONF3::e_drvc     ] = &(store->m_drvc);
        bitArray[GDLCONF2::e_finalrvc ] = &(store->m_finalrvc);
        bitArray[GDLCONF3::e_gdll1rvc ] = &(store->m_gdll1rvc);
        bitArray[GDLCONF3::e_coml1rvc ] = &(store->m_coml1rvc);
        bitArray[GDLCONF3::e_b2ldly   ] = &(store->m_b2ldly);
        bitArray[GDLCONF3::e_maxrvc   ] = &(store->m_maxrvc);
        bitArray[GDLCONF3::e_conf     ] = &(store->m_conf);
      }


    }

    namespace GDLCONF4 {

      // gdl0066h. 1866<=r<=4700
      // nCLK: 48 -> 32.

      /** num of leafs in data_b2l **/
      const int nLeafs = 26;

      /** num of leafs for others **/
      const int nLeafsExtra = 11;

      /** num of clk time window **/
      const int nClks = 32;

      /** CONF3 leaf names **/
      const char* LeafNames[nLeafs + nLeafsExtra] = {
        "rvc",
        "timtype",
        "etyp",
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
        "evt", "clk", "firmid", "firmver", "drvc", "finalrvc", "gdll1rvc", "coml1rvc", "b2ldly", "maxrvc",
        "conf"
      };

      /** enum of leafs **/
      enum EBits {
        e_rvc,
        e_timtype,
        e_etyp,
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
        e_evt, e_clk, e_firmid, e_firmver, e_drvc, e_finalrvc, e_gdll1rvc, e_coml1rvc, e_b2ldly, e_maxrvc,
        e_conf
      };

      /** bus bit map. (a downto a-b) **/
      /** gdl0065h e3r - **/
      const int BitMap[nLeafs][2] = {
        623, 11, // rvc
        611, 2,  // timtype
        608, 2,  // etyp
        583, 14, // rvcout
        557, 10, // toprvc
        546, 10, // eclrvc
        535, 10, // cdcrvc
        524, 13, // toptiming
        509, 13, // ecltiming
        494, 13, // cdctiming
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

      void setLeafPointersArray(TRGGDLUnpackerStore* store, int** bitArray)
      {
        bitArray[GDLCONF4::e_rvc      ] = &(store->m_rvc);
        bitArray[GDLCONF4::e_timtype  ] = &(store->m_timtype);
        bitArray[GDLCONF4::e_etyp     ] = &(store->m_etyp);
        bitArray[GDLCONF4::e_rvcout   ] = &(store->m_rvcout);
        bitArray[GDLCONF4::e_toprvc   ] = &(store->m_toprvc);
        bitArray[GDLCONF4::e_eclrvc   ] = &(store->m_eclrvc);
        bitArray[GDLCONF4::e_cdcrvc   ] = &(store->m_cdcrvc);
        bitArray[GDLCONF4::e_toptiming] = &(store->m_toptiming);
        bitArray[GDLCONF4::e_ecltiming] = &(store->m_ecltiming);
        bitArray[GDLCONF4::e_cdctiming] = &(store->m_cdctiming);
        bitArray[GDLCONF4::e_nim0rvc  ] = &(store->m_nim0rvc);
        bitArray[GDLCONF4::e_dtoprvc  ] = &(store->m_dtoprvc);
        bitArray[GDLCONF4::e_declrvc  ] = &(store->m_declrvc);
        bitArray[GDLCONF4::e_dcdcrvc  ] = &(store->m_dcdcrvc);
        bitArray[GDLCONF4::e_psn2     ] = &(store->m_psn2);
        bitArray[GDLCONF4::e_psn1     ] = &(store->m_psn1);
        bitArray[GDLCONF4::e_psn0     ] = &(store->m_psn0);
        bitArray[GDLCONF4::e_topslot1 ] = &(store->m_topslot1);
        bitArray[GDLCONF4::e_topslot0 ] = &(store->m_topslot0);
        bitArray[GDLCONF4::e_ntopslot ] = &(store->m_ntopslot);
        bitArray[GDLCONF4::e_ftd2     ] = &(store->m_ftd2);
        bitArray[GDLCONF4::e_ftd1     ] = &(store->m_ftd1);
        bitArray[GDLCONF4::e_ftd0     ] = &(store->m_ftd0);
        bitArray[GDLCONF4::e_itd2     ] = &(store->m_itd2);
        bitArray[GDLCONF4::e_itd1     ] = &(store->m_itd1);
        bitArray[GDLCONF4::e_itd0     ] = &(store->m_itd0);
        bitArray[GDLCONF4::e_evt      ] = &(store->m_evt);
        bitArray[GDLCONF4::e_clk      ] = &(store->m_clk);
        bitArray[GDLCONF4::e_firmid   ] = &(store->m_firmid);
        bitArray[GDLCONF4::e_firmver  ] = &(store->m_firmver);
        bitArray[GDLCONF4::e_drvc     ] = &(store->m_drvc);
        bitArray[GDLCONF2::e_finalrvc ] = &(store->m_finalrvc);
        bitArray[GDLCONF4::e_gdll1rvc ] = &(store->m_gdll1rvc);
        bitArray[GDLCONF4::e_coml1rvc ] = &(store->m_coml1rvc);
        bitArray[GDLCONF4::e_b2ldly   ] = &(store->m_b2ldly);
        bitArray[GDLCONF4::e_maxrvc   ] = &(store->m_maxrvc);
        bitArray[GDLCONF4::e_conf     ] = &(store->m_conf);
      }


    }

    namespace GDLCONF5 {

      // gdl0066, gdl0067g
      // 4876 <= run < 5314
      // timing pipes added to B2L data
      /** num of leafs in data_b2l **/
      const int nLeafs = 31;

      /** num of leafs for others **/
      const int nLeafsExtra = 11;

      /** num of clk time window **/
      const int nClks = 32;

      /** CONF2 leaf names **/
      const char* LeafNames[nLeafs + nLeafsExtra] = {
        "rvc",
        "timtype",
        "etyp",
        "tttmdl",
        "tdsrcp",
        "tdtopp",
        "tdeclp",
        "tdcdcp",
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
        "evt", "clk", "firmid", "firmver", "finalrvc", "drvc", "gdll1rvc", "coml1rvc", "b2ldly", "maxrvc",
        "conf"
      };

      /** enum of leafs **/
      enum EBits {
        e_rvc,
        e_timtype,
        e_etyp,
        e_tttmdl,
        e_tdsrcp,
        e_tdtopp,
        e_tdeclp,
        e_tdcdcp,
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
        e_evt, e_clk, e_firmid, e_firmver, e_finalrvc, e_drvc, e_gdll1rvc, e_coml1rvc, e_b2ldly, e_maxrvc,
        e_conf
      };

      /** bus bit map. (a downto a-b) **/
      const int BitMap[nLeafs][2] = {
        623, 11, // rvc
        611, 2,  // timtype
        608, 2,  // etyp
        603, 2,  // tttmdl
        600, 3,  // tdsrcp
        596, 2,  // tdtopp
        593, 2,  // tdeclp
        590, 2,  // tdcdcp
        583, 14, // rvcout
        557, 10, // toprvc
        546, 10, // eclrvc
        535, 10, // cdcrvc
        524, 13, // toptiming
        509, 13, // ecltiming
        494, 13, // cdctiming
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
      setLeafPointersArray(TRGGDLUnpackerStore* store, int** bitArray)
      {
        bitArray[GDLCONF5::e_rvc      ] = &(store->m_rvc);
        bitArray[GDLCONF5::e_timtype  ] = &(store->m_timtype);
        bitArray[GDLCONF5::e_etyp     ] = &(store->m_etyp);
        bitArray[GDLCONF5::e_tttmdl   ] = &(store->m_tttmdl);
        bitArray[GDLCONF5::e_tdsrcp   ] = &(store->m_tdsrcp);
        bitArray[GDLCONF5::e_tdtopp   ] = &(store->m_tdtopp);
        bitArray[GDLCONF5::e_tdeclp   ] = &(store->m_tdeclp);
        bitArray[GDLCONF5::e_tdcdcp   ] = &(store->m_tdcdcp);
        bitArray[GDLCONF5::e_rvcout   ] = &(store->m_rvcout);
        bitArray[GDLCONF5::e_toprvc   ] = &(store->m_toprvc);
        bitArray[GDLCONF5::e_eclrvc   ] = &(store->m_eclrvc);
        bitArray[GDLCONF5::e_cdcrvc   ] = &(store->m_cdcrvc);
        bitArray[GDLCONF5::e_toptiming] = &(store->m_toptiming);
        bitArray[GDLCONF5::e_ecltiming] = &(store->m_ecltiming);
        bitArray[GDLCONF5::e_cdctiming] = &(store->m_cdctiming);
        bitArray[GDLCONF5::e_nim0rvc  ] = &(store->m_nim0rvc);
        bitArray[GDLCONF5::e_dtoprvc  ] = &(store->m_dtoprvc);
        bitArray[GDLCONF5::e_declrvc  ] = &(store->m_declrvc);
        bitArray[GDLCONF5::e_dcdcrvc  ] = &(store->m_dcdcrvc);
        bitArray[GDLCONF5::e_psn2     ] = &(store->m_psn2);
        bitArray[GDLCONF5::e_psn1     ] = &(store->m_psn1);
        bitArray[GDLCONF5::e_psn0     ] = &(store->m_psn0);
        bitArray[GDLCONF5::e_topslot1 ] = &(store->m_topslot1);
        bitArray[GDLCONF5::e_topslot0 ] = &(store->m_topslot0);
        bitArray[GDLCONF5::e_ntopslot ] = &(store->m_ntopslot);
        bitArray[GDLCONF5::e_ftd2     ] = &(store->m_ftd2);
        bitArray[GDLCONF5::e_ftd1     ] = &(store->m_ftd1);
        bitArray[GDLCONF5::e_ftd0     ] = &(store->m_ftd0);
        bitArray[GDLCONF5::e_itd2     ] = &(store->m_itd2);
        bitArray[GDLCONF5::e_itd1     ] = &(store->m_itd1);
        bitArray[GDLCONF5::e_itd0     ] = &(store->m_itd0);
        bitArray[GDLCONF5::e_evt      ] = &(store->m_evt);
        bitArray[GDLCONF5::e_clk      ] = &(store->m_clk);
        bitArray[GDLCONF5::e_firmid   ] = &(store->m_firmid);
        bitArray[GDLCONF5::e_firmver  ] = &(store->m_firmver);
        bitArray[GDLCONF5::e_finalrvc ] = &(store->m_finalrvc);
        bitArray[GDLCONF5::e_drvc     ] = &(store->m_drvc);
        bitArray[GDLCONF5::e_gdll1rvc ] = &(store->m_gdll1rvc);
        bitArray[GDLCONF5::e_coml1rvc ] = &(store->m_coml1rvc);
        bitArray[GDLCONF5::e_b2ldly   ] = &(store->m_b2ldly);
        bitArray[GDLCONF5::e_maxrvc   ] = &(store->m_maxrvc);
        bitArray[GDLCONF5::e_conf     ] = &(store->m_conf);
      }
    }

    namespace GDLCONF6 {

      // gdl0068a. 5314 <= run
      // eed,fed,fp,bha3d,lowmult bits added
      // ftd3,psn3,itd3,itd4 added
      // gdl0068b. 5594 <= run
      // track_found signal added.
      /** num of leafs in data_b2l **/
      const int nLeafs = 32;

      /** num of leafs for others **/
      const int nLeafsExtra = 11;

      /** num of clk time window **/
      const int nClks = 32;

      /** CONF2 leaf names **/
      const char* LeafNames[nLeafs + nLeafsExtra] = {
        "rvc",
        "timtype",
        "etyp",
        "tttmdl",
        "tdsrcp",
        "tdtopp",
        "tdeclp",
        "tdcdcp",
        "rvcout",
        "toprvc",
        "eclrvc",
        "cdcrvc",
        "toptiming",
        "ecltiming",
        "cdctiming",
        "nim0rvc",
        "psn3",
        "psn2",
        "psn1",
        "psn0",
        "topslot1",
        "topslot0",
        "ntopslot",
        "ftd3",
        "ftd2",
        "ftd1",
        "ftd0",
        "itd4",
        "itd3",
        "itd2",
        "itd1",
        "itd0",
        "evt", "clk", "firmid", "firmver", "finalrvc", "drvc", "gdll1rvc", "coml1rvc", "b2ldly", "maxrvc",
        "conf"
      };

      /** enum of leafs **/
      enum EBits {
        e_rvc,
        e_timtype,
        e_etyp,
        e_tttmdl,
        e_tdsrcp,
        e_tdtopp,
        e_tdeclp,
        e_tdcdcp,
        e_rvcout,
        e_toprvc,
        e_eclrvc,
        e_cdcrvc,
        e_toptiming,
        e_ecltiming,
        e_cdctiming,
        e_nim0rvc,
        e_psn3,
        e_psn2,
        e_psn1,
        e_psn0,
        e_topslot1,
        e_topslot0,
        e_ntopslot,
        e_ftd3,
        e_ftd2,
        e_ftd1,
        e_ftd0,
        e_itd4,
        e_itd3,
        e_itd2,
        e_itd1,
        e_itd0,
        e_evt, e_clk, e_firmid, e_firmver, e_finalrvc, e_drvc, e_gdll1rvc, e_coml1rvc, e_b2ldly, e_maxrvc,
        e_conf
      };

      /** bus bit map. (a downto a-b) **/
      const int BitMap[nLeafs][2] = {
        623, 11, // rvc
        611, 2,  // timtype
        608, 2,  // etyp
        603, 2,  // tttmdl
        600, 3,  // tdsrcp
        596, 2,  // tdtopp
        593, 2,  // tdeclp
        590, 2,  // tdcdcp
        583, 14, // rvcout
        557, 10, // toprvc
        546, 10, // eclrvc
        535, 10, // cdcrvc
        524, 13, // toptiming
        509, 13, // ecltiming
        494, 13, // cdctiming
        479, 10, // nim0rvc
        415, 31, // psn3
        383, 31, // psn2
        351, 31, // psn1
        319, 31, // psn0
        287, 31, // topslot1
        255, 31, // topslot0
        223,  4, // ntopslot
        218, 26, // ftd3
        191, 31, // ftd2
        159, 31, // ftd1
        127, 31, // ftd0
        468, 15, // itd4
        452, 31, // itd3
        95, 31, // itd2
        63, 31, // itd1
        31, 31, // itd0
      };

      void
      setLeafPointersArray(TRGGDLUnpackerStore* store, int** bitArray)
      {
        bitArray[GDLCONF6::e_rvc      ] = &(store->m_rvc);
        bitArray[GDLCONF6::e_timtype  ] = &(store->m_timtype);
        bitArray[GDLCONF6::e_etyp     ] = &(store->m_etyp);
        bitArray[GDLCONF6::e_tttmdl   ] = &(store->m_tttmdl);
        bitArray[GDLCONF6::e_tdsrcp   ] = &(store->m_tdsrcp);
        bitArray[GDLCONF6::e_tdtopp   ] = &(store->m_tdtopp);
        bitArray[GDLCONF6::e_tdeclp   ] = &(store->m_tdeclp);
        bitArray[GDLCONF6::e_tdcdcp   ] = &(store->m_tdcdcp);
        bitArray[GDLCONF6::e_rvcout   ] = &(store->m_rvcout);
        bitArray[GDLCONF6::e_toprvc   ] = &(store->m_toprvc);
        bitArray[GDLCONF6::e_eclrvc   ] = &(store->m_eclrvc);
        bitArray[GDLCONF6::e_cdcrvc   ] = &(store->m_cdcrvc);
        bitArray[GDLCONF6::e_toptiming] = &(store->m_toptiming);
        bitArray[GDLCONF6::e_ecltiming] = &(store->m_ecltiming);
        bitArray[GDLCONF6::e_cdctiming] = &(store->m_cdctiming);
        bitArray[GDLCONF6::e_nim0rvc  ] = &(store->m_nim0rvc);
        bitArray[GDLCONF6::e_psn3     ] = &(store->m_psn3);
        bitArray[GDLCONF6::e_psn2     ] = &(store->m_psn2);
        bitArray[GDLCONF6::e_psn1     ] = &(store->m_psn1);
        bitArray[GDLCONF6::e_psn0     ] = &(store->m_psn0);
        bitArray[GDLCONF6::e_topslot1 ] = &(store->m_topslot1);
        bitArray[GDLCONF6::e_topslot0 ] = &(store->m_topslot0);
        bitArray[GDLCONF6::e_ntopslot ] = &(store->m_ntopslot);
        bitArray[GDLCONF6::e_ftd3     ] = &(store->m_ftd3);
        bitArray[GDLCONF6::e_ftd2     ] = &(store->m_ftd2);
        bitArray[GDLCONF6::e_ftd1     ] = &(store->m_ftd1);
        bitArray[GDLCONF6::e_ftd0     ] = &(store->m_ftd0);
        bitArray[GDLCONF6::e_itd4     ] = &(store->m_itd4);
        bitArray[GDLCONF6::e_itd3     ] = &(store->m_itd3);
        bitArray[GDLCONF6::e_itd2     ] = &(store->m_itd2);
        bitArray[GDLCONF6::e_itd1     ] = &(store->m_itd1);
        bitArray[GDLCONF6::e_itd0     ] = &(store->m_itd0);
        bitArray[GDLCONF6::e_evt      ] = &(store->m_evt);
        bitArray[GDLCONF6::e_clk      ] = &(store->m_clk);
        bitArray[GDLCONF6::e_firmid   ] = &(store->m_firmid);
        bitArray[GDLCONF6::e_firmver  ] = &(store->m_firmver);
        bitArray[GDLCONF6::e_finalrvc ] = &(store->m_finalrvc);
        bitArray[GDLCONF6::e_drvc     ] = &(store->m_drvc);
        bitArray[GDLCONF6::e_gdll1rvc ] = &(store->m_gdll1rvc);
        bitArray[GDLCONF6::e_coml1rvc ] = &(store->m_coml1rvc);
        bitArray[GDLCONF6::e_b2ldly   ] = &(store->m_b2ldly);
        bitArray[GDLCONF6::e_maxrvc   ] = &(store->m_maxrvc);
        bitArray[GDLCONF6::e_conf     ] = &(store->m_conf);
      }
    }

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

      /** Unpacker main function.*/
      virtual void fillTreeGDL0(int* buf, int evt);

      /** Unpacker main function.*/
      virtual void fillTreeGDL1(int* buf, int evt);

      /** Unpacker main function.*/
      virtual void fillTreeGDL2(int* buf, int evt);

      /** Unpacker main function.*/
      virtual void fillTreeGDL3(int* buf, int evt);

      /** Unpacker main function.*/
      virtual void fillTreeGDL4(int* buf, int evt);

      /** Unpacker main function.*/
      virtual void fillTreeGDL5(int* buf, int evt);

      /** Unpacker main function.*/
      virtual void fillTreeGDL6(int* buf, int evt);

    private:

      /** flag to select board search mode **/
      bool m_trgReadoutBoardSearch;

      StoreArray<TRGGDLUnpackerStore> store;

    };
  }
} // end namespace Belle2

#endif // TRGGDLUnpackerModule_H
