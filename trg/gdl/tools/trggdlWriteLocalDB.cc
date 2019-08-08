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
#include <mdst/dbobjects/TRGGDLDBPrescales.h>
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>
#include <mdst/dbobjects/TRGGDLDBBadrun.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
#include <trg/gdl/dbobjects/TRGGDLDBDelay.h>
#include <iostream>
#include <fstream>
//#include <TFile.h>
//#include <TH1F.h>

using namespace Belle2;

//#define ONLINE 0
#define ONLINE 1

//prescale setting
void setprescale()
{

  const int N_BITS_RESERVED = 320;
  const int N_PSNM_ARRAY = 11;

  const int run[N_PSNM_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0, 0,   -1, -1,
    0, 0,    3, 107,
    3, 108,  3, 295,
    3, 296,  3, 1511,
    3, 1512, 3, 1810,
    3, 1811, 3, 1994,
    3, 1995, 3, 2000,
    3, 2001, 3, 2313,
    3, 2314, 3, 3503,
    3, 3504, 3, 5340,
    3, 5341, 4, 0
  };

  const int nbit[N_PSNM_ARRAY] = { 0, 18, 44, 63, 63,
                                   63, 72, 75, 75, 75,
                                   75
                                 };

  const unsigned
  psnmValues[N_PSNM_ARRAY][N_BITS_RESERVED] = {
    // -1
    {
      0
    },

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
  if (ONLINE == 0) {
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
  } else if (ONLINE == 1) {
    //for (int i = N_PSNM_ARRAY-1; i < N_PSNM_ARRAY; i++) {
    for (int i = 0; i < 1; i++) {
      IntervalOfValidity iov(5, 0, -1, -1);
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


}


//input and output bits
void setftdlbits()
{

  const int N_BITS_RESERVED = 320;
  const int N_OUTPUT_ARRAY = 7;

  const int run[N_OUTPUT_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0,    0, -1,  -1,
    0, 0,    3,  291,
    3, 292,  3, 1314,
    3, 1315, 3, 1511,
    3, 1512, 3, 5313,
    3, 5314, 6,   -1,
    7, 0,    -1,  -1
  };

  const int nbit[N_OUTPUT_ARRAY]      = {62, 62, 63, 67, 75, 88,
                                         134
                                        };


  const char*
  outputBitNames[N_OUTPUT_ARRAY][N_BITS_RESERVED] = {
    //0
    {
      "zzz", "zzzo", "fff", "fffo", "zz", "zzo", "ff", "ffo", "hie", "lowe", "lume", "c2", "c3", "c4", "c5", "bha", "bha_trk", "bha_brl", "bha_ecp", "g_high", "g_c1", "gg", "mu_pair", "mu_b2b", "revo", "rand", "bg", "ecltiming", "nim0", "nima03", "nimo03", "period", "eclnima03", "eclnimo03", "pls", "poi", "klmhit", "f", "fe", "ffe", "fc", "ffc", "cdctiming", "cdcbb", "nim1c", "c1n0", "c1n1", "c1n2", "c1n3", "c1n4", "c2n1", "c2n2", "c2n3", "c2n4", "cdcecl1", "cdcecl2", "cdcecl3", "cdcecl4", "cdcklm1", "cdcklm2", "cdcklm3", "cdcklm4"
    },
    //1
    {
      "zzz", "zzzo", "fff", "fffo", "zz", "zzo", "ff", "ffo", "hie", "lowe", "lume", "c2", "c3", "c4", "c5", "bha", "bha_trk", "bha_brl", "bha_ecp", "g_high", "g_c1", "gg", "mu_pair", "mu_b2b", "revo", "rand", "bg", "ecltiming", "nim0", "nima03", "nimo03", "period", "eclnima03", "eclnimo03", "pls", "poi", "klmhit", "f", "fe", "ffe", "fc", "ffc", "cdctiming", "cdcbb", "nim1c", "c1n0", "c1n1", "c1n2", "c1n3", "c1n4", "c2n1", "c2n2", "c2n3", "c2n4", "cdcecl1", "cdcecl2", "cdcecl3", "cdcecl4", "cdcklm1", "cdcklm2", "cdcklm3", "cdcklm4"
    },
    //2
    {
      "zzz", "zzzo", "fff", "fffo", "zz", "zzo", "ffb", "ffo", "hie", "lowe", "lume", "c2", "c3", "c4", "c5", "bha", "bha_trk", "bha_brl", "bha_ecp", "g_high", "g_c1", "gg", "mu_pair", "mu_b2b", "revo", "rand", "bg", "ecltiming", "nim0", "nima03", "nimo03", "period", "eclnima03", "eclnimo03", "pls", "poi", "klmhit", "f", "fe", "ffe", "fc", "ffc", "cdctiming", "cdcbb", "nim1c", "c1n0", "c1n1", "c1n2", "c1n3", "c1n4", "c2n1", "c2n2", "c2n3", "c2n4", "cdcecl1", "cdcecl2", "cdcecl3", "cdcecl4", "cdcklm1", "cdcklm2", "cdcklm3", "cdcklm4", "ff"
    },
    //3
    {
      "zzz", "zzzo", "fff", "fffo", "zz", "zzo", "ffb", "ffo", "hie", "lowe", "lume", "c2", "c3", "c4", "c5", "bha", "bha_trk", "bha_brl", "bha_ecp", "g_high", "g_c1", "gg", "mu_pair", "mu_b2b", "revo", "rand", "bg", "ecltiming", "nim0", "nima03", "nimo03", "period", "eclnima03", "eclnimo03", "pls", "poi", "klmhit", "f", "fe", "ffe", "fc", "ffc", "cdctiming", "cdcbb", "nim1c", "c1n0", "c1n1", "c1n2", "c1n3", "c1n4", "c2n1", "c2n2", "c2n3", "c2n4", "cdcecl1", "cdcecl2", "cdcecl3", "cdcecl4", "cdcklm1", "cdcklm2", "cdcklm3", "cdcklm4", "ff", "uuu", "uuuo", "uub", "uuo"
    },
    //4
    {
      "zzz", "zzzo", "fff", "fffo", "zz", "zzo", "ffb", "ffo", "hie", "lowe", "lume", "c2", "c3", "c4", "c5", "bha", "bha_trk", "bha_brl", "bha_ecp", "g_high", "g_c1", "gg", "mu_pair", "mu_b2b", "revo", "rand", "bg", "ecltiming", "nim0", "nima03", "nimo03", "period", "eclnima03", "eclnimo03", "pls", "poi", "klmhit", "f", "fe", "ffe", "fc", "ffc", "cdctiming", "cdcbb", "nim1c", "c1n0", "c1n1", "c1n2", "c1n3", "c1n4", "c2n1", "c2n2", "c2n3", "c2n4", "cdcecl1", "cdcecl2", "cdcecl3", "cdcecl4", "cdcklm1", "cdcklm2", "cdcklm3", "cdcklm4", "ff", "uuu", "uuuo", "uub", "uuo", "c1hie", "c1lume", "n1hie", "n1lume", "c3hie", "c3lume", "n3hie", "n3lume"
    },
    //5
    {
      "zzz", "zzzo", "fff", "fffo", "zz", "zzo", "ffb", "ffo", "hie", "lowe", "lume", "c2", "c3", "c4", "c5", "bha", "bha_trk", "bha_brl", "bha_ecp", "g_high", "g_c1", "gg", "mu_pair", "mu_b2b", "revo", "rand", "bg", "ecltiming", "nim0", "nima03", "nimo03", "period", "eclnima03", "eclnimo03", "pls", "poi", "klmhit", "f", "fe", "ffe", "fc", "ffc", "cdctiming", "cdcbb", "nim1c", "c1n0", "c1n1", "c1n2", "c1n3", "c1n4", "c2n1", "c2n2", "c2n3", "c2n4", "cdcecl1", "cdcecl2", "cdcecl3", "cdcecl4", "cdcklm1", "cdcklm2", "cdcklm3", "cdcklm4", "ff", "eed", "fed", "fp", "bha3d", "c1hie", "c1lume", "n1hie", "n1lume", "c3hie", "c3lume", "n3hie", "n3lume", "shem", "ohem", "lml0", "lml1", "lml2", "lml3", "lml4", "lml5", "lml6", "lml7", "lml8", "lml9", "lml10"
    },
    //6
    {
      "fff", "ffs", "fss", "sss", "ffz", "fzz", "zzz", "ffy", "fyy", "yyy", "ff", "fs", "ss", "fz", "zz", "fy", "yy", "ffo", "fso", "sso", "fzo", "fyo", "ffb", "fsb", "ssb", "fzb", "fyb", "hie", "lowe", "lume", "c2", "c3", "c4", "c5", "bha3d", "bha", "bha_trk", "bha_brl", "bha_ecp", "bhapur", "eclmumu", "bhauni", "ecloflo", "g_high", "g_c1", "gg", "eed", "fed", "fp", "sp", "zp", "yp", "d_5", "shem", "ohem", "toptiming", "ecltiming", "cdctiming", "cdcbb", "mu_pair", "mu_b2b", "klmhit", "revolution", "random", "bg", "pls", "poi", "f", "s", "z", "y", "nim0", "nima03", "nimo03", "eclnima03", "eclnimo03", "n1gev0", "n1gev1", "n1gev2", "n1gev3", "n1gev4", "n2gev1", "n2gev2", "n2gev3", "n2gev4", "c2gev1", "c2gev2", "c2gev3", "c2gev4", "cdcecl1", "cdcecl2", "cdcecl3", "cdcecl4", "cdcklm1", "cdcklm2", "cdcklm3", "cdcklm4", "cdctop1", "cdctop2", "cdctop3", "cdctop4", "c1hie", "c1lume", "n1hie", "n1lume", "c3hie", "c3lume", "n3hie", "n3lume", "lml0", "lml1", "lml2", "lml3", "lml4", "lml5", "lml6", "lml7", "lml8", "lml9", "lml10", "lml11", "zzzv", "yyyv", "fffv", "zzv", "yyv", "ffov", "hiev", "lumev", "c4v", "bhav", "bhapurv", "mu_pairv", "bha3dv"
    }
  };



  DBImportObjPtr<TRGGDLDBFTDLBits> ftdlbits;
  ftdlbits.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_OUTPUT_ARRAY; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        ftdlbits->setoutbitname(j, "");
      }
      //set
      for (int j = 0; j < nbit[i]; j++) {
        //std::cout << i << " " << j << " " << outputBitNames[i][j] << std::endl;
        ftdlbits->setoutbitname(j, outputBitNames[i][j]);
      }
      ftdlbits->setnoutbit(nbit[i]);
      ftdlbits.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_OUTPUT_ARRAY - 1; i < N_OUTPUT_ARRAY; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        ftdlbits->setoutbitname(j, "");
      }
      //set
      for (int j = 0; j < nbit[i]; j++) {
        //std::cout << i << " " << j << " " << outputBitNames[i][j] << std::endl;
        ftdlbits->setoutbitname(j, outputBitNames[i][j]);
      }
      ftdlbits->setnoutbit(nbit[i]);
      ftdlbits.import(iov);
    }
  }

}

void setinputbits()
{

  const int N_BITS_RESERVED = 320;
  const int N_INPUT_ARRAY = 7;

  const int run[N_INPUT_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0, 0,    -1,  -1,
    0, 0,    3, 1314,
    3, 1315, 3, 1865,
    3, 1866, 3, 5313,
    3, 5314, 3, 5593,
    3, 5594,  6,  -1,
    7, 0,    -1,  -1
  };


  const int nbit[N_INPUT_ARRAY]      = {
    80, 80, 87,  89, 109,  111,
    130
  };

  const char*
  inputBitNames[N_INPUT_ARRAY][N_BITS_RESERVED] = {
    //0
    {
      "t3_0", "t3_1", "t3_2", "t2_0", "t2_1", "t2_2", "cdc_open90", "cdc_active", "cdc_b2b3", "cdc_b2b5", "cdc_b2b7", "cdc_b2b9", "ehigh", "elow", "elum", "ecl_bha", "bha_0", "bha_1", "bha_2", "bha_3", "bha_4", "bha_5", "bha_6", "bha_7", "bha_8", "bha_9", "bha_10", "bha_11", "bha_12", "bha_13", "c_0", "c_1", "c_2", "c_3", "ebg_0", "ebg_1", "ebg_2", "ecl_active", "ecl_tim_fwd", "ecl_tim_brl", "ecl_tim_bwd", "ecl_phys", "top_0", "top_1", "top_2", "top_bb", "top_active", "klm_hit", "klm_0", "klm_1", "klm_2", "klm_3", "klmb2b_0", "klmb2b_1", "klmb2b_2", "revo", "her_kick", "ler_kick", "bha_delay", "pseud_rand", "plsin", "poiin", "periodin", "veto", "n1_0", "n1_1", "n1_2", "n2_0", "n2_1", "n2_2", "cdcecl_0", "cdcecl_1", "cdcecl_2", "cdcklm_0", "cdcklm_1", "cdcklm_2", "nim0", "nim1", "nim2", "nim3"
    },
    //1
    {
      "t3_0", "t3_1", "t3_2", "t2_0", "t2_1", "t2_2", "cdc_open90", "cdc_active", "cdc_b2b3", "cdc_b2b5", "cdc_b2b7", "cdc_b2b9", "ehigh", "elow", "elum", "ecl_bha", "bha_0", "bha_1", "bha_2", "bha_3", "bha_4", "bha_5", "bha_6", "bha_7", "bha_8", "bha_9", "bha_10", "bha_11", "bha_12", "bha_13", "c_0", "c_1", "c_2", "c_3", "ebg_0", "ebg_1", "ebg_2", "ecl_active", "ecl_tim_fwd", "ecl_tim_brl", "ecl_tim_bwd", "ecl_phys", "top_0", "top_1", "top_2", "top_bb", "top_active", "klm_hit", "klm_0", "klm_1", "klm_2", "klm_3", "klmb2b_0", "klmb2b_1", "klmb2b_2", "revo", "her_kick", "ler_kick", "bha_delay", "pseud_rand", "plsin", "poiin", "periodin", "veto", "n1_0", "n1_1", "n1_2", "n2_0", "n2_1", "n2_2", "cdcecl_0", "cdcecl_1", "cdcecl_2", "cdcklm_0", "cdcklm_1", "cdcklm_2", "nim0", "nim1", "nim2", "nim3"
    },
    //2
    {
      "t3_0", "t3_1", "t3_2", "t3_3", "t2_0", "t2_1", "t2_2", "t2_3", "cdc_open90", "cdc_active", "cdc_b2b3", "cdc_b2b5", "cdc_b2b7", "cdc_b2b9", "ehigh", "elow", "elum", "ecl_bha", "bha_0", "bha_1", "bha_2", "bha_3", "bha_4", "bha_5", "bha_6", "bha_7", "bha_8", "bha_9", "bha_10", "bha_11", "bha_12", "bha_13", "c_0", "c_1", "c_2", "c_3", "ebg_0", "ebg_1", "ebg_2", "ecl_active", "ecl_tim_fwd", "ecl_tim_brl", "ecl_tim_bwd", "ecl_phys", "top_0", "top_1", "top_2", "top_bb", "top_active", "klm_hit", "klm_0", "klm_1", "klm_2", "klm_3", "klmb2b_0", "klmb2b_1", "klmb2b_2", "revo", "her_kick", "ler_kick", "bha_delay", "pseud_rand", "plsin", "poiin", "periodin", "veto", "n1_0", "n1_1", "n1_2", "n2_0", "n2_1", "n2_2", "cdcecl_0", "cdcecl_1", "cdcecl_2", "cdcecl_3", "cdcklm_0", "cdcklm_1", "cdcklm_2", "cdcklm_3", "nim0", "nim1", "nim2", "nim3", "u2_0", "u2_1", "u2_2"
    },
    //3
    {
      "t3_0", "t3_1", "t3_2", "t3_3", "t2_0", "t2_1", "t2_2", "t2_3", "cdc_open90", "cdc_active", "cdc_b2b3", "cdc_b2b5", "cdc_b2b7", "cdc_b2b9", "ehigh", "elow", "elum", "ecl_bha", "bha_0", "bha_1", "bha_2", "bha_3", "bha_4", "bha_5", "bha_6", "bha_7", "bha_8", "bha_9", "bha_10", "bha_11", "bha_12", "bha_13", "c_0", "c_1", "c_2", "c_3", "ebg_0", "ebg_1", "ebg_2", "ecl_active", "ecl_tim_fwd", "ecl_tim_brl", "ecl_tim_bwd", "ecl_phys", "top_0", "top_1", "top_2", "top_bb", "top_active", "klm_hit", "klm_0", "klm_1", "klm_2", "klm_3", "klmb2b_0", "klmb2b_1", "klmb2b_2", "revo", "her_kick", "ler_kick", "bha_delay", "pseud_rand", "plsin", "poiin", "periodin", "veto", "n1_0", "n1_1", "n1_2", "n1_3", "n2_0", "n2_1", "n2_2", "n2_3", "cdcecl_0", "cdcecl_1", "cdcecl_2", "cdcecl_3", "cdcklm_0", "cdcklm_1", "cdcklm_2", "cdcklm_3", "nim0", "nim1", "nim2", "nim3", "u2_0", "u2_1", "u2_2"
    },
    //4
    {
      "t3_0", "t3_1", "t3_2", "t3_3", "t2_0", "t2_1", "t2_2", "t2_3", "cdc_open90", "cdc_active", "cdc_b2b3", "cdc_b2b5", "cdc_b2b7", "cdc_b2b9", "ehigh", "elow", "elum", "ecl_bha", "bha_0", "bha_1", "bha_2", "bha_3", "bha_4", "bha_5", "bha_6", "bha_7", "bha_8", "bha_9", "bha_10", "bha_11", "bha_12", "bha_13", "c_0", "c_1", "c_2", "c_3", "ebg_0", "ebg_1", "ebg_2", "ecl_active", "ecl_tim_fwd", "ecl_tim_brl", "ecl_tim_bwd", "ecl_phys", "ecl_oflo", "ecl_3dbha", "lml_0", "lml_1", "lml_2", "lml_3", "lml_4", "lml_5", "lml_6", "lml_7", "lml_8", "lml_9", "lml_10", "top_0", "top_1", "top_2", "top_bb", "top_active", "klm_hit", "klm_0", "klm_1", "klm_2", "klm_3", "klmb2b_0", "klmb2b_1", "klmb2b_2", "revo", "her_kick", "ler_kick", "bha_delay", "pseud_rand", "plsin", "poiin", "periodin", "veto", "samhem", "opohem", "n1_0", "n1_1", "n1_2", "n1_3", "n2_0", "n2_1", "n2_2", "n2_3", "cdcecl_0", "cdcecl_1", "cdcecl_2", "cdcecl_3", "cdcklm_0", "cdcklm_1", "cdcklm_2", "cdcklm_3", "d_b2b3", "d_b2b5", "d_b2b7", "d_b2b9", "p_b2b3", "p_b2b5", "p_b2b7", "p_b2b9", "nim0", "nim1", "nim2", "nim3"
    },
    //5
    {
      "t3_0", "t3_1", "t3_2", "t3_3", "t2_0", "t2_1", "t2_2", "t2_3", "cdc_open90", "cdc_active", "cdc_b2b3", "cdc_b2b5", "cdc_b2b7", "cdc_b2b9", "ehigh", "elow", "elum", "ecl_bha", "bha_0", "bha_1", "bha_2", "bha_3", "bha_4", "bha_5", "bha_6", "bha_7", "bha_8", "bha_9", "bha_10", "bha_11", "bha_12", "bha_13", "c_0", "c_1", "c_2", "c_3", "ebg_0", "ebg_1", "ebg_2", "ecl_active", "ecl_tim_fwd", "ecl_tim_brl", "ecl_tim_bwd", "ecl_phys", "ecl_oflo", "ecl_3dbha", "lml_0", "lml_1", "lml_2", "lml_3", "lml_4", "lml_5", "lml_6", "lml_7", "lml_8", "lml_9", "lml_10", "top_0", "top_1", "top_2", "top_bb", "top_active", "klm_hit", "klm_0", "klm_1", "klm_2", "klm_3", "klmb2b_0", "klmb2b_1", "klmb2b_2", "revo", "her_kick", "ler_kick", "bha_delay", "pseud_rand", "plsin", "poiin", "periodin", "veto", "samhem", "opohem", "n1_0", "n1_1", "n1_2", "n1_3", "n2_0", "n2_1", "n2_2", "n2_3", "cdcecl_0", "cdcecl_1", "cdcecl_2", "cdcecl_3", "cdcklm_0", "cdcklm_1", "cdcklm_2", "cdcklm_3", "d_b2b3", "d_b2b5", "d_b2b7", "d_b2b9", "p_b2b3", "p_b2b5", "p_b2b7", "p_b2b9", "track", "trkflt", "nim0", "nim1", "nim2", "nim3"
    },
    //6
    {
      "t3_0", "t3_1", "t3_2", "t3_3", "ty_0", "ty_1", "ty_2", "ty_3", "t2_0", "t2_1", "t2_2", "t2_3", "ts_0", "ts_1", "ts_2", "ts_3", "cdc_open90", "cdc_active", "cdc_b2b3", "cdc_b2b5", "cdc_b2b7", "cdc_b2b9", "ehigh", "elow", "elum", "ecl_bha", "bha_type_0", "bha_type_1", "bha_type_2", "bha_type_3", "bha_type_4", "bha_type_5", "bha_type_6", "bha_type_7", "bha_type_8", "bha_type_9", "bha_type_10", "bha_type_11", "bha_type_12", "bha_type_13", "clst_0", "clst_1", "clst_2", "clst_3", "ecl_bg_0", "ecl_bg_1", "ecl_bg_2", "ecl_active", "ecl_timing_fwd", "ecl_timing_brl", "ecl_timing_bwd", "ecl_phys", "ecl_oflo", "ecl_3dbha", "ecl_lml_0", "ecl_lml_1", "ecl_lml_2", "ecl_lml_3", "ecl_lml_4", "ecl_lml_5", "ecl_lml_6", "ecl_lml_7", "ecl_lml_8", "ecl_lml_9", "ecl_lml_10", "ecl_lml_11", "ecl_bhauni", "ecl_mumu", "ecl_bhapur", "top_0", "top_1", "top_2", "top_bb", "top_active", "klm_hit", "klm_0", "klm_1", "klm_2", "klm_3", "klmb2b_0", "klmb2b_1", "klmb2b_2", "revo", "her_kick", "ler_kick", "bha_delay", "pseud_rand", "plsin", "poissonin", "veto", "samhem", "opohem", "n1gev_0", "n1gev_1", "n1gev_2", "n1gev_3", "n2gev_0", "n2gev_1", "n2gev_2", "n2gev_3", "c2gev_0", "c2gev_1", "c2gev_2", "c2gev_3", "cdcecl_0", "cdcecl_1", "cdcecl_2", "cdcecl_3", "cdcklm_0", "cdcklm_1", "cdcklm_2", "cdcklm_3", "cdctop_0", "cdctop_1", "cdctop_2", "cdctop_3", "d3", "d5", "d7", "d9", "p3", "p5", "p7", "p9", "track", "trkflt", "nimin0", "nimin1", "nimin2", "nimin3"
    }
  };


  DBImportObjPtr<TRGGDLDBInputBits> inputbits;
  inputbits.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_INPUT_ARRAY; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        inputbits->setinbitname(j, "");
      }
      //set
      for (int j = 0; j < nbit[i]; j++) {
        //std::cout << j << " " << inputBitNames[i][j] << std::endl;
        inputbits->setinbitname(j, inputBitNames[i][j]);
      }
      inputbits->setninbit(nbit[i]);
      inputbits.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_INPUT_ARRAY - 1; i < N_INPUT_ARRAY; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        inputbits->setinbitname(j, "");
      }
      //set
      for (int j = 0; j < nbit[i]; j++) {
        //std::cout << j << " " << inputBitNames[i][j] << std::endl;
        inputbits->setinbitname(j, inputBitNames[i][j]);
      }
      inputbits->setninbit(nbit[i]);
      inputbits.import(iov);
    }
  }

}

void setunpacker()
{

  const int N_LEAF = 320;
  const int N_UNPACKER_ARRAY = 12;

  const int run[N_UNPACKER_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0,    0, -1,  -1,
    0,    0, 3,  528,
    3,  529, 3,  676,
    3,  677, 3, 1314,
    3, 1315, 3, 1865,
    3, 1866, 3, 4790,
    3, 4791, 3, 5313,
    3, 5314, 4, 6379,
    4, 6380, 4, 7433,
    4, 7434, 5,    0,
    5,    1, 6,   -1,
    7,    0, -1,   -1
  };

  /** num of leafs in data_b2l **/
  const int nLeafs[N_UNPACKER_ARRAY] = {
    37, 37, 27, 26, 26,
    26, 31, 32, 31, 32,
    31, 31
  };
  /** num of leafs for others **/
  const int nLeafsExtra[N_UNPACKER_ARRAY] = {
    8,   8,  9, 11, 11,
    11, 11, 11, 11, 11,
    11, 11
  };
  /** num of clk time window **/
  const int nClks[N_UNPACKER_ARRAY] = {
    48, 48, 48, 48, 48,
    32, 32, 32, 32, 32,
    32, 32
  };
  /** num of bits **/
  const int nBits[N_UNPACKER_ARRAY] = {
    640, 640, 640, 640, 640,
    640, 640, 640, 640, 640,
    640, 640
  };

  /** num of inputleafmap raw **/
  const int nrows[N_UNPACKER_ARRAY] = {
    45, 45, 51, 52, 52,
    52, 57, 61, 61, 61,
    61, 61
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
      //-1
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
    },

    {
      //7
      -1, -1,  1,  2, -1,
      37, -1, -1,  9, -1,
      12, 13, 14,  8, -1,
      -1, -1, 15, 10,  0,
      36, 25, 19, 20, -1,
      -1, -1, 21, 26, 11,
      27, 28, 29, 30, -1,
      -1, -1, 31, 32, 33,
      34, 38, 39, 40, 41,
      -1, -1, -1, 22, 23,
      -1, 35,  3,  4,  5,
      6,   7, 18, 24, 16,
      17
    },

    {
      //8
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
    },

    {
      //9
      -1, -1,  1,  2, -1,
      37, -1, -1,  9, -1,
      12, 13, 14,  8, -1,
      -1, -1, 15, 10,  0,
      36, 25, 19, 20, -1,
      -1, -1, 21, 26, 11,
      27, 28, 29, 30, -1,
      -1, -1, 31, 32, 33,
      34, 38, 39, 40, 41,
      -1, -1, -1, 22, 23,
      -1, 35,  3,  4,  5,
      6,   7, 18, 24, 16,
      17
    },

    {
      //10
      -1, -1,  1,  2, -1,
      37, -1, -1,  9, -1,
      12, 13, 14,  8, -1,
      -1, -1, 15, 10,  0,
      36, 25, 19, 20, -1,
      -1, -1, 21, 26, 11,
      27, 28, 29, 30, -1,
      -1, -1, 31, 32, 33,
      34, 38, 39, 40, 41,
      -1, -1, -1, 22, 23,
      -1, 35,  3,  4,  5,
      6,   7, 18, 24, 16,
      17
    }

  };

  /** bus bit map. (a downto a-b) **/
  const int BitMap[N_UNPACKER_ARRAY][N_LEAF][2] = {

    {
      //-1
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
    },

    {
      //7
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 1,  // tdsrcp
      599, 1,  // tdtopp
      598, 1,  // tdeclp
      597, 1,  // tdcdcp
      583, 14, // rvcout

      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 15, // itd4
      452, 31, // itd3
      415, 31, // psn3
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0

      287, 31, // topslot1
      255, 31, // topslot0
      223, 31, // ftd3
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //8
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
    },

    {
      //9
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 1,  // tdsrcp
      599, 1,  // tdtopp
      598, 1,  // tdeclp
      597, 1,  // tdcdcp
      583, 14, // rvcout

      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 15, // itd4
      452, 31, // itd3
      415, 31, // psn3
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0

      287, 31, // topslot1
      255, 31, // topslot0
      223, 31, // ftd3
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //10
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 1,  // tdsrcp
      599, 1,  // tdtopp
      598, 1,  // tdeclp
      597, 1,  // tdcdcp
      583, 14, // rvcout

      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 15, // itd4
      452, 31, // itd3
      415, 31, // psn3
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0

      287, 31, // topslot1
      255, 31, // topslot0
      223, 31, // ftd3
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    }


  };

  int m_nword_header[N_UNPACKER_ARRAY] {
    3, 3, 4, 6, 6,
    6, 6, 6, 6, 6,
    6, 6
  };
  int m_conf[N_UNPACKER_ARRAY] {
    0, 0, 1, 2, 3,
    4, 5, 6, 7, 6,
    7, 7
  };
  int m_nword_input[N_UNPACKER_ARRAY] {
    3, 3, 3, 3, 3,
    3, 3, 5, 5, 5,
    5, 5
  };
  int m_nword_output[N_UNPACKER_ARRAY] {
    3, 3, 3, 3, 3,
    3, 3, 3, 3, 3,
    3, 4
  };

  const int BitMap_extra[N_UNPACKER_ARRAY][N_LEAF][3] = {
    {
      //-1
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
      //0
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
      //1
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
      //2
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
      //3
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
      //4
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
      //5
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
      //6
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
      //7
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
      //8
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
      //9
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
      //10
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
  if (ONLINE == 0) {
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
  } else if (ONLINE == 1) {
    for (int i = N_UNPACKER_ARRAY - 1; i < N_UNPACKER_ARRAY; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);

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

}

void setdelay()
{

  const int N_DELAY_ARRAY = 32;

  const int run[N_DELAY_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0,    0, -1,  -1,
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
    3, 5975, 4,   0
  };

  const int data_num[N_DELAY_ARRAY] = {
    -1,
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
  if (ONLINE == 0) {
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
  } else if (ONLINE == 1) {
    //for (int i = N_DELAY_ARRAY-1; i < N_DELAY_ARRAY; i++) {
    for (int i = 0; i < 1; i++) {
      IntervalOfValidity iov(5, 0, -1, -1);
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

}

void setbadrun()
{

  const int N_BADRUN_ARRAY = 13;

  const int run[N_BADRUN_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0, 0,   -1,   -1, // 0
    0, 0,    3, 3526, // 1
    3, 3527, 3, 3623, //-1
    3, 3624, 3, 3769, // 1
    3, 3770, 3, 3790, //-1
    3, 3791, 3, 4877, // 1
    3, 4878, 3, 5012, //-1
    3, 5013, 3, 5019, // 1
    3, 5020, 3, 5246, //-1
    3, 5247, 3, 5313, // 1
    3, 5314, 3, 5326, //-1
    3, 5327, 3, 6000, // 1
    3, 6001, 10,   0  // 0
  };

  int flag[N_BADRUN_ARRAY] = {
    0, 1, -1, 1, -1,
    1, -1, 1, -1,  1,
    -1, 1, 0
  };

  DBImportObjPtr<TRGGDLDBBadrun> badrun;
  badrun.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_BADRUN_ARRAY; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      badrun->setflag(flag[i]);
      badrun.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_BADRUN_ARRAY - 1; i < N_BADRUN_ARRAY; i++) {
      IntervalOfValidity iov(5, 0, -1, -1);
      badrun->setflag(flag[i]);
      badrun.import(iov);
    }
  }
}

int main()
{

  //setprescale();
  setftdlbits();
  setinputbits();
  setunpacker();
  //setdelay();
  //setbadrun();

}


