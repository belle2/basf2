/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/unpacking/PXDRawDataDefinitions.h>
#include <pxd/unpacking/PXDRawDataStructs.h>
#include <pxd/unpacking/PXDMappingLookup.h>
#include <pxd/modules/pxdUnpacking/PXDUnpackerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>

#include <boost/foreach.hpp>
#include <boost/cstdint.hpp>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::PXD::PXDError;

using namespace boost::spirit::endian;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

///******************************************************************
///*********************** Main unpacker code ***********************
///******************************************************************

PXDUnpackerModule::PXDUnpackerModule() :
  Module(),
  m_storeRawHits(),
  m_storeROIs(),
  m_storeRawAdc(),
  m_storeRawCluster()
{
  //Set module properties
  setDescription("Unpack Raw PXD Hits from ONSEN data stream");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("RawPXDsName", m_RawPXDsName, "The name of the StoreArray of RawPXDs to be processed", std::string(""));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "The name of the StoreArray of generated PXDRawHits", std::string(""));
  addParam("PXDDAQEvtStatsName", m_PXDDAQEvtStatsName, "The name of the StoreObjPtr of generated PXDDAQEvtStats", std::string(""));
  addParam("PXDRawAdcsName", m_PXDRawAdcsName, "The name of the StoreArray of generated PXDRawAdcs", std::string(""));
  addParam("PXDRawROIsName", m_PXDRawROIsName, "The name of the StoreArray of generated PXDRawROIs", std::string(""));
  addParam("HeaderEndianSwap", m_headerEndianSwap, "Swap the endianess of the ONSEN header", true);
  addParam("IgnoreDATCON", m_ignoreDATCON, "Ignore missing DATCON ROIs", true);
  addParam("IgnoreMetaFlags", m_ignoreMetaFlags, "Ignore wrong Meta event flags", true);
  addParam("DoNotStore", m_doNotStore, "only unpack and check, but do not store", false);
  addParam("ClusterName", m_RawClusterName, "The name of the StoreArray of PXD Clusters to be processed", std::string(""));
  addParam("CriticalErrorMask", m_criticalErrorMask, "Set error mask which stops processing by returning false by task", (uint64_t)0);
  addParam("ForceMapping", m_forceMapping, "Force Mapping even if DHH bit is NOT requesting it", false);
  addParam("ForceNoMapping", m_forceNoMapping, "Force NO Mapping even if DHH bit is requesting it", false);
  addParam("CheckPaddingCRC", m_checkPaddingCRC, "Check for susp. padding (debug option, many false positive)", false);
  addParam("IgnoreDHPMask", m_ignoreDHPMask, "Ignore active DHP mask in DHE (Ghost frame issue)", true);
  addParam("IgnoreDHELength", m_ignoreDHELength, "Ignore Length in DHE (GHOST frame issue)", true);
  addParam("MaxDHPFrameDiff", m_maxDHPFrameDiff, "Maximum DHP Frame Nr Difference w/o reporting error", 2u);
  addParam("FormatBonnDAQ", m_formatBonnDAQ, "ONSEN or BonnDAQ format", false);
//   (
//              /*EPXDErrFlag::c_DHC_END | EPXDErrFlag::c_DHE_START | EPXDErrFlag::c_DATA_OUTSIDE |*/
//              EPXDErrFlag::c_FIX_SIZE | EPXDErrFlag::c_DHE_CRC | EPXDErrFlag::c_DHC_UNKNOWN | /*EPXDErrFlag::c_MERGER_CRC |*/
//              EPXDErrFlag::c_DHP_SIZE | /*EPXDErrFlag::c_DHP_PIX_WO_ROW | EPXDErrFlag::c_DHE_START_END_ID | EPXDErrFlag::c_DHE_START_ID |*/
//              EPXDErrFlag::c_DHE_START_WO_END | EPXDErrFlag::c_DHP_NOT_CONT
//            ));
}

void PXDUnpackerModule::initialize()
{
  // Required input
  m_eventMetaData.isRequired();
  // Optional input
  m_storeRawPXD.isOptional(m_RawPXDsName);

  //Register output collections
  m_storeRawHits.registerInDataStore(m_PXDRawHitsName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
  m_storeRawAdc.registerInDataStore(m_PXDRawAdcsName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
  m_storeROIs.registerInDataStore(m_PXDRawROIsName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
  m_storeDAQEvtStats.registerInDataStore(m_PXDDAQEvtStatsName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
  m_storeRawCluster.registerInDataStore(m_RawClusterName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
  /// actually, later we do not want to store ROIs and raw ADC into output file ...  aside from debugging

  B2DEBUG(1, "HeaderEndianSwap: " << m_headerEndianSwap);
  B2DEBUG(1, "Ignore (missing) DATCON: " << m_ignoreDATCON);
  B2DEBUG(1, "Ignore (some) missing Meta flags: " << m_ignoreMetaFlags);
  B2DEBUG(1, "ForceMapping: " << m_forceMapping);
  B2DEBUG(1, "ForceNoMapping: " << m_forceNoMapping);
  B2DEBUG(1, "CheckPaddingCRC: " << m_checkPaddingCRC);
  B2DEBUG(1, "MaxDHPFrameDiff: " << m_maxDHPFrameDiff);

  m_sendunfiltered = 0;
  m_sendrois = 0;
  m_notaccepted = 0;
  m_unpackedEventsCount = 0;
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) m_errorCounter[i] = 0;

}

void PXDUnpackerModule::terminate()
{
  int flag = 0;
  string errstr = "Statistic ( ;";
  errstr += to_string(m_unpackedEventsCount) + ";";
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) { errstr += to_string(m_errorCounter[i]) + ";"; flag |= m_errorCounter[i];}
  if (flag != 0) {
    B2RESULT("PXD Unpacker --> Error Statistics (counted once per event!) in Events: " << m_unpackedEventsCount);
    B2RESULT(errstr + " )");
    for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      if (m_errorCounter[i]) {
        B2RESULT(getPXDBitErrorName(i) << ": " << m_errorCounter[i]);
      }
    }
  } else {
    B2RESULT("PXD Unpacker --> No Error found in Events: " << m_unpackedEventsCount);
  }
  B2RESULT("Statistic 2: !Accepted: " << m_notaccepted << " SendROIs: " << m_sendrois << " Unfiltered: " << m_sendunfiltered);
}

void PXDUnpackerModule::event()
{
  m_storeDAQEvtStats.create(EPXDErrMask::c_NO_ERROR);

  m_errorMask = 0;
  m_errorMaskEvent = 0;

  // if no input, nothing to do
  if (!m_storeRawPXD) {
    m_errorMask |= EPXDErrMask::c_NO_PXD;
  } else {
    int nRaws = m_storeRawPXD.getEntries();
    if (verbose) {
      B2DEBUG(20, "PXD Unpacker --> RawPXD Objects in event: " << nRaws);
    };

    m_meta_event_nr = m_eventMetaData->getEvent();
    m_meta_run_nr = m_eventMetaData->getRun();
    m_meta_subrun_nr = m_eventMetaData->getSubrun();
    m_meta_experiment = m_eventMetaData->getExperiment();
    m_meta_time = m_eventMetaData->getTime();

    int inx = 0; // count index for output objects
    for (auto& it : m_storeRawPXD) {
      if (verbose) {
        B2DEBUG(20, "PXD Unpacker --> Unpack Objects: ");
      };
      unpack_rawpxd(it, inx++);
    }

    if (nRaws == 0) m_errorMask |= EPXDErrMask::c_NO_PXD;
  }
  m_errorMaskEvent |= m_errorMask;
  m_storeDAQEvtStats->setErrorMask(m_errorMaskEvent);

  m_unpackedEventsCount++;
  {
    uint64_t j = 1;
    for (unsigned int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      if (m_errorMaskEvent & j) m_errorCounter[i]++;
      j <<= 1;
    }
  }

  setReturnValue(0 == (m_criticalErrorMask & m_errorMaskEvent));
}

void PXDUnpackerModule::unpack_rawpxd(RawPXD& px, int inx)
{
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  m_errorMaskDHE = 0;
  m_errorMaskDHC = 0;
  m_errorMaskPacket = 0;
  PXDDAQPacketStatus& daqpktstat = m_storeDAQEvtStats->newPacket(inx);

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD Unpacker --> invalid packet size (32bit words) $" << hex << px.size());
    m_errorMask |= EPXDErrMask::c_PACKET_SIZE;
    return;
  }
  std::vector<unsigned int> data(px.size());
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  std::copy_n(px.data(), px.size(), data.begin());

  if (fullsize < 8) {
    B2ERROR("Data is to small to hold a valid Header! Will not unpack anything. Size:" << fullsize);
    m_errorMask |= EPXDErrMask::c_PACKET_SIZE;
    return;
  }

  if (data[0] != 0xCAFEBABE && data[0] != 0xBEBAFECA) {
    B2ERROR("Magic invalid: Will not unpack anything. Header corrupted! $" << hex << data[0]);
    m_errorMask |= EPXDErrMask::c_MAGIC;
    return;
  }

  if (m_headerEndianSwap) Frames_in_event = ((ubig32_t*)data.data())[1];
  else Frames_in_event = ((ulittle32_t*)data.data())[1];
  if (Frames_in_event < 0 || Frames_in_event > 256) {
    B2ERROR("Number of Frames invalid: Will not unpack anything. Header corrupted! Frames in event: " << Frames_in_event);
    m_errorMask |= EPXDErrMask::c_FRAME_NR;
    return;
  }
  if (Frames_in_event < 3) {
    B2ERROR("Number of Frames too small: It cannot contain anything useful. Frames in event: " << Frames_in_event);
    m_errorMask |= c_NR_FRAMES_TO_SMALL;
  }

  /// NEW format
  if (verbose) {
    B2DEBUG(20, "PXD Unpacker --> data[0]: <-- Magic $" << hex << data[0]);
    B2DEBUG(20, "PXD Unpacker --> data[1]: <-- #Frames $" << hex << data[1]);
    if (data[1] >= 1 && fullsize < 12) B2DEBUG(20, "PXD Unpacker --> data[2]: <-- Frame 1 len $" << hex << data[2]);
    if (data[1] >= 2 && fullsize < 16) B2DEBUG(20, "PXD Unpacker --> data[3]: <-- Frame 2 len $" << hex << data[3]);
    if (data[1] >= 3 && fullsize < 20) B2DEBUG(20, "PXD Unpacker --> data[4]: <-- Frame 3 len $" << hex << data[4]);
    if (data[1] >= 4 && fullsize < 24) B2DEBUG(20, "PXD Unpacker --> data[5]: <-- Frame 4 len $" << hex << data[5]);
  };

  unsigned int* tableptr;
  tableptr = &data[2]; // skip header!!!

  unsigned int* dataptr;
  dataptr = &tableptr[Frames_in_event];
  datafullsize = fullsize - 2 * 4 - Frames_in_event * 4; // minus header, minus table

  int ll = 0; // Offset in dataptr in bytes
  for (int j = 0; j < Frames_in_event; j++) {
    int lo;/// len of frame in bytes
    if (m_headerEndianSwap) lo = ((ubig32_t*)tableptr)[j];
    else lo = ((ulittle32_t*)tableptr)[j];
    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      m_errorMask |= EPXDErrMask::c_FRAME_SIZE;
      return;
    }
    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " <<
              datafullsize << " of fullsize " << fullsize);
      m_errorMask |= EPXDErrMask::c_FRAME_SIZE;
      return;
    }
    if (lo & 0x3) {
      B2ERROR("SKIP Frame with Data with not MOD 4 length " << " ( $" << hex << lo << " ) ");
      ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
      m_errorMask |= EPXDErrMask::c_FRAME_SIZE;
    } else {
      B2DEBUG(20, "unpack DHE(C) frame: " << j << " with size " << lo << " at byte offset in dataptr " << ll);
      unpack_dhc_frame(ll + (char*)dataptr, lo, j, Frames_in_event, daqpktstat);
      ll += lo; /// no rounding needed
    }
    m_errorMaskDHE |= m_errorMask;
    m_errorMaskDHC |= m_errorMask;
    m_errorMaskPacket |= m_errorMask;
    m_errorMaskEvent |= m_errorMask;
    m_errorMask = 0;
  }
  daqpktstat.setErrorMask(m_errorMaskPacket);
}

void PXDUnpackerModule::unpack_dhp_raw(void* data, unsigned int frame_len, unsigned int dhe_ID, unsigned dhe_DHPport,
                                       VxdID vxd_id)
{
//   unsigned int nr_words = frame_len / 2; // frame_len in bytes (excl. CRC)!!!
  ubig16_t* dhp_pix = (ubig16_t*)data;

  //! *************************************************************
  //! Important Remark:
  //! Up to now the format for Raw frames as well as size etc
  //! is not well defined. It will most likely change!
  //! E.g. not the whole mem is dumped, but only a part of it.
  //! *************************************************************

  // Size: 64*768 + 8 bytes for a full frame readout
  if (frame_len != 0xC008) {
    B2ERROR("Frame size unsupported for RAW ADC frame! $" << hex << frame_len << " bytes");
    return;
  }
  unsigned int dhp_header_type  = 0;
//   unsigned int dhp_reserved     = 0;
  unsigned int dhp_dhe_id       = 0;
  unsigned int dhp_dhp_id       = 0;

  dhp_header_type  = (dhp_pix[2] & 0xE000) >> 13;
//   dhp_reserved     = (dhp_pix[2] >> 8) & 0x1F;
  dhp_dhe_id       = (dhp_pix[2] & 0x00FC) >> 2;
  dhp_dhp_id       =  dhp_pix[2] & 0x0003;

  if (dhe_ID != dhp_dhe_id) {
    B2ERROR("DHE ID in DHE and DHP header differ $" << hex << dhe_ID << " != $" << dhp_dhe_id);
    m_errorMask |= EPXDErrMask::c_DHE_DHP_DHEID;
  }
  if (dhe_DHPport != dhp_dhp_id) {
    B2ERROR("DHP ID (Chip/Port) in DHE and DHP header differ $" << hex << dhe_DHPport << " != $" << dhp_dhp_id);
    m_errorMask |= EPXDErrMask::c_DHE_DHP_PORT;
  }

  if (dhp_header_type != EDHPFrameHeaderDataType::c_RAW) {
    B2ERROR("Header type invalid for this kind of DHE frame: $" << hex << dhp_header_type);
    return;
  }

  /// Endian Swapping is done in Contructors of Raw Objects!
  B2DEBUG(20, "Raw ADC Data");
  // size checked already above
  m_storeRawAdc.appendNew(vxd_id, data, frame_len);
};

void PXDUnpackerModule::unpack_fce(unsigned short* data, unsigned int length, VxdID vxd_id)
{
  //! *************************************************************
  //! Important Remark:
  //! Up to now the format for cluster is not well defined.
  //! We need to wait for the final hardware implementation.
  //! Then the following code uste be re-checked TODO
  //! *************************************************************

  B2ERROR("FCE (Cluster) Packet have not yet been tested with real HW clusters. Dont assume that this code is working!");
  return;

  ubig16_t* cluster = (ubig16_t*)data;
  int nr_words; //words in dhp frame
  unsigned int words_in_cluster = 0; //counts 16bit words in cluster
  nr_words = length / 2;
  ubig16_t sor;
  sor = 0x0000;

  for (int i = 2 ; i < nr_words ; i++) {
    if (i != 2) { //skip header
      if ((((cluster[i] & 0x8000) == 0)
           && ((cluster[i] & 0x4000) >> 14) == 1)) {  //searches for start of row frame with start of cluster flag = 1 => new cluster
        if (!m_doNotStore) m_storeRawCluster.appendNew(&data[i - words_in_cluster], words_in_cluster, vxd_id);
        words_in_cluster = 0;
      }
    }
    if ((cluster[i] & 0x8000) == 0) {
      sor = cluster[i];
    }
    words_in_cluster++;

    if ((cluster[nr_words - 1] & 0xFFFF) == (sor &
                                             0xFFFF)) {//if frame is not 32bit aligned last word will be the last start of row word
      cluster[nr_words - 1] = 0x0000;//overwrites the last redundant word with zero to make checking easier in PXDHardwareClusterUnpacker
    }

    if (i == nr_words - 1) {
      if (!m_doNotStore) m_storeRawCluster.appendNew(&data[i - words_in_cluster + 1], words_in_cluster, vxd_id);
    }
  }
}

void PXDUnpackerModule::dump_dhp(void* data, unsigned int frame_len)
{
  unsigned int w = frame_len / 2;
  ubig16_t* d = (ubig16_t*)data;

  B2ERROR("HEADER --  $" << hex << d[0] << ",$" << hex << d[1] << ",$" << hex << d[2] << ",$" << hex << d[3] << " -- ");

  auto dhp_header_type  = (d[2] & 0xE000) >> 13;
  auto dhp_reserved     = (d[2] & 0x1F00) >> 8;
  auto dhp_dhe_id       = (d[2] & 0x00FC) >> 2;
  auto dhp_dhp_id       =  d[2] & 0x0003;

  B2ERROR("DHP type     | $" << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
  B2ERROR("DHP reserved | $" << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
  B2ERROR("DHP DHE ID   | $" << hex << dhp_dhe_id << " ( " << dec << dhp_dhe_id << " ) ");
  B2ERROR("DHP DHP ID   | $" << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  for (unsigned int i = 4; i < w; i++) {
    B2ERROR("DHP DATA $" << hex << d[i]);
  }
  B2ERROR("DHP CRC $" << hex << d[w] << ",$" << hex << d[w + 1]);
}

void PXDUnpackerModule::dump_roi(void* data, unsigned int frame_len)
{
  unsigned int w = frame_len / 4;
  ubig32_t* d = (ubig32_t*)data;

  B2ERROR("HEADER --  $" << hex << d[0] << ",$" << hex << d[1] << ",$" << hex << d[2] << ",$" << hex << d[3] << " -- Len $" << hex <<
          frame_len);

  for (unsigned int i = 0; i < w; i++) {
    B2ERROR("ROI DATA $" << hex << d[i]);
  }
  B2ERROR("ROI CRC $" << hex << d[w]);
}

void PXDUnpackerModule::unpack_dhp(void* data, unsigned int frame_len, unsigned int dhe_first_readout_frame_id_lo,
                                   unsigned int dhe_ID, unsigned dhe_DHPport, unsigned dhe_reformat, VxdID vxd_id,
                                   PXDDAQPacketStatus& daqpktstat)
{
  unsigned int nr_words = frame_len / 2; // frame_len in bytes (excl. CRC)!!!
  bool printflag = false;
  ubig16_t* dhp_pix = (ubig16_t*)data;

  unsigned int dhp_readout_frame_lo = 0;
  unsigned int dhp_header_type  = 0;
  unsigned int dhp_reserved     = 0;
  unsigned int dhp_dhe_id       = 0;
  unsigned int dhp_dhp_id       = 0;

  unsigned int dhp_row = 0, dhp_col = 0, dhp_adc = 0, dhp_cm = 0;
//   unsigned int dhp_offset = 0;
  bool rowflag = false;
  bool pixelflag = true; // just for first row start

  if (nr_words < 4) {
    B2ERROR("DHP frame size error (too small) " << nr_words);
    m_errorMask |= EPXDErrMask::c_DHP_SIZE;
//     dhp_size_error++;
    return;
    //return -1;
  }

  if (printflag)
    B2DEBUG(20, "HEADER -- $" << hex << dhp_pix[0] << hex << dhp_pix[1] << hex << dhp_pix[2] << hex << dhp_pix[3] << " -- ");

  if (printflag)
    B2DEBUG(20, "DHP Header   | $" << hex << dhp_pix[2] << " ( " << dec << dhp_pix[2] << " ) ");
  dhp_header_type  = (dhp_pix[2] & 0xE000) >> 13;
  dhp_reserved     = (dhp_pix[2] & 0x1F00) >> 8;
  dhp_dhe_id       = (dhp_pix[2] & 0x00FC) >> 2;
  dhp_dhp_id       =  dhp_pix[2] & 0x0003;

  if (printflag) {
    B2DEBUG(20, "DHP type     | $" << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
    B2DEBUG(20, "DHP reserved | $" << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
    B2DEBUG(20, "DHP DHE ID   | $" << hex << dhp_dhe_id << " ( " << dec << dhp_dhe_id << " ) ");
    B2DEBUG(20, "DHP DHP ID   | $" << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  }

  if (dhe_ID != dhp_dhe_id) {
    B2ERROR("DHE ID in DHE and DHP header differ $" << hex << dhe_ID << " != $" << dhp_dhe_id);
    m_errorMask |= EPXDErrMask::c_DHE_DHP_DHEID;
  }
  if (dhe_DHPport != dhp_dhp_id) {
    B2ERROR("DHP ID (Chip/Port) in DHE and DHP header differ $" << hex << dhe_DHPport << " != $" << dhp_dhp_id);
    m_errorMask |= EPXDErrMask::c_DHE_DHP_PORT;
  }

  if (dhp_header_type != EDHPFrameHeaderDataType::c_ZSD) {
    B2ERROR("Header type invalid for this kind of DHE frame: $" << hex << dhp_header_type);
    return;
  }

//  static int offtab[4] = {0, 64, 128, 192};
//   dhp_offset = offtab[dhp_dhp_id];

  dhp_readout_frame_lo  = dhp_pix[3] & 0xFFFF;
  if (printflag)
    B2DEBUG(20, "DHP Frame Nr     |  $" << hex << dhp_readout_frame_lo << " ( " << dec << dhp_readout_frame_lo << " ) ");

  /* // TODO removed because data format error is not to be fixed soon
  if (((dhp_readout_frame_lo - dhe_first_readout_frame_id_lo) & 0x3F) > m_maxDHPFrameDiff) {
    B2ERROR("DHP Frame Nr differ from DHE Frame Nr by >1 DHE " << dhe_first_readout_frame_id_lo << " != DHP " << (dhp_readout_frame_lo & 0x3F) << " delta "<< ((dhp_readout_frame_lo - dhe_first_readout_frame_id_lo) & 0x3F) );
    m_errorMask |= EPXDErrMask::c_DHP_DHE_FRAME_DIFFER;
  }
  */
  /* // TODO removed because data format error is not to be fixed soon
    if (last_dhp_readout_frame_lo[dhp_dhp_id] != -1) {
    if (((dhp_readout_frame_lo - last_dhp_readout_frame_lo[dhp_dhp_id]) & 0xFFFF) > m_maxDHPFrameDiff) {
      B2ERROR("Two DHP Frames per sensor which frame number differ more than one! " << last_dhp_readout_frame_lo[dhp_dhp_id] << ", " <<
              dhp_readout_frame_lo);
      m_errorMask |= EPXDErrMask::c_DHP_NOT_CONT;
    }
  }
  */

  if (daqpktstat.dhc_size() > 0) {
    if (daqpktstat.dhc_back().dhe_size() > 0) {
      // only is we have a DHC and DHE object... or back() is undefined
      // Remark, if we have a broken data (DHE_START/END) structure, we might fill the
      // previous DHE object ... but then the data is junk anyway
      daqpktstat.dhc_back().dhe_back().newDHP(dhp_dhp_id, dhp_readout_frame_lo);
    }
  }

  /* // TODO removed because the data is not ordered as expected in current firmware
  for (auto j = 0; j < 4; j++) {
    if (last_dhp_readout_frame_lo[j] != -1) {
      if (((dhp_readout_frame_lo - last_dhp_readout_frame_lo[j]) & 0xFFFF) > m_maxDHPFrameDiff) {
        B2ERROR("Two DHP Frames (different DHP) per sensor which frame number differ more than one! " << last_dhp_readout_frame_lo[j] <<
                ", " <<
                dhp_readout_frame_lo);
        m_errorMask |= EPXDErrMask::c_DHP_DHP_FRAME_DIFFER;
        break;// give msg only once
      }
    }
  }
  */
  last_dhp_readout_frame_lo[dhp_dhp_id] = dhp_readout_frame_lo;

// TODO Please check if this can happen by accident with valid data!
  if (dhp_pix[2] == dhp_pix[4] && dhp_pix[3] + 1 == dhp_pix[5]) {
    // We see a second "header" with framenr+1 ...
    B2ERROR("DHP data: seems to be double header! skipping ... len " << frame_len);
    m_errorMask |= EPXDErrMask::c_DHP_DBL_HEADER;
    // dump_dhp(data, frame_len); print out guilty dhp packet
//    B2ERROR("Mask $" << hex <<m_errorMask);
    return;
  }

  // Start with offset 4, thus skipping header words
  for (unsigned int i = 4; i < nr_words ; i++) {

    if (printflag)
      B2DEBUG(20, "-- $" << hex << dhp_pix[i] << " --   " << dec << i);
    {
      if (((dhp_pix[i] & 0x8000) >> 15) == 0) {
        rowflag = true;
        if (!pixelflag) {
          m_errorMask |= EPXDErrMask::c_DHP_ROW_WO_PIX;
          B2WARNING("DHP Unpacking: Row w/o Pix");
        }
        pixelflag = false;
        dhp_row = (dhp_pix[i] & 0xFFC0) >> 5;
        dhp_cm  = dhp_pix[i] & 0x3F;
        if (daqpktstat.dhc_size() > 0) {
          if (daqpktstat.dhc_back().dhe_size() > 0) {
            PXDDAQDHPComMode cm(dhp_dhp_id, dhp_row, dhp_cm);
            // only is we have a DHC and DHE object... or back() is undefined
            // Remark, if we have a broken data (DHE_START/END) structure, we might fill the
            // previous DHE object ... but then the data is junk anyway
            daqpktstat.dhc_back().dhe_back().addCM(cm);
          }
        }
        if (printflag)
          B2DEBUG(20, "SetRow: $" << hex << dhp_row << " CM $" << hex << dhp_cm);
      } else {
        if (!rowflag) {
          B2ERROR("DHP Unpacking: Pix without Row!!! skip dhp data ");
          m_errorMask |= EPXDErrMask::c_DHP_PIX_WO_ROW;
          // dump_dhp(data, frame_len);// print out faulty dhp frame
          return;
        } else {
          pixelflag = true;
          dhp_row = (dhp_row & 0xFFE) | ((dhp_pix[i] & 0x4000) >> 14);
          dhp_col = ((dhp_pix[i]  & 0x3F00) >> 8);
          unsigned int v_cellID, u_cellID;
          v_cellID = dhp_row;// defaults for no mapping
          if (dhp_row >= 768) {
            B2ERROR("DHP ROW Overflow " << dhp_row);
            m_errorMask |= EPXDErrMask::c_ROW_OVERFLOW;
          }
          // we cannot do col overflow check before mapping :-(

          if ((dhe_reformat == 0 && !m_forceNoMapping) || m_forceMapping) {
            u_cellID = dhp_col;// defaults for no mapping
            // data has not been pre-processed by DHH, thus we have to do the mapping ourselves
            if ((dhe_ID & 0x21) == 0x00 || (dhe_ID & 0x21) == 0x21) {
              // if IFOB
              PXDMappingLookup::map_rc_to_uv_IF_OB(v_cellID, u_cellID, dhp_dhp_id, dhe_ID);
            } else { // else OFIB
              PXDMappingLookup::map_rc_to_uv_IB_OF(v_cellID, u_cellID, dhp_dhp_id, dhe_ID);
            }
          } else {
            u_cellID = dhp_col + 64 * dhp_dhp_id; // defaults for already mapped
          }
          if (u_cellID >= 250) {
            B2WARNING("DHP COL Overflow (unconnected drain lines) " << u_cellID << ", ref " << dhe_reformat << ", dhpcol " << dhp_col << ", id "
                      << dhp_dhp_id);
            m_errorMask |= EPXDErrMask::c_COL_OVERFLOW;
          }
          dhp_adc = dhp_pix[i] & 0xFF;
          if (printflag)
            B2DEBUG(20, "SetPix: Row $" << hex << dhp_row << " Col $" << hex << dhp_col << " ADC $" << hex << dhp_adc
                    << " CM $" << hex << dhp_cm);

          /*if (verbose) {
            B2DEBUG(20, "raw    |   " << hex << d[i]);
            B2DEBUG(20, "row " << hex << ((d[i] >> 20) & 0xFFF) << "(" << ((d[i] >> 20) & 0xFFF) << ")" << " col " << "(" << hex << ((d[i] >> 8) & 0xFFF) << ((d[i] >> 8) & 0xFFF)
                   << " adc " << "(" << hex << (d[i] & 0xFF) << (d[i] & 0xFF) << ")");
            B2DEBUG(20, "dhe_ID " << dhe_ID);
            B2DEBUG(20, "start-Frame-Nr " << dec << dhe_first_readout_frame_id_lo);
          };*/

          if (!m_doNotStore) m_storeRawHits.appendNew(vxd_id, v_cellID, u_cellID, dhp_adc,
                                                        (dhp_readout_frame_lo - dhe_first_readout_frame_id_lo) & 0x3F);
        }
      }
    }
  }

  if (printflag) {
    B2DEBUG(20, "(DHE) DHE_ID $" << hex << dhe_ID << " (DHE) DHP ID $" << hex << dhe_DHPport << " (DHP) DHE_ID $" << hex << dhp_dhe_id
            <<
            " (DHP) DHP ID $" << hex << dhp_dhp_id);
    /*for (int i = 0; i < raw_nr_words ; i++) {
      B2DEBUG(20, "RAW      |   " << hex << p_pix[i]);
      printf("raw %08X  |  ", p_pix[i]);
      B2DEBUG(20, "row " << hex << ((p_pix[i] >> 20) & 0xFFF) << dec << " ( " << ((p_pix[i] >> 20) & 0xFFF) << " ) " << " col " << hex << ((p_pix[i] >> 8) & 0xFFF)
             << " ( " << dec << ((p_pix[i] >> 8) & 0xFFF) << " ) " << " adc " << hex << (p_pix[i] & 0xFF) << " ( " << (p_pix[i] & 0xFF) << " ) "
            );
    }*/
  }
};

int PXDUnpackerModule::nr5bits(int i)
{
  /// too lazy to count the bits myself, thus using a small lookup table
  const int lut[32] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5
  };
  return lut[i & 0x1F];
}

void PXDUnpackerModule::unpack_dhc_frame(void* data, const int len, const int Frame_Number, const int Frames_in_event,
                                         PXDDAQPacketStatus& daqpktstat)
{
  /// The following STATIC variables are used to save some state or count some things
  /// while depacking the frames. they are in most cases (re)set on the first frame or ONSEN trg frame
  /// Most could put in as a class member, but they are only needed within this function
  static unsigned int eventNrOfOnsenTrgFrame = 0;
  static int countedBytesInDHC = 0;
  static bool cancheck_countedBytesInDHC = false;
  static int countedBytesInDHE = 0;
  static bool cancheck_countedBytesInDHE = false;
  static int countedDHEStartFrames = 0;
  static int countedDHEEndFrames = 0;
  static int mask_active_dhe = 0;// DHE mask (5 bit)
  static int nr_active_dhe =
    0;// TODO just count the active DHEs. Until now, it is not possible to check for the bit mask. we would need the info on which DHE connects to which DHC at which port from gearbox/geometry?
  static int mask_active_dhp = 0;// DHP active mask, 4 bit, per current DHE
  static int found_mask_active_dhp = 0;// mask which DHP send data and check on DHE END frame if it matches
  static unsigned int dhe_first_readout_frame_id_lo = 0;
  static unsigned int dhe_first_triggergate = 0;
  static unsigned int currentDHCID = 0xFFFFFFFF;
  static unsigned int currentDHEID = 0xFFFFFFFF;
  static unsigned int currentVxdId = 0;
  static bool isFakedData_event = false;
  static bool isUnfiltered_event = false;


  dhc_frame_header_word0* hw = (dhc_frame_header_word0*)data;

  dhc_frames dhc;
  dhc.set(data, hw->getFrameType(), len);
  int s;
  s = dhc.getFixedSize();
  if (len != s && s != 0) {
    B2ERROR("Fixed frame type size does not match specs: expect " << len << " != " << s << " (in data) ");
    m_errorMask |= EPXDErrMask::c_FIX_SIZE;
  }

  unsigned int eventNrOfThisFrame = dhc.getEventNrLo();
  int type = dhc.getFrameType();

  if (Frame_Number == 0) { /// We reset the counters on the first event
    eventNrOfOnsenTrgFrame = 0;
    countedDHEStartFrames = 0;
    countedDHEEndFrames = 0;
    countedBytesInDHC = 0;
    cancheck_countedBytesInDHC = false;
    countedBytesInDHE = 0;
    cancheck_countedBytesInDHE = false;
    currentDHCID = 0xFFFFFFFF;
    currentDHEID = 0xFFFFFFFF;
    currentVxdId = 0;
    isUnfiltered_event = false;
    isFakedData_event = false;
    mask_active_dhe = 0;
    nr_active_dhe = 0;
    mask_active_dhp = 0;
    found_mask_active_dhp = 0;
    if (m_formatBonnDAQ) {
      if (type != EDHCFrameHeaderDataType::c_DHC_START) {
        B2ERROR("This looks not like BonnDAQ format.");
      }
    } else {
      if (type == EDHCFrameHeaderDataType::c_DHC_START) {
        B2ERROR("This looks like BonnDAQ or old Desy 2013/14 testbeam format. Please use formatBonnDAQ or the pxdUnpackerDesy1314 module.");
      }
    }
  }

  if (!m_formatBonnDAQ) {
    if (Frame_Number == 1) {
      if (type == EDHCFrameHeaderDataType::c_DHC_START) {
        isFakedData_event = dhc.data_dhc_start_frame->isFakedData();
      }
    }

    // please check if this mask is suitable. At least here we are limited by the 16 bit trigger number in the DHH packet header.
    // we can use more bits in the DHC and DHE START Frame
    if ((eventNrOfThisFrame & 0xFFFF) != (m_meta_event_nr & 0xFFFF)) {
      if (!isFakedData_event) {
        B2ERROR("Event Numbers do not match for this frame $" << hex << eventNrOfThisFrame << "!=$" << m_meta_event_nr <<
                "(MetaInfo) mask");
        m_errorMask |= EPXDErrMask::c_META_MM;
      }
    }

    if (Frame_Number > 1 && Frame_Number < Frames_in_event - 1) {
      if (countedDHEStartFrames != countedDHEEndFrames + 1)
        if (type != EDHCFrameHeaderDataType::c_ONSEN_ROI && type != EDHCFrameHeaderDataType::c_DHE_START) {
          B2ERROR("Data Frame outside a DHE START/END");
          m_errorMask |= EPXDErrMask::c_DATA_OUTSIDE;
        }
    }
  }

  if (hw->getErrorFlag()) {
    if (type != EDHCFrameHeaderDataType::c_GHOST) {
      m_errorMask |= EPXDErrMask::c_HEADER_ERR;
    }
  } else {
    if (type == EDHCFrameHeaderDataType::c_GHOST) {
      m_errorMask |= EPXDErrMask::c_HEADER_ERR_GHOST;
    }
  }
  switch (type) {
    case EDHCFrameHeaderDataType::c_DHP_RAW: {

      if (verbose) dhc.data_direct_readout_frame_raw->print();
      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_direct_readout_frame_raw->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();

      unpack_dhp_raw(data, len - 4,
                     dhc.data_direct_readout_frame->getDHEId(),
                     dhc.data_direct_readout_frame->getDHPPort(),
                     currentVxdId);

      break;
    };
    case EDHCFrameHeaderDataType::c_ONSEN_DHP:
      // Set the counted size invalid if negativ, needs a large negative value because we are adding up to that
      cancheck_countedBytesInDHC = false;
      cancheck_countedBytesInDHE = false;
      [[fallthrough]];
    case EDHCFrameHeaderDataType::c_DHP_ZSD: {

      if (verbose) dhc.data_direct_readout_frame->print();
      if (isUnfiltered_event) {
        if (type == EDHCFrameHeaderDataType::c_ONSEN_DHP) m_errorMask |= EPXDErrMask::c_SENDALL_TYPE;
      } else {
        if (type == EDHCFrameHeaderDataType::c_DHP_ZSD) m_errorMask |= EPXDErrMask::c_NOTSENDALL_TYPE;
      }

      //m_errorMask |= dhc.data_direct_readout_frame->check_error();

      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_direct_readout_frame_raw->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();
      if (m_checkPaddingCRC) m_errorMask |= dhc.check_padding(); // isUnfiltered_event


      unpack_dhp(data, len - 4,
                 dhe_first_readout_frame_id_lo,
                 dhc.data_direct_readout_frame->getDHEId(),
                 dhc.data_direct_readout_frame->getDHPPort(),
                 dhc.data_direct_readout_frame->getDataReformattedFlag(),
                 currentVxdId, daqpktstat);

      break;
    };
    case EDHCFrameHeaderDataType::c_ONSEN_FCE:
      // Set the counted size invalid if negativ, needs a large negative value because we are adding up to that
      cancheck_countedBytesInDHC = false;
      cancheck_countedBytesInDHE = false;
      [[fallthrough]];
    case EDHCFrameHeaderDataType::c_FCE_RAW: {

      m_errorMask |= EPXDErrMask::c_UNEXPECTED_FRAME_TYPE;
      B2ERROR("Unexpected Frame Type (Clustering FCE)");
      if (verbose) hw->print();
      if (isUnfiltered_event) {
        if (type == EDHCFrameHeaderDataType::c_ONSEN_FCE) m_errorMask |= EPXDErrMask::c_SENDALL_TYPE;
      } else {
        if (type == EDHCFrameHeaderDataType::c_FCE_RAW) m_errorMask |= EPXDErrMask::c_NOTSENDALL_TYPE;
      }

      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_direct_readout_frame_raw->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();

      B2DEBUG(20, "UNPACK FCE FRAME with len $" << hex << len);
      unpack_fce((unsigned short*) data, len - 4, currentVxdId);

      break;
    };
    case EDHCFrameHeaderDataType::c_COMMODE: {

      m_errorMask |= EPXDErrMask::c_UNEXPECTED_FRAME_TYPE;
      B2ERROR("Unexpected Frame Type (COMMODE)");

      if (verbose) hw->print();
      if (currentDHEID != dhc.data_commode_frame->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_commode_frame->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      break;
    };
    case EDHCFrameHeaderDataType::c_DHC_START: {
      countedBytesInDHC = 0;
      cancheck_countedBytesInDHC = true;
      if (isFakedData_event != dhc.data_dhc_start_frame->isFakedData()) {
        B2ERROR("DHC START is but no Fake event OR Fake Event but DHE END is not.");
      }
      if (dhc.data_dhc_start_frame->isFakedData()) {
        B2WARNING("Faked DHC START Data -> trigger without Data!");
        m_errorMask |= EPXDErrMask::c_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhc.data_dhc_start_frame->print();
      }

//      eventNrOfOnsenTrgFrame = eventNrOfThisFrame;
      currentDHEID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      currentDHCID = dhc.data_dhc_start_frame->get_dhc_id();
      m_errorMask |= dhc.check_crc();

      if (m_formatBonnDAQ)             eventNrOfOnsenTrgFrame = eventNrOfThisFrame;

      if (!isFakedData_event) {
        /// TODO here we should check full(!) Event Number, Run Number, Subrun Nr and Exp Number
        /// of this frame against the one from MEta Event Info
        if (dhc.data_dhc_start_frame->get_experiment() != m_meta_experiment) {
          B2ERROR("DHC EXP MM: " <<
                  dhc.data_dhc_start_frame->get_experiment() << " META " << m_meta_experiment);
          m_errorMask |= EPXDErrMask::c_META_MM_DHC_ERS;
        }
        if (dhc.data_dhc_start_frame->get_run() != m_meta_run_nr) {
          B2ERROR("DHC RUN MM: " << dhc.data_dhc_start_frame->get_run() << " META "
                  << m_meta_run_nr);
          m_errorMask |= EPXDErrMask::c_META_MM_DHC_ERS;
        }
        if (dhc.data_dhc_start_frame->get_subrun() != m_meta_subrun_nr) {
          B2ERROR("DHC SUBRUN MM: " << dhc.data_dhc_start_frame->get_subrun()
                  << " META " << m_meta_subrun_nr);
          m_errorMask |= EPXDErrMask::c_META_MM_DHC_ERS;
        }
        if ((((unsigned int)dhc.data_dhc_start_frame->getEventNrHi() << 16) | dhc.data_dhc_start_frame->getEventNrLo()) !=
            (m_meta_event_nr & 0xFFFFFFFF)) {
          B2ERROR("DHC EVT32b MM: " << ((dhc.data_dhc_start_frame->getEventNrHi() << 16) | dhc.data_dhc_start_frame->getEventNrLo()) <<
                  " META " << (unsigned int)(m_meta_event_nr & 0xFFFFFFFF));
          m_errorMask |= EPXDErrMask::c_META_MM_DHC;
        }
        uint32_t tt = (((unsigned int)dhc.data_dhc_start_frame->time_tag_mid & 0x7FFF) << 12) | ((unsigned int)
                      dhc.data_dhc_start_frame->time_tag_lo_and_type >> 4);
        uint32_t mm = (unsigned int)(std::round((m_meta_time % 1000000000ull) * 0.127216));
        // uint64_t cc = (unsigned int)(m_meta_time / 1000000000ull);
        // B2ERROR("Meta / 1e9: " << hex << cc << " Diff: " << (dhc.data_dhc_start_frame->time_tag_hi-cc));
        if ((tt - mm) != 0) {
          m_errorMask |= EPXDErrMask::c_META_MM_DHC_TT;
          if (!m_ignoreMetaFlags) {
            B2ERROR("DHC TT: $" << hex << dhc.data_dhc_start_frame->time_tag_hi << "." << dhc.data_dhc_start_frame->time_tag_mid << "." <<
                    dhc.data_dhc_start_frame->time_tag_lo_and_type << " META " << m_meta_time << " TRG Type " <<
                    (dhc.data_dhc_start_frame->time_tag_lo_and_type & 0xF));
            B2ERROR("Meta ns from 127MHz: " << hex << mm << " Diff: " << (tt - mm));
          }
        } else {
          B2DEBUG(20, "DHC TT: $" << hex << dhc.data_dhc_start_frame->time_tag_hi << "." << dhc.data_dhc_start_frame->time_tag_mid << "." <<
                  dhc.data_dhc_start_frame->time_tag_lo_and_type << " META " << m_meta_time << " TRG Type " <<
                  (dhc.data_dhc_start_frame->time_tag_lo_and_type & 0xF));
        }
      }
      mask_active_dhe = dhc.data_dhc_start_frame->get_active_dhe_mask();
      nr_active_dhe = nr5bits(mask_active_dhe);

      m_errorMaskDHC = m_errorMask; // forget about anything before this frame
      daqpktstat.newDHC(currentDHCID, m_errorMask);
      break;
    };
    case EDHCFrameHeaderDataType::c_DHE_START: {
      countedBytesInDHE = 0;
      cancheck_countedBytesInDHE = true;
      last_dhp_readout_frame_lo[0] = -1;
      last_dhp_readout_frame_lo[1] = -1;
      last_dhp_readout_frame_lo[2] = -1;
      last_dhp_readout_frame_lo[3] = -1;
      if (verbose)dhc.data_dhe_start_frame->print();
      dhe_first_readout_frame_id_lo = dhc.data_dhe_start_frame->getStartFrameNr();
      dhe_first_triggergate = dhc.data_dhe_start_frame->getTriggerGate();
      if (currentDHEID != 0xFFFFFFFF && (currentDHEID & 0xFFFF) >= dhc.data_dhe_start_frame->getDHEId()) {
        B2ERROR("DHH IDs are not in expected order! " << (currentDHEID & 0xFFFF) << " >= " << dhc.data_dhe_start_frame->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_WRONG_ID_SEQ;
      }
      currentDHEID = dhc.data_dhe_start_frame->getDHEId();
      m_errorMask |= dhc.check_crc();

      if (countedDHEStartFrames > countedDHEEndFrames) {
        B2ERROR("DHE_START without DHE_END");
        m_errorMask |= EPXDErrMask::c_DHE_START_WO_END;
      }
      countedDHEStartFrames++;

      found_mask_active_dhp = 0;
      mask_active_dhp = dhc.data_dhe_start_frame->getActiveDHPMask();

      if ((((unsigned int)dhc.data_dhe_start_frame->getEventNrHi() << 16) | dhc.data_dhe_start_frame->getEventNrLo()) != (unsigned int)(
            m_meta_event_nr & 0xFFFFFFFF)) {
        B2ERROR("DHE EVT32b (HI WORD): " << ((dhc.data_dhe_start_frame->getEventNrHi() << 16) | dhc.data_dhe_start_frame->getEventNrLo()) <<
                " META " << (m_meta_event_nr & 0xFFFFFFFF));
        m_errorMask |= EPXDErrMask::c_META_MM_DHE;
      }
//        B2ERROR("DHE TT: $" << hex << dhc.data_dhe_start_frame->dhe_time_tag_hi << "." << dhc.data_dhe_start_frame->dhe_time_tag_lo <<
//                " META " << m_meta_time);

      if (currentDHEID == 0) {
        B2WARNING("DHE ID is invalid=0 (not initialized)");
        m_errorMask |= EPXDErrMask::c_DHE_ID_INVALID;
      }
      // calculate the VXDID for DHE and save them for DHP unpacking
      {
        /// refering to BelleII Note Nr 0010, the numbers run from ... to
        ///   unsigned int layer, ladder, sensor;
        ///   layer= vxdid.getLayerNumber();/// 1 ... 2
        ///   ladder= vxdid.getLadderNumber();/// 1 ... 8 and 1 ... 12
        ///   sensor= vxdid.getSensorNumber();/// 1 ... 2
        ///   dhe_id = ((layer-1)<<5) | ((ladder)<<1) | (sensor-1);
        unsigned short sensor, ladder, layer;
        sensor = (currentDHEID & 0x1) + 1;
        ladder = (currentDHEID & 0x1E) >> 1; // no +1
        layer = ((currentDHEID & 0x20) >> 5) + 1;
        currentVxdId = VxdID(layer, ladder, sensor);
        if (ladder == 0 || (layer == 1 && ladder > 8) || (layer == 2 && ladder > 12)) {
          B2WARNING("DHE ID is invalid = $" << hex << currentDHEID << ", " << dec << currentDHEID << " (" << layer << "," << ladder << "," <<
                    sensor << ")");
          m_errorMask |= EPXDErrMask::c_DHE_ID_INVALID;
        }
      }

      m_errorMaskDHE = m_errorMask; // forget about anything before this frame
      if (daqpktstat.dhc_size() > 0) {
        // if no DHC has been defined yet, do nothing!
        daqpktstat.dhc_back().newDHE(currentVxdId, currentDHEID, m_errorMask, dhe_first_triggergate, dhe_first_readout_frame_id_lo);
      }
      break;
    };
    case EDHCFrameHeaderDataType::c_GHOST:
      if (verbose)dhc.data_ghost_frame->print();
      if (currentDHEID != dhc.data_ghost_frame->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_ghost_frame->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      /// Attention: Firmware might be changed such, that ghostframe come for all DHPs, not only active ones...
      found_mask_active_dhp |= 1 << dhc.data_ghost_frame->getDHPPort();

      //found_mask_active_dhp = mask_active_dhp;/// TODO Workaround for DESY TB 2016 doesnt work

      m_errorMask |= dhc.check_crc();

      break;
    case EDHCFrameHeaderDataType::c_DHC_END: {
      if (dhc.data_dhc_end_frame->isFakedData() != isFakedData_event) {
        B2ERROR("DHC END is but no Fake event OR Fake Event but DHE END is not.");
      }
      if (isFakedData_event) {
        B2WARNING("Faked DHC END Data -> trigger without Data!");
        m_errorMask |= EPXDErrMask::c_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhc.data_dhc_end_frame->print();
      }

      if (!isFakedData_event) {
        if (dhc.data_dhc_end_frame->get_dhc_id() != currentDHCID) {
          m_errorMask |= EPXDErrMask::c_DHC_DHCID_START_END_MM;
          B2ERROR("DHC ID Mismatch between Start and End $" << std::hex << currentDHCID << "!=$" << dhc.data_dhc_end_frame->get_dhc_id());
        }
        int w;
        w = dhc.data_dhc_end_frame->get_words() * 4;
        if (cancheck_countedBytesInDHC) {
          if (countedBytesInDHC != w) {
            B2ERROR("Error: WIE $" << hex << countedBytesInDHC << " != DHC END $" << hex << w);
            m_errorMask |= EPXDErrMask::c_DHC_WIE;
          } else {
            if (verbose)
              B2DEBUG(20, "EVT END: WIE $" << hex << countedBytesInDHC << " == DHC END $" << hex << w);
          }
          // else ... processed data -> length invalid
        }
      }
      m_errorMask |= dhc.check_crc();
      m_errorMaskDHC |= m_errorMask; // do latest updates

      if (daqpktstat.dhc_size() > 0) {
        // only is we have a DHC object... or back() is undefined
        // Remark, if we have a broken data (DHC_START/END) structure, we might fill the
        // previous DHC object ... but then the data is junk anyway
        daqpktstat.dhc_back().setErrorMask(m_errorMaskDHC);
        //B2DEBUG(98,"** DHC "<<currentDHCID<<" Raw"<<dhc.data_dhc_end_frame->get_words() * 4 <<" Red"<<countedBytesInDHC);
        daqpktstat.dhc_back().setCounters(dhc.data_dhc_end_frame->get_words() * 4, countedBytesInDHC);
      }
      m_errorMaskDHC = 0;
      currentDHEID = 0xFFFFFFFF;
      currentDHCID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      break;
    };
    case EDHCFrameHeaderDataType::c_DHE_END: {
      if (verbose) dhc.data_dhe_end_frame->print();
      if (currentDHEID != dhc.data_dhe_end_frame->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_dhe_end_frame->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_END_ID;
      }
      m_errorMask |= dhc.check_crc();
      if (found_mask_active_dhp != mask_active_dhp) {
        if (!m_ignoreDHPMask) B2WARNING("DHE_END: DHP active mask $" << hex << mask_active_dhp << " != $" << hex << found_mask_active_dhp <<
                                          " mask of found dhp/ghost frames");
        m_errorMask |= EPXDErrMask::c_DHP_ACTIVE;
      }
      countedDHEEndFrames++;
      if (countedDHEStartFrames < countedDHEEndFrames) {
        // the other case is checked in Start
        B2ERROR("DHE_END without DHE_START");
        m_errorMask |= EPXDErrMask::c_DHE_END_WO_START;
      }
      {
        int w;
        w = dhc.data_dhe_end_frame->get_words() * 2;
        if (cancheck_countedBytesInDHE) {
          if (countedBytesInDHE != w) {
            if (!m_ignoreDHELength) B2ERROR("Error: WIE $" << hex << countedBytesInDHE << " != DHE END $" << hex << w);
            m_errorMask |= EPXDErrMask::c_DHE_WIE;
          } else {
            if (verbose)
              B2DEBUG(20, "EVT END: WIE $" << hex << countedBytesInDHE << " == DHE END $" << hex << w);
          }
          // else ... processed data -> length invalid
        }
      }
      m_errorMaskDHE |= m_errorMask; // do latest updates

      if (daqpktstat.dhc_size() > 0) {
        if (daqpktstat.dhc_back().dhe_size() > 0) {
          // only is we have a DHC and DHE object... or back() is undefined
          // Remark, if we have a broken data (DHE_START/END) structure, we might fill the
          // previous DHE object ... but then the data is junk anyway
          daqpktstat.dhc_back().dhe_back().setErrorMask(m_errorMaskDHE);
          // B2DEBUG(98,"** DHC "<<currentDHEID<<" Raw "<<dhc.data_dhe_end_frame->get_words() * 2 <<" Red"<<countedBytesInDHE);
          daqpktstat.dhc_back().dhe_back().setCounters(dhc.data_dhe_end_frame->get_words() * 2, countedBytesInDHE);
        }
      }
      m_errorMaskDHE = 0;
      currentDHEID |= 0xFF000000;// differenciate from 0xFFFFFFFFF as initial value
      currentVxdId = 0; /// invalid
      break;
    };
    case EDHCFrameHeaderDataType::c_ONSEN_ROI:
      if (verbose) dhc.data_onsen_roi_frame->print();
      m_errorMask |= dhc.data_onsen_roi_frame->check_error(len);
      m_errorMask |= dhc.data_onsen_roi_frame->check_inner_crc(len - 4); /// CRC is without the DHC header, see reason in function
      m_errorMask |= dhc.check_crc();
      if (!m_doNotStore) {
        //dhc.data_onsen_roi_frame->save(m_storeROIs, len, (unsigned int*) data);
        // void save(StoreArray<PXDRawROIs>& sa, unsigned int length, unsigned int* data) const
        // 4 byte header, ROIS (n*8), 4 byte copy of inner CRC, 4 byte outer CRC
        if (len >= dhc.data_onsen_roi_frame->getMinSize()) {
          //if ((len - dhc.data_onsen_roi_frame->getMinSize()) % 8 != 0) {
          // error checking in check_error() above, this is only for dump-ing
          // dump_roi(data, len - 4); // dump ROI payload, minus CRC
          //}
          unsigned int l;
          l = (len - dhc.data_onsen_roi_frame->getMinSize()) / 8;
          // Endian swapping is done in Contructor of RawRoi object
          m_storeROIs.appendNew(l, &((unsigned int*) data)[1]);
        }
      }
      break;
    case EDHCFrameHeaderDataType::c_ONSEN_TRG:
      eventNrOfOnsenTrgFrame = eventNrOfThisFrame;
      if (dhc.data_onsen_trigger_frame->get_trig_nr1() != (unsigned int)(m_meta_event_nr & 0xFFFFFFFF)) {
        B2ERROR("TRG HLT MM: $" << dhc.data_onsen_trigger_frame->get_trig_nr1() << " META " <<
                (m_meta_event_nr & 0xFFFFFFFF));
        m_errorMask |= EPXDErrMask::c_META_MM_ONS_HLT;
      }
      if (dhc.data_onsen_trigger_frame->get_experiment1() != m_meta_experiment) {
        B2ERROR("TRG HLT EXP MM: $" << dhc.data_onsen_trigger_frame->get_experiment1() << " META " <<
                m_meta_experiment);
        m_errorMask |= EPXDErrMask::c_META_MM_ONS_HLT;
      }
      if (dhc.data_onsen_trigger_frame->get_run1() != m_meta_run_nr) {
        B2ERROR("TRG HLT RUN MM: $" << dhc.data_onsen_trigger_frame->get_run1() << " META " <<
                m_meta_run_nr);
        m_errorMask |= EPXDErrMask::c_META_MM_ONS_HLT;
      }
      if (dhc.data_onsen_trigger_frame->get_subrun1() != m_meta_subrun_nr) {
        B2ERROR("TRG HLT SUBRUN MM: $" << dhc.data_onsen_trigger_frame->get_subrun1() << " META " <<
                m_meta_subrun_nr);
        m_errorMask |= EPXDErrMask::c_META_MM_ONS_HLT;
      }

      if (!dhc.data_onsen_trigger_frame->is_fake_datcon()) {
        if (dhc.data_onsen_trigger_frame->get_trig_nr2() != (unsigned int)(m_meta_event_nr & 0xFFFFFFFF)) {
          B2ERROR("TRG DC MM: $" << dhc.data_onsen_trigger_frame->get_trig_nr2() << " META " <<
                  (m_meta_event_nr & 0xFFFFFFFF));
          m_errorMask |= EPXDErrMask::c_META_MM_ONS_DC;
        }
        if (dhc.data_onsen_trigger_frame->get_experiment2() != m_meta_experiment) {
          B2ERROR("TRG DC EXP MM: $" << dhc.data_onsen_trigger_frame->get_experiment2() << " META " <<
                  m_meta_experiment);
          m_errorMask |= EPXDErrMask::c_META_MM_ONS_DC;
        }
        if (dhc.data_onsen_trigger_frame->get_run2() != m_meta_run_nr) {
          B2ERROR("TRG DC RUN MM: $" << dhc.data_onsen_trigger_frame->get_run2() << " META " <<
                  m_meta_run_nr);
          m_errorMask |= EPXDErrMask::c_META_MM_ONS_DC;
        }
        if (dhc.data_onsen_trigger_frame->get_subrun2() != m_meta_subrun_nr) {
          B2ERROR("TRG DC SUBRUN MM: $" << dhc.data_onsen_trigger_frame->get_subrun2() << " META " <<
                  m_meta_subrun_nr);
          m_errorMask |= EPXDErrMask::c_META_MM_ONS_DC;
        }
      }

//       B2ERROR("TRG TAG HLT: $" << hex << dhc.data_onsen_trigger_frame->get_trig_tag1() << " DATCON $" <<  dhc.data_onsen_trigger_frame->get_trig_tag2() << " META " << m_meta_time);

      if (verbose) dhc.data_onsen_trigger_frame->print();
      m_errorMask |= dhc.data_onsen_trigger_frame->check_error(m_ignoreDATCON);
      m_errorMask |= dhc.check_crc();
      if (Frame_Number != 0) {
        B2ERROR("ONSEN TRG Frame must be the first one.");
      }
      isUnfiltered_event = dhc.data_onsen_trigger_frame->is_SendUnfiltered();
      if (isUnfiltered_event) m_sendunfiltered++;
      if (dhc.data_onsen_trigger_frame->is_SendROIs()) m_sendrois++;
      if (!dhc.data_onsen_trigger_frame->is_Accepted()) m_notaccepted++;
      break;
    default:
      B2ERROR("UNKNOWN DHC frame type");
      m_errorMask |= EPXDErrMask::c_DHC_UNKNOWN;
      if (verbose) hw->print();
      break;
  }

  if (eventNrOfThisFrame != eventNrOfOnsenTrgFrame && !isFakedData_event) {
    B2ERROR("Frame TrigNr != ONSEN Trig Nr $" << hex << eventNrOfThisFrame << " != $" << eventNrOfOnsenTrgFrame);
    m_errorMask |= EPXDErrMask::c_FRAME_TNR_MM;
  }

  if (Frame_Number == 0) {
    /// Check that ONSEN Trg is first Frame
    if (type != EDHCFrameHeaderDataType::c_ONSEN_TRG) {
      if (!m_formatBonnDAQ) {
        B2ERROR("First frame is not a ONSEN Trigger frame in Event Nr " << eventNrOfThisFrame);
        m_errorMask |= EPXDErrMask::c_ONSEN_TRG_FIRST;
      }
    }
  } else { // (Frame_Number != 0 &&
    /// Check that there is no other DHC Start
    if (type == EDHCFrameHeaderDataType::c_ONSEN_TRG) {
      B2ERROR("More than one ONSEN Trigger frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_ONSEN_TRG_FIRST;
    }
  }

  if (!m_formatBonnDAQ) {
    if (Frame_Number == 1) {
      /// Check that DHC Start is first Frame
      if (type != EDHCFrameHeaderDataType::c_DHC_START) {
        B2ERROR("Second frame is not a DHC start of subevent frame in Event Nr " << eventNrOfThisFrame);
        m_errorMask |= EPXDErrMask::c_DHC_START_SECOND;
      }
    } else { // (Frame_Number != 0 &&
      /// Check that there is no other DHC Start
      if (type == EDHCFrameHeaderDataType::c_DHC_START) {
        B2ERROR("More than one DHC start of subevent frame in Event Nr " << eventNrOfThisFrame);
        m_errorMask |= EPXDErrMask::c_DHC_START_SECOND;
      }
    }
  }

  if (Frame_Number == Frames_in_event - 1) {
    /// Check that DHC End is last Frame
    if (type != EDHCFrameHeaderDataType::c_DHC_END) {
      B2ERROR("Last frame is not a DHC end of subevent frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_DHC_END_MISS;
    }

    /// As we now have processed the whole event, we can do some more consistency checks!
    if (countedDHEStartFrames != countedDHEEndFrames || countedDHEStartFrames != nr_active_dhe) {
      B2ERROR("The number of DHE Start/End does not match the number of active DHE in DHC Header! Header: " << nr_active_dhe <<
              " Start: " << countedDHEStartFrames << " End: " << countedDHEEndFrames << " Mask: $" << hex << mask_active_dhe << " in Event Nr " <<
              eventNrOfThisFrame);
      if (countedDHEStartFrames == countedDHEEndFrames) m_errorMask |= EPXDErrMask::c_DHE_ACTIVE;
      if (countedDHEStartFrames > countedDHEEndFrames)  m_errorMask |= EPXDErrMask::c_DHE_START_WO_END;
      if (countedDHEStartFrames < countedDHEEndFrames)  m_errorMask |= EPXDErrMask::c_DHE_END_WO_START;
    }

  } else { //  (Frame_Number != Frames_in_event - 1 &&
    /// Check that there is no other DHC End
    if (type == EDHCFrameHeaderDataType::c_DHC_END) {
      B2ERROR("More than one DHC end of subevent frame in frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_DHC_END_DBL;
    }
  }

  if (!m_formatBonnDAQ) {
    /// Check that (if there is at least one active DHE) the second Frame is DHE Start, actually this is redundant if the other checks work
    if (Frame_Number == 2 && nr_active_dhe != 0 && type != EDHCFrameHeaderDataType::c_DHE_START) {
      B2ERROR("Third frame is not a DHE start frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_DHE_START_THIRD;
    }
  }

  if (type != EDHCFrameHeaderDataType::c_ONSEN_ROI  && type != EDHCFrameHeaderDataType::c_ONSEN_TRG) {
    // actually, they should not be withing Start and End, but better be sure.
    countedBytesInDHC += len;
    countedBytesInDHE += len;
  }
  B2DEBUG(20, "DHC/DHE $" << hex << countedBytesInDHC << ", $" << hex << countedBytesInDHE);
}

