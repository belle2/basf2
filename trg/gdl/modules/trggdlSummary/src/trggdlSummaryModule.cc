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
  unsigned itds[3];
  unsigned ftds[3];
  unsigned psns[3];
  unsigned nword_output = 0;
  unsigned nword_input = 3;
  void (*setPointer)(TRGGDLUnpackerStore * store, int** bitArray);
  if (exprun >= 3000677) {
    if (exprun >= 3001158) {
      n_clocks = nClks2;
    } else {
      n_clocks = nClks1;
    }
    n_leafs = nLeafs1;
    n_leafsExtra = nLeafsExtra1;
    itds[0] = GDLCONF1::e_itd0; itds[1] = GDLCONF1::e_itd1; itds[2] = GDLCONF1::e_itd2;
    ftds[0] = GDLCONF1::e_ftd0; ftds[1] = GDLCONF1::e_ftd1; ftds[2] = GDLCONF1::e_ftd2;
    psns[0] = GDLCONF1::e_psn0; psns[1] = GDLCONF1::e_psn1; psns[2] = GDLCONF1::e_psn2;
    _e_timtype = GDLCONF1::e_timtype;
    setPointer = GDL::setLeafPointersArray1;
    nword_output = 3;
  } else if (exprun >= 3000529) {
    n_clocks = nClks0;
    n_leafs = nLeafs0;
    n_leafsExtra = nLeafsExtra0;
    itds[0] = GDLCONF0::e_itd0; itds[1] = GDLCONF0::e_itd1; itds[2] = GDLCONF0::e_itd2;
    ftds[0] = GDLCONF0::e_ftd0; ftds[1] = GDLCONF0::e_ftd1;
    psns[0] = GDLCONF0::e_psn0; psns[1] = GDLCONF0::e_psn1;
    _e_timtype = GDLCONF0::e_timtype;
    setPointer = GDL::setLeafPointersArray0;
    nword_output = 2;
  } else {
    setPointer = GDL::setLeafPointersArray0;
  }

  StoreArray<TRGGDLUnpackerStore> entAry;
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
      ored |= _data[itds[j]][clk];
    }
    GDLResult->setInputBits(j, ored);
  }

  for (int j = 0; j < nword_output; j++) {
    ored = 0;
    for (int clk = 0; clk < n_clocks; clk++) {
      ored |= _data[ftds[j]][clk];
    }
    GDLResult->setFtdlBits(j, ored);
  }

  for (int j = 0; j < nword_output; j++) {
    ored = 0;
    for (int clk = 0; clk < n_clocks; clk++) {
      ored |= _data[psns[j]][clk];
    }
    GDLResult->setPsnmBits(j, ored);
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
