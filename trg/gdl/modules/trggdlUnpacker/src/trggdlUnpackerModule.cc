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
  if (!m_unpacker)B2INFO("no db prescales");
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
            fillTreeGDLDB(raw_trgarray[i]->GetDetectorBuffer(j, 0),
                          raw_trgarray[i]->GetEveNo(j));

            //int _exp = raw_trgarray[i]->GetExpNo(j);
            //int _run = raw_trgarray[i]->GetRunNo(j);
            //int exprun = _exp * 1000000 + _run;
            //if (exprun >= 3005314) {
            //  fillTreeGDL6(raw_trgarray[i]->GetDetectorBuffer(j, 0),
            //               raw_trgarray[i]->GetEveNo(j));
            //} else if (exprun >= 3004791) {
            //  fillTreeGDL5(raw_trgarray[i]->GetDetectorBuffer(j, 0),
            //               raw_trgarray[i]->GetEveNo(j));
            //} else if (exprun >= 3001866) {
            //  fillTreeGDL4(raw_trgarray[i]->GetDetectorBuffer(j, 0),
            //               raw_trgarray[i]->GetEveNo(j));
            //} else if (exprun >= 3001315) {
            //  fillTreeGDL3(raw_trgarray[i]->GetDetectorBuffer(j, 0),
            //               raw_trgarray[i]->GetEveNo(j));
            //} else if (exprun >= 3000677) {
            //  fillTreeGDL2(raw_trgarray[i]->GetDetectorBuffer(j, 0),
            //               raw_trgarray[i]->GetEveNo(j));
            //} else if (exprun >= 3000529) {
            //  fillTreeGDL1(raw_trgarray[i]->GetDetectorBuffer(j, 0),
            //               raw_trgarray[i]->GetEveNo(j));
            //} else {
            //  fillTreeGDL0(raw_trgarray[i]->GetDetectorBuffer(j, 0),
            //               raw_trgarray[i]->GetEveNo(j));
            //}
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


void TRGGDLUnpackerModule::fillTreeGDLDB(int* buf, int evt)
{

  // gdl0068. itd3, itd4, ftd3, psn3 added.
  const int nword_header = m_unpacker->get_nword_header();
  const int n_clocks = m_unpacker->getnClks();
  const int n_leafs  = m_unpacker->getnLeafs();
  const int n_leafsExtra = m_unpacker->getnLeafsExtra();

  int BitMap[n_leafs][2];
  for (int i = 0; i < n_leafs; i++) {
    BitMap[i][0] = m_unpacker->getBitMap(i, 0);
    BitMap[i][1] = m_unpacker->getBitMap(i, 1);
    //std::cout << i << " " << BitMap[i][0] << " " << BitMap[i][1] << std::endl;
  }

  int LeafBitMap[200];
  for (int i = 0; i < 200; i++) {
    LeafBitMap[i] = m_unpacker->getLeafMap(i);
  }

  int conf = m_unpacker->getconf();
  int firmid[3]  = {m_unpacker->getfirmid(0)  , m_unpacker->getfirmid(1)  , m_unpacker->getfirmid(2)};
  int firmver[3] = {m_unpacker->getfirmver(0) , m_unpacker->getfirmver(1) , m_unpacker->getfirmver(2)};
  int finalrvc[3] = {m_unpacker->getfinalrvc(0), m_unpacker->getfinalrvc(1), m_unpacker->getfinalrvc(2)};
  int drvc[3]    = {m_unpacker->getdrvc(0)    , m_unpacker->getdrvc(1)    , m_unpacker->getdrvc(2)};
  int gdll1rvc[3] = {m_unpacker->getgdll1rvc(0), m_unpacker->getgdll1rvc(1), m_unpacker->getgdll1rvc(2)};
  int coml1rvc[3] = {m_unpacker->getcoml1rvc(0), m_unpacker->getcoml1rvc(1), m_unpacker->getcoml1rvc(2)};
  int b2ldly[3]  = {m_unpacker->getb2ldly(0)  , m_unpacker->getb2ldly(1)  , m_unpacker->getb2ldly(2)};
  int maxrvc[3]  = {m_unpacker->getmaxrvc(0)  , m_unpacker->getmaxrvc(1)  , m_unpacker->getmaxrvc(2)};


  StoreArray<TRGGDLUnpackerStore> storeAry;
  if (storeAry.isValid()) storeAry.clear();
  for (int clk = 0; clk < n_clocks; clk++) {

    storeAry.appendNew();
    int ntups = storeAry.getEntries() - 1;
    int* Bits[n_leafs + n_leafsExtra];

    //setLeafPointersArray_db(storeAry[ntups], Bits);
    if (LeafBitMap[0] != -1)Bits[LeafBitMap[0] ] = &(storeAry[ntups]->m_etffmv);
    if (LeafBitMap[1] != -1)Bits[LeafBitMap[1] ] = &(storeAry[ntups]->m_l1rvc);
    if (LeafBitMap[2] != -1)Bits[LeafBitMap[2] ] = &(storeAry[ntups]->m_timtype);
    if (LeafBitMap[3] != -1)Bits[LeafBitMap[3] ] = &(storeAry[ntups]->m_etyp);
    if (LeafBitMap[4] != -1)Bits[LeafBitMap[4] ] = &(storeAry[ntups]->m_final);
    if (LeafBitMap[5] != -1)Bits[LeafBitMap[5] ] = &(storeAry[ntups]->m_gdll1rvc);
    if (LeafBitMap[6] != -1)Bits[LeafBitMap[6] ] = &(storeAry[ntups]->m_etfout);
    if (LeafBitMap[7] != -1)Bits[LeafBitMap[7] ] = &(storeAry[ntups]->m_etfvd);
    if (LeafBitMap[8] != -1)Bits[LeafBitMap[8] ] = &(storeAry[ntups]->m_toprvc);
    if (LeafBitMap[9] != -1)Bits[LeafBitMap[9] ] = &(storeAry[ntups]->m_topvd);
    if (LeafBitMap[10] != -1)Bits[LeafBitMap[10]] = &(storeAry[ntups]->m_toptiming);
    if (LeafBitMap[11] != -1)Bits[LeafBitMap[11]] = &(storeAry[ntups]->m_ecltiming);
    if (LeafBitMap[12] != -1)Bits[LeafBitMap[12]] = &(storeAry[ntups]->m_cdctiming);
    if (LeafBitMap[13] != -1)Bits[LeafBitMap[13]] = &(storeAry[ntups]->m_rvcout);
    if (LeafBitMap[14] != -1)Bits[LeafBitMap[14]] = &(storeAry[ntups]->m_rvcout0);
    if (LeafBitMap[15] != -1)Bits[LeafBitMap[15]] = &(storeAry[ntups]->m_comrvc);
    if (LeafBitMap[16] != -1)Bits[LeafBitMap[16]] = &(storeAry[ntups]->m_etnrvc);
    if (LeafBitMap[17] != -1)Bits[LeafBitMap[17]] = &(storeAry[ntups]->m_nim0rvc);
    if (LeafBitMap[18] != -1)Bits[LeafBitMap[18]] = &(storeAry[ntups]->m_eclrvc);
    if (LeafBitMap[19] != -1)Bits[LeafBitMap[19]] = &(storeAry[ntups]->m_rvc);
    if (LeafBitMap[20] != -1)Bits[LeafBitMap[20]] = &(storeAry[ntups]->m_drvc);
    if (LeafBitMap[21] != -1)Bits[LeafBitMap[21]] = &(storeAry[ntups]->m_ftd2);
    if (LeafBitMap[22] != -1)Bits[LeafBitMap[22]] = &(storeAry[ntups]->m_psn2);
    if (LeafBitMap[23] != -1)Bits[LeafBitMap[23]] = &(storeAry[ntups]->m_psn1);
    if (LeafBitMap[24] != -1)Bits[LeafBitMap[24]] = &(storeAry[ntups]->m_etfth2);
    if (LeafBitMap[25] != -1)Bits[LeafBitMap[25]] = &(storeAry[ntups]->m_etfhdt);
    if (LeafBitMap[26] != -1)Bits[LeafBitMap[26]] = &(storeAry[ntups]->m_etfth);
    if (LeafBitMap[27] != -1)Bits[LeafBitMap[27]] = &(storeAry[ntups]->m_psn0);
    if (LeafBitMap[28] != -1)Bits[LeafBitMap[28]] = &(storeAry[ntups]->m_ftd1);
    if (LeafBitMap[29] != -1)Bits[LeafBitMap[29]] = &(storeAry[ntups]->m_cdcrvc);
    if (LeafBitMap[30] != -1)Bits[LeafBitMap[30]] = &(storeAry[ntups]->m_ftd0);
    if (LeafBitMap[31] != -1)Bits[LeafBitMap[31]] = &(storeAry[ntups]->m_itd2);
    if (LeafBitMap[32] != -1)Bits[LeafBitMap[32]] = &(storeAry[ntups]->m_itd1);
    if (LeafBitMap[33] != -1)Bits[LeafBitMap[33]] = &(storeAry[ntups]->m_itd0);
    //std::cout << LeafBitMap[33] << std::endl;
    if (LeafBitMap[34] != -1)Bits[LeafBitMap[34]] = &(storeAry[ntups]->m_inp2);
    if (LeafBitMap[35] != -1)Bits[LeafBitMap[35]] = &(storeAry[ntups]->m_inp1);
    if (LeafBitMap[36] != -1)Bits[LeafBitMap[36]] = &(storeAry[ntups]->m_inp0);
    if (LeafBitMap[37] != -1)Bits[LeafBitMap[37]] = &(storeAry[ntups]->m_evt);
    if (LeafBitMap[38] != -1)Bits[LeafBitMap[38]] = &(storeAry[ntups]->m_clk);
    if (LeafBitMap[39] != -1)Bits[LeafBitMap[39]] = &(storeAry[ntups]->m_firmid);
    if (LeafBitMap[40] != -1)Bits[LeafBitMap[40]] = &(storeAry[ntups]->m_firmver);
    if (LeafBitMap[41] != -1)Bits[LeafBitMap[41]] = &(storeAry[ntups]->m_coml1rvc);
    if (LeafBitMap[42] != -1)Bits[LeafBitMap[42]] = &(storeAry[ntups]->m_b2ldly);
    if (LeafBitMap[43] != -1)Bits[LeafBitMap[43]] = &(storeAry[ntups]->m_maxrvc);
    if (LeafBitMap[44] != -1)Bits[LeafBitMap[44]] = &(storeAry[ntups]->m_conf);
    if (LeafBitMap[45] != -1)Bits[LeafBitMap[45]] = &(storeAry[ntups]->m_dtoprvc);
    if (LeafBitMap[46] != -1)Bits[LeafBitMap[46]] = &(storeAry[ntups]->m_declrvc);
    if (LeafBitMap[47] != -1)Bits[LeafBitMap[47]] = &(storeAry[ntups]->m_dcdcrvc);
    if (LeafBitMap[48] != -1)Bits[LeafBitMap[48]] = &(storeAry[ntups]->m_topslot1);
    if (LeafBitMap[49] != -1)Bits[LeafBitMap[49]] = &(storeAry[ntups]->m_topslot0);
    if (LeafBitMap[50] != -1)Bits[LeafBitMap[50]] = &(storeAry[ntups]->m_ntopslot);
    if (LeafBitMap[51] != -1)Bits[LeafBitMap[51]] = &(storeAry[ntups]->m_finalrvc);
    if (LeafBitMap[52] != -1)Bits[LeafBitMap[52]] = &(storeAry[ntups]->m_tttmdl);
    if (LeafBitMap[53] != -1)Bits[LeafBitMap[53]] = &(storeAry[ntups]->m_tdsrcp);
    if (LeafBitMap[54] != -1)Bits[LeafBitMap[54]] = &(storeAry[ntups]->m_tdtopp);
    if (LeafBitMap[55] != -1)Bits[LeafBitMap[55]] = &(storeAry[ntups]->m_tdeclp);
    if (LeafBitMap[56] != -1)Bits[LeafBitMap[56]] = &(storeAry[ntups]->m_tdcdcp);
    if (LeafBitMap[57] != -1)Bits[LeafBitMap[57]] = &(storeAry[ntups]->m_psn3);
    if (LeafBitMap[58] != -1)Bits[LeafBitMap[58]] = &(storeAry[ntups]->m_ftd3);
    if (LeafBitMap[59] != -1)Bits[LeafBitMap[59]] = &(storeAry[ntups]->m_itd4);
    if (LeafBitMap[60] != -1)Bits[LeafBitMap[60]] = &(storeAry[ntups]->m_itd3);


    for (int l = 0; l < n_leafs + n_leafsExtra; l++) *Bits[l] = 0;

    storeAry[ntups]->m_conf = conf;
    storeAry[ntups]->m_evt = evt;
    storeAry[ntups]->m_clk = clk;
    if (firmid[0] != -1)   storeAry[ntups]->m_firmid   =  buf[firmid[0]];
    if (firmver[0] != -1)  storeAry[ntups]->m_firmver  =  buf[firmver[0]];
    if (drvc[0] != -1)     storeAry[ntups]->m_drvc     = (buf[drvc[0]]     >> drvc[1])     & ((1 << drvc[2]) - 1);
    if (gdll1rvc[0] != -1) storeAry[ntups]->m_gdll1rvc = (buf[gdll1rvc[0]] >> gdll1rvc[1]) & ((1 << gdll1rvc[2]) - 1);
    if (coml1rvc[0] != -1) storeAry[ntups]->m_coml1rvc = (buf[coml1rvc[0]] >> coml1rvc[1]) & ((1 << coml1rvc[2]) - 1);
    if (b2ldly[0] != -1)   storeAry[ntups]->m_b2ldly   = (buf[b2ldly[0]]   >> b2ldly[1])   & ((1 << b2ldly[2]) - 1);
    if (maxrvc[0] != -1)   storeAry[ntups]->m_maxrvc   = (buf[maxrvc[0]]   >> maxrvc[1])   & ((1 << maxrvc[2]) - 1);
    if (finalrvc[0] != -1) storeAry[ntups]->m_finalrvc = (buf[finalrvc[0]] >> finalrvc[1]) & ((1 << finalrvc[2]) - 1);


    for (int _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
      int wd = buf[clk * (nBits / 32) + _wd + nword_header];
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
            //if(leaf==31)std::cout << leaf << " " << BitMap[leaf][0] << " " << BitMap[leaf][1] << std::endl;
          }
        }
      }
    }
  }//clk
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
