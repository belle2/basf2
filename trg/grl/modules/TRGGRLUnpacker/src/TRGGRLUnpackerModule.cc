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

    // Check PCIe40 data or Copper data
    if (raw_trgarray[i]->GetMaxNumOfCh(0) == 48) { m_pciedata = true; }
    else if (raw_trgarray[i]->GetMaxNumOfCh(0) == 4) { m_pciedata = false; }
    else { B2FATAL("TRGGRLUnpackerModule: Invalid value of GetMaxNumOfCh from raw data: " << LogVar("Number of ch: ", raw_trgarray[i]->GetMaxNumOfCh(0))); }

    int node_id = 0;
    int ch_id = 0;
    if (m_pciedata) {
      node_id = 0x10000001;
      ch_id = 22;
    } else {
      node_id = 0x15000002;
      ch_id = 0;
    }


    if (raw_trgarray[i]->GetTRGType(0) == 7) {continue;}

    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      if ((int)raw_trgarray[i]->GetNodeID(j) == node_id) {
        //cout << raw_trgarray[i]->GetDetectorNwords(j, 0) << endl;
        //if (raw_trgarray[i]->GetDetectorNwords(j, 0) == 0xC03)
        if (raw_trgarray[i]->GetDetectorNwords(j, ch_id) > 0) {
          fillTreeTRGGRLUnpacker(raw_trgarray[i]->GetDetectorBuffer(j, ch_id), raw_trgarray[i]->GetEveNo(j));
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
//
  //int* bitArray[nLeafs + nLeafsExtra];
  //setLeafPointersArray(rawstore, bitArray);
  //for (int l = 0; l < nLeafs + nLeafsExtra; l++) *bitArray[l] = 0;
  for (int l = 0; l < nLeafs + nLeafsExtra; l++) {
    SetStoreLeaf(rawstore, l, 0);
  }

  //set or get?
  rawstore->set_evt(evt);
  rawstore->set_clk(0);
  rawstore->set_firmid(buf[0]);
  rawstore->set_firmver(buf[1]);
  rawstore->set_coml1(buf[2] & ((1 << 12) - 1));
  rawstore->set_b2ldly((buf[2] >> 12) & ((1 << 9) - 1));
  rawstore->set_maxrvc((buf[2] >> 21) & ((1 << 11) - 1));

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
            SetStoreLeaf(rawstore, leaf, GetStoreLeaf(rawstore, leaf) | (1 << (bitPosition - bitMinOfTheLeaf)));
          }
        }
      }
    }
  }
//----------
  rawstore->set_N_cluster(rawstore->get_N_cluster_0() + rawstore->get_N_cluster_1());
  evtinfo.assign(rawstore);
  std::vector<int> index_ECL;
  std::vector<int> clkindex_ECL;
  index_ECL.clear();
  clkindex_ECL.clear();
  for (int i = 0; i < rawstore->get_N_cluster_0(); i++) {
    index_ECL.push_back(i); clkindex_ECL.push_back(0);
  }
  for (int i = 0; i < rawstore->get_N_cluster_1(); i++) {
    index_ECL.push_back(i + 6); clkindex_ECL.push_back(1);
  }
//  for (int i = 0; i < rawstore->m_N_cluster_2; i++) {
//    index_ECL.push_back(i+12); clkindex_ECL.push_back(2);}

  evtinfo->ClearVectors();
  for (int i = 0; i < rawstore->get_N_cluster(); i++) {
    int index = index_ECL[i];
    int clkindex = clkindex_ECL[i];

    evtinfo->Addto_clk_ECL(clkindex);
    evtinfo->Addto_E_ECL(rawstore->get_E_ECL(index));
    evtinfo->Addto_t_ECL(rawstore->get_t_ECL(index));
    evtinfo->Addto_theta_ECL(rawstore->get_theta_ECL(index));
    evtinfo->Addto_phi_ECL(rawstore->get_phi_ECL(index));
    evtinfo->Addto_E_ECL(rawstore->get_E_ECL(index));
    evtinfo->Addto_1GeV_ECL(rawstore->get_1GeV_ECL(index));
    evtinfo->Addto_2GeV_ECL(rawstore->get_2GeV_ECL(index));
  }

//----------

  for (int i = 0; i < 32; i++) {
    evtinfo->set_phi_i(i, (rawstore->get_phi_i_int(0) & (1u << i)) != 0);
    evtinfo->set_phi_CDC(i, (rawstore->get_phi_CDC_int(0) & (1u << i)) != 0);
  }
  for (int i = 32; i < 36; i++) {
    evtinfo->set_phi_i(i, (rawstore->get_phi_i_int(1) & (1 << (i - 32))) != 0);
    evtinfo->set_phi_CDC(i, (rawstore->get_phi_CDC_int(1) & (1 << (i - 32))) != 0);
  }

  for (int i = 0; i < 16; i++) {
    evtinfo->set_slot_CDC(i, (rawstore->get_slot_CDC_int() & (1 << i)) != 0);
    evtinfo->set_slot_TOP(i, (rawstore->get_slot_TOP_int() & (1 << i)) != 0);
  }

  for (int i = 0; i < 8; i++) {
    evtinfo->set_sector_CDC(i, (rawstore->get_sector_CDC_int() & (1 << i)) != 0);
    evtinfo->set_sector_KLM(i, (rawstore->get_sector_KLM_int() & (1 << i)) != 0);
  }

  for (int i = 0; i < 32; i++) {
    evtinfo->set_map_ST(i, (rawstore->get_map_ST_int(0) & (1u << i)) != 0);
    evtinfo->set_map_ST2(i, (rawstore->get_map_ST2_int(0) & (1u << i)) != 0);
    evtinfo->set_map_veto(i, (rawstore->get_map_veto_int(0) & (1u << i)) != 0);
    evtinfo->set_map_TSF0(i, (rawstore->get_map_TSF0_int(0) & (1u << i)) != 0);
    evtinfo->set_map_TSF2(i, (rawstore->get_map_TSF2_int(0) & (1u << i)) != 0);
    evtinfo->set_map_TSF4(i, (rawstore->get_map_TSF4_int(0) & (1u << i)) != 0);
    evtinfo->set_map_TSF1(i, (rawstore->get_map_TSF1_int(0) & (1u << i)) != 0);
    evtinfo->set_map_TSF3(i, (rawstore->get_map_TSF3_int(0) & (1u << i)) != 0);
  }
  for (int i = 32; i < 64; i++) {
    evtinfo->set_map_ST(i, (rawstore->get_map_ST_int(1) & (1u << (i - 32))) != 0);
    evtinfo->set_map_ST2(i, (rawstore->get_map_ST2_int(1) & (1u << (i - 32))) != 0);
    evtinfo->set_map_veto(i, (rawstore->get_map_veto_int(1) & (1u << (i - 32))) != 0);
    evtinfo->set_map_TSF0(i, (rawstore->get_map_TSF0_int(1) & (1u << (i - 32))) != 0);
    evtinfo->set_map_TSF2(i, (rawstore->get_map_TSF2_int(1) & (1u << (i - 32))) != 0);
    evtinfo->set_map_TSF4(i, (rawstore->get_map_TSF4_int(1) & (1u << (i - 32))) != 0);
    evtinfo->set_map_TSF1(i, (rawstore->get_map_TSF1_int(1) & (1u << (i - 32))) != 0);
    evtinfo->set_map_TSF3(i, (rawstore->get_map_TSF3_int(1) & (1u << (i - 32))) != 0);
  }


}
