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

#include <iostream>

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
  addParam("print_dbmap", m_print_dbmap, "Print Database Bit Map", false);
  B2DEBUG(20, "trggdlunpacker: Constructor done.");
}

void TRGGDLUnpackerModule::initialize()
{
  store.registerInDataStore();

  unpack_flag = 1;

  //check bad run or not
  bad_flag = m_dbbadrun->getflag();
  if (bad_flag == -1) {
    B2INFO("bad run");
    unpack_flag = 0;
  }

  //load unpacker format
  if (!m_dbunpacker) {
    B2INFO("no database of gdl unpacker");
    unpack_flag = 0;
  } else {
    nword_header = m_dbunpacker->get_nword_header();
    n_clocks = m_dbunpacker->getnClks();
    nBits    = m_dbunpacker->getnBits();
    n_leafs  = m_dbunpacker->getnLeafs();
    n_leafsExtra = m_dbunpacker->getnLeafsExtra();
    conf = m_dbunpacker->getconf();
    for (int i = 0; i < 320; i++) {
      LeafBitMap[i] = m_dbunpacker->getLeafMap(i);
    }
    for (int i = 0; i < 320; i++) {
      strcpy(LeafNames[i], m_dbunpacker->getLeafnames(i));
    }
    conf_map = -1;
    evt_map = -1;
    clk_map = -1;
    for (int i = 0; i < 320; i++) {
      if (strcmp(LeafNames[i], "conf") == 0)conf_map = i;
      if (strcmp(LeafNames[i], "evt") == 0) evt_map = i;
      if (strcmp(LeafNames[i], "clk") == 0) clk_map = i;
    }

    BitMap = vector<vector<int>>(n_leafs, vector<int>(2, 0));
    for (int i = 0; i < n_leafs; i++) {
      BitMap[i][0] = m_dbunpacker->getBitMap(i, 0);
      BitMap[i][1] = m_dbunpacker->getBitMap(i, 1);
    }
    BitMap_extra = vector<vector<int>>(n_leafsExtra, vector<int>(3, 0));
    for (int i = 0; i < n_leafsExtra; i++) {
      BitMap_extra[i][0] = m_dbunpacker->getBitMap_extra(i, 0);
      BitMap_extra[i][1] = m_dbunpacker->getBitMap_extra(i, 1);
      BitMap_extra[i][2] = m_dbunpacker->getBitMap_extra(i, 2);
    }

    if (m_print_dbmap) {

      int aBitMap[320][2] = {0};
      int aBitMap_extra[100][3] = { -1};
      for (int i = 0; i < n_leafsExtra; i++) {
        aBitMap_extra[i][0] = m_dbunpacker->getBitMap_extra(i, 0);
        aBitMap_extra[i][1] = m_dbunpacker->getBitMap_extra(i, 1);
        aBitMap_extra[i][2] = m_dbunpacker->getBitMap_extra(i, 2);
      }

      for (int i = 0; i < 200; i++) {
        LeafBitMap[i] = m_dbunpacker->getLeafMap(i);
        std::cout << "LeafBitMap[" << i << "] = " << m_dbunpacker->getLeafMap(i) << std::endl;
        strcpy(LeafNames[i], m_dbunpacker->getLeafnames(i));
        std::cout << "LeafNames[" << i << "] = " << m_dbunpacker->getLeafnames(i) << std::endl;
      }
      for (int i = 0; i < n_leafs; i++) {
        aBitMap[i][0] = m_dbunpacker->getBitMap(i, 0);
        aBitMap[i][1] = m_dbunpacker->getBitMap(i, 1);
      }
      for (int i = 0; i < 320; i++) {
        int bin = m_dbunpacker->getLeafMap(i) + 1;
        if (0 < bin && bin <= n_leafs) {
          std::cout << "leaf(" << i
                    << "), bin(" << bin
                    << "), LeafNames[leaf](" << LeafNames[i]
                    << "), BitMap[bin-1][0](" << aBitMap[bin - 1][0]
                    << "), BitMap[bin-1][1](" << aBitMap[bin - 1][1]
                    << ")" << std::endl;
        }
      }
      // for leafsExtra
      for (int i = 0; i < 320; i++) {
        int bin = m_dbunpacker->getLeafMap(i) + 1;
        int j = bin - n_leafs - 1;
        if (n_leafs < bin && bin <= n_leafs + n_leafsExtra) {
          std::cout << "i(" << i
                    << "), bin(" << bin
                    << "), LeafNames[leaf](" << LeafNames[i]
                    << "), BitMap_extra[j][0](buf[" << aBitMap_extra[j][0]
                    << "]), BitMap_extra[j][1](downto " << aBitMap_extra[j][1]
                    << "), BitMap_extra[j][2](" << aBitMap_extra[j][2]
                    << " bit length)" << std::endl;
        }
      }

    }

  }

}

void TRGGDLUnpackerModule::event()
{
  if (unpack_flag == 1) {
    StoreArray<RawTRG> raw_trgarray;
    for (int i = 0; i < raw_trgarray.getEntries(); i++) {
      for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
        if (! m_trgReadoutBoardSearch) {
          if (raw_trgarray[i]->GetNodeID(j) == 0x15000001) {
            int nword = raw_trgarray[i]->GetDetectorNwords(j, 0);
            if (nword > 0) {
              fillTreeGDLDB(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                            raw_trgarray[i]->GetEveNo(j));
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
}


void TRGGDLUnpackerModule::fillTreeGDLDB(int* buf, int evt)
{


  StoreArray<TRGGDLUnpackerStore> storeAry;
  if (storeAry.isValid()) storeAry.clear();
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    vector<int*> Bits(n_leafs + n_leafsExtra);

    for (int i = 0; i < 320; i++) {
      if (LeafBitMap[i] != -1) {
        Bits[LeafBitMap[i]] = &(storeAry[ntups]->m_unpacker[i]);
        strcpy(storeAry[ntups]->m_unpackername[i], LeafNames[i]);
      }
    }


    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    if (conf_map >= 0) storeAry[ntups]->m_unpacker[conf_map] = conf;
    if (evt_map >= 0)  storeAry[ntups]->m_unpacker[evt_map]  = evt;
    if (clk_map >= 0)  storeAry[ntups]->m_unpacker[clk_map]  = clk;

    for (int i = 0; i < n_leafsExtra; i++) {
      if (BitMap_extra[i][0] != -1) {
        if (BitMap_extra[i][1] == -1)  *Bits[i + n_leafs]  =  buf[BitMap_extra[i][0]];
        else                          *Bits[i + n_leafs]  = (buf[BitMap_extra[i][0]] >> BitMap_extra[i][1]) & ((
                                                                1 << BitMap_extra[i][2]) - 1);
      }
    }


    for (int _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
      unsigned wd = buf[clk * (nBits / 32) + _wd + nword_header];
      for (int bb = 0; bb < 32; bb++) { // bit by bit
        if ((wd >> (31 - bb)) & 1) { /* MSB to LSB */
          int bitIn640 = (nBits - 1) - _wd * 32 - bb;
          for (int leaf = 0; // Find a leaf that covers the bit.
               leaf < n_leafs; leaf++) {
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
  }//clk
}


