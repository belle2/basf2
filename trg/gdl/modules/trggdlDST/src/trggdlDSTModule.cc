//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : trggdlDSTModule.cc
// Section  : TRG GDL DST
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : A trigger module for TRG GDL DST
//---------------------------------------------------------------
// 1.00 : 2017/05/08 : First version
//---------------------------------------------------------------
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>

#include <mdst/dataobjects/TRGSummary.h>
#include <trg/gdl/dataobjects/TRGGDLDST.h>
#include <trg/gdl/modules/trggdlDST/trggdlDSTModule.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>

#include <string.h>
#include <unistd.h>

using namespace Belle2;
using namespace GDL;

REG_MODULE(TRGGDLDST);

TRGGDLDSTModule::TRGGDLDSTModule()
  : Module::Module()
{

  setDescription("Fill experiment data to TRGGDLDST");
  setPropertyFlags(c_ParallelProcessingCertified);

}

void TRGGDLDSTModule::initialize()
{

  GDLResult.registerInDataStore();

}

void TRGGDLDSTModule::event()
{

  StoreObjPtr<EventMetaData> bevt;
  int _exp = bevt->getExperiment();
  int _run = bevt->getRun();
  int exprun = _exp * 1000000 + _run;
  int n_clocks = 0;
  int n_leafs = 0;
  int n_leafsExtra = 0;
  int _e_timtype = 0;
  void (*setPointer)(TRGGDLUnpackerStore * store, int** bitArray);
  int nconf = -1;
  if (exprun >= 3000677) {
    if (exprun >= 3001158) {
      n_clocks = nClks2;
      nconf = 2;
    } else {
      n_clocks = nClks1;
      nconf = 1;
    }
    n_leafs = nLeafs1;
    n_leafsExtra = nLeafsExtra1;
    setPointer = GDL::setLeafPointersArray1;
    _e_timtype = GDLCONF1::e_timtype;
  } else if (exprun >= 3000529) {
    nconf = 0;
    n_clocks = nClks0;
    n_leafs = nLeafs0;
    n_leafsExtra = nLeafsExtra0;
    setPointer = GDL::setLeafPointersArray0;
    _e_timtype = GDLCONF0::e_timtype;
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

  if (1 <= nconf && nconf <= 2) {
    GDLResult->setGdlL1Time(_data[GDLCONF1::e_gdll1rvc][n_clocks - 1]);
    GDLResult->setComL1Time(_data[GDLCONF1::e_coml1rvc][n_clocks - 1]);
    GDLResult->setTimsrcGdlTime(_data[GDLCONF1::e_toprvc][n_clocks - 1],
                                _data[GDLCONF1::e_eclrvc][n_clocks - 1],
                                _data[GDLCONF1::e_cdcrvc][n_clocks - 1]);
    GDLResult->setT0(_data[GDLCONF1::e_toptiming][n_clocks - 1],
                     _data[GDLCONF1::e_ecltiming][n_clocks - 1],
                     _data[GDLCONF1::e_cdctiming][n_clocks - 1]);
  } else if (nconf == 0) {
    GDLResult->setGdlL1Time(_data[GDLCONF0::e_gdll1][n_clocks - 1]);
    GDLResult->setComL1Time(_data[GDLCONF0::e_coml1][n_clocks - 1]);
    GDLResult->setTimsrcGdlTime(_data[GDLCONF0::e_toprvc][n_clocks - 1],
                                _data[GDLCONF0::e_etm0rvc][n_clocks - 1],
                                _data[GDLCONF0::e_etfvdrvc][n_clocks - 1]);

    GDLResult->setT0(_data[GDLCONF0::e_topt0][n_clocks - 1],
                     _data[GDLCONF0::e_eclmsb7][n_clocks - 1] * 128 +
                     _data[GDLCONF0::e_ecllsb7][n_clocks - 1],
                     _data[GDLCONF0::e_cdc_timing][n_clocks - 1]);
  }

  GDL::EGDLTimingType gtt = (GDL::EGDLTimingType)_data[_e_timtype][n_clocks - 1];
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
