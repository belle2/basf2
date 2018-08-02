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

  int n_clocks = 0;
  int n_leafs = 0;
  int n_leafsExtra = 0;
  int _e_timtype = 0;
  void (*setPointer)(TRGGDLUnpackerStore * store, int** bitArray);
  StoreArray<TRGGDLUnpackerStore> entAry;
  int nconf = entAry[0]->m_conf;

  if (nconf == 6) {
    n_clocks = GDLCONF6::nClks;
    n_leafs = GDLCONF6::nLeafs;
    n_leafsExtra = GDLCONF6::nLeafsExtra;
    _e_timtype = GDLCONF6::e_timtype;
    setPointer = GDLCONF6::setLeafPointersArray;
  } else if (nconf == 5) {
    n_clocks = GDLCONF5::nClks;
    n_leafs = GDLCONF5::nLeafs;
    n_leafsExtra = GDLCONF5::nLeafsExtra;
    _e_timtype = GDLCONF5::e_timtype;
    setPointer = GDLCONF5::setLeafPointersArray;
  } else if (nconf == 4) {
    n_clocks = GDLCONF4::nClks;
    n_leafs = GDLCONF4::nLeafs;
    n_leafsExtra = GDLCONF4::nLeafsExtra;
    _e_timtype = GDLCONF4::e_timtype;
    setPointer = GDLCONF4::setLeafPointersArray;
  } else if (nconf == 3) {
    n_clocks = GDLCONF3::nClks;
    n_leafs = GDLCONF3::nLeafs;
    n_leafsExtra = GDLCONF3::nLeafsExtra;
    _e_timtype = GDLCONF3::e_timtype;
    setPointer = GDLCONF3::setLeafPointersArray;
  } else if (nconf == 2) {
    n_clocks = GDLCONF2::nClks;
    n_leafs = GDLCONF2::nLeafs;
    n_leafsExtra = GDLCONF2::nLeafsExtra;
    _e_timtype = GDLCONF2::e_timtype;
    setPointer = GDLCONF2::setLeafPointersArray;
  } else if (nconf == 1) {
    n_clocks = GDLCONF1::nClks;
    n_leafs = GDLCONF1::nLeafs;
    n_leafsExtra = GDLCONF1::nLeafsExtra;
    _e_timtype = GDLCONF1::e_timtype;
    setPointer = GDLCONF1::setLeafPointersArray;
  } else {
    n_clocks = GDLCONF0::nClks;
    n_leafs = GDLCONF0::nLeafs;
    n_leafsExtra = GDLCONF0::nLeafsExtra;
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

  if (6 == nconf) {
    GDLResult->setGdlL1Time(_data[GDLCONF6::e_gdll1rvc][n_clocks - 1]);
    GDLResult->setComL1Time(_data[GDLCONF6::e_coml1rvc][n_clocks - 1]);
    GDLResult->setTimsrcGdlTime(_data[GDLCONF6::e_toprvc][n_clocks - 1],
                                _data[GDLCONF6::e_eclrvc][n_clocks - 1],
                                _data[GDLCONF6::e_cdcrvc][n_clocks - 1]);
    GDLResult->setT0(_data[GDLCONF6::e_toptiming][n_clocks - 1],
                     _data[GDLCONF6::e_ecltiming][n_clocks - 1],
                     _data[GDLCONF6::e_cdctiming][n_clocks - 1]);
  } else if (nconf == 5) {
    GDLResult->setGdlL1Time(_data[GDLCONF5::e_gdll1rvc][n_clocks - 1]);
    GDLResult->setComL1Time(_data[GDLCONF5::e_coml1rvc][n_clocks - 1]);
    GDLResult->setTimsrcGdlTime(_data[GDLCONF5::e_toprvc][n_clocks - 1],
                                _data[GDLCONF5::e_eclrvc][n_clocks - 1],
                                _data[GDLCONF5::e_cdcrvc][n_clocks - 1]);
    GDLResult->setT0(_data[GDLCONF5::e_toptiming][n_clocks - 1],
                     _data[GDLCONF5::e_ecltiming][n_clocks - 1],
                     _data[GDLCONF5::e_cdctiming][n_clocks - 1]);
  } else if (nconf == 4) {
    GDLResult->setGdlL1Time(_data[GDLCONF4::e_gdll1rvc][n_clocks - 1]);
    GDLResult->setComL1Time(_data[GDLCONF4::e_coml1rvc][n_clocks - 1]);
    GDLResult->setTimsrcGdlTime(_data[GDLCONF4::e_toprvc][n_clocks - 1],
                                _data[GDLCONF4::e_eclrvc][n_clocks - 1],
                                _data[GDLCONF4::e_cdcrvc][n_clocks - 1]);
    GDLResult->setT0(_data[GDLCONF4::e_toptiming][n_clocks - 1],
                     _data[GDLCONF4::e_ecltiming][n_clocks - 1],
                     _data[GDLCONF4::e_cdctiming][n_clocks - 1]);
  } else if (nconf == 3) {
    GDLResult->setGdlL1Time(_data[GDLCONF3::e_gdll1rvc][n_clocks - 1]);
    GDLResult->setComL1Time(_data[GDLCONF3::e_coml1rvc][n_clocks - 1]);
    GDLResult->setTimsrcGdlTime(_data[GDLCONF3::e_toprvc][n_clocks - 1],
                                _data[GDLCONF3::e_eclrvc][n_clocks - 1],
                                _data[GDLCONF3::e_cdcrvc][n_clocks - 1]);
    GDLResult->setT0(_data[GDLCONF3::e_toptiming][n_clocks - 1],
                     _data[GDLCONF3::e_ecltiming][n_clocks - 1],
                     _data[GDLCONF3::e_cdctiming][n_clocks - 1]);
  } else if (nconf == 2) {
    GDLResult->setGdlL1Time(_data[GDLCONF2::e_gdll1rvc][n_clocks - 1]);
    GDLResult->setComL1Time(_data[GDLCONF2::e_coml1rvc][n_clocks - 1]);
    GDLResult->setTimsrcGdlTime(_data[GDLCONF2::e_toprvc][n_clocks - 1],
                                _data[GDLCONF2::e_eclrvc][n_clocks - 1],
                                _data[GDLCONF2::e_cdcrvc][n_clocks - 1]);
    GDLResult->setT0(_data[GDLCONF2::e_toptiming][n_clocks - 1],
                     _data[GDLCONF2::e_ecltiming][n_clocks - 1],
                     _data[GDLCONF2::e_cdctiming][n_clocks - 1]);
  } else if (nconf == 1) {
    GDLResult->setGdlL1Time(_data[GDLCONF1::e_gdll1rvc][n_clocks - 1]);
    GDLResult->setComL1Time(_data[GDLCONF1::e_coml1rvc][n_clocks - 1]);
    GDLResult->setTimsrcGdlTime(_data[GDLCONF1::e_toprvc][n_clocks - 1],
                                _data[GDLCONF1::e_eclrvc][n_clocks - 1],
                                _data[GDLCONF1::e_cdcrvc][n_clocks - 1]);
    GDLResult->setT0(_data[GDLCONF1::e_toptiming][n_clocks - 1],
                     _data[GDLCONF1::e_ecltiming][n_clocks - 1],
                     _data[GDLCONF1::e_cdctiming][n_clocks - 1]);
  } else if (nconf == 0) {
    GDLResult->setGdlL1Time(_data[GDLCONF0::e_gdll1rvc][n_clocks - 1]);
    GDLResult->setComL1Time(_data[GDLCONF0::e_coml1rvc][n_clocks - 1]);
    GDLResult->setTimsrcGdlTime(_data[GDLCONF0::e_toprvc][n_clocks - 1],
                                _data[GDLCONF0::e_eclrvc][n_clocks - 1],
                                _data[GDLCONF0::e_cdcrvc][n_clocks - 1]);

    GDLResult->setT0(_data[GDLCONF0::e_toptiming][n_clocks - 1],
                     _data[GDLCONF0::e_ecltiming][n_clocks - 1],
                     _data[GDLCONF0::e_cdctiming][n_clocks - 1]);
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
