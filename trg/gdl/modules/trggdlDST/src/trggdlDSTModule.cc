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

  GDLResult->setGdlL1Time(_data[GDL::e_l1rvc][GDL::nClks - 1]);
  GDLResult->setComL1Time(_data[GDL::e_coml1][GDL::nClks - 1]);
  GDLResult->setTimsrcGdlTime(_data[GDL::e_toprvc][GDL::nClks - 1],
                              _data[GDL::e_etm0rvc][GDL::nClks - 1],
                              _data[GDL::e_etfvdrvc][GDL::nClks - 1]);
  GDLResult->setT0(_data[GDL::e_topt0][GDL::nClks - 1],
                   (_data[GDL::e_eclmsb7][GDL::nClks - 1] << 7) +
                   _data[GDL::e_ecllsb7][GDL::nClks - 1],
                   _data[GDL::e_etfout][GDL::nClks - 1]);

  GDL::EGDLTimingType gtt = (GDL::EGDLTimingType)_data[GDL::e_timtype][GDL::nClks - 1];
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
