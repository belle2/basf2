/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/LogSystem.h>
#include <trg/cdc/dbobjects/CDCTriggerDeadch.h>
#include <iostream>
#include <fstream>
//#include <TFile.h>
//#include <TH1F.h>

using namespace Belle2;

#define ONLINE 0
//#define ONLINE 1

bool get_wire_mgr(int mask_sl[], int mask_layer[], int mask_ch[], int mgr)
{
  int mgr_sl = mgr / 1000;
  int mgr_i  = (mgr - mgr_sl * 1000) / 10;
  int mgr_u  = mgr % 10;
  //std::cout << mgr_sl << " " << mgr_i << " " << mgr_u << std::endl;
  //MGR0 0-4 :L3-7   lch0-159
  //MGR1 0-4 :L8-12  lch0-159
  //MGR2 0-5 :L14-18 lch0-191
  //MGR3 0-6 :L20-24 lch0-223
  //MGR4 0-7 :L26-30 lch0-255
  //MGR5 0-8 :L32-36 lch0-287
  //MGR6 0-9 :L38-42 lch0-319
  //MGR7 0-10:L44-48 lch0-351
  //MGR8 0-11:L50-54 lch0-383
  //160*8 + 160*6 + 192*6 + 224*6 + 256*6 + 288*6 + 320*6 + 352*6 + 384*6 = 14336 = nSenseWires

  //first inner layer
  for (int i = 0; i < 32; i++) {
    mask_sl[i]                = mgr_sl;
    mask_ch[i]                = mgr_i * 32 + i;
    mask_layer[i] = 0;
    if (mgr_u == 1)  mask_layer[i] += 3;
    if (mgr_sl == 0) mask_layer[i] += 2;
  }
  //second inner layer
  for (int i = 0; i < 32; i++) {
    mask_sl[i + 32]                = mgr_sl;
    mask_ch[i + 32]                = mgr_i * 32 + i;
    mask_layer[i + 32] = 1;
    if (mgr_u == 1)  mask_layer[i + 32] += 3;
    if (mgr_sl == 0) mask_layer[i + 32] += 2;
  }
  //third inner layer
  for (int i = 0; i < 32; i++) {
    mask_sl[i + 64]                = mgr_sl;
    mask_ch[i + 64]                = mgr_i * 32 + i;
    mask_layer[i + 64] = 2;
    if (mgr_u == 1)  mask_layer[i + 64] += 3;
    if (mgr_sl == 0) mask_layer[i + 64] += 2;
  }

  return true;
}


void setdeadch()
{
  const static int MAX_N_LAYERS = 8;

  const int N_config = 75;

  const int run[N_config][4] = { //itnitial exp, initial run, end exp, end run
    0,    0,    7, 3585, // 0
    7, 3586,    8, 1052, // 1
    8, 1053,    8, 1108, // 2
    8, 1109,    8, 1115, // 3
    8, 1116,    8, 1832, // 4
    8, 1833,    8, 1916, // 5
    8, 1917,    8, 2528, // 6
    8, 2529,    8, 2550, // 7
    8, 2551,    8, 2578, // 8
    8, 2579,    8, 2580, // 9
    8, 2581,    9,   -1, // 10
    10,    0,   10, 5210, // 11
    10, 5211,   11,   -1, // 12
    12,    0,   12, 1197, // 13
    12, 1198,   12, 1294, // 14
    12, 1295,   12, 2830, // 15
    12, 2831,   12, 2846, // 16
    12, 2847,   12, 2883, // 17
    12, 2884,   12, 3212, // 18
    12, 3213,   12, 3489, // 19
    12, 3490,   12, 4214, // 20
    12, 4215,   12, 4406, // 21
    12, 4407,   12, 4419, // 22
    12, 4420,   14, 1382, // 23
    14, 1383,   16,  685, // 24
    16,  686,   18,   90, // 25
    18,   91,   20,  207, // 26
    20,  208,   20,  208, // 27
    20,  209,   20,  272, // 28
    20,  273,   20,  273, // 29
    20,  274,   22,  102, // 30
    22,  103,   22,  121, // 31
    22,  122,   22,  135, // 32
    22,  136,   22,  334, // 33
    22,  335,   22,  523, // 34
    22,  524,   22,  568, // 35
    22,  569,   22,  596, // 36
    22,  597,   22,  604, // 37
    22,  605,   23,   -1, // 38
    24,    0,   24,  915, // 39
    24,  916,   24,  923, // 40
    24,  924,   24, 1184, // 41
    24, 1185,   24, 1190, // 42
    24, 1191,   24, 1404, // 43
    24, 1405,   24, 1406, // 44
    24, 1407,   24, 1599, // 45
    24, 1600,   24, 1613, // 46
    24, 1614,   24, 1618, // 47
    24, 1619,   24, 1844, // 48
    24, 1845,   24, 1852, // 49
    24, 1853,   24, 2058, // 50
    24, 2059,   24, 2061, // 51
    24, 2062,   24, 2062, // 52
    24, 2063,   24, 2315, // 53
    24, 2316,   25,   99, // 54
    25,  100,   25,  100, // 55
    25,  101,   25,  314, // 56
    25,  315,   25,  352, // 57
    25,  353,   26,   96, // 58
    26,   97,   26,  137, // 59
    26,  138,   26,  147, // 60
    26,  148,   26,  364, // 61
    26,  365,   26,  368, // 62
    26,  369,   26,  523, // 63
    26,  524,   26,  531, // 64
    26,  532,   26,  545, // 65
    26,  546,   26,  564, // 66
    26,  565,   26,  615, // 67
    26,  616,   26,  644, // 68
    26,  645,   26,  646, // 69
    26,  647,   26,  904, // 70
    26,  905,   26, 1389, // 71
    26, 1390,   26, 1430, // 72
    26, 1431,   26, 1431, // 73
    26, 1432,   -1,   -1  // 74
  };


  std::vector<std::vector<int>> mgr(N_config);
  //0  nomask
  mgr[ 1].push_back(7010); //MGR7-1U1
  mgr[ 2].push_back(7010); mgr[ 2].push_back(7071);
  mgr[ 3].push_back(7010); mgr[ 3].push_back(7071); mgr[ 3].push_back(8110);
  mgr[ 4].push_back(7010); mgr[ 4].push_back(7071);
  mgr[ 5].push_back(7010); mgr[ 5].push_back(7071); mgr[ 5].push_back(1000);
  mgr[ 6].push_back(7010); mgr[ 6].push_back(7071);
  mgr[ 7].push_back(7010); mgr[ 7].push_back(7071); mgr[ 7].push_back(2030);
  mgr[ 8].push_back(7010); mgr[ 8].push_back(7071);
  mgr[ 9].push_back(7010); mgr[ 9].push_back(7071); mgr[ 9].push_back(6061);
  mgr[10].push_back(7010); mgr[10].push_back(7071);
  //11 nomask
  mgr[12].push_back(7010);
  mgr[13].push_back(7010); mgr[13].push_back(7090); mgr[13].push_back(3010);
  mgr[14].push_back(7010); mgr[14].push_back(7090); mgr[14].push_back(3010); mgr[14].push_back(4040);
  mgr[15].push_back(7010); mgr[15].push_back(7090); mgr[15].push_back(3010);
  mgr[16].push_back(7010); mgr[16].push_back(7090);
  mgr[17].push_back(7010); mgr[17].push_back(7090); mgr[17].push_back(3010);
  mgr[18].push_back(7010); mgr[18].push_back(7090);
  //19 nomask
  mgr[20].push_back(3010);
  //21 nomask
  mgr[22].push_back(3031);
  //23 nomask
  mgr[24].push_back(3010);
  //25 nomask
  mgr[26].push_back(3010);
  mgr[27].push_back(3010); mgr[27].push_back(2040); mgr[27].push_back(11); mgr[27].push_back(4060); mgr[27].push_back(4061);
  mgr[27].push_back(8060); mgr[27].push_back(8061); mgr[27].push_back(8070);
  mgr[28].push_back(3010);
  mgr[29].push_back(3010); mgr[29].push_back(3031);
  mgr[30].push_back(3010);
  mgr[31].push_back(3010); mgr[31].push_back(8060); mgr[31].push_back(8061);
  mgr[32].push_back(3010);
  mgr[33].push_back(3010); mgr[33].push_back(8060); mgr[33].push_back(8061);
  mgr[34].push_back(3010);
  mgr[35].push_back(3010); mgr[35].push_back(4001);
  mgr[36].push_back(3010);
  mgr[37].push_back(3010); mgr[37].push_back(4001); mgr[37].push_back(5080);
  mgr[38].push_back(3010);
  mgr[39].push_back(3010); mgr[39].push_back(5050);
  mgr[40].push_back(3010); mgr[40].push_back(5050); mgr[40].push_back(8021);
  mgr[41].push_back(3010); mgr[41].push_back(5050);
  mgr[42].push_back(3010); mgr[42].push_back(5050); mgr[42].push_back(21);
  mgr[43].push_back(3010); mgr[43].push_back(5050);
  mgr[44].push_back(3010); mgr[44].push_back(5050); mgr[44].push_back(30);
  mgr[45].push_back(3010); mgr[45].push_back(5050);
  mgr[46].push_back(3010); mgr[46].push_back(5050); mgr[46].push_back(1030);
  mgr[47].push_back(3010); mgr[47].push_back(5050); mgr[47].push_back(1030); mgr[47].push_back(1011);
  mgr[48].push_back(3010); mgr[48].push_back(5050); mgr[48].push_back(1030);
  mgr[49].push_back(3010); mgr[49].push_back(5050); mgr[49].push_back(1030); mgr[49].push_back(2021);
  mgr[50].push_back(3010); mgr[50].push_back(5050); mgr[50].push_back(1030);
  mgr[51].push_back(3010); mgr[51].push_back(5050); mgr[51].push_back(1030); mgr[51].push_back(8020); mgr[51].push_back(8060);
  mgr[51].push_back(8061);
  mgr[52].push_back(3010); mgr[52].push_back(5050); mgr[52].push_back(1030); mgr[52].push_back(8060); mgr[52].push_back(8061);
  mgr[53].push_back(3010); mgr[53].push_back(5050); mgr[53].push_back(1030); mgr[53].push_back(8020); mgr[53].push_back(8060);
  mgr[53].push_back(8061);
  mgr[54].push_back(3010); mgr[54].push_back(5450); mgr[54].push_back(1030);
  mgr[55].push_back(3010); mgr[55].push_back(5050); mgr[55].push_back(1030); mgr[55].push_back(8020); mgr[55].push_back(8060);
  mgr[55].push_back(8061);
  mgr[56].push_back(3010); mgr[56].push_back(5050); mgr[56].push_back(1030);
  mgr[57].push_back(3010); mgr[57].push_back(5050); mgr[57].push_back(1030); mgr[57].push_back(1000);
  mgr[58].push_back(3010); mgr[58].push_back(5050); mgr[58].push_back(1030);
  mgr[59].push_back(3010); mgr[59].push_back(5050); mgr[59].push_back(1030); mgr[59].push_back(6051);
  mgr[60].push_back(3010); mgr[60].push_back(5050); mgr[60].push_back(1030); mgr[60].push_back(40);   mgr[60].push_back(6051);
  mgr[60].push_back(6060);
  mgr[61].push_back(3010); mgr[61].push_back(5050); mgr[61].push_back(1030);
  mgr[62].push_back(3010); mgr[62].push_back(5050); mgr[62].push_back(1030); mgr[62].push_back(6080); mgr[62].push_back(6081);
  mgr[63].push_back(3010); mgr[63].push_back(5050); mgr[63].push_back(1030);
  mgr[64].push_back(3010); mgr[64].push_back(5050); mgr[64].push_back(1030); mgr[64].push_back(8041);
  mgr[65].push_back(3010); mgr[65].push_back(5050); mgr[65].push_back(1030);
  mgr[66].push_back(3010); mgr[66].push_back(5050); mgr[66].push_back(1030); mgr[66].push_back(1021); mgr[66].push_back(8041);
  mgr[67].push_back(3010); mgr[67].push_back(5050); mgr[67].push_back(1030);
  mgr[68].push_back(3010); mgr[68].push_back(5050); mgr[68].push_back(1030); mgr[68].push_back(30);
  mgr[69].push_back(3010); mgr[69].push_back(30);   mgr[69].push_back(7000); mgr[69].push_back(7001); mgr[69].push_back(7010);
  mgr[69].push_back(7011);
  mgr[70].push_back(3010); mgr[70].push_back(30);
  mgr[71].push_back(30);
  //72 nomask
  mgr[73].push_back(1010); mgr[73].push_back(2000); mgr[73].push_back(2001);
  //74 nomask


  bool badch_map[N_config][9][8][384]; //sl layer ch
  for (int i = 0; i < N_config; i++) {
    for (unsigned int j = 0; j < c_nSuperLayers; j++) {
      for (unsigned int k = 0; k < MAX_N_LAYERS; k++) {
        for (unsigned int l = 0; l < c_maxNDriftCells; l++) {
          badch_map[i][j][k][l] = true;
        }
      }
    }
  }

  //mask L54 for all runs
  for (int i = 0; i < N_config; i++) {
    for (unsigned int j = 8; j < 9; j++) {
      for (unsigned int k = 4; k < 5; k++) {
        for (unsigned int l = 0; l < c_maxNDriftCells; l++) {
          badch_map[i][j][k][l] = false;
        }
      }
    }
  }

  //mask merger
  for (int i = 0; i < N_config; i++) {
    for (unsigned int j = 0; j < mgr[i].size(); j++) {
      int mgr_sl[96];
      int mgr_layer[96];
      int mgr_ch[96];
      get_wire_mgr(mgr_sl, mgr_layer, mgr_ch, mgr[i][j]);
      //std::cout << mgr[i][j] << std::endl;
      for (int k = 0; k < 96; k++) {
        //std::cout << mgr_sl[k] << " " << mgr_layer[k] << " " << mgr_ch[k] << std::endl;
        badch_map[i][mgr_sl[k]][mgr_layer[k]][mgr_ch[k]] = false;
      }
    }
  }

  DBImportObjPtr<CDCTriggerDeadch> db_dead;
  db_dead.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_config; i++) {
      //std::cout << i << " " << run[i][0] << " " << run[i][1] << " " << run[i][2] << " " << run[i][3] << std::endl;
      //for(int j=0;j<mgr[i].size();j++)std::cout << mgr[i][j] << std::endl;
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      for (unsigned int j = 0; j < c_nSuperLayers; j++) {
        for (unsigned int k = 0; k < MAX_N_LAYERS; k++) {
          for (unsigned int l = 0; l < c_maxNDriftCells; l++) {
            if (!badch_map[i][j][k][l]) {
              //std::cout << j << " " << k << " " << l << std::endl;
              db_dead->setdeadch(j, k, l, false);
            } else {
              db_dead->setdeadch(j, k, l, true);
            }
          }
        }
      }
      db_dead.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = 0; i < 1; i++) { //no merger dead channel for run-independent MC. L54 is masked.
      IntervalOfValidity iov(0, 0, -1, -1);
      for (unsigned int j = 0; j < c_nSuperLayers; j++) {
        for (unsigned int k = 0; k < MAX_N_LAYERS; k++) {
          for (unsigned int l = 0; l < c_maxNDriftCells; l++) {
            if (!badch_map[i][j][k][l]) {
              //std::cout << j << " " << k << " " << l << std::endl;
              db_dead->setdeadch(j, k, l, false);
            } else {
              db_dead->setdeadch(j, k, l, true);
            }
          }
        }
      }
      db_dead.import(iov);
    }
  }
}

int main()
{

  setdeadch();

}


