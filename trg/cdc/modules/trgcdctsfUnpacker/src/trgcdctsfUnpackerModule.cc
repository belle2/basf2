//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : trgcdctsfUnpackerModule.cc
// Section  : TRG CDCTSF
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRG CDCTSF Unpacker Module
//---------------------------------------------------------------
// 1.00 : 2017/07/26 : First version
//---------------------------------------------------------------

#include <trg/cdc/modules/trgcdctsfUnpacker/trgcdctsfUnpackerModule.h>
#include <bitset>

using namespace std;
using namespace Belle2;
using namespace TRGCDCTSF;

//! Register Module
REG_MODULE(TRGCDCTSFUnpacker);

string TRGCDCTSFUnpackerModule::version() const
{
  return string("1.00");
}

TRGCDCTSFUnpackerModule::TRGCDCTSFUnpackerModule()
  : Module::Module()
{

  string desc = "TRGCDCTSFUnpackerModule(" + version() + ")";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("TSFMOD", m_TSFMOD,
           "TSF module number",
           0);



  B2INFO("trgcdctsfunpacker: Constructor done.");


}

TRGCDCTSFUnpackerModule::~TRGCDCTSFUnpackerModule()
{
}

void TRGCDCTSFUnpackerModule::terminate()
{
}

void TRGCDCTSFUnpackerModule::initialize()
{

  StoreArray<TRGCDCTSFUnpackerStore> storeAry;
  storeAry.registerInDataStore();


  //set copper address
  if (m_TSFMOD == 0) {
    m_copper_address = 0x11000007;
    m_copper_ab = 0;
  } else if (m_TSFMOD == 1) {
    m_copper_address = 0x11000007;
    m_copper_ab = 1;
  } else if (m_TSFMOD == 2) {
    m_copper_address = 0x11000008;
    m_copper_ab = 0;
  } else if (m_TSFMOD == 3) {
    m_copper_address = 0x11000008;
    m_copper_ab = 1;
  } else if (m_TSFMOD == 4) {
    m_copper_address = 0x11000009;
    m_copper_ab = 0;
  } else {
    B2ERROR("trgcdctsfunpacker:cooper address is not set");
    m_copper_address = 0;
  }


  //set bitmap
  if (m_TSFMOD == 0 || m_TSFMOD == 4) {
    m_nBits = nBits_2k;
    m_nword = nword_2k;
    for (int i = 0; i < nLeafs; i++) {
      for (int j = 0; j < 2; j++) {
        m_BitMap[i][j] = BitMap_2k[i][j];
      }
    }
  } else if (m_TSFMOD == 1 || m_TSFMOD == 2 || m_TSFMOD == 3) {
    m_nBits = nBits_4k;
    m_nword = nword_4k;
    for (int i = 0; i < nLeafs; i++) {
      for (int j = 0; j < 2; j++) {
        m_BitMap[i][j] = BitMap_4k[i][j];
      }
    }
  } else {
    B2ERROR("trgcdctsfunpacker:cooper address is not set");
    m_nBits = 0;
    m_nword = 0;
    for (int i = 0; i < nLeafs; i++) {
      for (int j = 0; j < 2; j++) {
        m_BitMap[i][j] = 0;
      }
    }
  }

}

void TRGCDCTSFUnpackerModule::beginRun()
{
}

void TRGCDCTSFUnpackerModule::endRun()
{
}

void TRGCDCTSFUnpackerModule::event()
{
  StoreArray<RawTRG> raw_trgarray;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      if (raw_trgarray[i]->GetNodeID(j) == m_copper_address) {
        if (raw_trgarray[i]->GetDetectorNwords(j, m_copper_ab) == m_nword) {
          fillTreeCDCTSF(raw_trgarray[i]->GetDetectorBuffer(j, m_copper_ab), raw_trgarray[j]->GetEveNo(j));
        }
      }
    }
  }
}

void TRGCDCTSFUnpackerModule::fillTreeCDCTSF(int* buf, int evt)
{

  const unsigned nword_header = 3;

  StoreArray<TRGCDCTSFUnpackerStore> storeAry;
  for (int clk = 0; clk < nClks; clk++) { // 0..47

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* bitArray[nLeafs + nLeafsExtra];
    setLeafPointersArray(storeAry[ntups], bitArray);
    for (int l = 0; l < nLeafs + nLeafsExtra; l++) *bitArray[l] = 0;

    storeAry[ntups]->m_evt = evt;
    storeAry[ntups]->m_clk = clk;
    storeAry[ntups]->m_firmid  = buf[0];
    storeAry[ntups]->m_firmver = buf[1];


    for (int _wd = 0; _wd < m_nBits / 32; _wd++) { // 0..19
      int wd = buf[clk * (m_nBits / 32) + _wd + nword_header];
      bitset<32> bwd(wd);
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitPosition = (m_nBits - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < nLeafs; leaf++) {
            int bitMaxOfTheLeaf = m_BitMap[leaf][0];
            int bitWidOfTheLeaf = m_BitMap[leaf][1];
            int bitMinOfTheLeaf = bitMaxOfTheLeaf - bitWidOfTheLeaf;
            if (bitMinOfTheLeaf <= bitPosition && bitPosition <= bitMaxOfTheLeaf) {
              *bitArray[leaf] |= (1 << (bitPosition - bitMinOfTheLeaf));
            }
          }
        }
      }
    }
  }
}
