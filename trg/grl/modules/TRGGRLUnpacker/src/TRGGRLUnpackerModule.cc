/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/grl/modules/TRGGRLUnpacker/TRGGRLUnpackerModule.h>

using namespace std;
using namespace Belle2;
using namespace TRGGRLUNPACKERSPACE;

//! Register Module
REG_MODULE(TRGGRLUnpacker);

string TRGGRLUnpackerModule::version() const
{
  return string("1.00");
}

TRGGRLUnpackerModule::TRGGRLUnpackerModule()
  : Module::Module()
{

  string desc = "TRGGRLUnpackerModule(" + version() + ")";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);
  B2DEBUG(20, "TRGGRLUnpacker: Constructor done.");
}

TRGGRLUnpackerModule::~TRGGRLUnpackerModule()
{
}

void TRGGRLUnpackerModule::terminate()
{
}

void TRGGRLUnpackerModule::initialize()
{
  StoreObjPtr<TRGGRLUnpackerStore> evtinfo("TRGGRLUnpackerStore");
  evtinfo.registerInDataStore();
}

void TRGGRLUnpackerModule::beginRun()
{
}

void TRGGRLUnpackerModule::endRun()
{
}

void TRGGRLUnpackerModule::event()
{
  StoreArray<RawTRG> raw_trgarray;

  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    if (raw_trgarray[i]->GetTRGType(0) == 7) {continue;}

    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      if (raw_trgarray[i]->GetNodeID(j) == 0x15000002) {
        //cout << raw_trgarray[i]->GetDetectorNwords(j, 0) << endl;
        //if (raw_trgarray[i]->GetDetectorNwords(j, 0) == 0xC03)
        if (raw_trgarray[i]->GetDetectorNwords(j, 0) > 0) {
          fillTreeTRGGRLUnpacker(raw_trgarray[i]->GetDetectorBuffer(j, 0), raw_trgarray[i]->GetEveNo(j));
        }
      }
    }
  }
}

void TRGGRLUnpackerModule::fillTreeTRGGRLUnpacker(int* buf, int evt)
{

  const unsigned nword_header = 3;

//  StoreArray<TRGGRLUnpackerStore> storeAry("GRLclk");
  TRGGRLUnpackerStore* rawstore = new TRGGRLUnpackerStore();
  StoreObjPtr<TRGGRLUnpackerStore> evtinfo("TRGGRLUnpackerStore");
//  evtinfo.registerInDataStore();
//    storeAry.appendNew();
//    int ntups = storeAry.getEntries() - 1;
  int* bitArray[nLeafs + nLeafsExtra];
  setLeafPointersArray(rawstore, bitArray);
  for (int l = 0; l < nLeafs + nLeafsExtra; l++) *bitArray[l] = 0;

  rawstore->m_evt = evt;
  rawstore->m_clk = 0;
  rawstore->m_firmid  = buf[0];
  rawstore->m_firmver = buf[1];
  rawstore->m_coml1   = buf[2] & ((1 << 12) - 1);
  rawstore->m_b2ldly  = (buf[2] >> 12) & ((1 << 9) - 1);
  rawstore->m_maxrvc  = (buf[2] >> 21) & ((1 << 11) - 1);

  //cout<<"nClks: "<<nClks<<endl;
  //for (int _wd = 0; _wd < nBits / 32; _wd++)
  //{
  //  bitset<32> buf_b(buf[clk * (nBits / 32) + _wd + nword_header]);
  //  cout<<"clk["<<clk<<"] wd["<<_wd<<"] "<<buf_b<<endl;
  //}

  for (int _wd = 0; _wd < nBits / 32; _wd++) { // 0..19
    unsigned wd = buf[0 * (nBits / 32) + _wd + nword_header];
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
//----------
  rawstore->m_N_cluster = rawstore->m_N_cluster_0 + rawstore->m_N_cluster_1;
  evtinfo.assign(rawstore);
  std::vector<int> index_ECL;
  std::vector<int> clkindex_ECL;
  for (int i = 0; i < rawstore->m_N_cluster_0; i++) {
    index_ECL.push_back(i); clkindex_ECL.push_back(0);
  }
  for (int i = 0; i < rawstore->m_N_cluster_1; i++) {
    index_ECL.push_back(i + 6); clkindex_ECL.push_back(1);
  }
//  for (int i = 0; i < rawstore->m_N_cluster_2; i++) {
//    index_ECL.push_back(i+12); clkindex_ECL.push_back(2);}

  evtinfo->m_E_ECL.clear();
  evtinfo->m_t_ECL.clear();
  evtinfo->m_clk_ECL.clear();
  evtinfo->m_theta_ECL.clear();
  evtinfo->m_phi_ECL.clear();
  evtinfo->m_1GeV_ECL.clear();
  evtinfo->m_2GeV_ECL.clear();

  for (int i = 0; i < rawstore->m_N_cluster; i++) {
    int index = index_ECL[i];
    int clkindex = clkindex_ECL[i];
    evtinfo->m_clk_ECL.push_back(clkindex);
    evtinfo->m_E_ECL.push_back(rawstore->m_E_ECL[index]);
    evtinfo->m_t_ECL.push_back(rawstore->m_t_ECL[index]);
    evtinfo->m_theta_ECL.push_back(rawstore->m_theta_ECL[index]);
    evtinfo->m_phi_ECL.push_back(rawstore->m_phi_ECL[index]);
    evtinfo->m_E_ECL.push_back(rawstore->m_E_ECL[index]);
    evtinfo->m_1GeV_ECL.push_back(rawstore->m_1GeV_ECL[index]);
    evtinfo->m_2GeV_ECL.push_back(rawstore->m_2GeV_ECL[index]);
  }

//----------

  for (int i = 0; i < 32; i++) {
    evtinfo->m_phi_i[i] = ((rawstore->m_phi_i_int[0] & (1u << i)) != 0);
    evtinfo->m_phi_CDC[i] = ((rawstore->m_phi_CDC_int[0] & (1u << i)) != 0);
  }
  for (int i = 32; i < 36; i++) {
    evtinfo->m_phi_i[i] = ((rawstore->m_phi_i_int[1] & (1 << (i - 32))) != 0);
    evtinfo->m_phi_CDC[i] = ((rawstore->m_phi_CDC_int[1] & (1 << (i - 32))) != 0);
  }

  for (int i = 0; i < 16; i++) {
    evtinfo->m_slot_CDC[i] = ((rawstore->m_slot_CDC_int & (1 << i)) != 0);
    evtinfo->m_slot_TOP[i] = ((rawstore->m_slot_TOP_int & (1 << i)) != 0);
  }

  for (int i = 0; i < 8; i++) {
    evtinfo->m_sector_CDC[i] = ((rawstore->m_sector_CDC_int & (1 << i)) != 0);
    evtinfo->m_sector_KLM[i] = ((rawstore->m_sector_KLM_int & (1 << i)) != 0);
  }

  for (int i = 0; i < 32; i++) {
    evtinfo->m_map_ST[i] = ((rawstore->m_map_ST_int[0] & (1u << i)) != 0);
    evtinfo->m_map_ST2[i] = ((rawstore->m_map_ST2_int[0] & (1u << i)) != 0);
    evtinfo->m_map_veto[i] = ((rawstore->m_map_veto_int[0] & (1u << i)) != 0);
    evtinfo->m_map_TSF0[i] = ((rawstore->m_map_TSF0_int[0] & (1u << i)) != 0);
    evtinfo->m_map_TSF2[i] = ((rawstore->m_map_TSF2_int[0] & (1u << i)) != 0);
    evtinfo->m_map_TSF4[i] = ((rawstore->m_map_TSF4_int[0] & (1u << i)) != 0);
    evtinfo->m_map_TSF1[i] = ((rawstore->m_map_TSF1_int[0] & (1u << i)) != 0);
    evtinfo->m_map_TSF3[i] = ((rawstore->m_map_TSF3_int[0] & (1u << i)) != 0);
  }
  for (int i = 32; i < 64; i++) {
    evtinfo->m_map_ST[i] = ((rawstore->m_map_ST_int[1] & (1u << (i - 32))) != 0);
    evtinfo->m_map_ST2[i] = ((rawstore->m_map_ST2_int[1] & (1u << (i - 32))) != 0);
    evtinfo->m_map_veto[i] = ((rawstore->m_map_veto_int[1] & (1u << (i - 32))) != 0);
    evtinfo->m_map_TSF0[i] = ((rawstore->m_map_TSF0_int[1] & (1u << (i - 32))) != 0);
    evtinfo->m_map_TSF2[i] = ((rawstore->m_map_TSF2_int[1] & (1u << (i - 32))) != 0);
    evtinfo->m_map_TSF4[i] = ((rawstore->m_map_TSF4_int[1] & (1u << (i - 32))) != 0);
    evtinfo->m_map_TSF1[i] = ((rawstore->m_map_TSF1_int[1] & (1u << (i - 32))) != 0);
    evtinfo->m_map_TSF3[i] = ((rawstore->m_map_TSF3_int[1] & (1u << (i - 32))) != 0);
  }


}
