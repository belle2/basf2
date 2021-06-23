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

#include <boost/endian/arithmetic.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::PXD::PXDError;

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
  m_storeRawAdc()
{
  //Set module properties
  setDescription("Unpack Raw PXD Hits from ONSEN data stream");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("RawPXDsName", m_RawPXDsName, "The name of the StoreArray of RawPXDs to be processed", std::string(""));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "The name of the StoreArray of generated PXDRawHits", std::string(""));
  addParam("PXDDAQEvtStatsName", m_PXDDAQEvtStatsName, "The name of the StoreObjPtr of generated PXDDAQEvtStats", std::string(""));
  addParam("PXDRawAdcsName", m_PXDRawAdcsName, "The name of the StoreArray of generated PXDRawAdcs", std::string(""));
  addParam("PXDRawROIsName", m_PXDRawROIsName, "The name of the StoreArray of generated PXDRawROIs", std::string(""));
  addParam("DoNotStore", m_doNotStore, "only unpack and check, but do not store", false);
//   addParam("CriticalErrorMask", m_criticalErrorMask, "Set error mask which stops processing by returning false by task", 0);
//   addParam("SuppressErrorMask", m_suppressErrorMask, "Set mask for errors msgs which are not printed", getSilenceMask());
  addParam("ForceMapping", m_forceMapping, "Force Mapping even if DHH bit is NOT requesting it", false);
  addParam("ForceNoMapping", m_forceNoMapping, "Force NO Mapping even if DHH bit is requesting it", false);
  addParam("CheckPaddingCRC", m_checkPaddingCRC, "Check for susp. padding (debug option, many false positive)", false);
  addParam("MaxDHPFrameDiff", m_maxDHPFrameDiff, "Maximum DHP Frame Nr Difference w/o reporting error", 2u);
  addParam("FormatBonnDAQ", m_formatBonnDAQ, "ONSEN or BonnDAQ format", false);
  addParam("Verbose", m_verbose, "Turn on extra verbosity for log-level debug", false);
  addParam("ContinueOnError", m_continueOnError, "Continue package depacking on error (for debugging)", false);
//   (
//              /*EPXDErrFlag::c_DHC_END | EPXDErrFlag::c_DHE_START | EPXDErrFlag::c_DATA_OUTSIDE |*/
//              EPXDErrFlag::c_FIX_SIZE | EPXDErrFlag::c_DHE_CRC | EPXDErrFlag::c_DHC_UNKNOWN | /*EPXDErrFlag::c_MERGER_CRC |*/
//              EPXDErrFlag::c_DHP_SIZE | /*EPXDErrFlag::c_DHP_PIX_WO_ROW | EPXDErrFlag::c_DHE_START_END_ID | EPXDErrFlag::c_DHE_START_ID |*/
//              EPXDErrFlag::c_DHE_START_WO_END | EPXDErrFlag::c_DHP_NOT_CONT
//            ));

  // this is not really a parameter, it should be fixed.
  m_errorSkipPacketMask[c_nrDHE_CRC] = true;
  m_errorSkipPacketMask[c_nrFIX_SIZE] = true;
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
  /// actually, later we do not want to store ROIs and raw ADC into output file ...  aside from debugging

  B2DEBUG(29, "ForceMapping: " << m_forceMapping);
  B2DEBUG(29, "ForceNoMapping: " << m_forceNoMapping);
  B2DEBUG(29, "CheckPaddingCRC: " << m_checkPaddingCRC);
  B2DEBUG(29, "MaxDHPFrameDiff: " << m_maxDHPFrameDiff);

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
  m_storeDAQEvtStats.create(c_NO_ERROR);

  m_errorMask = 0;
  m_errorMaskEvent = 0;

  m_meta_event_nr = m_eventMetaData->getEvent();// used for error output below

  if (!m_storeRawPXD) {// if no input, nothing to do
    m_errorMask[c_nrNO_PXD] = true;
  } else {
    int nRaws = m_storeRawPXD.getEntries();
    if (m_verbose) {
      B2DEBUG(29, "PXD Unpacker --> RawPXD Objects in event: " << LogVar("Objects", nRaws));
    };

    m_meta_run_nr = m_eventMetaData->getRun();
    m_meta_subrun_nr = m_eventMetaData->getSubrun();
    m_meta_experiment = m_eventMetaData->getExperiment();
    m_meta_time = m_eventMetaData->getTime();
    m_meta_ticks = (unsigned int)std::round((m_meta_time % 1000000000ull) * 0.127216); // calculate ticks in 127MHz RF clock
    m_meta_sec = (unsigned int)(m_meta_time / 1000000000ull) & 0x1FFFF;

    int inx = 0; // count index for output objects
    for (auto& it : m_storeRawPXD) {
      if (m_verbose) {
        B2DEBUG(29, "PXD Unpacker --> Unpack Objects: ");
      };
      unpack_rawpxd(it, inx++);
    }

    if (nRaws == 0) m_errorMask[c_nrNO_PXD] = true;
  }
  m_errorMaskEvent |= m_errorMask;
  m_storeDAQEvtStats->setErrorMask(m_errorMaskEvent);

  m_unpackedEventsCount++;
  {
    for (unsigned int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      if (m_errorMaskEvent[i]) m_errorCounter[i]++;
    }
  }

  if ((PXDErrorFlags(m_criticalErrorMask) & m_errorMaskEvent) != PXDErrorFlags(0)) B2ERROR("Error in PXD unpacking" <<
        LogVar("event nr", m_meta_event_nr));
  setReturnValue(PXDErrorFlags(0) == (PXDErrorFlags(m_criticalErrorMask) & m_errorMaskEvent));
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
    if (!(m_suppressErrorMask[c_nrPACKET_SIZE])) {
      B2WARNING("PXD Unpacker --> invalid packet size" <<
                LogVar("size [32bit words] $", static_cast < std::ostringstream && >(std::ostringstream() << hex << px.size()).str()));
    }
    m_errorMask[c_nrPACKET_SIZE] = true;
    return;
  }
  std::vector<unsigned int> data(px.size());
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  std::copy_n(px.data(), px.size(), data.begin());

  if (fullsize < 8) {
    if (!(m_suppressErrorMask[c_nrPACKET_SIZE])) {
      B2WARNING("Data is to small to hold a valid Header! Will not unpack anything." << LogVar("size [32bit words] $",
                static_cast < std::ostringstream && >(std::ostringstream() << hex << fullsize).str()));
    }
    m_errorMask[c_nrPACKET_SIZE] = true;
    return;
  }

  if (data[0] != 0xCAFEBABE && data[0] != 0xBEBAFECA) {
    if (!(m_suppressErrorMask[c_nrMAGIC])) {
      B2WARNING("Magic invalid: Will not unpack anything. Header corrupted." <<
                LogVar("Header Magic $", static_cast < std::ostringstream && >(std::ostringstream() << hex << data[0]).str()));
    }
    m_errorMask[c_nrMAGIC] = true;
    return;
  }


  Frames_in_event = ((ubig32_t*)data.data())[1];
  if (Frames_in_event < 0 || Frames_in_event > 256) {
    if (!(m_suppressErrorMask[c_nrFRAME_NR])) {
      B2WARNING("Number of Frames invalid: Will not unpack anything. Header corrupted!" << LogVar("Frames in event", Frames_in_event));
    }
    m_errorMask[c_nrFRAME_NR] = true;
    return;
  }
  if (Frames_in_event < 3) {
    if (!(m_suppressErrorMask[c_nrNR_FRAMES_TO_SMALL])) {
      B2WARNING("Number of Frames too small: It cannot contain anything useful." << LogVar("Frames in event", Frames_in_event));
    }
    m_errorMask[c_nrNR_FRAMES_TO_SMALL] = true;
  }

  /// NEW format
  if (m_verbose) {
    B2DEBUG(29, "PXD Unpacker --> data[0]: <-- Magic $" << hex << data[0]);
    B2DEBUG(29, "PXD Unpacker --> data[1]: <-- #Frames $" << hex << data[1]);
    if (data[1] >= 1 && fullsize < 12) B2DEBUG(29, "PXD Unpacker --> data[2]: <-- Frame 1 len $" << hex << data[2]);
    if (data[1] >= 2 && fullsize < 16) B2DEBUG(29, "PXD Unpacker --> data[3]: <-- Frame 2 len $" << hex << data[3]);
    if (data[1] >= 3 && fullsize < 20) B2DEBUG(29, "PXD Unpacker --> data[4]: <-- Frame 3 len $" << hex << data[4]);
    if (data[1] >= 4 && fullsize < 24) B2DEBUG(29, "PXD Unpacker --> data[5]: <-- Frame 4 len $" << hex << data[5]);
  };

  unsigned int* tableptr;
  tableptr = &data[2]; // skip header!!!

  unsigned int* dataptr;
  dataptr = &tableptr[Frames_in_event];
  datafullsize = fullsize - 2 * 4 - Frames_in_event * 4; // Size is fullsize minus header minus table

  int ll = 0; // Offset in dataptr in bytes
  for (int j = 0; j < Frames_in_event; j++) {
    int lo;/// len of frame in bytes

    lo = ((ubig32_t*)tableptr)[j];
    if (lo <= 0) {
      if (!(m_suppressErrorMask[c_nrFRAME_SIZE])) {
        B2WARNING("size of frame invalid");
        B2DEBUG(29, "size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      }
      m_errorMask[c_nrFRAME_SIZE] = true;
      return;
    }
    if (ll + lo > datafullsize) {
      if (!(m_suppressErrorMask[c_nrFRAME_SIZE])) {
        B2WARNING("Frames exceed packet size");
        B2DEBUG(29, "Frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " <<
                datafullsize << " of fullsize " << fullsize);
      }
      m_errorMask[c_nrFRAME_SIZE] = true;
      return;
    }
    if (lo & 0x3) {
      if (!(m_suppressErrorMask[c_nrFRAME_SIZE])) {
        B2WARNING("SKIP Frame with Data with not MOD 4 length");
        B2DEBUG(29, "SKIP Frame with Data with not MOD 4 length " << " ( $" << hex << lo  << " ) ");
      }
      ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
      m_errorMask[c_nrFRAME_SIZE] = true;
    } else {
      B2DEBUG(29, "unpack DHE(C) frame: " << j << " with size " << lo << " at byte offset in dataptr " << ll);
      unpack_dhc_frame(ll + (char*)dataptr, lo, j, Frames_in_event, daqpktstat);
      ll += lo; /// no rounding needed
    }
    m_errorMaskDHE |= m_errorMask;
    m_errorMaskDHC |= m_errorMask;
    m_errorMaskPacket |= m_errorMask;
    m_errorMaskEvent |= m_errorMask;
    m_errorMask = 0;

    if (!m_continueOnError && (m_errorMaskPacket & PXDErrorFlags(m_errorSkipPacketMask)) != PXDErrorFlags(0)) {
      // skip full package on error, recovery to next DHC/DHE Start might be possible in some cases
      // But thats to hard to implement
      // Remark: PXD data for broken events is removed in next PXDPostChecker module, thus skipping the
      // unpacking is not strictly necessary here.
      break;
    }
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
    if (!(m_suppressErrorMask[c_nrFIX_SIZE])) B2WARNING("Frame size unsupported for RAW ADC frame! $" <<
                                                          LogVar("size [bytes] $", static_cast < std::ostringstream && >(std::ostringstream() << hex << frame_len).str())
                                                          << LogVar("DHE", dhe_ID) << LogVar("DHP", dhe_DHPport));
    m_errorMask[c_nrFIX_SIZE] = true;
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
    if (!(m_suppressErrorMask[c_nrDHE_DHP_DHEID])) {
      B2WARNING("DHE ID in DHE and DHP header differ");
      B2DEBUG(29, "DHE ID in DHE and DHP header differ $" << hex << dhe_ID << " != $" << dhp_dhe_id);
    }
    m_errorMask[c_nrDHE_DHP_DHEID] = true;
  }
  if (dhe_DHPport != dhp_dhp_id) {
    if (!(m_suppressErrorMask[c_nrDHE_DHP_PORT])) {
      B2WARNING("DHP ID (Chip/Port) in DHE and DHP header differ");
      B2DEBUG(29, "DHP ID (Chip/Port) in DHE and DHP header differ $" << hex << dhe_DHPport << " != $" << dhp_dhp_id);
    }
    m_errorMask[c_nrDHE_DHP_PORT] = true;
  }

  if (dhp_header_type != EDHPFrameHeaderDataType::c_RAW) {
    if (!(m_suppressErrorMask[c_nrHEADERTYPE_INV])) {
      B2WARNING("Header type invalid for this kind of DHE frame");
      B2DEBUG(29, "Header type invalid for this kind of DHE frame: $" << hex << dhp_header_type);
    }
    m_errorMask[c_nrHEADERTYPE_INV] = true;
    return;
  }

  /// Endian Swapping is done in Contructors of Raw Objects!
  B2DEBUG(29, "Raw ADC Data");
  // size checked already above
  m_storeRawAdc.appendNew(vxd_id, data, frame_len);
};

void PXDUnpackerModule::unpack_fce([[maybe_unused]] unsigned short* data, [[maybe_unused]] unsigned int length,
                                   [[maybe_unused]] VxdID vxd_id)
{
  //! *************************************************************
  //! Important Remark:
  //! Up to now the format for cluster is not well defined.
  //! We need to wait for the final hardware implementation.
  //! Then the following code must be re-checked TODO
  //! *************************************************************

  B2WARNING("FCE (Cluster) Packet have not yet been tested with real HW clusters. Dont assume that this code is working!");
  return;

  // implement the unpacking here and not as a separate module ... when it is available in HW
//   ubig16_t* cluster = (ubig16_t*)data;
//   int nr_words; //words in dhp frame
//   unsigned int words_in_cluster = 0; //counts 16bit words in cluster
//   nr_words = length / 2;
//   ubig16_t sor;
//   sor = 0x0000;
//
//   for (int i = 2 ; i < nr_words ; i++) {
//     if (i != 2) { //skip header
//       if ((((cluster[i] & 0x8000) == 0)
//            && ((cluster[i] & 0x4000) >> 14) == 1)) {  //searches for start of row frame with start of cluster flag = 1 => new cluster
//         if (!m_doNotStore) m_storeRawCluster.appendNew(&data[i - words_in_cluster], words_in_cluster, vxd_id);
//         words_in_cluster = 0;
//       }
//     }
//     if ((cluster[i] & 0x8000) == 0) {
//       sor = cluster[i];
//     }
//     words_in_cluster++;
//
//     if ((cluster[nr_words - 1] & 0xFFFF) == (sor &
//                                              0xFFFF)) {//if frame is not 32bit aligned last word will be the last start of row word
//       cluster[nr_words - 1] = 0x0000;//overwrites the last redundant word with zero to make checking easier in PXDHardwareClusterUnpacker
//     }
//
//     if (i == nr_words - 1) {
//       if (!m_doNotStore) m_storeRawCluster.appendNew(&data[i - words_in_cluster + 1], words_in_cluster, vxd_id);
//     }
//   }
}

void PXDUnpackerModule::dump_dhp(void* data, unsigned int frame_len)
{
  // called only for debugging purpose, will never be called in normal running
  unsigned int w = frame_len / 2;
  ubig16_t* d = (ubig16_t*)data;

  B2WARNING("HEADER --  $" << hex << d[0] << ",$" << hex << d[1] << ",$" << hex << d[2] << ",$" << hex << d[3] << " -- ");

  auto dhp_header_type  = (d[2] & 0xE000) >> 13;
  auto dhp_reserved     = (d[2] & 0x1F00) >> 8;
  auto dhp_dhe_id       = (d[2] & 0x00FC) >> 2;
  auto dhp_dhp_id       =  d[2] & 0x0003;

  B2WARNING("DHP type     | $" << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
  B2WARNING("DHP reserved | $" << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
  B2WARNING("DHP DHE ID   | $" << hex << dhp_dhe_id << " ( " << dec << dhp_dhe_id << " ) ");
  B2WARNING("DHP DHP ID   | $" << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  for (unsigned int i = 4; i < w; i++) {
    B2WARNING("DHP DATA $" << hex << d[i]);
  }
  B2WARNING("DHP CRC $" << hex << d[w] << ",$" << hex << d[w + 1]);
}

void PXDUnpackerModule::dump_roi(void* data, unsigned int frame_len)
{
  // called only for debugging purpose, will never be called in normal running
  unsigned int w = frame_len / 4;
  ubig32_t* d = (ubig32_t*)data;

  B2WARNING("HEADER --  $" << hex << d[0] << ",$" << hex << d[1] << ",$" << hex << d[2] << ",$" << hex << d[3] << " -- Len $" << hex
            << frame_len);

  for (unsigned int i = 0; i < w; i++) {
    B2WARNING("ROI DATA $" << hex << d[i]);
  }
  B2WARNING("ROI CRC $" << hex << d[w]);
}

void PXDUnpackerModule::unpack_dhp(void* data, unsigned int frame_len, unsigned int dhe_first_readout_frame_id_lo,
                                   unsigned int dhe_ID, unsigned dhe_DHPport, unsigned dhe_reformat, VxdID vxd_id,
                                   PXDDAQPacketStatus& daqpktstat)
{
  unsigned int nr_words = frame_len / 2; // frame_len in bytes (excl. CRC)!!!
  ubig16_t* dhp_pix = (ubig16_t*)data;

  unsigned int dhp_readout_frame_lo = 0;
  unsigned int dhp_header_type  = 0;
  unsigned int dhp_reserved     = 0;
  unsigned int dhp_dhe_id       = 0;
  unsigned int dhp_dhp_id       = 0;

  // cppcheck-suppress unreadVariable
  unsigned int dhp_row = 0, dhp_col = 0, dhp_adc = 0, dhp_cm = 0;
//  unsigned int dhp_offset = 0;
  bool rowflag = false;
  bool pixelflag = true; // just for first row start

  if (nr_words < 4) {
    if (!(m_suppressErrorMask[c_nrDHP_SIZE])) B2WARNING("DHP frame size error (too small)" << LogVar("Nr words", nr_words));
    m_errorMask[c_nrDHP_SIZE] = true;
    return;
  }

  B2DEBUG(29, "HEADER -- $" << hex << dhp_pix[0] << hex << dhp_pix[1] << hex << dhp_pix[2] << hex << dhp_pix[3] << " -- ");

  B2DEBUG(29, "DHP Header   | $" << hex << dhp_pix[2] << " ( " << dec << dhp_pix[2] << " ) ");
  dhp_header_type  = (dhp_pix[2] & 0xE000) >> 13;
  dhp_reserved     = (dhp_pix[2] & 0x1F00) >> 8;
  dhp_dhe_id       = (dhp_pix[2] & 0x00FC) >> 2;
  dhp_dhp_id       =  dhp_pix[2] & 0x0003;

  B2DEBUG(29, "DHP type     | $" << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
  B2DEBUG(29, "DHP reserved | $" << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
  B2DEBUG(29, "DHP DHE ID   | $" << hex << dhp_dhe_id << " ( " << dec << dhp_dhe_id << " ) ");
  B2DEBUG(29, "DHP DHP ID   | $" << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");

  if (dhe_ID != dhp_dhe_id) {
    if (!(m_suppressErrorMask[c_nrDHE_DHP_DHEID])) {
      B2WARNING("DHE ID in DHE and DHP header differ");
      B2DEBUG(29, "DHE ID in DHE and DHP header differ $" << hex << dhe_ID << " != $" << dhp_dhe_id);
    }
    m_errorMask[c_nrDHE_DHP_DHEID] = true;
  }
  if (dhe_DHPport != dhp_dhp_id) {
    if (!(m_suppressErrorMask[c_nrDHE_DHP_PORT])) {
      B2WARNING("DHP ID (Chip/Port) in DHE and DHP header differ");
      B2DEBUG(29, "DHP ID (Chip/Port) in DHE and DHP header differ $" << hex << dhe_DHPport << " != $" << dhp_dhp_id);
    }
    m_errorMask[c_nrDHE_DHP_PORT] = true;
  }

  if (dhp_header_type != EDHPFrameHeaderDataType::c_ZSD) {
    if (!(m_suppressErrorMask[c_nrHEADERTYPE_INV])) {
      B2WARNING("Header type invalid for this kind of DHE frame");
      B2DEBUG(29, "Header type invalid for this kind of DHE frame: $" << hex << dhp_header_type);
    }
    m_errorMask[c_nrHEADERTYPE_INV] = true;
    return;
  }

//  static int offtab[4] = {0, 64, 128, 192};
//   dhp_offset = offtab[dhp_dhp_id];

  dhp_readout_frame_lo  = dhp_pix[3] & 0xFFFF;
  B2DEBUG(29, "DHP Frame Nr     |  $" << hex << dhp_readout_frame_lo << " ( " << dec << dhp_readout_frame_lo << " ) ");

  /* // TODO removed because data format error is not to be fixed soon
  if (((dhp_readout_frame_lo - dhe_first_readout_frame_id_lo) & 0x3F) > m_maxDHPFrameDiff) {
    if (!m_suppressErrorMask[c_nrDHP_DHE_FRAME_DIFFER]) B2WARNING("DHP Frame Nr differ from DHE Frame Nr by >1 DHE " <<
          dhe_first_readout_frame_id_lo << " != DHP " << (dhp_readout_frame_lo & 0x3F) << " delta " << ((
                dhp_readout_frame_lo - dhe_first_readout_frame_id_lo) & 0x3F));
    m_errorMask[c_nrDHP_DHE_FRAME_DIFFER] = true;
  }
  */
  /* // TODO removed because data format error is not to be fixed soon
    if (m_last_dhp_readout_frame_lo[dhp_dhp_id] != -1) {
    if (((dhp_readout_frame_lo - m_last_dhp_readout_frame_lo[dhp_dhp_id]) & 0xFFFF) > m_maxDHPFrameDiff) {
      if(!m_suppressErrorMask&c_DHP_NOT_CONT ) B2WARNING("Two DHP Frames per sensor which frame number differ more than one! " << m_last_dhp_readout_frame_lo[dhp_dhp_id] << ", " <<
              dhp_readout_frame_lo);
      m_errorMask[c_nrDHP_NOT_CONT] = true;
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
    if (m_last_dhp_readout_frame_lo[j] != -1) {
      if (((dhp_readout_frame_lo - m_last_dhp_readout_frame_lo[j]) & 0xFFFF) > m_maxDHPFrameDiff) {
        if(!m_suppressErrorMask&c_DHP_DHP_FRAME_DIFFER ) B2WARNING("Two DHP Frames (different DHP) per sensor which frame number differ more than one! " << m_last_dhp_readout_frame_lo[j] <<
                ", " <<
                dhp_readout_frame_lo);
        m_errorMask[c_nrDHP_DHP_FRAME_DIFFER] = true;
        break;// give msg only once
      }
    }
  }
  */
  m_last_dhp_readout_frame_lo[dhp_dhp_id] = dhp_readout_frame_lo;

// TODO Please check if this can happen by accident with valid data!
  if (dhp_pix[2] == dhp_pix[4] && dhp_pix[3] + 1 == dhp_pix[5]) {
    // We see a second "header" with framenr+1 ...
    if (!(m_suppressErrorMask[c_nrDHP_DBL_HEADER])) {
      B2WARNING("DHP data: seems to be double header! skipping.");
      B2DEBUG(29, "DHP data: seems to be double header! skipping." << LogVar("Length",
              frame_len));
    }
    m_errorMask[c_nrDHP_DBL_HEADER] = true;
    // dump_dhp(data, frame_len); print out guilty dhp packet
    return;
  }

  // Start with offset 4, thus skipping header words
  for (unsigned int i = 4; i < nr_words ; i++) {

    B2DEBUG(29, "-- $" << hex << dhp_pix[i] << " --   " << dec << i);
    {
      if (((dhp_pix[i] & 0x8000) >> 15) == 0) {
        rowflag = true;
        if (!pixelflag) {
          if (!(m_suppressErrorMask[c_nrDHP_ROW_WO_PIX])) B2WARNING("DHP Unpacking: Row w/o Pix");
          m_errorMask[c_nrDHP_ROW_WO_PIX] = true;
        }
        pixelflag = false;
        dhp_row = (dhp_pix[i] & 0xFFC0) >> 5;
        dhp_cm  = dhp_pix[i] & 0x3F;
        if (dhp_cm == 63) { // fifo overflow
          B2WARNING("DHP data loss (CM=63) in " << LogVar("DHE", dhe_ID) << LogVar("DHP", dhp_dhp_id));
          /// FIXME TODO set an error bit ... but define one first
          m_errorMask[c_nrDHH_MISC_ERROR] = true;
        }
        if (daqpktstat.dhc_size() > 0) {
          if (daqpktstat.dhc_back().dhe_size() > 0) {
            PXDDAQDHPComMode cm(dhp_dhp_id, dhp_row, dhp_cm);
            // only is we have a DHC and DHE object... or back() is undefined
            // Remark, if we have a broken data (DHE_START/END) structure, we might fill the
            // previous DHE object ... but then the data is junk anyway
            daqpktstat.dhc_back().dhe_back().addCM(cm);
          }
        }
        B2DEBUG(29, "SetRow: $" << hex << dhp_row << " CM $" << hex << dhp_cm);
      } else {
        if (!rowflag) {
          if (!(m_suppressErrorMask[c_nrDHP_PIX_WO_ROW])) B2WARNING("DHP Unpacking: Pix without Row!!! skip dhp data ");
          m_errorMask[c_nrDHP_PIX_WO_ROW] = true;
          // dump_dhp(data, frame_len);// print out faulty dhp frame
          return;
        } else {
          pixelflag = true;
          dhp_row = (dhp_row & 0xFFE) | ((dhp_pix[i] & 0x4000) >> 14);
          dhp_col = ((dhp_pix[i]  & 0x3F00) >> 8);
          unsigned int v_cellID, u_cellID;
          v_cellID = dhp_row;// defaults for no mapping
          if (dhp_row >= 768) {
            if (!(m_suppressErrorMask[c_nrROW_OVERFLOW])) B2WARNING("DHP ROW Overflow " << LogVar("Row", dhp_row));
            m_errorMask[c_nrROW_OVERFLOW] = true;
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
            if (!(m_suppressErrorMask[c_nrCOL_OVERFLOW])) {
              B2WARNING("DHP COL Overflow (unconnected drain lines)");
              B2DEBUG(29, "DHP COL Overflow (unconnected drain lines) " << u_cellID << ", reformat " << dhe_reformat << ", dhpcol " << dhp_col <<
                      ", id " << dhp_dhp_id);
            }
            m_errorMask[c_nrCOL_OVERFLOW] = true;
          }
          dhp_adc = dhp_pix[i] & 0xFF;
          B2DEBUG(29, "SetPix: Row $" << hex << dhp_row << " Col $" << hex << dhp_col << " ADC $" << hex << dhp_adc
                  << " CM $" << hex << dhp_cm);

          if (dhp_adc == 0) {
            // if !supress error flag
            B2WARNING("DHE Event truncation in DHE " << dhe_ID << " DHP " << dhp_dhp_id);
            // m_errorMask |= c_DHE_EVENT_TRUNC;
            daqpktstat.dhc_back().dhe_back().dhp_back().setTruncated();
          } else {
            if (!m_doNotStore) m_storeRawHits.appendNew(vxd_id, v_cellID, u_cellID, dhp_adc,
                                                          (dhp_readout_frame_lo - dhe_first_readout_frame_id_lo) & 0x3F);
          }
        }
      }
    }
  }

  B2DEBUG(29, "(DHE) DHE_ID $" << hex << dhe_ID << " (DHE) DHP ID $" << hex << dhe_DHPport << " (DHP) DHE_ID $" << hex << dhp_dhe_id
          << " (DHP) DHP ID $" << hex << dhp_dhp_id);
  /*for (int i = 0; i < raw_nr_words ; i++) {
    B2DEBUG(29, "RAW      |   " << hex << p_pix[i]);
    printf("raw %08X  |  ", p_pix[i]);
    B2DEBUG(29, "row " << hex << ((p_pix[i] >> 20) & 0xFFF) << dec << " ( " << ((p_pix[i] >> 20) & 0xFFF) << " ) " << " col " << hex << ((p_pix[i] >> 8) & 0xFFF)
           << " ( " << dec << ((p_pix[i] >> 8) & 0xFFF) << " ) " << " adc " << hex << (p_pix[i] & 0xFF) << " ( " << (p_pix[i] & 0xFF) << " ) "
          );
  }*/
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
  // cppcheck-suppress variableScope
  static unsigned int dhe_first_triggergate = 0;
  static unsigned int currentDHCID = 0xFFFFFFFF;
  static unsigned int currentDHEID = 0xFFFFFFFF;
  static unsigned int currentVxdId = 0;
  static bool isFakedData_event = false;
  static bool isUnfiltered_event = false;


  if (Frame_Number == 0) {
    // We reset the counters on the first event
    // we do this before any other check is done
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
  }

  dhc_frame_header_word0* hw = (dhc_frame_header_word0*)data;

  dhc_frames dhc;
  dhc.set(data, hw->getFrameType(), len);

  {
    // if a fixed size frame has a different length, how can we rely on its content???
    // AND we could by typecasting access memory beyond end of data (but very unlikely)
    // for that reason this we have to check before any CRC and stop unpacking the frame
    int s = dhc.getFixedSize();
    if (len != s && s != 0) {
      if (!(m_suppressErrorMask[c_nrFIX_SIZE])) {
        B2WARNING("Fixed frame type size does not match specs" << LogVar("expected length",
                  len) << LogVar("length in data", s));
      }
      m_errorMask[c_nrFIX_SIZE] = true;
      if (!m_continueOnError) return;
    }
  }

  // What do we do with wrong checksum frames? As we do not know WHAT is wrong, we have to skip them alltogether.
  // As they might contain HEADER Info, we might better skip the processing of the full package, too.
  dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);
  if (!m_continueOnError && m_errorMask[c_nrDHE_CRC]) {
    // if CRC is wrong, we cannot rely on the content of the frame, thus skipping is the best option
    return;
  }

  unsigned int eventNrOfThisFrame = dhc.getEventNrLo();
  int frame_type = dhc.getFrameType();

  if (Frame_Number == 0) { /// We reset the counters on the first event
    if (m_formatBonnDAQ) {
      if (frame_type != EDHCFrameHeaderDataType::c_DHC_START) {
        if (!(m_suppressErrorMask[c_nrEVENT_STRUCT])) B2WARNING("This looks not like BonnDAQ format.");
        m_errorMask[c_nrEVENT_STRUCT] = true;
//         if (!m_continueOnError) return; // requires more testing
      }
    } else {
      if (frame_type == EDHCFrameHeaderDataType::c_DHC_START) {
        if (!(m_suppressErrorMask[c_nrEVENT_STRUCT]))
          B2WARNING("This looks like BonnDAQ or old Desy 2013/14 testbeam format. Please use formatBonnDAQ or the pxdUnpackerDesy1314 module.");
        m_errorMask[c_nrEVENT_STRUCT] = true;
//         if (!m_continueOnError) return; // requires more testing
      }
    }
  }

  if (!m_formatBonnDAQ) {
    if (Frame_Number == 1) {
      if (frame_type == EDHCFrameHeaderDataType::c_DHC_START) {
        isFakedData_event = dhc.data_dhc_start_frame->isFakedData();
      }
    }

    // please check if this mask is suitable. At least here we are limited by the 16 bit trigger number in the DHH packet header.
    // we can use more bits in the DHC and DHE START Frame
    if ((eventNrOfThisFrame & 0xFFFF) != (m_meta_event_nr & 0xFFFF)) {
      if (!isFakedData_event) {
        if (!(m_suppressErrorMask[c_nrMETA_MM])) {
          B2WARNING("Event Numbers do not match for this frame");
          B2DEBUG(29, "Event Numbers do not match for this frame" <<
                  LogVar("Event nr in frame $", static_cast < std::ostringstream
                         && >(std::ostringstream() << hex << eventNrOfThisFrame).str()) <<
                  LogVar("Event nr in MetaInfo (bits masked) $",
                         static_cast < std::ostringstream && >(std::ostringstream() << hex << m_meta_event_nr).str()));
        }
        m_errorMask[c_nrMETA_MM] = true;
//         if (!m_continueOnError) return; // requires more testing
      }
    }

    if (Frame_Number > 1 && Frame_Number < Frames_in_event - 1) {
      if (countedDHEStartFrames != countedDHEEndFrames + 1)
        if (frame_type != EDHCFrameHeaderDataType::c_ONSEN_ROI && frame_type != EDHCFrameHeaderDataType::c_DHE_START) {
          if (!(m_suppressErrorMask[c_nrDATA_OUTSIDE])) B2WARNING("Data Frame outside a DHE START/END");
          m_errorMask[c_nrDATA_OUTSIDE] = true;
//           if (!m_continueOnError) return; // requires more testing
        }
    }
  }

  // TODO How do we handle Frames where Error Bit is set in header?
  // Currently there is no documentation what it actually means... only an error bit is set (below)
  // the following errors must be "accepted", as all firmware sets it wrong from Ghost frames.
  if (hw->getErrorFlag()) {
    if (frame_type != EDHCFrameHeaderDataType::c_GHOST) {
      if (!(m_suppressErrorMask[c_nrHEADER_ERR])) B2ERROR("Error Bit set in DHE Header");
      m_errorMask[c_nrHEADER_ERR] = true;// TODO this should have some effect ... when does it mean something? documentation missing
    }
  } else {
    if (frame_type == EDHCFrameHeaderDataType::c_GHOST) {
      m_errorMask[c_nrHEADER_ERR_GHOST] = true;
    }
  }

  switch (frame_type) {
    case EDHCFrameHeaderDataType::c_DHP_RAW: {

      if (m_verbose) dhc.data_direct_readout_frame_raw->print();
      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        if (!(m_suppressErrorMask[c_nrDHE_START_ID])) {
          B2WARNING("DHE ID from DHE Start and this frame do not match");
          B2DEBUG(29, "DHE ID from DHE Start and this frame do not match" <<
                  LogVar("DHEID in this frame $", static_cast < std::ostringstream
                         && >(std::ostringstream() << hex << dhc.data_direct_readout_frame_raw->getDHEId()).str()) <<
                  LogVar("DHEID expected $", static_cast < std::ostringstream && >(std::ostringstream() << hex << currentDHEID).str()));
        }
        m_errorMask[c_nrDHE_START_ID] = true;
      }
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);
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

      if (m_verbose) dhc.data_direct_readout_frame->print();
      if (isUnfiltered_event) {
        if (frame_type == EDHCFrameHeaderDataType::c_ONSEN_DHP) m_errorMask[c_nrSENDALL_TYPE] = true;
      } else {
        if (frame_type == EDHCFrameHeaderDataType::c_DHP_ZSD) m_errorMask[c_nrNOTSENDALL_TYPE] = true;
      }

      //m_errorMask |= dhc.data_direct_readout_frame->check_error();

      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        if (!(m_suppressErrorMask[c_nrDHE_START_ID])) {
          B2WARNING("DHE ID from DHE Start and this frame do not match");
          B2DEBUG(29, "DHE ID from DHE Start and this frame do not match" <<
                  LogVar("DHEID in this frame $", static_cast < std::ostringstream
                         && >(std::ostringstream() << hex << dhc.data_direct_readout_frame_raw->getDHEId()).str()) <<
                  LogVar("DHEID expected $", static_cast < std::ostringstream && >(std::ostringstream() << hex << currentDHEID).str()));
        }
        m_errorMask[c_nrDHE_START_ID] = true;
      }
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();
      if (m_checkPaddingCRC) dhc.check_padding(m_errorMask); // isUnfiltered_event


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
      if (!(m_suppressErrorMask[c_nrUNEXPECTED_FRAME_TYPE])) B2WARNING("Unexpected Frame Type (Clustering FCE)");
      m_errorMask[c_nrUNEXPECTED_FRAME_TYPE] = true;
      if (m_verbose) hw->print();
      if (isUnfiltered_event) {
        if (frame_type == EDHCFrameHeaderDataType::c_ONSEN_FCE) {
          // TODO add error message
          m_errorMask[c_nrSENDALL_TYPE] = true;
        }
      } else {
        if (frame_type == EDHCFrameHeaderDataType::c_FCE_RAW) {
          // TODO add error message
          m_errorMask[c_nrNOTSENDALL_TYPE] = true;
        }
      }

      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        if (!(m_suppressErrorMask[c_nrDHE_START_ID])) {
          B2WARNING("DHE ID from DHE Start and this frame do not match");
          B2DEBUG(29, "DHE ID from DHE Start and this frame do not match" <<
                  LogVar("DHEID in this frame $", static_cast < std::ostringstream
                         && >(std::ostringstream() << hex << dhc.data_direct_readout_frame_raw->getDHEId()).str()) <<
                  LogVar("DHEID expected $", static_cast < std::ostringstream && >(std::ostringstream() << hex << currentDHEID).str()));
        }
        m_errorMask[c_nrDHE_START_ID] = true;
      }
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();

      B2DEBUG(29, "UNPACK FCE FRAME with len $" << hex << len);
      unpack_fce((unsigned short*) data, len - 4, currentVxdId);

      break;
    };
    case EDHCFrameHeaderDataType::c_COMMODE: {
      // this frame type has up to now not been well defined, we do not expect it until
      // the firmware supports clustering in hardware
      if (!(m_suppressErrorMask[c_nrUNEXPECTED_FRAME_TYPE])) B2WARNING("Unexpected Frame Type (COMMODE)");
      m_errorMask[c_nrUNEXPECTED_FRAME_TYPE] = true;

      if (m_verbose) hw->print();
      if (currentDHEID != dhc.data_commode_frame->getDHEId()) {
        if (!(m_suppressErrorMask[c_nrDHE_START_ID])) {
          B2WARNING("DHE ID from DHE Start and this frame do not match");
          B2DEBUG(29, "DHE ID from DHE Start and this frame do not match" <<
                  LogVar("DHEID in this frame $", static_cast < std::ostringstream
                         && >(std::ostringstream() << hex << dhc.data_commode_frame->getDHEId()).str()) <<
                  LogVar("DHEID expected $", static_cast < std::ostringstream && >(std::ostringstream() << hex << currentDHEID).str()));
        }
        m_errorMask[c_nrDHE_START_ID] = true;
      }
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);
      break;
    };
    case EDHCFrameHeaderDataType::c_DHC_START: {
      countedBytesInDHC = 0;
      cancheck_countedBytesInDHC = true;
      if (isFakedData_event != dhc.data_dhc_start_frame->isFakedData()) {
        if (!(m_suppressErrorMask[c_nrFAKE_NO_FAKE_DATA])) B2WARNING("DHC START mixed Fake/no Fake event.");
        m_errorMask[c_nrFAKE_NO_FAKE_DATA] = true;
      }
      if (dhc.data_dhc_start_frame->isFakedData()) {
        if (!(m_suppressErrorMask[c_nrFAKE_NO_DATA_TRIG])) B2WARNING("Faked DHC START Data -> trigger without Data!");
        m_errorMask[c_nrFAKE_NO_DATA_TRIG] = true;
      } else {
        if (m_verbose) dhc.data_dhc_start_frame->print();
      }

//      eventNrOfOnsenTrgFrame = eventNrOfThisFrame;
      currentDHEID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      currentDHCID = dhc.data_dhc_start_frame->get_dhc_id();
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);

      if (m_formatBonnDAQ) eventNrOfOnsenTrgFrame = eventNrOfThisFrame;

      if (!isFakedData_event) {
        /// TODO here we should check full(!) Event Number, Run Number, Subrun Nr and Exp Number
        /// of this frame against the one from MEta Event Info
        if (dhc.data_dhc_start_frame->get_experiment() != m_meta_experiment) {
          if (!(m_suppressErrorMask[c_nrMETA_MM_DHC_ERS])) {
            B2WARNING("DHC-Meta Experiment number mismatch");
            B2DEBUG(29, "DHC-Meta Experiment number mismatch" <<
                    LogVar("DHC exp nr",
                           dhc.data_dhc_start_frame->get_experiment()) <<
                    LogVar("META exp nr" , m_meta_experiment));
          }
          m_errorMask[c_nrMETA_MM_DHC_ERS] = true;
        }
        if (dhc.data_dhc_start_frame->get_run() != m_meta_run_nr) {
          if (!(m_suppressErrorMask[c_nrMETA_MM_DHC_ERS])) {
            B2WARNING("DHC-Meta Run number mismatch");
            B2DEBUG(29, "DHC-Meta Run number mismatch" <<
                    LogVar("DHC Run nr" ,
                           dhc.data_dhc_start_frame->get_run()) <<
                    LogVar("META run nr", m_meta_run_nr));
          }
          m_errorMask[c_nrMETA_MM_DHC_ERS] = true;
        }
        if (dhc.data_dhc_start_frame->get_subrun() != m_meta_subrun_nr) {
          if (!(m_suppressErrorMask[c_nrMETA_MM_DHC_ERS])) {
            B2WARNING("DHC-Meta Sub-Run number mismatch");
            B2DEBUG(29, "DHC-Meta Sub-Run number mismatch" <<
                    LogVar("DHC subrun nr",
                           dhc.data_dhc_start_frame->get_subrun()) <<
                    LogVar("META subrun nr", m_meta_subrun_nr));
          }
          m_errorMask[c_nrMETA_MM_DHC_ERS] = true;
        }
        if ((((unsigned int)dhc.data_dhc_start_frame->getEventNrHi() << 16) | dhc.data_dhc_start_frame->getEventNrLo()) !=
            (m_meta_event_nr & 0xFFFFFFFF)) {
          if (!(m_suppressErrorMask[c_nrMETA_MM_DHC])) {
            B2WARNING("DHC-Meta 32 bit event number mismatch");
            B2DEBUG(29, "DHC-Meta 32 bit event number mismatch" <<
                    LogVar("DHC trigger nr", (((unsigned int) dhc.data_dhc_start_frame->getEventNrHi() << 16) |
                                              dhc.data_dhc_start_frame->getEventNrLo())) <<
                    LogVar("META trigger nr" , (unsigned int)(m_meta_event_nr & 0xFFFFFFFF)));
          }
          m_errorMask[c_nrMETA_MM_DHC] = true;
        }
        uint32_t trig_ticks = (((unsigned int)dhc.data_dhc_start_frame->time_tag_mid & 0x7FFF) << 12) | ((unsigned int)
                              dhc.data_dhc_start_frame->time_tag_lo_and_type >> 4);
        uint32_t trig_sec = (dhc.data_dhc_start_frame->time_tag_hi * 2) ;
        if (dhc.data_dhc_start_frame->time_tag_mid & 0x8000) trig_sec++;

        if ((trig_ticks - m_meta_ticks) != 0 || (trig_sec - m_meta_sec) != 0) {
          m_errorMask[c_nrMETA_MM_DHC_TT] = true;
          if (!(m_suppressErrorMask[c_nrMETA_MM_DHC_TT])) {
            B2WARNING("DHC-Meta TimeTag mismatch");
            B2DEBUG(29, "DHC-Meta TimeTag mismatch" <<
                    LogVar("Header Time $", static_cast < std::ostringstream && >(std::ostringstream() <<
                           hex << dhc.data_dhc_start_frame->time_tag_hi << "." <<
                           dhc.data_dhc_start_frame->time_tag_mid << "." <<
                           dhc.data_dhc_start_frame->time_tag_lo_and_type).str()) <<
                    LogVar("Meta Time $", static_cast < std::ostringstream && >(std::ostringstream() << hex << m_meta_time).str()) <<
                    LogVar("Trigger Type",  static_cast < std::ostringstream
                           && >(std::ostringstream() << hex << (dhc.data_dhc_start_frame->time_tag_lo_and_type & 0xF)).str()) <<
                    LogVar("Meta seconds: $" , static_cast < std::ostringstream && >(std::ostringstream() <<  hex << m_meta_sec).str()) <<
                    LogVar("DHC seconds $" , static_cast < std::ostringstream && >(std::ostringstream() <<  hex << trig_sec).str()) <<
                    LogVar("Seconds difference $" , static_cast < std::ostringstream
                           && >(std::ostringstream() << hex << (trig_sec - m_meta_sec)).str()) <<
                    LogVar("Meta ticks from 127MHz $" , static_cast < std::ostringstream && >(std::ostringstream() <<  hex << m_meta_ticks).str()) <<
                    LogVar("DHC ticks from 127MHz $" , static_cast < std::ostringstream && >(std::ostringstream() << hex << trig_ticks).str()) <<
                    LogVar("Tick difference $" , static_cast < std::ostringstream
                           && >(std::ostringstream() << hex << (trig_ticks - m_meta_ticks)).str()));
          }
        } else {
          B2DEBUG(29, "DHC TT: $" << hex << dhc.data_dhc_start_frame->time_tag_hi << "." << dhc.data_dhc_start_frame->time_tag_mid << "." <<
                  dhc.data_dhc_start_frame->time_tag_lo_and_type << " META " << m_meta_time << " TRG Type " <<
                  (dhc.data_dhc_start_frame->time_tag_lo_and_type & 0xF));
        }
      }
      mask_active_dhe = dhc.data_dhc_start_frame->get_active_dhe_mask();
      nr_active_dhe = nr5bits(mask_active_dhe);

      m_errorMaskDHC = m_errorMask; // forget about anything before this frame
      daqpktstat.newDHC(currentDHCID, m_errorMask);
      daqpktstat.dhc_back().setGatedFlag(dhc.data_dhc_start_frame->get_gated_flag());
      daqpktstat.dhc_back().setGatedHER(dhc.data_dhc_start_frame->get_gated_isher());

      break;
    };
    case EDHCFrameHeaderDataType::c_DHE_START: {
      countedBytesInDHE = 0;
      cancheck_countedBytesInDHE = true;
      m_last_dhp_readout_frame_lo[0] = -1;
      m_last_dhp_readout_frame_lo[1] = -1;
      m_last_dhp_readout_frame_lo[2] = -1;
      m_last_dhp_readout_frame_lo[3] = -1;
      if (m_verbose) dhc.data_dhe_start_frame->print();
      dhe_first_readout_frame_id_lo = dhc.data_dhe_start_frame->getStartFrameNr();
      dhe_first_triggergate = dhc.data_dhe_start_frame->getTriggerGate();
      if (currentDHEID != 0xFFFFFFFF && (currentDHEID & 0xFFFF) >= dhc.data_dhe_start_frame->getDHEId()) {
        if (!(m_suppressErrorMask[c_nrDHE_WRONG_ID_SEQ])) {
          B2WARNING("DHH IDs are not in expected order");
          B2DEBUG(29, "DHH IDs are not in expected order" <<
                  LogVar("Previous ID", (currentDHEID & 0xFFFF)) <<
                  LogVar("Current ID", dhc.data_dhe_start_frame->getDHEId()));
        }
        m_errorMask[c_nrDHE_WRONG_ID_SEQ] = true;
      }
      currentDHEID = dhc.data_dhe_start_frame->getDHEId();
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);

      if (countedDHEStartFrames > countedDHEEndFrames) {
        if (!(m_suppressErrorMask[c_nrDHE_START_WO_END])) B2WARNING("DHE_START without DHE_END");
        m_errorMask[c_nrDHE_START_WO_END] = true;
      }
      countedDHEStartFrames++;

      found_mask_active_dhp = 0;
      mask_active_dhp = dhc.data_dhe_start_frame->getActiveDHPMask();

      if ((((unsigned int)dhc.data_dhe_start_frame->getEventNrHi() << 16) | dhc.data_dhe_start_frame->getEventNrLo()) != (unsigned int)(
            m_meta_event_nr & 0xFFFFFFFF)) {
        if (!(m_suppressErrorMask[c_nrMETA_MM_DHE])) {
          B2WARNING("DHE START trigger mismatch in EVT32b/HI WORD");
          B2DEBUG(29, "DHE START trigger mismatch in EVT32b/HI WORD" <<
                  LogVar("DHE Start trigger nr", (dhc.data_dhe_start_frame->getEventNrHi() << 16) | dhc.data_dhe_start_frame->getEventNrLo()) <<
                  LogVar("Meta trigger nr", (m_meta_event_nr & 0xFFFFFFFF)));
        }
        m_errorMask[c_nrMETA_MM_DHE] = true;
      }
//        B2WARNING("DHE TT: $" << hex << dhc.data_dhe_start_frame->dhe_time_tag_hi << "." << dhc.data_dhe_start_frame->dhe_time_tag_lo <<
//                " META " << m_meta_time);

      if (currentDHEID == 0) {
        if (!(m_suppressErrorMask[c_nrDHE_ID_INVALID])) B2WARNING("DHE ID is invalid=0 (not initialized)");
        m_errorMask[c_nrDHE_ID_INVALID] = true;
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
          if (!(m_suppressErrorMask[c_nrDHE_ID_INVALID])) {
            B2WARNING("DHE ID is invalid");
            B2DEBUG(29, "DHE ID is invalid" <<
                    LogVar("DHE ID", currentDHEID) <<
                    LogVar("Layer", layer) <<
                    LogVar("Ladder", ladder) <<
                    LogVar("Sensor", sensor));
          }
          m_errorMask[c_nrDHE_ID_INVALID] = true;
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
      if (m_verbose) dhc.data_ghost_frame->print();
      if (currentDHEID != dhc.data_ghost_frame->getDHEId()) {
        if (!(m_suppressErrorMask[c_nrDHE_START_ID])) {
          B2WARNING("DHE ID from DHE Start and this frame do not match");
          B2DEBUG(29, "Start ID $" << hex << currentDHEID << " != $" << dhc.data_ghost_frame->getDHEId());
        }
        m_errorMask[c_nrDHE_START_ID] = true;
      }
      /// Attention: Firmware might be changed such, that ghostframe come for all DHPs, not only active ones...
      found_mask_active_dhp |= 1 << dhc.data_ghost_frame->getDHPPort();

      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);

      break;
    case EDHCFrameHeaderDataType::c_DHC_END: {
      if (dhc.data_dhc_end_frame->isFakedData() != isFakedData_event) {
        if (!(m_suppressErrorMask[c_nrFAKE_NO_FAKE_DATA])) B2WARNING("DHC END mixed Fake/no Fake event.");
        m_errorMask[c_nrFAKE_NO_FAKE_DATA] = true;
      }
      if (dhc.data_dhc_end_frame->isFakedData()) {
        if (!(m_suppressErrorMask[c_nrFAKE_NO_DATA_TRIG])) B2WARNING("Faked DHC END Data -> trigger without Data!");
        m_errorMask[c_nrFAKE_NO_DATA_TRIG] = true;
      } else {
        if (m_verbose) dhc.data_dhc_end_frame->print();
      }

      if (!isFakedData_event) {
        if (dhc.data_dhc_end_frame->get_dhc_id() != currentDHCID) {
          if (!(m_suppressErrorMask[c_nrDHC_DHCID_START_END_MM])) {
            B2WARNING("DHC ID Mismatch between Start and End");
            B2DEBUG(29, "DHC ID Mismatch between Start and End $" << std::hex <<
                    currentDHCID << "!=$" << dhc.data_dhc_end_frame->get_dhc_id());
          }
          m_errorMask[c_nrDHC_DHCID_START_END_MM] = true;
        }
        int w;
        w = dhc.data_dhc_end_frame->get_words() * 4;
        if (cancheck_countedBytesInDHC) {
          if (countedBytesInDHC != w) {
            if (!(m_suppressErrorMask[c_nrDHC_WIE])) {
              B2WARNING("Number of Words in DHC END does not match");
              B2DEBUG(29, "Number of Words in DHC END does not match: WIE $" << hex << countedBytesInDHC << " != DHC END $" << hex << w);
            }
            m_errorMask[c_nrDHC_WIE] = true;
          } else {
            if (m_verbose)
              B2DEBUG(29, "EVT END: WIE $" << hex << countedBytesInDHC << " == DHC END $" << hex << w);
          }
          // else ... processed data -> length invalid
        }
      }
      /// TODO how to handle error flags set in in DHC_END?
      if (dhc.data_dhc_end_frame->getErrorInfo() != 0) {
        if (!(m_suppressErrorMask[c_nrDHH_END_ERRORBITS])) B2ERROR("DHC END Error Info set to $" << hex <<
                                                                     dhc.data_dhc_end_frame->getErrorInfo());
        m_errorMask[c_nrDHH_END_ERRORBITS] = true;
      }
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);
      m_errorMaskDHC |= m_errorMask; // do latest updates

      if (daqpktstat.dhc_size() > 0) {
        // only is we have a DHC object... or back() is undefined
        // Remark, if we have a broken data (DHC_START/END) structure, we might fill the
        // previous DHC object ... but then the data is junk anyway
        daqpktstat.dhc_back().setErrorMask(m_errorMaskDHC);
        //B2DEBUG(98,"** DHC "<<currentDHCID<<" Raw"<<dhc.data_dhc_end_frame->get_words() * 4 <<" Red"<<countedBytesInDHC);
        daqpktstat.dhc_back().setCounters(dhc.data_dhc_end_frame->get_words() * 4, countedBytesInDHC);
        daqpktstat.dhc_back().setEndErrorInfo(dhc.data_dhc_end_frame->getErrorInfo());
      }
      m_errorMaskDHC = 0;
      currentDHEID = 0xFFFFFFFF;
      currentDHCID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      break;
    };
    case EDHCFrameHeaderDataType::c_DHE_END: {
      if (m_verbose) dhc.data_dhe_end_frame->print();
      if (currentDHEID != dhc.data_dhe_end_frame->getDHEId()) {
        if (!(m_suppressErrorMask[c_nrDHE_START_END_ID])) {
          B2WARNING("DHE ID from DHE Start and this frame do not match");
          B2DEBUG(29, "DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                  dhc.data_dhe_end_frame->getDHEId());
        }
        m_errorMask[c_nrDHE_START_END_ID] = true;
      }
      /// TODO how to handle error flags set in in DHE_END?
      if (dhc.data_dhe_end_frame->getErrorInfo() != 0) {
        if (!(m_suppressErrorMask[c_nrDHH_END_ERRORBITS])) {
          B2ERROR("DHE END Error Info set to $" << hex <<
                  dhc.data_dhe_end_frame->getErrorInfo());
        }
        m_errorMask[c_nrDHH_END_ERRORBITS] = true;
      }
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);
      if (found_mask_active_dhp != mask_active_dhp) {
        if (!(m_suppressErrorMask[c_nrDHP_ACTIVE])) {
          B2WARNING("DHE_END: DHP active mask differs from found data");
          B2DEBUG(29, "DHE_END: DHP active mask differs from found data $" << hex << mask_active_dhp << " != $" << hex <<
                  found_mask_active_dhp
                  << " mask of found dhp/ghost frames");
        }
        m_errorMask[c_nrDHP_ACTIVE] = true;
      }
      countedDHEEndFrames++;
      if (countedDHEStartFrames < countedDHEEndFrames) {
        // the other case is checked in Start
        if (!(m_suppressErrorMask[c_nrDHE_END_WO_START])) B2WARNING("DHE_END without DHE_START");
        m_errorMask[c_nrDHE_END_WO_START] = true;
      }
      {
        int w;
        w = dhc.data_dhe_end_frame->get_words() * 2;
        if (cancheck_countedBytesInDHE) {
          if (countedBytesInDHE != w) {
            if (!(m_suppressErrorMask[c_nrDHE_WIE])) {
              B2WARNING("Number of Words in DHE END does not match");
              B2DEBUG(29, "Number of Words in DHE END does not match: WIE $" << hex << countedBytesInDHE << " != DHE END $" << hex << w);
            }
            m_errorMask[c_nrDHE_WIE] = true;
          } else {
            if (m_verbose)
              B2DEBUG(29, "EVT END: WIE $" << hex << countedBytesInDHE << " == DHE END $" << hex << w);
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
          daqpktstat.dhc_back().dhe_back().setEndErrorInfo(dhc.data_dhe_end_frame->getErrorInfo());
        }
      }
      m_errorMaskDHE = 0;
      currentDHEID |= 0xFF000000;// differenciate from 0xFFFFFFFFF as initial value
      currentVxdId = 0; /// invalid
      break;
    };
    case EDHCFrameHeaderDataType::c_ONSEN_ROI:
      if (m_verbose) dhc.data_onsen_roi_frame->print();
      dhc.data_onsen_roi_frame->check_error(m_errorMask, len, m_suppressErrorMask[c_nrROI_PACKET_INV_SIZE]);
      dhc.data_onsen_roi_frame->check_inner_crc(m_errorMask,
                                                len - 4); /// CRC is without the DHC header, see reason in function
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);
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
        if (!(m_suppressErrorMask[c_nrMETA_MM_ONS_HLT])) {
          B2WARNING("Trigger Frame HLT Trigger Nr mismatch");
          B2DEBUG(29, "Trigger Frame HLT Trigger Nr mismatch: HLT $" <<
                  dhc.data_onsen_trigger_frame->get_trig_nr1() << " META " << (m_meta_event_nr & 0xFFFFFFFF));
        }
        m_errorMask[c_nrMETA_MM_ONS_HLT] = true;
      }
      if (dhc.data_onsen_trigger_frame->get_experiment1() != m_meta_experiment ||
          dhc.data_onsen_trigger_frame->get_run1() != m_meta_run_nr ||
          dhc.data_onsen_trigger_frame->get_subrun1() != m_meta_subrun_nr) {
        if (!(m_suppressErrorMask[c_nrMETA_MM_ONS_HLT])) {
          B2WARNING("Trigger Frame HLT Exp/Run/Subrun Nr mismatch");
          B2DEBUG(29, "Trigger Frame HLT Exp/Run/Subrun Nr mismatch: Exp HLT $" <<
                  dhc.data_onsen_trigger_frame->get_experiment1() << " META " <<  m_meta_experiment <<
                  " Run HLT $" << dhc.data_onsen_trigger_frame->get_run1()  << " META " << m_meta_run_nr <<
                  " Subrun HLT $" << dhc.data_onsen_trigger_frame->get_subrun1() << " META " <<  m_meta_subrun_nr);
        }
        m_errorMask[c_nrMETA_MM_ONS_HLT] = true;
      }

      if (!dhc.data_onsen_trigger_frame->is_fake_datcon()) {
        if (dhc.data_onsen_trigger_frame->get_trig_nr2() != (unsigned int)(m_meta_event_nr & 0xFFFFFFFF)) {
          if (!(m_suppressErrorMask[c_nrMETA_MM_ONS_DC])) {
            B2WARNING("Trigger Frame DATCON Trigger Nr mismatch");
            B2DEBUG(29, "Trigger Frame DATCON Trigger Nr mismatch: DC $" <<
                    dhc.data_onsen_trigger_frame->get_trig_nr2() << " META " << (m_meta_event_nr & 0xFFFFFFFF));
          }
          m_errorMask[c_nrMETA_MM_ONS_DC] = true;
        }
        if (dhc.data_onsen_trigger_frame->get_experiment2() != m_meta_experiment ||
            dhc.data_onsen_trigger_frame->get_run2() != m_meta_run_nr ||
            dhc.data_onsen_trigger_frame->get_subrun2() != m_meta_subrun_nr) {
          if (!(m_suppressErrorMask[c_nrMETA_MM_ONS_DC])) {
            B2WARNING("Trigger Frame DATCON Exp/Run/Subrun Nr mismatch");
            B2DEBUG(29, "Trigger Frame DATCON Exp/Run/Subrun Nr mismatch: Exp DC $" <<
                    dhc.data_onsen_trigger_frame->get_experiment2() << " META " <<  m_meta_experiment <<
                    " Run DC $" << dhc.data_onsen_trigger_frame->get_run2() << " META " <<  m_meta_run_nr <<
                    " Subrun DC $" << dhc.data_onsen_trigger_frame->get_subrun2() << " META " << m_meta_subrun_nr);
          }
          m_errorMask[c_nrMETA_MM_ONS_DC] = true;
        }
      }

//       B2WARNING("TRG TAG HLT: $" << hex << dhc.data_onsen_trigger_frame->get_trig_tag1() << " DATCON $" <<  dhc.data_onsen_trigger_frame->get_trig_tag2() << " META " << m_meta_time);

      if (m_verbose) dhc.data_onsen_trigger_frame->print();
      dhc.data_onsen_trigger_frame->check_error(m_errorMask, m_suppressErrorMask[c_nrNO_DATCON],
                                                m_suppressErrorMask[c_nrHLTROI_MAGIC],
                                                m_suppressErrorMask[c_nrMERGER_TRIGNR]);
      dhc.check_crc(m_errorMask, m_suppressErrorMask[c_nrDHE_CRC]);
      if (Frame_Number != 0) {
        if (!(m_suppressErrorMask[c_nrEVENT_STRUCT])) B2WARNING("ONSEN TRG Frame must be the first one.");
        m_errorMask[c_nrEVENT_STRUCT] = true;
      }
      isUnfiltered_event = dhc.data_onsen_trigger_frame->is_SendUnfiltered();
      if (isUnfiltered_event) m_sendunfiltered++;
      if (dhc.data_onsen_trigger_frame->is_SendROIs()) m_sendrois++;
      if (!dhc.data_onsen_trigger_frame->is_Accepted()) m_notaccepted++;
      break;
    default:
      if (!(m_suppressErrorMask[c_nrDHC_UNKNOWN])) B2WARNING("UNKNOWN DHC frame type");
      m_errorMask[c_nrDHC_UNKNOWN] = true;
      if (m_verbose) hw->print();
      break;
  }

  if (eventNrOfThisFrame != eventNrOfOnsenTrgFrame && !isFakedData_event) {
    if (!(m_suppressErrorMask[c_nrFRAME_TNR_MM])) {
      B2WARNING("Frame TrigNr != ONSEN Trig Nr");
      B2DEBUG(29, "Frame TrigNr != ONSEN Trig Nr $" << hex << eventNrOfThisFrame << " != $" << eventNrOfOnsenTrgFrame);
    }
    m_errorMask[c_nrFRAME_TNR_MM] = true;
  }

  if (Frame_Number == 0) {
    /// Check that ONSEN Trg is first Frame
    if (frame_type != EDHCFrameHeaderDataType::c_ONSEN_TRG) {
      if (!m_formatBonnDAQ) {
        if (!(m_suppressErrorMask[c_nrONSEN_TRG_FIRST])) B2WARNING("First frame is not a ONSEN Trigger frame");
        m_errorMask[c_nrONSEN_TRG_FIRST] = true;
      }
    }
  } else { // (Frame_Number != 0 &&
    /// Check that there is no other DHC Start
    if (frame_type == EDHCFrameHeaderDataType::c_ONSEN_TRG) {
      if (!(m_suppressErrorMask[c_nrONSEN_TRG_FIRST])) B2WARNING("More than one ONSEN Trigger frame");
      m_errorMask[c_nrONSEN_TRG_FIRST] = true;
    }
  }

  if (!m_formatBonnDAQ) {
    if (Frame_Number == 1) {
      /// Check that DHC Start is first Frame
      if (frame_type != EDHCFrameHeaderDataType::c_DHC_START) {
        if (!(m_suppressErrorMask[c_nrDHC_START_SECOND])) B2WARNING("Second frame is not a DHC start of subevent frame");
        m_errorMask[c_nrDHC_START_SECOND] = true;
      }
    } else { // (Frame_Number != 0 &&
      /// Check that there is no other DHC Start
      if (frame_type == EDHCFrameHeaderDataType::c_DHC_START) {
        if (!(m_suppressErrorMask[c_nrDHC_START_SECOND])) B2WARNING("More than one DHC start of subevent frame");
        m_errorMask[c_nrDHC_START_SECOND] = true;
      }
    }
  }

  if (Frame_Number == Frames_in_event - 1) {
    /// Check that DHC End is last Frame
    if (frame_type != EDHCFrameHeaderDataType::c_DHC_END) {
      if (!(m_suppressErrorMask[c_nrDHC_END_MISS])) B2WARNING("Last frame is not a DHC end of subevent frame");
      m_errorMask[c_nrDHC_END_MISS] = true;
    }

    /// As we now have processed the whole event, we can do some more consistency checks!
    if (countedDHEStartFrames != countedDHEEndFrames || countedDHEStartFrames != nr_active_dhe) {
      if (!(m_suppressErrorMask[c_nrDHE_ACTIVE]) || !(m_suppressErrorMask[c_nrDHE_START_WO_END])
          || !(m_suppressErrorMask[c_nrDHE_END_WO_START])) {
        B2WARNING("The number of DHE Start/End does not match the number of active DHE in DHC Header!");
        B2DEBUG(29, "The number of DHE Start/End does not match the number of active DHE in DHC Header! Header: " << nr_active_dhe <<
                " Start: " << countedDHEStartFrames << " End: " << countedDHEEndFrames << " Mask: $" << hex << mask_active_dhe << " in Event Nr " <<
                eventNrOfThisFrame);
      }
      if (countedDHEStartFrames == countedDHEEndFrames) m_errorMask[c_nrDHE_ACTIVE] = true;
      if (countedDHEStartFrames > countedDHEEndFrames)  m_errorMask[c_nrDHE_START_WO_END] = true;
      if (countedDHEStartFrames < countedDHEEndFrames)  m_errorMask[c_nrDHE_END_WO_START] = true;
    }

  } else { //  (Frame_Number != Frames_in_event - 1 &&
    /// Check that there is no other DHC End
    if (frame_type == EDHCFrameHeaderDataType::c_DHC_END) {
      if (!(m_suppressErrorMask[c_nrDHC_END_DBL])) B2WARNING("More than one DHC end of subevent frame");
      m_errorMask[c_nrDHC_END_DBL] = true;
    }
  }

  if (!m_formatBonnDAQ) {
    /// Check that (if there is at least one active DHE) the second Frame is DHE Start, actually this is redundant if the other checks work
    if (Frame_Number == 2 && nr_active_dhe != 0 && frame_type != EDHCFrameHeaderDataType::c_DHE_START) {
      if (!(m_suppressErrorMask[c_nrDHE_START_THIRD])) B2WARNING("Third frame is not a DHE start frame");
      m_errorMask[c_nrDHE_START_THIRD] = true;
    }
  }

  if (frame_type != EDHCFrameHeaderDataType::c_ONSEN_ROI  && frame_type != EDHCFrameHeaderDataType::c_ONSEN_TRG) {
    // actually, they should not be withing Start and End, but better be sure.
    countedBytesInDHC += len;
    countedBytesInDHE += len;
  }
  B2DEBUG(29, "DHC/DHE $" << hex << countedBytesInDHC << ", $" << hex << countedBytesInDHE);
}

int PXDUnpackerModule::nr5bits(int i)
{
  /// too lazy to count the bits myself, thus using a small lookup table
  const int lut[32] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5
  };
  return lut[i & 0x1F];
}

