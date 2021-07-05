/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGCDCT3DUnpackerModule.cc
// Section  :
// Owner    : JB Kim, physjg
// Email    : physjg@hep1.phys.ntu.edu.tw
//---------------------------------------------------------------
// Description : TRGCDCT3DUnpacker Module
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#include <trg/cdc/modules/trgcdct3dUnpacker/TRGCDCT3DUnpackerModule.h>

using namespace std;
using namespace Belle2;
using namespace TRGCDCT3DUNPACKERSPACE;

//! Register Module
REG_MODULE(TRGCDCT3DUnpacker);

string TRGCDCT3DUnpackerModule::version() const
{
  return string("1.10");
}

TRGCDCT3DUnpackerModule::TRGCDCT3DUnpackerModule()
  : Module::Module()
{

  string desc = "TRGCDCT3DUnpackerModule(" + version() + ")";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("T3DMOD", m_T3DMOD,
           "T3D module number",
           0);
  B2DEBUG(20, "TRGCDCT3DUnpacker: Constructor done.");
}

TRGCDCT3DUnpackerModule::~TRGCDCT3DUnpackerModule()
{
}

void TRGCDCT3DUnpackerModule::terminate()
{
}

void TRGCDCT3DUnpackerModule::initialize()
{
  char c_name[100];
  sprintf(c_name, "TRGCDCT3DUnpackerStore%d", m_T3DMOD);
  m_store.registerInDataStore(c_name);
  //StoreArray<TRGCDCT3DUnpackerStore>::registerPersistent();

  //set copper address
  if (m_T3DMOD == 0) {
    m_copper_address = 0x11000003;
    m_copper_ab = 0;
//    m_nword = 3075;
  } else if (m_T3DMOD == 1) {
    m_copper_address = 0x11000003;
    m_copper_ab = 1;
//    m_nword = 3075;
  } else if (m_T3DMOD == 2) {
    m_copper_address = 0x11000004;
    m_copper_ab = 0;
//    m_nword = 3075;
  } else if (m_T3DMOD == 3) {
    m_copper_address = 0x11000004;
    m_copper_ab = 1;
//    m_nword = 3075;
  } else {
    B2ERROR("trgcdct3dunpacker:cooper address is not set");
    m_copper_address = 0;
    m_copper_ab = 0;
//    m_nword = 3075;
  }

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
      if (raw_trgarray[i]->GetNodeID(j) == m_copper_address) {

        if (raw_trgarray[i]->GetDetectorNwords(j, m_copper_ab) == m_nword_2k) {
          int firm_id = (raw_trgarray[i]->GetDetectorBuffer(j, m_copper_ab))[0];
          if (firm_id == 0x32444620) { // 2D fitter
            fillTreeTRGCDCT3DUnpacker_2dfitter(raw_trgarray[i]->GetDetectorBuffer(j, m_copper_ab), raw_trgarray[i]->GetEveNo(j));
          } else {
            fillTreeTRGCDCT3DUnpacker(raw_trgarray[i]->GetDetectorBuffer(j, m_copper_ab), raw_trgarray[i]->GetEveNo(j));
          }
        }
        // 2.6k, 15 TS version
        else if (raw_trgarray[i]->GetDetectorNwords(j, m_copper_ab) == m_nword_2624) {
          fillTreeTRGCDCT3DUnpacker_2624(raw_trgarray[i]->GetDetectorBuffer(j, m_copper_ab), raw_trgarray[i]->GetEveNo(j));
        }
      }
    }
  }
}

void TRGCDCT3DUnpackerModule::fillTreeTRGCDCT3DUnpacker(int* buf, int evt)
{

  const unsigned nword_header = 3;  // updated from 2 to 3

  long dataHeader = buf[nword_header] & 0xffff0000;
  if (dataHeader != 0xdddd0000) {
    // wrong data block header
    return ;
  }

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

    for (unsigned _wd = 0; _wd < nBits_2k / 32; _wd++) { // 0..19
      unsigned wd = buf[clk * (nBits_2k / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitPosition = (nBits_2k - 1) - _wd * 32 - bb;
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

void TRGCDCT3DUnpackerModule::fillTreeTRGCDCT3DUnpacker_2dfitter(int* buf, int evt)
{

  const unsigned nword_header = 3;  // updated from 2 to 3

  long dataHeader = buf[nword_header] & 0xffff0000;
  if (dataHeader != 0xdddd0000) {
    // wrong data block header
    return ;
  }

  //StoreArray<TRGCDCT3DUnpackerStore> storeAry;
  for (int clk = 0; clk < nClks; clk++) { // 0..47

    m_store.appendNew();
    int ntups = m_store.getEntries() - 1;
    int* bitArray[nLeafs_2dfitter + nLeafsExtra];
    setLeafPointersArray_2dfitter(m_store[ntups], bitArray);
    for (int l = 0; l < nLeafs_2dfitter + nLeafsExtra; l++) *bitArray[l] = 0;

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

    for (unsigned _wd = 0; _wd < nBits_2k / 32; _wd++) { // 0..19
      unsigned wd = buf[clk * (nBits_2k / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitPosition = (nBits_2k - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < nLeafs_2dfitter; leaf++) {
            int bitMaxOfTheLeaf = BitMap_2dfitter[leaf][0];
            int bitWidOfTheLeaf = BitMap_2dfitter[leaf][1];
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


void TRGCDCT3DUnpackerModule::fillTreeTRGCDCT3DUnpacker_2624(int* buf, int evt)
{

  const unsigned nword_header = 3;  // updated from 2 to 3

  long dataHeader = buf[nword_header] & 0xffff0000;
  if (dataHeader != 0xdddd0000) {
    // wrong data block header
    return ;
  }

  //StoreArray<TRGCDCT3DUnpackerStore> storeAry;
  for (int clk = 0; clk < nClks; clk++) { // 0..47

    m_store.appendNew();
    int ntups = m_store.getEntries() - 1;
    int* bitArray[nLeafs_2624 + nLeafsExtra];
    setLeafPointersArray_2624(m_store[ntups], bitArray);
    for (int l = 0; l < nLeafs_2624 + nLeafsExtra; l++) *bitArray[l] = 0;

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

    for (unsigned _wd = 0; _wd < nBits_2624 / 32; _wd++) { // 0..19
      unsigned wd = buf[clk * (nBits_2624 / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitPosition = (nBits_2624 - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < nLeafs_2624; leaf++) {
            int bitMaxOfTheLeaf = BitMap_2624[leaf][0];
            int bitWidOfTheLeaf = BitMap_2624[leaf][1];
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
