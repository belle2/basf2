/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef __clang__
// Disable stack-usage diagnostics for this file
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif

#include <pxd/modules/pxdUnpacking/PXDHardwareClusterUnpackerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vector>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

using namespace boost::spirit::endian;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDHardwareClusterUnpacker)


#define ONSEN_ERR_FLAG_PACKET_SIZE  0x00004000ul


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


///*********************************************************************************
///****************** DHC Data Frame Code starts here *****************************
///*********************************************************************************

PXDHardwareClusterUnpackerModule::PXDHardwareClusterUnpackerModule() :
  Module(),

  ////Cluster store
  m_storeHardwareCluster()

{
  //Set module properties
  setDescription("Unpack Raw PXD Cluster from ONSEN data stream");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("HeaderEndianSwap", m_headerEndianSwap, "Swap the endianess of the ONSEN header", true);
  addParam("DoNotStore", m_doNotStore, "only unpack and check, but do not store", false);
  addParam("rawClusterName", m_RawClustersName, "The name of the StoreArray of PXD Raw Clusters to be processed", std::string(""));
  addParam("harwareClusterName", m_HardwareClusterName, "The name of the StoreArray of PXD Hardware Cluster to be processed",
           std::string(""));
}

void PXDHardwareClusterUnpackerModule::initialize()
{
//  StoreArray<PXDRawCluster>::required(m_RawClustersName);
  m_rawClusters.isRequired(m_RawClustersName);
  m_storeHardwareCluster.registerInDataStore(m_HardwareClusterName);

  B2INFO("HeaderEndianSwap: " << m_headerEndianSwap);

  ignore_datcon_flag = m_ignoreDATCON;

  m_unpackedEventsCount = 0;
}

void PXDHardwareClusterUnpackerModule::terminate()
{
  B2INFO("Number of PXD Raw Clusters unpacked to PXD Hardware Clusters : " << m_unpackedEventsCount);
}

void PXDHardwareClusterUnpackerModule::event()
{
//  StoreArray<PXDRawCluster> RawClusters;
  StoreObjPtr<EventMetaData> evtPtr;
//  int nRaws = RawClusters.getEntries();
  int nRaws = m_rawClusters.getEntries();

  if (verbose) {
    B2INFO("PXD Hardware Cluster Unpacker Info  --> RawClusters Objects in event: " << nRaws);
  };

  ftsw_evt_nr = 0;
  ftsw_evt_mask = 0;


  m_meta_event_nr = evtPtr->getEvent();
  m_meta_run_nr = evtPtr->getRun();
  m_meta_subrun_nr = evtPtr->getSubrun();
  m_meta_experiment = evtPtr->getExperiment();


  int nsr = 0;
  for (auto& it : m_rawClusters) {
    if (verbose) {
      B2INFO("PXD Hardware Cluster Unpacker Module --> Unpack Objects");
    };
    unpack_event(it);
    nsr++;
  }

  m_unpackedEventsCount++;
}

void PXDHardwareClusterUnpackerModule::unpack_event(const PXDRawCluster& cl)
{
  int fullsize;
  VxdID vxdID;

  if (cl.getLength() <= 0 || cl.getLength() > 16 * 1024 * 1024) {
    B2ERROR("PXD Hardware Cluster Unpacker Module --> invalid packet size (32bit words) " << hex << cl.getLength());
    return;
  }

  std::vector<unsigned short> data;

  fullsize = cl.getLength();
  B2INFO("size of data array " << fullsize);

  vxdID = cl.getVxdID();

  for (int i = 0 ; i < fullsize ; i++) {
    data.push_back(i);
    data[i] = cl.getData(i);
  }

  if ((data[fullsize - 1] & 0xFFFF) == 0) {
    fullsize = fullsize - 1;
  }

  unpack_fce(&data[0], fullsize, vxdID);
  B2INFO("unpack Cluster frame with size " << fullsize);
}

unsigned int PXDHardwareClusterUnpackerModule::calc_cluster_charge(const unsigned short* data, unsigned int nr_pixel_words)
{
  unsigned char charge;//[nr_pixel_words];
  unsigned int cluster_charge = 0;
  for (unsigned int i = 1 ; i < nr_pixel_words ; i++) {
    if ((data[i] & 0x8000) != 0) {
      charge = (data[i] & 0xFF);
      cluster_charge = cluster_charge + charge;
    }
  }
  return cluster_charge;
}

PXDHardwareClusterUnpackerModule::seed_pixel PXDHardwareClusterUnpackerModule::find_seed_pixel(const unsigned short* data,
    unsigned int nr_pixel_words, unsigned int dhp_id)
{
  seed_pixel addr;
  unsigned short row_addr = 0, col = 0, index = 0;
  unsigned short row = (data[0] & 0x3FF);
  unsigned char seed_charge = (data[1] & 0xFF) >> 8;
  for (unsigned int i = 0 ; i < nr_pixel_words ; i++) {
    if ((data[i] & 0x8000) == 0) {
      row = (data[i] & 0x3FF);
    }
    if ((data[i] & 0x8000) != 0) {
      if (((data[i] & 0x4000) >> 14) == 1) {
        row++;
      }
      if ((data[i] & 0xFF) > seed_charge) {
        seed_charge = (data[i] & 0xFF);
        row_addr = row;
        index = i;
      }
    }
  }
  if (dhp_id == 0) {
    col = (data[index] & 0x3F00) >> 8;
  }
  if (dhp_id == 1) { //if cluster continues on the second dhp
    col = (data[index] & 0x3F00) >> 8;
    col = col + 64;
  }
  if (dhp_id == 2) { //if cluster continues on the third dhp
    col = (data[index] & 0x3F00) >> 8;
    col = col + 64 + 64;
  }
  if (dhp_id == 3) { //if cluster continues on the fourth dhp
    col = (data[index] & 0x3F00) >> 8;
    col = col + 64 + 64 + 64;
  }
  addr.col = col;   //column of seed pixel
  addr.row = row_addr;    //row of seed pixel
  addr.charge = seed_charge; //charge of seed pixel
  return addr;
}

void PXDHardwareClusterUnpackerModule::unpack_fce(const void* data, unsigned int nr_words,
                                                  VxdID vxdID)  //frame_len in bytes (excl. CRC )!!!
{
  const unsigned short* dhe_fce = (unsigned short*)data;
  int nrPixel = 0;
  unsigned int increment_row_flag = 0;
  unsigned short rows[nr_words];
  unsigned short cols[nr_words];
  unsigned short row_address = 0, column_address = 0;
  unsigned char adcs[nr_words];
  unsigned char adc_value = 0;
  unsigned int all;
  unsigned int dhp_id = 0;
  unsigned int index[nr_words];
  seed_pixel addr;
  bool start_of_row = false;

  for (unsigned int i = 1; i < nr_words ; i++) {
    rows[i] = 0;
    cols[i] = 0;
    adcs[i] = 0;
    index[i] = 0;
  }


  for (unsigned int i = 0; i < nr_words; i++) {
    if ((dhe_fce[i] & 0x8000) == 0 && i != nr_words - 1) {
      if (i > 0 && start_of_row == true) { B2ERROR("Start of row words with now pixelswords in between"); }
      if (i == 0 && (((dhe_fce[i] & 0x4000) >> 14) == 0)) { B2ERROR("Cluster has no start of cluster flag"); }
      //Start Of Row
      row_address = (dhe_fce[i] & 0x3FF);
      dhp_id = (dhe_fce[i] & 0xC00) >> 10;
      index[i] = i;
      if (i == 0) {start_of_row = true;}
    } else {
      if (i == 0) { B2ERROR("First word is no Start of row word"); }
      start_of_row = false;
      //Pixel word
      nrPixel++;
      increment_row_flag = (dhe_fce[i] & 0x4000) >> 14;
      if (increment_row_flag ==  1) {
        row_address++;
      }
      column_address = (dhe_fce[i] & 0x3F00) >> 8;
      if (dhp_id == 1) { //if cluster continues on the second dhp
        column_address = column_address + 64;
      }
      if (dhp_id == 2) { //if cluster continues on the third dhp
        column_address = column_address + 64 + 64;
      }
      if (dhp_id == 3) { //if cluster continues on the fourth dhp
        column_address = column_address + 64 + 64 + 64;
      }
      adc_value = (dhe_fce[i] & 0xFF);
      adcs[i] = adc_value;
      cols[i] = column_address;
      rows[i] = row_address;
    }
  }
  //checks that the arrays adcs, cols, rows do not have entry 0 which would
  //correspond to a Start of Row word from the previous loop
  for (unsigned int j = 1 ; j < nr_words ; j++) {
    if (index[j] != 0 /*&& j!=0*/) {
      for (unsigned int l = j ; l < nr_words; l++) {
        cols[l] = cols[l + 1];
        rows[l] = rows[l + 1];
        adcs[l] = adcs[l + 1];
      }
    }
  }

  unsigned short r[nrPixel];
  unsigned short c[nrPixel];
  unsigned char q[nrPixel];

  //shifts the arrays rows cols adcs so that they dont have entrys with value 0 at the end
  //which can derive from more than one Start of Row word per cluster
  for (int k = 1; k <= nrPixel ; k++) {
    r[k - 1] = rows[k];
    c[k - 1] = cols[k];
    q[k - 1] = adcs[k];
  }

  all = calc_cluster_charge(dhe_fce, nr_words);
  addr = find_seed_pixel(dhe_fce, nr_words, dhp_id);

  if (!m_doNotStore) m_storeHardwareCluster.appendNew(&r[0], &c[0], &q[0], nrPixel, addr.charge, addr.row, addr.col, all, vxdID,
                                                        dhp_id);

}





