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
        int nword = raw_trgarray[i]->GetDetectorNwords(j, 0);
        if (nword > 0) {
          int _exp = raw_trgarray[i]->GetExpNo(j);
          int _run = raw_trgarray[i]->GetRunNo(j);
          int exprun = _exp * 1000000 + _run;
          if (exprun >= 3001158) {
            fillTreeGDL2(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                         raw_trgarray[i]->GetEveNo(j));
          } else if (exprun >= 3000677) {
            fillTreeGDL1(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                         raw_trgarray[i]->GetEveNo(j));
          } else if (exprun >= 3000529) {
            fillTreeGDL0(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                         raw_trgarray[i]->GetEveNo(j));
          }
        }
      }
    }
  }
}

// r529 - r583 gdl0065h
void TRGGDLUnpackerModule::fillTreeGDL0(int* buf, int evt)
{

  unsigned nword_header = 4;
  unsigned n_clocks = nClks0;
  unsigned n_leafs = nLeafs0;
  unsigned n_leafsExtra = nLeafsExtra0;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    setLeafPointersArray0(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_evt = evt;
    storeAry[ntups]->m_clk = clk;
    storeAry[ntups]->m_firmid  =  buf[0];
    storeAry[ntups]->m_firmver =  buf[1];
    storeAry[ntups]->m_gdll1rvc = buf[2]        & ((1 << 11) - 1);
    storeAry[ntups]->m_b2ldly  = (buf[3] >> 12) & ((1 <<  9) - 1);
    storeAry[ntups]->m_maxrvc  = (buf[3] >> 21) & ((1 << 11) - 1);

    for (int _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
      int wd = buf[clk * (nBits / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitIn640 = (nBits - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < n_leafs; leaf++) {
            int bitMaxOfTheLeaf = BitMap0[leaf][0];
            int bitWidOfTheLeaf = BitMap0[leaf][1];
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

void TRGGDLUnpackerModule::fillTreeGDL1(int* buf, int evt)
{

  unsigned nword_header = 6;
  unsigned n_clocks = nClks1;
  unsigned n_leafs = nLeafs1;
  unsigned n_leafsExtra = nLeafsExtra1;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    setLeafPointersArray1(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_evt = evt;
    storeAry[ntups]->m_clk = clk;
    storeAry[ntups]->m_firmid  =  buf[0];
    storeAry[ntups]->m_firmver =  buf[1];
    storeAry[ntups]->m_finalrvc =  buf[2]        & ((1 << 11) - 1);
    storeAry[ntups]->m_drvc    = (buf[3] >> 11) & ((1 << 11) - 1);
    storeAry[ntups]->m_gdll1rvc =  buf[3]        & ((1 << 11) - 1);
    storeAry[ntups]->m_coml1rvc =  buf[5]        & ((1 << 12) - 1);
    storeAry[ntups]->m_b2ldly  = (buf[5] >> 12) & ((1 <<  9) - 1);
    storeAry[ntups]->m_maxrvc  = (buf[5] >> 21) & ((1 << 11) - 1);

    for (int _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
      int wd = buf[clk * (nBits / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitIn640 = (nBits - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < n_leafs; leaf++) {
            int bitMaxOfTheLeaf = BitMap1[leaf][0];
            int bitWidOfTheLeaf = BitMap1[leaf][1];
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

void TRGGDLUnpackerModule::fillTreeGDL2(int* buf, int evt)
{

  unsigned nword_header = 6;
  unsigned n_clocks = nClks2;
  unsigned n_leafs = nLeafs1;
  unsigned n_leafsExtra = nLeafsExtra1;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    setLeafPointersArray1(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

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
               leaf < n_leafs; leaf++) {
            int bitMaxOfTheLeaf = BitMap1[leaf][0];
            int bitWidOfTheLeaf = BitMap1[leaf][1];
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
