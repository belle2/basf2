//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGCDCT3DUnpackerModule.cc
// Section  :
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRGCDCT3DUnpacker Module
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#include <trg/cdc/modules/trgcdct3dUnpacker/TRGCDCT3DUnpackerModule.h>
#include <bitset>
#include <iomanip>

using namespace std;
using namespace Belle2;
using namespace TRGCDCT3DUNPACKERSPACE;

//! Register Module
REG_MODULE(TRGCDCT3DUnpacker);

string TRGCDCT3DUnpackerModule::version() const
{
  return string("1.00");
}

TRGCDCT3DUnpackerModule::TRGCDCT3DUnpackerModule()
  : Module::Module()
{

  string desc = "TRGCDCT3DUnpackerModule(" + version() + ")";
  setDescription(desc);
  B2INFO("TRGCDCT3DUnpacker: Constructor done.");
}

TRGCDCT3DUnpackerModule::~TRGCDCT3DUnpackerModule()
{
}

void TRGCDCT3DUnpackerModule::terminate()
{
}

void TRGCDCT3DUnpackerModule::initialize()
{
  m_store.registerInDataStore();
  //StoreArray<TRGCDCT3DUnpackerStore>::registerPersistent();
}

void TRGCDCT3DUnpackerModule::beginRun()
{
}

void TRGCDCT3DUnpackerModule::endRun()
{
}

void TRGCDCT3DUnpackerModule::event()
{
  StoreArray<RawTRG> raw_trgarray;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      if (raw_trgarray[i]->GetNodeID(j) == 0x11000003) {
        if (raw_trgarray[i]->GetDetectorNwords(j, 1) > 0) {
          fillTreeTRGCDCT3DUnpacker(raw_trgarray[i]->GetDetectorBuffer(j, 1), raw_trgarray[j]->GetEveNo(j));
        }
      }
    }
  }
}

void TRGCDCT3DUnpackerModule::fillTreeTRGCDCT3DUnpacker(int* buf, int evt)
{

  const unsigned nword_header = 2;

  //StoreArray<TRGCDCT3DUnpackerStore> storeAry;
  for (int clk = 0; clk < nClks; clk++) { // 0..47

    m_store.appendNew();
    int ntups = m_store.getEntries() - 1;
    int* bitArray[nLeafs + nLeafsExtra];
    setLeafPointersArray(m_store[ntups], bitArray);
    for (int l = 0; l < nLeafs + nLeafsExtra; l++) *bitArray[l] = 0;

    m_store[ntups]->m_evt = evt;
    m_store[ntups]->m_clk = clk;
    m_store[ntups]->m_firmid  = buf[0];
    m_store[ntups]->m_firmver = buf[1];

    //// Print data
    //cout<<"nClks: "<<nClks<<endl;
    //cout<<"Up,Left is MSB, Down,Right is LSB"<<endl;
    //for (unsigned _wd = 0; _wd < nBits / 32 + nword_header; _wd++)
    //{
    //  bitset<32> buf_b(buf[clk * (nBits / 32) + _wd]);
    //  stringstream wd_s;
    //  wd_s << setfill('0') << setw(2) << _wd;
    //  stringstream wd_s_d;
    //  wd_s_d << setfill('0') << setw(2) << _wd-nword_header;
    //  if (_wd < nword_header) cout<<"clk["<<clk<<"] hd["<<wd_s.str()<<"] "<<buf_b<<endl;
    //  else cout<<"clk["<<clk<<"] wd["<<wd_s_d.str()<<"] "<<buf_b<<endl;
    //}

    //cout<<"nClks: "<<nClks<<endl;
    //for (int _wd = 0; _wd < nBits / 32; _wd++)
    //{
    //  bitset<32> buf_b(buf[clk * (nBits / 32) + _wd + nword_header]);
    //  cout<<"clk["<<clk<<"] wd["<<_wd<<"] "<<buf_b<<endl;
    //}

    for (unsigned _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
      int wd = buf[clk * (nBits / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitPosition = (nBits - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < nLeafs; leaf++) {
            int bitMaxOfTheLeaf = BitMap[leaf][0];
            int bitWidOfTheLeaf = BitMap[leaf][1];
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
