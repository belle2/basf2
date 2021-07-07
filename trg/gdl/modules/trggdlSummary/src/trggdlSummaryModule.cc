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

#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/TRGSummary.h>

#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>

#include <string.h>


using namespace Belle2;
using namespace GDL;

REG_MODULE(TRGGDLSummary);

TRGGDLSummaryModule::TRGGDLSummaryModule()
  : Module::Module()
{

  setDescription("Fill experiment data to TRGSummary");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("debugLevel", _debugLevel, "Debug Level", 0);

}

void TRGGDLSummaryModule::initialize()
{

  if (_debugLevel > 9) printf("TRGGDLSummaryModule::initialize() start\n");

  GDLResult.registerInDataStore();
  for (int i = 0; i < 320; i++) {
    LeafBitMap[i] = m_unpacker->getLeafMap(i);
  }
  for (int i = 0; i < 320; i++) {
    strcpy(LeafNames[i], m_unpacker->getLeafnames(i));
  }

  _e_timtype = 0;
  _e_rvcout = 0;
  for (int i = 0; i < 10; i++) {
    ee_psn[i] = 0;
    ee_ftd[i] = 0;
    ee_itd[i] = 0;
  }
  for (int i = 0; i < 320; i++) {
    if (strcmp(LeafNames[i], "timtype") == 0)_e_timtype = LeafBitMap[i];
    if (strcmp(LeafNames[i], "rvcout") == 0) _e_rvcout = LeafBitMap[i];
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


  if (_debugLevel > 9) printf("TRGGDLSummaryModule::initialize() end\n");
}

void TRGGDLSummaryModule::event()
{

  if (_debugLevel > 9) printf("TRGGDLSummaryModule::event() start\n");

  int n_leafs = 0;
  n_leafs  = m_unpacker->getnLeafs();
  int n_leafsExtra = 0;
  n_leafsExtra = m_unpacker->getnLeafsExtra();
  int n_clocks = m_unpacker->getnClks();
  int nconf = m_unpacker->getconf();
  int nword_input  = m_unpacker->get_nword_input();
  int nword_output = m_unpacker->get_nword_output();

  if (_debugLevel > 89)
    printf("trggdlSummaryModule:n_leafs(%d), n_leafsExtra(%d), n_clocks(%d), nconf(%d), nword_input(%d), nword_output(%d)\n",
           n_leafs, n_leafsExtra, n_clocks, nconf, nword_input, nword_output);

  StoreArray<TRGGDLUnpackerStore> entAry;
  if (!entAry || !entAry.getEntries()) return;

  //prepare entAry adress
  int clk_map = 0;
  for (int i = 0; i < 320; i++) {
    if (strcmp(entAry[0]->m_unpackername[i], "clk") == 0) clk_map = i;
  }
  if (_debugLevel > 89)
    printf("trggdlSummaryModule:clk_map(%d)\n", clk_map);

  std::vector<std::vector<int> > _data(n_leafs + n_leafsExtra);
  for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
    std::vector<int> _v(n_clocks);
    _data[leaf] = _v;
  }

  // fill "bit vs clk" for the event
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    if (_debugLevel > 89)
      printf("trggdlSummaryModule:a:ii(%d)\n", ii);
    std::vector<int*> Bits(n_leafs + n_leafsExtra);
    //set pointer
    for (int i = 0; i < 320; i++) {
      if (LeafBitMap[i] != -1) {
        Bits[LeafBitMap[i]] = &(entAry[ii]->m_unpacker[i]);
        if (_debugLevel > 89)
          printf("trggdlSummaryModule:ab:i(%d), LeafBitMap[i](%d), *Bits[LeafBitMap[i]](%d)\n",
                 i, LeafBitMap[i], *Bits[LeafBitMap[i]]);
      } else {
        if (_debugLevel > 89)
          printf("trggdlSummaryModule:ab:i(%d), LeafBitMap[i](%d)\n",
                 i, LeafBitMap[i]);
      }
    }
    for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
      if (_debugLevel > 89)
        printf("trggdlSummaryModule:ad:leaf(%d),ii(%d),clk_map(%d),*Bits[leaf](%d), entAry[ii]->m_unpacker[clk_map](%d)\n",
               leaf, ii, clk_map, *Bits[leaf], entAry[ii]->m_unpacker[clk_map]);
      _data[leaf][entAry[ii]->m_unpacker[clk_map]] =  *Bits[leaf];
    }
  }

  GDLResult.create(true);

  unsigned ored = 0;

  for (int j = 0; j < (int)nword_input; j++) {
    if (_debugLevel > 89) printf("b:j(%d),", j);
    ored = 0;
    for (int clk = 0; clk < n_clocks; clk++) {
      if (_debugLevel > 89) printf("clk(%d),", clk);
      ored |= _data[ee_itd[j]][clk];
    }
    GDLResult->setInputBits(j, ored);
    if (_debugLevel > 89) printf("\n");
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

  // reg_tmdl_timtype in header. 3bit, no quality info.
  GDL::EGDLTimingType gtt = (GDL::EGDLTimingType)_data[_e_timtype][0];

  //get prescales
  for (int i = 0; i < 320; i++) {
    int bit1 = i / 32;
    int bit2 = i % 32;
    GDLResult->setPreScale(bit1, bit2, m_prescales->getprescales(i));
  }

  TRGSummary::ETimingType tt;
  if (gtt == GDL::e_tt_cdc) {
    tt = TRGSummary::TTYP_CDC;
  } else if (gtt == GDL::e_tt_ecl) {
    tt = TRGSummary::TTYP_ECL;
  } else if (gtt == GDL::e_tt_top) {
    tt = TRGSummary::TTYP_TOP;
  } else if (gtt == GDL::e_tt_dphy) {
    tt = TRGSummary::TTYP_DPHY;
  } else if (gtt == GDL::e_tt_rand) {
    tt = TRGSummary::TTYP_RAND;
  } else if (gtt == GDL::e_tt_psnm) {
    tt = TRGSummary::TTYP_SELF;
  } else {
    tt = TRGSummary::TTYP_NONE;
  }

  StoreObjPtr<EventMetaData> bevt;
  unsigned _exp = bevt->getExperiment();
  unsigned _run = bevt->getRun();
  unsigned exprun = _exp * 1000000 + _run;
  if (exprun < 13000500) {
    GDLResult->setTimQuality(TRGSummary::TTYQ_CORS); // coarse
  } else {
    int rvcout = _data[_e_rvcout][0];
    int q = (rvcout >> 1) & 3;
    TRGSummary::ETimingQuality timQuality = TRGSummary::TTYQ_NONE;
    switch (q) {
      case 1:
        timQuality = TRGSummary::TTYQ_CORS;
        break;
      case 2:
        timQuality = TRGSummary::TTYQ_FINE;
        break;
      case 3:
        timQuality = TRGSummary::TTYQ_SFIN;
        break;
      default:
        timQuality = TRGSummary::TTYQ_NONE;
        break;
    }
    GDLResult->setTimQuality(timQuality);
  }

  if (exprun > 16000271) {
    if (tt == TRGSummary::TTYP_RAND) {
      int i_poissonin = GDLResult->getInputBitNumber(std::string("poissonin"));
      int j_poissonin = i_poissonin / TRGSummary::c_trgWordSize;
      int k_poissonin = i_poissonin % TRGSummary::c_trgWordSize;
      int i_veto      = GDLResult->getInputBitNumber(std::string("veto"));
      int j_veto      = i_veto / TRGSummary::c_trgWordSize;
      int k_veto      = i_veto % TRGSummary::c_trgWordSize;
      for (int clk = 5; clk < n_clocks - 5; clk++) {
        if ((1 << k_poissonin) & _data[ee_itd[j_poissonin]][clk]) {
          tt = TRGSummary::TTYP_POIS;
          if ((1 << k_veto) & _data[ee_itd[j_veto]][clk]) {
            GDLResult->setPoissonInInjectionVeto();
          }
        }
      }
    }
  }
  GDLResult->setTimType(tt);

}
