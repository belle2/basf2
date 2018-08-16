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

}

void TRGGDLSummaryModule::event()
{

  StoreObjPtr<EventMetaData> bevt;
  int _exp = bevt->getExperiment();
  int _run = bevt->getRun();
  int exprun = _exp * 1000000 + _run;
  int n_clocks = 0;
  int n_leafs = 0;
  int n_leafsExtra = 0;
  int _e_timtype = 0;
  unsigned ee_itd[10];
  unsigned ee_ftd[10];
  unsigned ee_psn[10];
  unsigned nword_output = 3;
  unsigned nword_input = 3;
  void (*setPointer)(TRGGDLUnpackerStore * store, int** bitArray);
  StoreArray<TRGGDLUnpackerStore> entAry;
  if (!entAry || !entAry.getEntries()) return;
  int nconf = entAry[0]->m_conf;

  if (nconf == 6) {
    nword_input = 5;
    n_clocks = GDLCONF6::nClks;
    n_leafs = GDLCONF6::nLeafs;
    n_leafsExtra = GDLCONF6::nLeafsExtra;
    ee_itd[0] = GDLCONF6::e_psn0; ee_itd[1] = GDLCONF6::e_itd1; ee_itd[2] = GDLCONF6::e_itd2;
    ee_itd[3] = GDLCONF6::e_itd3; ee_itd[4] = GDLCONF6::e_itd4;
    ee_psn[0] = GDLCONF6::e_psn0; ee_psn[1] = GDLCONF6::e_psn1; ee_psn[2] = GDLCONF6::e_psn2;
    ee_ftd[0] = GDLCONF6::e_ftd0; ee_ftd[1] = GDLCONF6::e_ftd1; ee_ftd[2] = GDLCONF6::e_ftd2;
    _e_timtype = GDLCONF6::e_timtype;
    setPointer = GDLCONF6::setLeafPointersArray;
  } else if (nconf == 5) {
    n_clocks = GDLCONF5::nClks;
    n_leafs = GDLCONF5::nLeafs;
    n_leafsExtra = GDLCONF5::nLeafsExtra;
    ee_itd[0] = GDLCONF5::e_psn0; ee_itd[1] = GDLCONF5::e_itd1; ee_itd[2] = GDLCONF5::e_itd2;
    ee_psn[0] = GDLCONF5::e_psn0; ee_psn[1] = GDLCONF5::e_psn1; ee_psn[2] = GDLCONF5::e_psn2;
    ee_ftd[0] = GDLCONF5::e_ftd0; ee_ftd[1] = GDLCONF5::e_ftd1; ee_ftd[2] = GDLCONF5::e_ftd2;
    _e_timtype = GDLCONF5::e_timtype;
    setPointer = GDLCONF5::setLeafPointersArray;
  } else if (nconf == 4) {
    n_clocks = GDLCONF4::nClks;
    n_leafs = GDLCONF4::nLeafs;
    n_leafsExtra = GDLCONF4::nLeafsExtra;
    ee_itd[0] = GDLCONF4::e_psn0; ee_itd[1] = GDLCONF4::e_itd1; ee_itd[2] = GDLCONF4::e_itd2;
    ee_psn[0] = GDLCONF4::e_psn0; ee_psn[1] = GDLCONF4::e_psn1; ee_psn[2] = GDLCONF4::e_psn2;
    ee_ftd[0] = GDLCONF4::e_ftd0; ee_ftd[1] = GDLCONF4::e_ftd1; ee_ftd[2] = GDLCONF4::e_ftd2;
    _e_timtype = GDLCONF4::e_timtype;
    setPointer = GDLCONF4::setLeafPointersArray;
  } else if (nconf == 3) {
    n_clocks = GDLCONF3::nClks;
    n_leafs = GDLCONF3::nLeafs;
    n_leafsExtra = GDLCONF3::nLeafsExtra;
    ee_itd[0] = GDLCONF3::e_psn0; ee_itd[1] = GDLCONF3::e_itd1; ee_itd[2] = GDLCONF3::e_itd2;
    ee_psn[0] = GDLCONF3::e_psn0; ee_psn[1] = GDLCONF3::e_psn1; ee_psn[2] = GDLCONF3::e_psn2;
    ee_ftd[0] = GDLCONF3::e_ftd0; ee_ftd[1] = GDLCONF3::e_ftd1; ee_ftd[2] = GDLCONF3::e_ftd2;
    _e_timtype = GDLCONF3::e_timtype;
    setPointer = GDLCONF3::setLeafPointersArray;
  } else if (nconf == 2) {
    n_clocks = GDLCONF2::nClks;
    n_leafs = GDLCONF2::nLeafs;
    n_leafsExtra = GDLCONF2::nLeafsExtra;
    ee_itd[0] = GDLCONF2::e_psn0; ee_itd[1] = GDLCONF2::e_itd1; ee_itd[2] = GDLCONF2::e_itd2;
    ee_psn[0] = GDLCONF2::e_psn0; ee_psn[1] = GDLCONF2::e_psn1; ee_psn[2] = GDLCONF2::e_psn2;
    ee_ftd[0] = GDLCONF2::e_ftd0; ee_ftd[1] = GDLCONF2::e_ftd1; ee_ftd[2] = GDLCONF2::e_ftd2;
    _e_timtype = GDLCONF2::e_timtype;
    setPointer = GDLCONF2::setLeafPointersArray;
  } else if (nconf == 1) {
    n_clocks = GDLCONF1::nClks;
    n_leafs = GDLCONF1::nLeafs;
    n_leafsExtra = GDLCONF1::nLeafsExtra;
    ee_itd[0] = GDLCONF1::e_psn0; ee_itd[1] = GDLCONF1::e_itd1; ee_itd[2] = GDLCONF1::e_itd2;
    ee_psn[0] = GDLCONF1::e_psn0; ee_psn[1] = GDLCONF1::e_psn1; ee_psn[2] = GDLCONF1::e_psn2;
    ee_ftd[0] = GDLCONF1::e_ftd0; ee_ftd[1] = GDLCONF1::e_ftd1; ee_ftd[2] = GDLCONF1::e_ftd2;
    _e_timtype = GDLCONF1::e_timtype;
    setPointer = GDLCONF1::setLeafPointersArray;
  } else {
    n_clocks = GDLCONF0::nClks;
    n_leafs = GDLCONF0::nLeafs;
    n_leafsExtra = GDLCONF0::nLeafsExtra;
    ee_itd[0] = GDLCONF0::e_psn0; ee_itd[1] = GDLCONF0::e_itd1; ee_itd[2] = GDLCONF0::e_itd2;
    ee_psn[0] = GDLCONF0::e_psn0; ee_psn[1] = GDLCONF0::e_psn1; ee_psn[2] = GDLCONF0::e_psn2;
    ee_ftd[0] = GDLCONF0::e_ftd0; ee_ftd[1] = GDLCONF0::e_ftd1; ee_ftd[2] = GDLCONF0::e_ftd2;
    _e_timtype = GDLCONF0::e_timtype;
    setPointer = GDLCONF0::setLeafPointersArray;
  }

  std::vector<std::vector<int> > _data(n_leafs + n_leafsExtra);
  for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
    std::vector<int> _v(n_clocks);
    _data[leaf] = _v;
  }

  // fill "bit vs clk" for the event
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    int* Bits[n_leafs + n_leafsExtra];
    setPointer(entAry[ii], Bits);
    for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
      _data[leaf][entAry[ii]->m_clk] =  *Bits[leaf];
    }
  }

  GDLResult.create();

  unsigned ored = 0;

  for (int j = 0; j < nword_input; j++) {
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
    for (int j = 0; j < nword_output; j++) {
      ored = 0;
      for (int clk = 0; clk < n_clocks; clk++) {
        ored |= _data[ee_ftd[j]][clk];
      }
      GDLResult->setFtdlBits(j, ored);
    }

    for (int j = 0; j < nword_output; j++) {
      ored = 0;
      for (int clk = 0; clk < n_clocks; clk++) {
        ored |= _data[ee_psn[j]][clk];
      }
      GDLResult->setPsnmBits(j, ored);
    }
  }

  GDL::EGDLTimingType gtt = (GDL::EGDLTimingType)_data[_e_timtype][n_clocks - 1];

  TRGSummary::ETimingType tt = TRGSummary::TTYP_NONE;
  if (gtt == GDL::e_tt_cdc) {
    tt = TRGSummary::TTYP_CDC;
  } else if (gtt == GDL::e_tt_ecl) {
    tt = TRGSummary::TTYP_PID0;
  } else if (gtt == GDL::e_tt_dphy) {
    tt = TRGSummary::TTYP_DPHY;
  } else if (gtt == GDL::e_tt_rand) {
    tt = TRGSummary::TTYP_RAND;
  } else {
    tt = TRGSummary::TTYP_NONE;
  }

  GDLResult->setTimType(tt);

}
