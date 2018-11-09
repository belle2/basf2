/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty   hearty@physics.ubc.ca               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/LogSystem.h>
#include <trg/gdl/dbobjects/TRGGDLDBPrescales.h>
#include <trg/gdl/dbobjects/TRGGDLDBFTDLBits.h>
#include <trg/gdl/dbobjects/TRGGDLDBInputBits.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
#include <trg/gdl/dbobjects/TRGGDLDBDelay.h>
#include <iostream>
#include <fstream>
//#include <TFile.h>
//#include <TH1F.h>

using namespace Belle2;

//prescale setting
void setprescale()
{

  const int N_BITS_RESERVED = 320;
  const int N_PSNM_ARRAY = 10;

  const int run[N_PSNM_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0, 0,    3, 107,
    3, 108,  3, 295,
    3, 296,  3, 1511,
    3, 1512, 3, 1810,
    3, 1811, 3, 1994,
    3, 1995, 3, 2000,
    3, 2001, 3, 2313,
    3, 2314, 3, 3503,
    3, 3504, 3, 5340,
    3, 5341, 10, 0
  };

  const int nbit[N_PSNM_ARRAY] = {18, 44, 63, 63, 63,
                                  72, 75, 75, 75, 75
                                 };

  const unsigned
  psnmValues[N_PSNM_ARRAY][N_BITS_RESERVED] = {
    // 0
    // psn_0055 r59-
    {
      0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 1
    },

    // 1
    // psn_0056 r108-
    {
      0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 1, 0, 1
    },

    // 2
    // psn_0057 r296-
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20
    },

    // 3
    // psn_0058 r1512-r1615 ecltiming=1
    // Not used for good data
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 1, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20
    },

    // 4
    // psn_0057 r1811- ecltiming=0
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20
    },

    // 5
    // psn_0059 r1995- 5 Dark bits
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 0, 0, 0, 0, 1, 1, 1,
      1, 1
    },

    // 6
    // psn_0059 r2001- 3 Dark bits
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 1
    },

    // 7
    // psn_0060 r2314- eclnimo03 30->300
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 300, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 1
    },

    // 8
    // psn_0063 e3504- bg(26)->1
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 300, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 1
    },

    // 9
    // psn_0064=65. e5341- eed,fed,fp,bha3d
    // Can be earlier than 5341 but
    // runsum files lost.
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 300, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1
    }
  };


  DBImportObjPtr<TRGGDLDBPrescales> prescales;
  prescales.construct();
  for (int i = 0; i < N_PSNM_ARRAY; i++) {
    IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
    //initialize
    for (int j = 0; j < N_BITS_RESERVED; j++) {
      prescales->setprescales(j, 0);
    }
    //set
    for (int j = 0; j < nbit[i]; j++) {
      prescales->setprescales(j, psnmValues[i][j]);
    }
    prescales->setnoutbit(nbit[i]);
    prescales.import(iov);
  }



}


//input and output bits
void setftdlbits()
{

  const int N_BITS_RESERVED = 320;
  const int N_OUTPUT_ARRAY = 5;

  const int run[N_OUTPUT_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0, 0,    3, 291,
    3, 292,  3, 1314,
    3, 1315, 3, 1511,
    3, 1512, 3, 5313,
    3, 5314, 10,    0
  };

  const int nbit[N_OUTPUT_ARRAY]     = {62, 63, 67, 75, 88};
  const int nbit_temp[N_OUTPUT_ARRAY] = {62, 63, 67, 75, 88 + 4};

  const char*
  outputBitNames[N_BITS_RESERVED] = {
    "zzz", "zzzo", "fff", "fffo", "zz", "zzo", "ff", "ffo", "hie", "lowe",
    "lume", "c2", "c3", "c4", "c5", "bha", "bha_trk", "bha_brl", "bha_ecp", "g_high",
    "g_c1", "gg", "mu_pair", "mu_b2b", "revo", "rand", "bg", "ecltiming", "nim0", "nima03",
    "nimo03", "period", "eclnima03", "eclnimo03", "pls", "poi", "klmhit", "f", "fe", "ffe",
    "fc", "ffc", "cdctiming", "cdcbb", "nim1c", "c1n0", "c1n1", "c1n2", "c1n3", "c1n4",
    "c2n1", "c2n2", "c2n3", "c2n4", "cdcecl1", "cdcecl2", "cdcecl3", "cdcecl4", "cdcklm1", "cdcklm2",
    "cdcklm3", "cdcklm4", "ffb", "uuu", "uuuo", "uub", "uuo", "c1hie", "c1lume", "n1hie",
    "n1lume", "c3hie", "c3lume", "n3hie", "n3lume", "eed", "fed", "fp", "bha3d", "shem",
    "ohem", "lml0", "lml1", "lml2", "lml3", "lml4", "lml5", "lml6", "lml7", "lml8",
    "lml9", "lml10"
  };

  const int
  outputMap[N_OUTPUT_ARRAY][N_BITS_RESERVED] = {

    // 0
    // 62 bit. gdl0065c
    // 10 <= run <= 261
    {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
      50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
      60, 61
    },

    // 1
    // 63 bit. gdl0065e, 65h, 65j, 65k
    // 292 <= run <= 480
    {
      0, 1, 2, 3, 4, 5, 62, 7, 8, 9,
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
      50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
      60, 61, 6
    },

    // 2
    // 67 bit. gdl0066a
    // 292 <= run <= 480
    {
      0, 1, 2, 3, 4, 5, 62, 7, 8, 9,
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
      50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
      60, 61, 6, 63, 64, 65, 66
    },

    // 3
    // 75 bit. gdl0066b(71bit), 66c,e,f,h,k, 67g.
    // 1512 <= run < 5314
    {
      0,  1,  2,  3,  4,  5,  62, 7,  8,  9,
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
      50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
      60, 61, 6,  63, 64, 65, 66, 67, 68, 69,
      70,  71,  72,  73,  74
    },

    // 4
    // 88 bit. gdl0068a, 68b
    // 5314 <= run
    {
      0,  1,  2,  3,  4,  5,  62,  7,  8,  9,
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
      50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
      60, 61, 6,  -1, -1, -1, -1, 67, 68, 69,
      70, 71, 72, 73, 74, 63, 64, 65, 66, 75,
      76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
      86,  87
    }

  };


  DBImportObjPtr<TRGGDLDBFTDLBits> ftdlbits;
  ftdlbits.construct();
  for (int i = 0; i < N_OUTPUT_ARRAY; i++) {
    IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
    //initialize
    for (int j = 0; j < N_BITS_RESERVED; j++) {
      ftdlbits->setoutbitname(j, "");
    }
    //set
    for (int j = 0; j < nbit_temp[i]; j++) {
      if ((outputMap[i][j] >= 0) && (outputMap[i][j] < nbit[i]))ftdlbits->setoutbitname(outputMap[i][j], outputBitNames[j]);
    }
    ftdlbits->setnoutbit(nbit[i]);
    ftdlbits.import(iov);
  }
}

void setinputbits()
{

  const int N_BITS_RESERVED = 320;
  const int N_INPUT_ARRAY = 5;

  const int run[N_INPUT_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0, 0,    3, 1314,
    3, 1315, 3, 1865,
    3, 1866, 3, 5313,
    3, 5314, 3, 5593,
    3, 5594, 10,    0
  };


  const int nbit[N_INPUT_ARRAY]     = {80, 87,  89, 109,  111};
  const int nbit_temp[N_INPUT_ARRAY] = {80, 87 + 2, 89, 109 + 3, 111 + 3};

  const char*
  inputBitNames[N_BITS_RESERVED] = {
    "t3_0", "t3_1", "t3_2", "t2_0", "t2_1", "t2_2", "cdc_open90", "cdc_active", "cdc_b2b3", "cdc_b2b5",
    "cdc_b2b7", "cdc_b2b9", "ehigh", "elow", "elum", "ecl_bha", "bha_0", "bha_1", "bha_2", "bha_3",
    "bha_4", "bha_5", "bha_6", "bha_7", "bha_8", "bha_9", "bha_10", "bha_11", "bha_12", "bha_13",
    "c_0", "c_1", "c_2", "c_3", "ebg_0", "ebg_1", "ebg_2", "ecl_active", "ecl_tim_fwd", "ecl_tim_brl",
    "ecl_tim_bwd", "ecl_phys", "top_0", "top_1", "top_2", "top_bb", "top_active", "klm_hit", "klm_0", "klm_1",
    "klm_2", "klm_3", "klmb2b_0", "klmb2b_1", "klmb2b_2", "revo", "her_kick", "ler_kick", "bha_delay", "pseud_rand",
    "plsin", "poiin", "periodin", "veto", "n1_0", "n1_1", "n1_2", "n2_0", "n2_1", "n2_2",
    "cdcecl_0", "cdcecl_1", "cdcecl_2", "cdcklm_0", "cdcklm_1", "cdcklm_2", "nim0", "nim1", "nim2", "nim3",
    "t3_3", "t2_3", "n1_3", "n2_3", "cdcecl_3", "cdcklm_3", "u2_0", "u2_1", "u2_2", "ecl_oflo",
    "ecl_3dbha", "lml_0", "lml_1", "lml_2", "lml_3", "lml_4", "lml_5", "lml_6", "lml_7", "lml_8", // 100
    "lml_9", "lml_10", "samhem", "opohem", "d_b2b3", "d_b2b5", "d_b2b7", "d_b2b9", "p_b2b3", "p_b2b5",
    "p_b2b7", "p_b2b9", "track", "trkflt"
  };

  const int
  inputMap[N_INPUT_ARRAY][N_BITS_RESERVED] = {
    {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
      50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
      60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
      70, 71, 72, 73, 74, 75, 76, 77, 78, 79
    },

    // "1"
    // 87 bits. gdl0065k-gdl0066e.
    // e3r1315 <= run < e3r1866
    {
      0, 1, 2, 4, 5, 6, 8, 9, 10, 11,
      12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
      22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
      32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
      42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
      52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
      62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
      72, 73, 74, 76, 77, 78, 80, 81, 82, 83,
      3, 7, -1, -1, 75, 79, 84, 85, 86
    },

    // "2"
    // 89 bits. gdl0066f-gdl0067g.
    // e3r1866 <= run < 5314
    {
      0, 1, 2, 4, 5, 6, 8, 9, 10, 11,
      12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
      22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
      32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
      42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
      52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
      62, 63, 64, 65, 66, 67, 68, 70, 71, 72,
      74, 75, 76, 78, 79, 80, 82, 83, 84, 85,
      3, 7, 69, 73, 77, 81, 86, 87, 88
    },

    // "3"
    // 109 bits. gdl0068a.
    // e3r5314 <= run < 5594
    {
      0, 1, 2, 4, 5, 6, 8, 9, 10, 11,
      12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
      22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
      32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
      42, 43, 57, 58, 59, 60, 61, 62, 63, 64,
      65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
      75, 76, 77, 78, 81, 82, 83, 85, 86, 87,
      89, 90, 91, 93, 94, 95, 105, 106, 107, 108,
      3, 7, 84, 88, 92, 96, -1, -1, -1, 44,
      45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
      55, 56, 79, 80, 97, 98, 99, 100, 101, 102,
      103, 104
    },

    // "4"
    // 111 bits. track, trkflt added. gdl0068b
    // e3r5594 <= run
    {
      0, 1, 2, 4, 5, 6, 8, 9, 10, 11,
      12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
      22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
      32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
      42, 43, 57, 58, 59, 60, 61, 62, 63, 64,
      65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
      75, 76, 77, 78, 81, 82, 83, 85, 86, 87,
      89, 90, 91, 93, 94, 95, 107, 108, 109, 110,
      3, 7, 84, 88, 92, 96, -1, -1, -1, 44,
      45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
      55, 56, 79, 80, 97, 98, 99, 100, 101, 102,
      103, 104, 105, 106
    },

  };


  DBImportObjPtr<TRGGDLDBInputBits> inputbits;
  inputbits.construct();
  for (int i = 0; i < N_INPUT_ARRAY; i++) {
    IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
    //initialize
    for (int j = 0; j < N_BITS_RESERVED; j++) {
      inputbits->setinbitname(j, "");
    }
    //set
    for (int j = 0; j < nbit_temp[i]; j++) {
      if ((inputMap[i][j] >= 0) && (inputMap[i][j] < nbit[i]))inputbits->setinbitname(inputMap[i][j], inputBitNames[j]);
    }
    inputbits->setninbit(nbit[i]);
    inputbits.import(iov);
  }

}

void setunpacker()
{

  const int N_LEAF = 320;
  const int N_UNPACKER_ARRAY = 7;

  const int run[N_UNPACKER_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0,    0, 3,  528,
    3,  529, 3,  676,
    3,  677, 3, 1314,
    3, 1315, 3, 1865,
    3, 1866, 3, 4790,
    3, 4791, 3, 5313,
    3, 5314, 10,    0
  };

  /** num of leafs in data_b2l **/
  const int nLeafs[N_UNPACKER_ARRAY] = {
    37, 27, 26, 26, 26, 31, 32
  };
  /** num of leafs for others **/
  const int nLeafsExtra[N_UNPACKER_ARRAY] = {
    8, 9, 11, 11, 11, 11, 11
  };
  /** num of clk time window **/
  const int nClks[N_UNPACKER_ARRAY] = {
    48, 48, 48, 48, 32, 32, 32
  };
  /** num of bits **/
  const int nBits[N_UNPACKER_ARRAY] = {
    640, 640, 640, 640, 640, 640, 640
  };

  /** num of inputleafmap raw **/
  const int nrows[N_UNPACKER_ARRAY] = {
    45, 51, 52, 52, 52, 57, 61
  };


  /** leaf names **/
  const char LeafNames[N_LEAF][100] = {
    "etffmv",   "l1rvc",    "timtype",  "etyp",    "final",
    "gdll1rvc", "etfout",   "etfvd",    "toprvc",  "topvd",
    "toptiming", "ecltiming", "cdctiming", "rvcout",  "rvcout0",
    "comrvc",   "etnrvc",   "nim0rvc",  "eclrvc",  "rvc",
    "drvc",     "ftd2",     "psn2",     "psn1",    "etfth2",
    "etfhdt",   "etfth",    "psn0",     "ftd1",    "cdcrvc",
    "ftd0",     "itd2",     "itd1",     "itd0",    "inp2",
    "inp1",     "inp0",     "evt",      "clk",     "firmid",
    "firmver",  "coml1rvc", "b2ldly",   "maxrvc",  "conf",
    "dtoprvc",  "declrvc",  "dcdcrvc",  "topslot1", "topslot0",
    "ntopslot", "finalrvc", "tttmdl",   "tdsrcp",  "tdtopp",
    "tdeclp",   "tdcdcp",   "psn3",     "ftd3",    "itd4",
    "itd3"
  };



  const int
  inputleafMap[N_UNPACKER_ARRAY][N_LEAF] = {
    {
      //0
      0, 1, 2, 3, 4,
      5, 6, 7, 8, 9,
      10, 11, 12, 13, 14,
      15, 16, 17, 18, 19,
      20, 21, 22, 23, 24,
      25, 26, 27, 28, 29,
      30, 31, 32, 33, 34,
      35, 36, 37, 38, 39,
      40, 41, 42, 43, 44
    },

    {
      //1
      -1, -1, 1, 2, -1,
      31, -1, -1, 5, -1,
      8, 9, 10, 3, -1,
      -1, -1, 11, 6, 0,
      4, 21, 15, 16, -1,
      -1, -1, 17, 22, 7,
      23, 24, 25, 26, -1,
      -1, -1, 27, 28, 29,
      30, 32, 33, 34, 35,
      12, 13, 14, 18, 19,
      20
    },

    {
      //2
      -1, -1, 1, 2, -1,
      32, -1, -1, 4, -1,
      7, 8, 9, 3, -1,
      -1, -1, 10, 5, 0,
      30, 20, 14, 15, -1,
      -1, -1, 16, 21, 6,
      22, 23, 24, 25, -1,
      -1, -1, 26, 27, 28,
      29, 33, 34, 35, 36,
      11, 12, 13, 17, 18,
      19, 31
    },

    {
      //3
      -1, -1, 1, 2, -1,
      32, -1, -1, 4, -1,
      7, 8, 9, 3, -1,
      -1, -1, 10, 5, 0,
      30, 20, 14, 15, -1,
      -1, -1, 16, 21, 6,
      22, 23, 24, 25, -1,
      -1, -1, 26, 27, 28,
      29, 33, 34, 35, 36,
      11, 12, 13, 17, 18,
      19, 31
    },

    {
      //4
      -1, -1, 1, 2, -1,
      32, -1, -1, 4, -1,
      7,  8, 9, 3, -1,
      -1, -1, 10, 5, 0,
      30, 20, 14, 15, -1,
      -1, -1, 16, 21, 6,
      22, 23, 24, 25, -1,
      -1, -1, 26, 27, 28,
      29, 33, 34, 35, 36,
      11, 12, 13, 17, 18,
      19, 31
    },

    {
      //5
      -1, -1, 1, 2, -1,
      37, -1, -1, 9, -1,
      12, 13, 14, 8, -1,
      -1, -1, 15, 10, 0,
      36, 25, 19, 20, -1,
      -1, -1, 21, 26, 11,
      27, 28, 29, 30, -1,
      -1, -1, 31, 32, 33,
      34, 38, 39, 40, 41,
      16, 17, 18, 22, 23,
      24, 35, 3, 4, 5,
      6, 7
    },

    {
      //6
      -1, -1, 1, 2, -1,
      38, -1, -1, 9, -1,
      12, 13, 14, 8, -1,
      -1, -1, 15, 10, 0,
      37, 24, 17, 18, -1,
      -1, -1, 19, 25, 11,
      26, 29, 30, 31, -1,
      -1, -1, 32, 33, 34,
      35, 39, 40, 41, 42,
      -1, -1, -1, 20, 21,
      22, 36, 3, 4, 5,
      6, 7, 16, 23, 27,
      28
    }

  };

  /** bus bit map. (a downto a-b) **/
  const int BitMap[N_UNPACKER_ARRAY][N_LEAF][2] = {
    {
      //0
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
    },

    {
      //1
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
    },

    {
      //2
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
    },

    {
      //3
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
    },

    {
      //4
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
    },

    {
      //5
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
    },

    {
      //6
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
    }
  };

  int m_nword_header[N_UNPACKER_ARRAY] {
    3, 4, 6, 6, 6, 6, 6
  };
  int m_conf[N_UNPACKER_ARRAY] {
    0, 1, 2, 3, 4, 5, 6
  };
  int m_nword_input[N_UNPACKER_ARRAY] {
    3, 3, 3, 3, 3, 3, 5
  };
  int m_nword_output[N_UNPACKER_ARRAY] {
    3, 3, 3, 3, 3, 3, 3
  };

  const int BitMap_extra[N_UNPACKER_ARRAY][N_LEAF][3] = {
    {
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      2,  0, 12, //coml1rvc
      2, 12,  9, //b2ldly
      2, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3,  0,  9, //gdll1rvc
      3,  1, 12, //coml1rvc
      3, 12,  9, //b2ldly
      3, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    }

  };


  DBImportObjPtr<TRGGDLDBUnpacker> unpacker;
  unpacker.construct();
  for (int i = 0; i < N_UNPACKER_ARRAY; i++) {
    IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);

    unpacker->setnLeafs(nLeafs[i]);
    unpacker->setnLeafsExtra(nLeafsExtra[i]);
    unpacker->setnClks(nClks[i]);
    unpacker->setnBits(nBits[i]);
    unpacker->set_nword_header(m_nword_header[i]);
    unpacker->set_nword_input(m_nword_input[i]);
    unpacker->set_nword_output(m_nword_output[i]);
    unpacker->setconf(m_conf[i]);

    //initialize
    for (int j = 0; j < N_LEAF; j++) {
      unpacker->setLeafName(j, "");
      unpacker->setLeafMap(j, -1);
    }
    //set
    for (int j = 0; j < nrows[i]; j++) {
      if ((inputleafMap[i][j] >= 0) && (inputleafMap[i][j] < nLeafs[i] + nLeafsExtra[i])) {
        unpacker->setLeafName(j, LeafNames[j]);
        unpacker->setLeafMap(j, inputleafMap[i][j]);
      }
    }

    for (int j = 0; j < N_LEAF; j++) {
      unpacker->setBitMap(j, 0, BitMap[i][j][0]);
      unpacker->setBitMap(j, 1, BitMap[i][j][1]);
    }
    for (int j = 0; j < N_LEAF; j++) {
      unpacker->setBitMap_extra(j, 0, BitMap_extra[i][j][0]);
      unpacker->setBitMap_extra(j, 1, BitMap_extra[i][j][1]);
      unpacker->setBitMap_extra(j, 2, BitMap_extra[i][j][2]);
    }


    unpacker.import(iov);
  }

}

void setdelay()
{

  const int N_DELAY_ARRAY = 31;

  const int run[N_DELAY_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    //0,    0, 3,   63,
    //0,  108, 3,  480,
    0,    0, 3,  480,
    3,  529, 3,  816,
    3,  932, 3, 1162,
    3, 1176, 3, 1562,
    3, 1596, 3, 2118,
    3, 2119, 3, 2119,
    3, 2120, 3, 2288,
    3, 2292, 3, 2392,
    3, 2401, 3, 2586,
    3, 2587, 3, 2587,
    3, 2588, 3, 3127,
    3, 3128, 3, 3128,
    3, 3129, 3, 3414,
    3, 3415, 3, 3421,
    3, 3422, 3, 3526,
    3, 3527, 3, 3530,
    3, 3531, 3, 3790,
    3, 3822, 3, 4068,
    3, 4069, 3, 4072,
    3, 4073, 3, 4790,
    3, 4791, 3, 4897,
    3, 4898, 3, 5187,
    3, 5188, 3, 5188,
    3, 5189, 3, 5226,
    3, 5227, 3, 5255,
    3, 5314, 3, 5326,
    3, 5341, 3, 5556,
    3, 5557, 3, 5592,
    3, 5594, 3, 5869,
    3, 5870, 3, 5959,
    3, 5975, 10,    0
  };

  const int data_num[N_DELAY_ARRAY] = {
    //29,
    //30,
    30,
    31,
    32,
    33,
    34,
    -1,
    34,
    35,
    36,
    -1,
    36,
    -1,
    36,
    -1,
    36,
    -1,
    36,
    37,
    -1,
    37,
    -1,
    37,
    -1,
    37,
    38,
    -1,
    39,
    40,
    41,
    42,
    41
  };


  DBImportObjPtr<TRGGDLDBDelay> delay;
  delay.construct();
  char logname[2000];
  for (int i = 0; i < N_DELAY_ARRAY; i++) {
    IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
    for (int j = 0; j < 320; j++) {
      delay->setdelay(j, 0);
    }
    sprintf(logname, "trg/gdl/dbobjects/log/itd_%04d.dat", data_num[i]);
    std::ifstream ifs(logname);
    if (!ifs) {
      std::cout << "No logfile to read delay values" << " " << logname << std::endl;
    }
    int k = 0;
    int delay_temp = 0;
    while (ifs >> k >> delay_temp) {
      delay->setdelay(k, delay_temp);
    }
    delay.import(iov);
    ifs.close();
  }

}

int main()
{

  setprescale();
  setftdlbits();
  setinputbits();
  setunpacker();
  setdelay();

}


