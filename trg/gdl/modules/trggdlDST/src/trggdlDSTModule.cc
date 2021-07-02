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

#include <mdst/dataobjects/TRGSummary.h>
#include <trg/gdl/dataobjects/TRGGDLDST.h>
#include <trg/gdl/modules/trggdlDST/trggdlDSTModule.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>

#include <string.h>


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

  for (int i = 0; i < 320; i++) {
    LeafBitMap[i] = m_unpacker->getLeafMap(i);
  }
  for (int i = 0; i < 320; i++) {
    strcpy(LeafNames[i], m_unpacker->getLeafnames(i));
  }
  _e_timtype   = 0;
  _e_gdll1rvc  = 0;
  _e_coml1rvc  = 0;
  _e_toprvc    = 0;
  _e_eclrvc    = 0;
  _e_cdcrvc    = 0;
  _e_toptiming = 0;
  _e_ecltiming = 0;
  _e_cdctiming = 0;
  for (int i = 0; i < 320; i++) {
    if (strcmp(LeafNames[i], "timtype") == 0)    _e_timtype  = LeafBitMap[i];
    if (strcmp(LeafNames[i], "gdll1rvc") == 0)   _e_gdll1rvc = LeafBitMap[i];
    if (strcmp(LeafNames[i], "coml1rvc") == 0)   _e_coml1rvc = LeafBitMap[i];
    if (strcmp(LeafNames[i], "toprvc") == 0)     _e_toprvc   = LeafBitMap[i];
    if (strcmp(LeafNames[i], "eclrvc") == 0)     _e_eclrvc   = LeafBitMap[i];
    if (strcmp(LeafNames[i], "cdcrvc") == 0)     _e_cdcrvc   = LeafBitMap[i];
    if (strcmp(LeafNames[i], "toptiming") == 0)  _e_toptiming = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ecltiming") == 0)  _e_ecltiming = LeafBitMap[i];
    if (strcmp(LeafNames[i], "cdctiming") == 0)  _e_cdctiming = LeafBitMap[i];
  }
}

void TRGGDLDSTModule::event()
{

  int n_leafs = 0;
  n_leafs  = m_unpacker->getnLeafs();
  int n_leafsExtra = 0;
  n_leafsExtra = m_unpacker->getnLeafsExtra();
  int n_clocks = m_unpacker->getnClks();
  //int nconf = m_unpacker->getconf(); // unused


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
    std::vector<int*> Bits(n_leafs + n_leafsExtra);
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

  GDLResult.create();

  GDLResult->setGdlL1Time(_data[_e_gdll1rvc][n_clocks - 1]);
  GDLResult->setComL1Time(_data[_e_coml1rvc][n_clocks - 1]);
  GDLResult->setTimsrcGdlTime(_data[_e_toprvc][n_clocks - 1],
                              _data[_e_eclrvc][n_clocks - 1],
                              _data[_e_cdcrvc][n_clocks - 1]);
  GDLResult->setT0(_data[_e_toptiming][n_clocks - 1],
                   _data[_e_ecltiming][n_clocks - 1],
                   _data[_e_cdctiming][n_clocks - 1]);


  GDL::EGDLTimingType gtt = (GDL::EGDLTimingType)_data[_e_timtype][0];
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

  GDLResult->setTimType(tt);

}
