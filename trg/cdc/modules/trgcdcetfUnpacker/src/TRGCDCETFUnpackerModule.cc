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
// Filename : TRGCDCETFUnpackerModule.cc
// Section  :
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRGCDCETFUnpacker Module
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#include <trg/cdc/modules/trgcdcetfUnpacker/TRGCDCETFUnpackerModule.h>

using namespace std;
using namespace Belle2;
using namespace TRGCDCETFUNPACKERSPACE;

//! Register Module
REG_MODULE(TRGCDCETFUnpacker);

string TRGCDCETFUnpackerModule::version() const
{
  return string("1.00");
}

TRGCDCETFUnpackerModule::TRGCDCETFUnpackerModule()
  : Module::Module()
{

  string desc = "TRGCDCETFUnpackerModule(" + version() + ")";
  setDescription(desc);
  B2INFO("TRGCDCETFUnpacker: Constructor done.");
}

TRGCDCETFUnpackerModule::~TRGCDCETFUnpackerModule()
{
}

void TRGCDCETFUnpackerModule::terminate()
{
}

void TRGCDCETFUnpackerModule::initialize()
{
  m_store.registerInDataStore();
  //StoreArray<TRGCDCETFUnpackerStore>::registerPersistent();
}

void TRGCDCETFUnpackerModule::beginRun()
{
}

void TRGCDCETFUnpackerModule::endRun()
{
}

void TRGCDCETFUnpackerModule::event()
{
  StoreArray<RawTRG> raw_trgarray;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {

    // Check PCIe40 data or Copper data
    if (raw_trgarray[i]->GetMaxNumOfCh(0) == 48) { m_pciedata = true; }
    else if (raw_trgarray[i]->GetMaxNumOfCh(0) == 4) { m_pciedata = false; }
    else { B2FATAL("TRGCDCETFUnpackerModule: Invalid value of GetMaxNumOfCh from raw data: " << LogVar("Number of ch: ", raw_trgarray[i]->GetMaxNumOfCh(0))); }

    unsigned int node_id = 0;
    unsigned int ch_id = 0;
    if (m_pciedata) {
      node_id = m_pcie40_address;
      ch_id = m_pcie40_ch;
    } else {
      node_id = m_copper_address;
      ch_id = m_copper_ab;
    }

    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      if (raw_trgarray[i]->GetNodeID(j) == node_id) {
        if (raw_trgarray[i]->GetDetectorNwords(j, ch_id) > 0) {
          fillTreeTRGCDCETFUnpacker(raw_trgarray[i]->GetDetectorBuffer(j, ch_id), raw_trgarray[j]->GetEveNo(j));
        }
      }
    }
  }
}

void TRGCDCETFUnpackerModule::fillTreeTRGCDCETFUnpacker(int* buf, int evt)
{

  const unsigned nword_header = 3;

  //StoreArray<TRGCDCETFUnpackerStore> storeAry;
  for (int clk = 0; clk < nClks; clk++) { // 0..47

    m_store.appendNew();
    int ntups = m_store.getEntries() - 1;
    int* bitArray[nLeafs + nLeafsExtra];
    setLeafPointersArray(m_store[ntups], bitArray);
    for (int l = 0; l < nLeafs + nLeafsExtra; l++) *bitArray[l] = 0;

    m_store[ntups]->m_evt = evt;
    m_store[ntups]->m_clk = clk;
    m_store[ntups]->m_firmid  = buf[0];
    m_store[ntups]->m_firmvers = buf[1];
    //m_store[ntups]->m_etf_firmvers = buf[1];

    // Print data
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
      unsigned wd = buf[clk * (nBits / 32) + _wd + nword_header];
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
