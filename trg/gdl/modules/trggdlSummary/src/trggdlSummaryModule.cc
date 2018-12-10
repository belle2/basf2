//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : trggdlSummaryModule.cc
// Section  : TRG GDL Summary
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : A trigger module for TRG GDL
//---------------------------------------------------------------
// 1.00 : 2017/05/08 : First version
//---------------------------------------------------------------
#include <trg/gdl/modules/trggdlSummary/trggdlSummaryModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>

#include <mdst/dataobjects/TRGSummary.h>

#include <string.h>
#include <unistd.h>

#ifndef __clang__
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif

using namespace Belle2;
using namespace GDL;

REG_MODULE(TRGGDLSummary);

TRGGDLSummaryModule::TRGGDLSummaryModule()
  : Module::Module()
{

  setDescription("Fill experiment data to TRGSummary");
  setPropertyFlags(c_ParallelProcessingCertified);

}

void TRGGDLSummaryModule::initialize()
{

  GDLResult.registerInDataStore();
  for (int i = 0; i < 320; i++) {
    LeafBitMap[i] = m_unpacker->getLeafMap(i);
  }
  for (int i = 0; i < 320; i++) {
    strcpy(LeafNames[i], m_unpacker->getLeafnames(i));
  }

  _e_timtype = 0;
  for (int i = 0; i < 10; i++) {
    ee_psn[i] = 0;
    ee_ftd[i] = 0;
    ee_itd[i] = 0;
  }
  for (int i = 0; i < 320; i++) {
    if (strcmp(LeafNames[i], "timtype") == 0)_e_timtype = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn0") == 0)   ee_psn[0] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn1") == 0)   ee_psn[1] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn2") == 0)   ee_psn[2] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn3") == 0)   ee_psn[3] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn4") == 0)   ee_psn[4] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn5") == 0)   ee_psn[5] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn6") == 0)   ee_psn[6] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn7") == 0)   ee_psn[7] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn8") == 0)   ee_psn[8] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn9") == 0)   ee_psn[9] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd0") == 0)   ee_ftd[0] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd1") == 0)   ee_ftd[1] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd2") == 0)   ee_ftd[2] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd3") == 0)   ee_ftd[3] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd4") == 0)   ee_ftd[4] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd5") == 0)   ee_ftd[5] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd6") == 0)   ee_ftd[6] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd7") == 0)   ee_ftd[7] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd8") == 0)   ee_ftd[8] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd9") == 0)   ee_ftd[9] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd0") == 0)   ee_itd[0] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd1") == 0)   ee_itd[1] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd2") == 0)   ee_itd[2] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd3") == 0)   ee_itd[3] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd4") == 0)   ee_itd[4] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd5") == 0)   ee_itd[5] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd6") == 0)   ee_itd[6] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd7") == 0)   ee_itd[7] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd8") == 0)   ee_itd[8] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd9") == 0)   ee_itd[9] = LeafBitMap[i];
  }


}

void TRGGDLSummaryModule::event()
{


  int n_leafs = 0;
  n_leafs  = m_unpacker->getnLeafs();
  int n_leafsExtra = 0;
  n_leafsExtra = m_unpacker->getnLeafsExtra();
  int n_clocks = m_unpacker->getnClks();
  int nconf = m_unpacker->getconf();
  int nword_input  = m_unpacker->get_nword_input();
  int nword_output = m_unpacker->get_nword_output();




  StoreArray<TRGGDLUnpackerStore> entAry;
  if (!entAry || !entAry.getEntries()) return;

  //prepare entAry adress
  int clk_map = 0;
  for (int i = 0; i < 320; i++) {
    if (strcmp(entAry[0]->m_unpackername[i], "clk") == 0) clk_map = i;
  }

  std::vector<std::vector<int> > _data(n_leafs + n_leafsExtra);
  for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
    std::vector<int> _v(n_clocks);
    _data[leaf] = _v;
  }

  // fill "bit vs clk" for the event
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    int* Bits[n_leafs + n_leafsExtra];
    //set pointer
    for (int i = 0; i < 320; i++) {
      if (LeafBitMap[i] != -1) {
        Bits[LeafBitMap[i]] = &(entAry[ii]->m_unpacker[i]);
      }
    }
    for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
      _data[leaf][entAry[ii]->m_unpacker[clk_map]] =  *Bits[leaf];
    }
  }

  GDLResult.create(true);

  unsigned ored = 0;

  for (int j = 0; j < (int)nword_input; j++) {
    ored = 0;
    for (int clk = 0; clk < n_clocks; clk++) {
      ored |= _data[ee_itd[j]][clk];
    }
    GDLResult->setInputBits(j, ored);
  }

  if (nconf == 0) {
    ored = 0;
    for (int clk = 0; clk < n_clocks; clk++) {
      ored |= _data[ee_ftd[0]][clk];
    }
    GDLResult->setFtdlBits(0, ored);

    ored = 0;
    for (int clk = 0; clk < n_clocks; clk++) {
      ored |= (_data[ee_ftd[2]][clk] << 16) + _data[ee_ftd[1]][clk];
    }
    GDLResult->setFtdlBits(1, ored);

    ored = 0;
    for (int clk = 0; clk < n_clocks; clk++) {
      ored |= _data[ee_psn[0]][clk];
    }
    GDLResult->setPsnmBits(0, ored);

    ored = 0;
    for (int clk = 0; clk < n_clocks; clk++) {
      ored |= (_data[ee_psn[2]][clk] << 16) + _data[ee_psn[1]][clk];
    }
    GDLResult->setPsnmBits(1, ored);
  } else {
    for (int j = 0; j < (int)nword_output; j++) {
      ored = 0;
      for (int clk = 0; clk < n_clocks; clk++) {
        ored |= _data[ee_ftd[j]][clk];
      }
      GDLResult->setFtdlBits(j, ored);
    }

    for (int j = 0; j < (int)nword_output; j++) {
      ored = 0;
      for (int clk = 0; clk < n_clocks; clk++) {
        ored |= _data[ee_psn[j]][clk];
      }
      GDLResult->setPsnmBits(j, ored);
    }
  }


  GDL::EGDLTimingType gtt = (GDL::EGDLTimingType)_data[_e_timtype][n_clocks - 1];

  /*
  TRGSummary::ETimingType tt = TRGSummary::TTYP_NONE;
  if (gtt == GDL::e_tt_cdc) {
    tt = TRGSummary::TTYP_CDC;
  } else if (gtt == GDL::e_tt_top) {
    tt = TRGSummary::TTYP_PID0;
  } else if (gtt == GDL::e_tt_ecl) {
    tt = TRGSummary::TTYP_ECL;
  } else if (gtt == GDL::e_tt_dphy) {
    tt = TRGSummary::TTYP_DPHY;
  } else if (gtt == GDL::e_tt_rand) {
    tt = TRGSummary::TTYP_RAND;
  } else {
    tt = TRGSummary::TTYP_NONE;
  }
  */

  GDLResult->setTimType((TRGSummary::ETimingType)gtt);

}
