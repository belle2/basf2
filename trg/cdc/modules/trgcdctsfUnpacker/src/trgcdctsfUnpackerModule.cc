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

  StoreArray<TRGCDCTSFUnpackerStore>::registerPersistent();
}

void TRGCDCTSFUnpackerModule::beginRun()
{
}

void TRGCDCTSFUnpackerModule::endRun()
{
}

void TRGCDCTSFUnpackerModule::event()
{
//  cout << "TSFunpacker start!" << endl;
  StoreArray<RawTRG> raw_trgarray;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
//      if (raw_trgarray[i]->GetNodeID(j) == 0x11000002) {
      if (raw_trgarray[i]->GetNodeID(j) == 0x11000007) {
//      cout << raw_trgarray[j]->GetDetectorNwords(j,0) << endl;
//       if (raw_trgarray[i]->GetDetectorNwords(j, 0) > 0) {
//    cout << raw_trgarray[i]->GetDetectorNwords(j,0) << endl;
        if (raw_trgarray[i]->GetDetectorNwords(j, 0) == 0xC03) {
//        if (raw_trgarray[i]->GetDetectorNwords(j, 0) ==6147) {
//      cout << "Nwords " << endl;
          fillTreeCDCTSF(raw_trgarray[i]->GetDetectorBuffer(j, 0), raw_trgarray[j]->GetEveNo(j));
        }
      }
    }
  }
}

void TRGCDCTSFUnpackerModule::fillTreeCDCTSF(int* buf, int evt)
{

//cout << "FillTSF" << endl;
  const unsigned nword_header = 3;

  StoreArray<TRGCDCTSFUnpackerStore> storeAry;
  for (int clk = 0; clk < nClks; clk++) { // 0..47

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* bitArray[nLeafs + nLeafsExtra];
    setLeafPointersArray(storeAry[ntups], bitArray);
    //cout << "SetLeafPointer " << endl;
    for (int l = 0; l < nLeafs + nLeafsExtra; l++) *bitArray[l] = 0;

    storeAry[ntups]->m_evt = evt;
    storeAry[ntups]->m_clk = clk;
    storeAry[ntups]->m_firmid  = buf[0];
    storeAry[ntups]->m_firmver = buf[1];

//    cout<<"nClks: "<<nClks<<endl;
//    cout<<"Up,Left is MSB, Down,Right is LSB"<<endl;
//    for (int _wd = 0; _wd < nBits / 32 + nword_header; _wd++)
//    {
//      bitset<32> buf_b(buf[clk * (nBits / 32) + _wd]);
//      stringstream wd_s;
//      wd_s << setfill('0') << setw(2) << _wd;
//      stringstream wd_s_d;
//      wd_s_d << setfill('0') << setw(2) << _wd-nword_header;
//      if (_wd < nword_header) cout<<"clk["<<clk<<"] hd["<<wd_s.str()<<"] "<<buf_b<<endl;
//      else cout<<"clk["<<clk<<"] wd["<<wd_s_d.str()<<"] "<<buf_b<<endl;
//    }
//    ////

    for (int _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
      int wd = buf[clk * (nBits / 32) + _wd + nword_header];
      bitset<32> bwd(wd);
      // cout << bwd << endl;
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
            //    cout << std::dec << leaf << endl;
            //    cout << bitMaxOfTheLeaf << " " << std::hex << *bitArray[leaf] << endl;

          }
        }
      }
    }
//  cout << "out " << endl;
  }
}
