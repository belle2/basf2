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
  addParam("trgReadoutBoardSearch", m_trgReadoutBoardSearch,
           "Print trigger readout board included in the data.",
           false);
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
      if (! m_trgReadoutBoardSearch) {
        if (raw_trgarray[i]->GetNodeID(j) == 0x15000001) {
          int nword = raw_trgarray[i]->GetDetectorNwords(j, 0);
          if (nword > 0) {
            int _exp = raw_trgarray[i]->GetExpNo(j);
            int _run = raw_trgarray[i]->GetRunNo(j);
            int exprun = _exp * 1000000 + _run;
            if (exprun >= 3005314) {
              fillTreeGDL6(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                           raw_trgarray[i]->GetEveNo(j));
            } else if (exprun >= 3004791) {
              fillTreeGDL5(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                           raw_trgarray[i]->GetEveNo(j));
            } else if (exprun >= 3001866) {
              fillTreeGDL4(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                           raw_trgarray[i]->GetEveNo(j));
            } else if (exprun >= 3001315) {
              fillTreeGDL3(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                           raw_trgarray[i]->GetEveNo(j));
            } else if (exprun >= 3000677) {
              fillTreeGDL2(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                           raw_trgarray[i]->GetEveNo(j));
            } else if (exprun >= 3000529) {
              fillTreeGDL1(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                           raw_trgarray[i]->GetEveNo(j));
            } else {
              fillTreeGDL0(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                           raw_trgarray[i]->GetEveNo(j));
            }
          }
        }
      } else {
        unsigned cprid = raw_trgarray[i]->GetNodeID(j);
        if ((0x15000001 <= cprid && cprid <= 0x15000002) ||
            (0x11000001 <= cprid && cprid <= 0x11000010)) {
          int _exp = raw_trgarray[i]->GetExpNo(j);
          int _run = raw_trgarray[i]->GetRunNo(j);
          for (int hslb = 0; hslb < 2; hslb++) {
            int nword = raw_trgarray[i]->GetDetectorNwords(j, hslb);
            int* buf  = raw_trgarray[i]->GetDetectorBuffer(j, hslb);
            printf("0x%x%c exp(%d), run(%d), nword(%d)",
                   cprid, 'a' + hslb, _exp, _run, nword);
            if (nword > 2) {
              printf(", 0x%x 0x%x 0x%x",
                     buf[0], buf[1], buf[2]);
            }
            printf("\n");
          }
        }
      }
    }
  }
}

// GDLCONF0. gdl0065c, gdl0065e < e3r0529
void TRGGDLUnpackerModule::fillTreeGDL0(int* buf, int evt)
{

  int nword_header = 3;
  int n_clocks = GDLCONF0::nClks;
  int n_leafs = GDLCONF0::nLeafs;
  int n_leafsExtra = GDLCONF0::nLeafsExtra;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  if (storeAry.isValid()) storeAry.clear();
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    GDLCONF0::setLeafPointersArray(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_conf = 0;
    storeAry[ntups]->m_evt = evt;
    storeAry[ntups]->m_clk = clk;
    storeAry[ntups]->m_firmid  =  buf[0];
    storeAry[ntups]->m_firmver =  buf[1];
    storeAry[ntups]->m_coml1rvc = buf[2]        & ((1 << 12) - 1);
    storeAry[ntups]->m_b2ldly  = (buf[2] >> 12) & ((1 <<  9) - 1);
    storeAry[ntups]->m_maxrvc  = (buf[2] >> 21) & ((1 << 11) - 1);

    for (int _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
      int wd = buf[clk * (nBits / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitIn640 = (nBits - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < n_leafs; leaf++) {
            int bitMaxOfTheLeaf = GDLCONF0::BitMap[leaf][0];
            int bitWidOfTheLeaf = GDLCONF0::BitMap[leaf][1];
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

// GDLCONF1. r529 - r583 gdl0065h, < r677.
void TRGGDLUnpackerModule::fillTreeGDL1(int* buf, int evt)
{

  int nword_header = 4;
  int n_clocks = GDLCONF1::nClks;
  int n_leafs = GDLCONF1::nLeafs;
  int n_leafsExtra = GDLCONF1::nLeafsExtra;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  if (storeAry.isValid()) storeAry.clear();
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    GDLCONF1::setLeafPointersArray(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_conf = 1;
    storeAry[ntups]->m_evt = evt;
    storeAry[ntups]->m_clk = clk;
    storeAry[ntups]->m_firmid  =  buf[0];
    storeAry[ntups]->m_firmver =  buf[1];
    storeAry[ntups]->m_gdll1rvc = buf[2]        & ((1 << 11) - 1);
    storeAry[ntups]->m_coml1rvc = buf[3]        & ((1 << 12) - 1);
    storeAry[ntups]->m_b2ldly  = (buf[3] >> 12) & ((1 <<  9) - 1);
    storeAry[ntups]->m_maxrvc  = (buf[3] >> 21) & ((1 << 11) - 1);

    for (int _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
      int wd = buf[clk * (nBits / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitIn640 = (nBits - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < n_leafs; leaf++) {
            int bitMaxOfTheLeaf = GDLCONF1::BitMap[leaf][0];
            int bitWidOfTheLeaf = GDLCONF1::BitMap[leaf][1];
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

// GDLCONF2.
// r677 - r816. gdl0065j (recorded as 65i). < r932.
// 969,70,71,72, 1158,62.  gdl0065k.
void TRGGDLUnpackerModule::fillTreeGDL2(int* buf, int evt)
{

  int nword_header = 6;
  int n_clocks = GDLCONF2::nClks;
  int n_leafs = GDLCONF2::nLeafs;
  int n_leafsExtra = GDLCONF2::nLeafsExtra;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  if (storeAry.isValid()) storeAry.clear();
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    GDLCONF2::setLeafPointersArray(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_conf = 2;
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
            int bitMaxOfTheLeaf = GDLCONF2::BitMap[leaf][0];
            int bitWidOfTheLeaf = GDLCONF2::BitMap[leaf][1];
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

// GDLCONF3. r1315 - r1828. gdl0066a, 66b, 66c, 66e.
// < r1866.
void TRGGDLUnpackerModule::fillTreeGDL3(int* buf, int evt)
{

  int nword_header = 6;
  int n_clocks = GDLCONF3::nClks;
  int n_leafs = GDLCONF3::nLeafs;
  int n_leafsExtra = GDLCONF3::nLeafsExtra;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  if (storeAry.isValid()) storeAry.clear();
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    GDLCONF3::setLeafPointersArray(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_conf = 3;
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
            int bitMaxOfTheLeaf = GDLCONF3::BitMap[leaf][0];
            int bitWidOfTheLeaf = GDLCONF3::BitMap[leaf][1];
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

// GDLCONF4. r1866 - r4700. gdl0066h. < r4876.
void TRGGDLUnpackerModule::fillTreeGDL4(int* buf, int evt)
{
  int nword_header = 6;
  int n_clocks = GDLCONF4::nClks;
  int n_leafs = GDLCONF4::nLeafs;
  int n_leafsExtra = GDLCONF4::nLeafsExtra;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  if (storeAry.isValid()) storeAry.clear();
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    GDLCONF4::setLeafPointersArray(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_conf = 4;
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
            int bitMaxOfTheLeaf = GDLCONF4::BitMap[leaf][0];
            int bitWidOfTheLeaf = GDLCONF4::BitMap[leaf][1];
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

// GDLCONF5. r4876 <= run < 5313. gdl0066k, gdl0067g.
void TRGGDLUnpackerModule::fillTreeGDL5(int* buf, int evt)
{

  // gdl0068. itd3, itd4, ftd3, psn3 added.
  int nword_header = 6;
  int n_clocks = GDLCONF5::nClks;
  int n_leafs = GDLCONF5::nLeafs;
  int n_leafsExtra = GDLCONF5::nLeafsExtra;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  if (storeAry.isValid()) storeAry.clear();
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    GDLCONF5::setLeafPointersArray(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_conf = 5;
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
            int bitMaxOfTheLeaf = GDLCONF5::BitMap[leaf][0];
            int bitWidOfTheLeaf = GDLCONF5::BitMap[leaf][1];
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

// GDLCONF6. r5314 <= run. gdl0068a, gdl0068b.
void TRGGDLUnpackerModule::fillTreeGDL6(int* buf, int evt)
{

  // gdl0068. itd3, itd4, ftd3, psn3 added.
  int nword_header = 6;
  int n_clocks = GDLCONF6::nClks;
  int n_leafs = GDLCONF6::nLeafs;
  int n_leafsExtra = GDLCONF6::nLeafsExtra;

  StoreArray<TRGGDLUnpackerStore> storeAry;
  if (storeAry.isValid()) storeAry.clear();
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];
    GDLCONF6::setLeafPointersArray(storeAry[ntups], Bits);
    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_conf = 6;
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
            int bitMaxOfTheLeaf = GDLCONF6::BitMap[leaf][0];
            int bitWidOfTheLeaf = GDLCONF6::BitMap[leaf][1];
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
