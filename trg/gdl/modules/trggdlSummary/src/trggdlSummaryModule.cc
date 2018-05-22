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

  StoreArray<TRGGDLUnpackerStore> entAry;

  std::vector<std::vector<int> > _data(GDL::nLeafs + GDL::nLeafsExtra);
  for (int leaf = 0; leaf < GDL::nLeafs + GDL::nLeafsExtra; leaf++) {
    std::vector<int> _v(GDL::nClks);
    _data[leaf] = _v;
  }

  // fill "bit vs clk" for the event
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    int* Bits[GDL::nLeafs + GDL::nLeafsExtra];
    setLeafPointersArray(entAry[ii], Bits);
    for (int leaf = 0; leaf < GDL::nLeafs + GDL::nLeafsExtra; leaf++) {
      _data[leaf][entAry[ii]->m_clk] =  *Bits[leaf];
    }
  }


  GDLResult.create();

  unsigned ored = 0;

  const unsigned itds[] = {GDL::e_itd0, GDL::e_itd1, GDL::e_itd2};
  for (int j = 0; j < 3; j++) {
    ored = 0;
    for (int clk = 0; clk < GDL::nClks; clk++) {
      ored |= _data[itds[j]][clk];
    }
    GDLResult->setInputBits(j, ored);
  }

  const unsigned ftds[] = {GDL::e_ftd0, GDL::e_ftd1};
  for (int j = 0; j < 2; j++) {
    ored = 0;
    for (int clk = 0; clk < GDL::nClks; clk++) {
      ored |= _data[ftds[j]][clk];
    }
    GDLResult->setFtdlBits(j, ored);
  }

  const unsigned psns[] = {GDL::e_psn0, GDL::e_psn1};
  for (int j = 0; j < 2; j++) {
    ored = 0;
    for (int clk = 0; clk < GDL::nClks; clk++) {
      ored |= _data[psns[j]][clk];
    }
    GDLResult->setPsnmBits(j, ored);
  }

  GDL::EGDLTimingType gtt = (GDL::EGDLTimingType)_data[e_timtype][GDL::nClks - 1];

  TRGSummary::ETimingType tt = TRGSummary::TTYP_NONE;
  if (gtt == GDL::e_tt_cdc) {
    tt = TRGSummary::TTYP_CDC;
  } else if (gtt == GDL::e_tt_ecl) {
    tt = TRGSummary::TTYP_PID0;
  } else if (gtt == GDL::e_tt_rand) {
    tt = TRGSummary::TTYP_RAND;
  }

  GDLResult->setTimType(tt);

}
