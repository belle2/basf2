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

  const int N_config = 27;

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
    18,   91,   -1,   -1  // 26
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

  bool badch_map[N_config][9][8][384]; //sl layer ch
  for (int i = 0; i < N_config; i++) {
    for (unsigned int j = 0; j < nSuperLayers; j++) {
      for (unsigned int k = 0; k < MAX_N_LAYERS; k++) {
        for (unsigned int l = 0; l < MAX_N_SCELLS; l++) {
          badch_map[i][j][k][l] = true;
        }
      }
    }
  }

  //mask L54 for all runs
  for (int i = 0; i < N_config; i++) {
    for (unsigned int j = 8; j < 9; j++) {
      for (unsigned int k = 4; k < 5; k++) {
        for (unsigned int l = 0; l < MAX_N_SCELLS; l++) {
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
      for (unsigned int j = 0; j < nSuperLayers; j++) {
        for (unsigned int k = 0; k < MAX_N_LAYERS; k++) {
          for (unsigned int l = 0; l < MAX_N_SCELLS; l++) {
            if (!badch_map[i][j][k][l]) {
              //std::cout << j << " " << k << " " << l << std::endl;
              db_dead->setdeadch(j, k, l, false);
            }
          }
        }
      }
      db_dead.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = 0; i < 1; i++) { //no merger dead channel for run-independent MC. L54 is masked.
      IntervalOfValidity iov(0, 0, -1, -1);
      for (unsigned int j = 0; j < nSuperLayers; j++) {
        for (unsigned int k = 0; k < MAX_N_LAYERS; k++) {
          for (unsigned int l = 0; l < MAX_N_SCELLS; l++) {
            if (!badch_map[i][j][k][l]) {
              //std::cout << j << " " << k << " " << l << std::endl;
              db_dead->setdeadch(j, k, l, false);
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


