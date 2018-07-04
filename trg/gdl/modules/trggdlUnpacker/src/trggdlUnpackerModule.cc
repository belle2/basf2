//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : trggdlUnpackerModule.cc
// Section  : TRG GDL
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRG GDL Unpacker Module
//---------------------------------------------------------------
// 1.00 : 2017/07/26 : First version
//---------------------------------------------------------------

#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>

using namespace std;
using namespace Belle2;
using namespace GDL;

//! Register Module
REG_MODULE(TRGGDLUnpacker);

string TRGGDLUnpackerModule::version() const
{
  return string("1.00");
}

TRGGDLUnpackerModule::TRGGDLUnpackerModule()
  : Module::Module()
{
  string desc = "TRGGDLUnpackerModule(" + version() + ")";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);
  B2INFO("trggdlunpacker: Constructor done.");
}

void TRGGDLUnpackerModule::initialize()
{
  store.registerInDataStore();
}

void TRGGDLUnpackerModule::event()
{
  StoreArray<RawTRG> raw_trgarray;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      if (raw_trgarray[i]->GetNodeID(j) == 0x15000001) {
        if (raw_trgarray[i]->GetDetectorNwords(j, 0) > 0) {
          fillTreeGDL1(raw_trgarray[i]->GetDetectorBuffer(j, 0), raw_trgarray[i]->GetEveNo(j));
        }
      }
    }
  }

}

void TRGGDLUnpackerModule::fillTreeGDL1(int* buf, int evt)
{

  const unsigned nword_header = 3;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  for (int clk = 0; clk < nClks; clk++) { // 0..47

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[nLeafs + nLeafsExtra];
    setLeafPointersArray(storeAry[ntups], Bits);
    for (int l = 0; l < nLeafs + nLeafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_evt = evt;
    storeAry[ntups]->m_clk = clk;
    storeAry[ntups]->m_firmid  = buf[0];
    storeAry[ntups]->m_firmver = buf[1];
    storeAry[ntups]->m_coml1   = buf[2] & ((1 << 12) - 1);
    storeAry[ntups]->m_b2ldly  = (buf[2] >> 12) & ((1 << 9) - 1);
    storeAry[ntups]->m_maxrvc  = (buf[2] >> 21) & ((1 << 11) - 1);

    for (int _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
      int wd = buf[clk * (nBits / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitIn640 = (nBits - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < nLeafs; leaf++) {
            int bitMaxOfTheLeaf = BitMap[leaf][0];
            int bitWidOfTheLeaf = BitMap[leaf][1];
            int bitMinOfTheLeaf = bitMaxOfTheLeaf - bitWidOfTheLeaf;
            if (bitMinOfTheLeaf <= bitIn640 && bitIn640 <= bitMaxOfTheLeaf) {
              *Bits[leaf] |= (1 << (bitIn640 - bitMinOfTheLeaf));
            }
          }
        }
      }
    }
  }
}
