/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/unpacking/PXDRawDataDefinitions.h>
#include <pxd/modules/pxdUnpacking/PXDPackerErrModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/algorithm/clamp.hpp>

#include <boost/spirit/home/support/detail/endian.hpp>

#include <pxd/unpacking/PXDMappingLookup.h>


#include <TRandom.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::PXD::PXDError;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDPackerErr)


using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhc_crc_32_type;

///******************************************************************
///*********************** Main packer code *************************
///******************************************************************

bool PXDPackerErrModule::CheckErrorMaskInEvent(unsigned int eventnr, PXDErrorFlags mask)
{
  /** Check that at least the expected error bit are set, there could be more ... */
  static std::vector <PXDErrorFlags> errors =  {
    // Event 0 does not exist...
    c_NO_ERROR,
    /*  1 - HLT-Onsen Trigger differs from Meta (meta changed! thus all differ)
     *  2 - HLT-Onsen Trigger is zero
     *  3 - Run Nr differs between HLT-Onsen and Meta
     *  4 - Subrun Nr differs between HLT-Onsen and Meta
     *  5 - Exp differs between HLT-Onsen and Meta */
    c_META_MM | c_META_MM_DHC | c_META_MM_DHE | c_META_MM_ONS_HLT | c_META_MM_ONS_DC,
    c_META_MM | c_META_MM_DHC | c_META_MM_DHE | c_META_MM_ONS_HLT | c_META_MM_ONS_DC,
    c_META_MM_DHC_ERS | c_META_MM_ONS_HLT | c_META_MM_ONS_DC,
    c_META_MM_DHC_ERS | c_META_MM_ONS_HLT | c_META_MM_ONS_DC,
    c_META_MM_DHC_ERS | c_META_MM_ONS_HLT | c_META_MM_ONS_DC,
    /*  6 - Wrong ONS Trigger frame length
     *  7 - Wrong DHC Start frame length
     *  8 - Wrong DHC End frame length
     *  9 - Wrong DHE Start frame length
     * 10 - Wrong DHE End frame length */
    c_FIX_SIZE, // TODO why mismatch?
    c_FIX_SIZE, // TODO Problem
    c_FIX_SIZE, // TODO why is DHC End missing not detected?
    c_FIX_SIZE, // TODO why is DHE Start missing not detected?
    c_FIX_SIZE, // TODO why is DHE End missing not detected?
    /* 11 - Wrong DHE Start frame length (by 2 bytes), unalign 32 bit frame
     * 12 - Missing ONS Trig frame
     * 13 - Missing DHC Start frame
     * 14 - Missing DHC End frame
     * 15 - Missing DHE Start frame */
    c_FRAME_SIZE, // TODO can check more errors
    c_ONSEN_TRG_FIRST | c_DHC_START_SECOND | c_DHE_START_THIRD, // TODO
    c_DHC_START_SECOND, // TODO
    c_DHC_END_MISS,
    c_DHE_END_WO_START, // TODO
    /* 16 - Missing DHE End frame
     * 17 - Double ONS Trig frame
     * 18 - Double DHC Start frame
     * 19 - Double DHC End frame
     * 20 - Double DHE Start frame*/
    c_DHE_START_WO_END, // TODO if two DHE, another error condition shoudl trigger, too
    c_ONSEN_TRG_FIRST | c_DHC_START_SECOND | c_DHE_START_THIRD, // TODO why data outside of ...
    c_DHC_START_SECOND | c_DHE_START_THIRD, // TODO why data outside of ...
    c_DHC_END_DBL, // TODO
    c_DHE_WRONG_ID_SEQ | c_DHE_START_WO_END, // TODO
    /* 21 - Double DHE End frame
     * 22 - DATCON triggernr+1
     * 23 - HLT Magic broken
     * 24 - DATCON Magic broken
     * 25 - HLT with Accepted not set */
    c_DHE_END_WO_START | c_DHE_START_END_ID, // TODO
    c_MERGER_TRIGNR | c_META_MM_ONS_DC,
    c_HLTROI_MAGIC,
    c_HLTROI_MAGIC,
    c_NO_ERROR | c_NOTSENDALL_TYPE, // TODO =============================================== CHECK TODO Unpacker is not detecting this yet
    /* 26 - HLT triggernr+1
     * 27 - CRC error in second frame (DHC start)
     * 28 - data for all DHE even if disabled in mask
     * 29 - no DHE at all, even so mask tell us otherwise
     * 30 - no DHC at all */
    c_MERGER_TRIGNR | c_META_MM_ONS_HLT,
    c_DHE_CRC, // TODO why DHE start missing not detected?
    c_DHE_ACTIVE, // TODO
    c_DHE_START_THIRD | c_DHE_ACTIVE,
    c_DHC_END_MISS, // TODO need some better checks
    /* 31 - DHC end has wrong DHC id
     * 32 - DHE end has wrong DHE id
     * 33 - DHC wordcount wrong by 4 bytes
     * 34 - DHE wordcount wrong by 4 bytes
     * 35 - DHE Trigger Nr Hi word messed up */
    c_DHC_DHCID_START_END_MM,
    c_DHE_START_END_ID | c_DHE_WRONG_ID_SEQ | c_DHE_ID_INVALID,
    c_DHC_WIE,
    c_DHE_WIE,
    c_META_MM_DHE,
    /* 36 - DHC Trigger Nr Hi word messed up
     * 37 - DHP data, even if mask says no DHP TODO Check
     * 38 - No DHP data, even if mask tell otherwise
     * 39 - DHE id differ in DHE and DHP header
     * 40 - Chip ID differ in DHE and DHP header */
    c_META_MM_DHC,
    EPXDErrMask::c_NO_ERROR, // TODO
    EPXDErrMask::c_NO_ERROR, // TODO
    EPXDErrMask::c_NO_ERROR, // TODO
    EPXDErrMask::c_NO_ERROR, // TODO
    /* 41 - Row overflow by 1
     * 42 - Col overflow by 1
     * 43 - Missing Start Row (Pixel w/o row)
     * 44 - No PXD raw packet at all
     * 45 - No DATCON data */
    EPXDErrMask::c_NO_ERROR, // TODO
    EPXDErrMask::c_NO_ERROR, // TODO
    EPXDErrMask::c_NO_ERROR, // TODO
    EPXDErrMask::c_NO_ERROR, // TODO
    EPXDErrMask::c_NO_ERROR, // TODO
    /* 46 - unused frame type: common mode
     * 47 - unused frame type: FCE
     * 48 - unused frame type: ONS FCE
     * 49 - unused frame type: 0x7, 0x8, 0xA
     * 50 - Rows w/o Pixel (several rows after each other) */
    EPXDErrMask::c_UNEXPECTED_FRAME_TYPE, // TODO
    EPXDErrMask::c_UNEXPECTED_FRAME_TYPE, // TODO
    EPXDErrMask::c_UNEXPECTED_FRAME_TYPE, // TODO
    EPXDErrMask::c_UNEXPECTED_FRAME_TYPE, // TODO
    EPXDErrMask::c_DHP_ROW_WO_PIX, // TODO
    /* 51 - NO ERROR
     *
     *
     * */
    EPXDErrMask::c_NO_ERROR,
  };

  PXDErrorFlags expected = c_NO_ERROR;
  if (eventnr > 0 && eventnr < errors.size()) {
    expected = errors[eventnr];
  }
  B2INFO("-- PXD Packer Error Check for Event Nr: " << eventnr);
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
    uint64_t m;
    m = 1ull << i; // ull is important!
    if ((m & (mask | expected)) == m) {
      if ((m & expected) == m && (m & mask) != m) {
        B2ERROR("Bit " << i << ": Was NOT Set: " << getPXDBitErrorName(i));
        m_found_fatal = true;
      } else if ((m & expected) != m && (m & mask) == m) {
        B2RESULT("Bit " << i << ": Optional   : " << getPXDBitErrorName(i));
      } else if ((m & expected) == m && (m & mask) == m) {
        B2INFO("Bit " << i << ": As Expected: " << getPXDBitErrorName(i));
      }
    }
  }
  bool flag = (mask & expected) == expected;
  if (expected == EPXDErrMask::c_NO_ERROR) {
    // special check, this event should not contain any error!
    if (mask != EPXDErrMask::c_NO_ERROR) {
      B2ERROR("There should be no error in this event, but there were (see above)!");
      m_found_fatal = true;
    }
    flag = (mask == EPXDErrMask::c_NO_ERROR);
  }
  B2INFO("-- PXD Packer Error Check END --- ");
  if (m_found_fatal) B2FATAL("At least one of the checks failed (see details above)!");
  return flag;
}

PXDPackerErrModule::PXDPackerErrModule() :
  Module(),
  m_storeRaws()
{
  m_trigger_nr = 0;
  m_packed_events = 0;
  m_run_nr_word1 = 0;
  m_run_nr_word2 = 0;
  //Set module properties
  setDescription("Create 'broken' PXD raw data to challenge Unpacker");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("RawPXDsName", m_RawPXDsName, "The name of the StoreArray of generated RawPXDs", std::string(""));
  addParam("dhe_to_dhc", m_dhe_to_dhc,  "DHE to DHC mapping (DHC_ID, DHE1, DHE2, ..., DHE5) ; -1 disable port");
  addParam("InvertMapping",  m_InvertMapping, "Use invers mapping to DHP row/col instead of \"remapped\" coordinates", false);
  addParam("Clusterize",  m_Clusterize, "Use clusterizer (FCE format)", false);
  addParam("Check",  m_Check, "Check the result of Unpacking", false);
  addParam("PXDDAQEvtStatsName", m_PXDDAQEvtStatsName, "The name of the StoreObjPtr of read PXDDAQEvtStats", std::string(""));

}

void PXDPackerErrModule::initialize()
{
  m_found_fatal = false;
  B2DEBUG(20, "PXD Packer Err --> Init");
  if (m_Check) {
    m_daqStatus.isRequired();
  } else {
    //Register output collections
    m_storeRaws.registerInDataStore(m_RawPXDsName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
    m_packed_events = 0;

    /// read in the mapping for ONSEN->DHC->DHE->DHP
    /// until now ONSEN->DHC is not needed yet (might be based on event numbers per event)
    /// DHE->DHP is only defined by port number/active mask ... not implemented yet.
    for (auto& it : m_dhe_to_dhc) {
      bool flag;
      int dhc_id;
      B2DEBUG(20, "PXD Packer Err --> DHC/DHE");
      flag = false;
      if (it.size() != 6) {
        /// means [ 1 2 3 4 5 -1 ] DHC 1 has DHE 2,3,4,5 on port 0-3 and nothing on port 4
        B2WARNING("PXD Packer Err --> DHC/DHE maps 1 dhc to 5 dhe (1+5 values), but I found " << it.size());
      }
      for (auto& it2 : it) {
        if (flag) {
          int v;
          v = it2;
          B2DEBUG(20, "PXD Packer Err --> ... DHE " << it2);
          if (it2 < -1 || it2 >= 64) {
            if (it2 != -1) B2ERROR("PXD Packer Err --> DHC id " << it2 << " is out of range (0-64 or -1)! disable channel.");
            v = -1;
          }
          m_dhc_mapto_dhe[dhc_id].push_back(v);
        } else {
          dhc_id = it2;
          B2DEBUG(20, "PXD Packer Err --> DHC .. " << it2);
          if (dhc_id < 0 || dhc_id >= 16) {
            B2ERROR("PXD Packer Err --> DHC id " << it2 << " is out of range (0-15)! skip");
            break;
          }
        }
        flag = true;
      }
    }
    B2DEBUG(20, "PXD Packer Err --> DHC/DHE done");

    for (auto& it : m_dhc_mapto_dhe) {
      int port = 0;
      B2DEBUG(20, "PXD Packer Err --> DHC " << it.first);
      for (auto& it2 : it.second) {
        B2DEBUG(20, "PXD Packer Err --> .. connects to DHE " << it2 << " port " << port);
        port++;
      }
    }
  }
}

void PXDPackerErrModule::terminate()
{
  if (m_found_fatal) B2FATAL("At least one of the checks failed (see details above)!");
}

void PXDPackerErrModule::event()
{
  B2DEBUG(20, "PXD Packer Err --> Event");
  StoreObjPtr<EventMetaData> evtPtr;

  m_real_trigger_nr = m_trigger_nr = evtPtr->getEvent();

  if (m_Check) {
    CheckErrorMaskInEvent(m_trigger_nr, m_daqStatus->getErrorMask());
  } else {

    // First, throw the dices for a few event-wise properties

    m_trigger_dhp_framenr = gRandom->Integer(0x10000);
    m_trigger_dhe_gate = gRandom->Integer(192);


    if (isErrorIn(1)) m_trigger_nr += 10;
    if (isErrorIn(2)) m_trigger_nr = 0;

    uint32_t run = evtPtr->getRun();
    uint32_t exp = evtPtr->getExperiment();
    uint32_t sub = evtPtr->getSubrun();

    if (isErrorIn(3)) run++;
    if (isErrorIn(4)) sub++;
    if (isErrorIn(5)) exp++;

    m_run_nr_word1 = ((run & 0xFF) << 8) | (sub & 0xFF);
    m_run_nr_word2 = ((exp & 0x3FF) << 6) | ((run >> 8) & 0x3F);
    m_meta_time = evtPtr->getTime();

    B2INFO("Pack Event : " << evtPtr->getEvent() << ","  << evtPtr->getRun() << "," << evtPtr->getSubrun() << "," <<
           evtPtr->getExperiment() << "," << evtPtr->getTime() << " (MetaInfo)");

    if (!isErrorIn(44)) {
      pack_event();
      m_packed_events++;
    }
  }
}

void PXDPackerErrModule::endian_swap_frame(unsigned short* dataptr, int len)
{
  boost::spirit::endian::ubig16_t* p = (boost::spirit::endian::ubig16_t*)dataptr;

  /// swap endianess of all shorts in frame BUT not the CRC (2 shorts)
  for (int i = 0; i < len / 2 - 2; i++) {
    p[i] = dataptr[i];// Endian Swap! (it doesnt matter if you swap from little to big or vice versa)
  }
}

void PXDPackerErrModule::pack_event(void)
{
  int dhe_ids[5] = {0, 0, 0, 0, 0};
  B2DEBUG(20, "PXD Packer Err --> pack_event");

  // loop for each DHC in system
  // get active DHCs from a database?
  for (auto& it : m_dhc_mapto_dhe) {
    int port = 1, port_inx = 0;
    int act_port = 0;

    for (auto& it2 : it.second) {
      if (it2 >= 0) act_port += port;
      port += port;
      dhe_ids[port_inx] = it2;
      port_inx++;
      if (port_inx == 5) break; // not more than five.. checked above
    }

    //  if(act_port&0x1F) B2ERROR();... checked above
    //  act_port&=0x1F;... checked above

    // get active DHE mask from a database?

    m_onsen_header.clear();// Reset
    m_onsen_payload.clear();// Reset
    pack_dhc(it.first, act_port, dhe_ids);
    // and write to PxdRaw object
    // header will be finished and endian swapped by constructor; payload already has be on filling the vector
    m_storeRaws.appendNew(m_onsen_header, m_onsen_payload);
  }

}

void PXDPackerErrModule::add_frame_to_payload(void)
{
  if (m_current_frame.size() & 0x3) {
    B2WARNING("Frame is not 32bit aligned!!! Unsupported by Unpacker!");
  }
  // checksum frame
  dhc_crc_32_type current_crc;
  current_crc.process_bytes(m_current_frame.data(), m_current_frame.size());
  if (isErrorIn(27) and m_onsen_header.size() == 1) current_crc.process_byte(0x42); // mess up CRC in second frame
  append_int32(current_crc.checksum());

  // and add it
  m_onsen_header.push_back(m_current_frame.size());
  m_onsen_payload.push_back(m_current_frame);
}

void PXDPackerErrModule::append_int8(unsigned char w)
{
  m_current_frame.push_back(w);
  dhe_byte_count++;
  dhc_byte_count++;
}

void PXDPackerErrModule::append_int16(unsigned short w)
{
  m_current_frame.push_back((unsigned char)(w >> 8));
  m_current_frame.push_back((unsigned char)(w));
  dhe_byte_count += 2;
  dhc_byte_count += 2;
}

void PXDPackerErrModule::append_int32(unsigned int w)
{
  m_current_frame.push_back((unsigned char)(w >> 24));
  m_current_frame.push_back((unsigned char)(w >> 16));
  m_current_frame.push_back((unsigned char)(w >> 8));
  m_current_frame.push_back((unsigned char)(w));
  dhe_byte_count += 4;
  dhc_byte_count += 4;
}

// void PXDPackerErrModule::remove_back_int32(void)
// {
//   m_current_frame.pop_back();
//   m_current_frame.pop_back();
//   m_current_frame.pop_back();
//   m_current_frame.pop_back();
//   dhe_byte_count -= 4;
//   dhc_byte_count -= 4;
// }

void PXDPackerErrModule::start_frame(void)
{
  m_current_frame.clear();
}

void PXDPackerErrModule::pack_dhc(int dhc_id, int dhe_active, int* dhe_ids)
{
  B2DEBUG(20, "PXD Packer Err --> pack_dhc ID " << dhc_id << " DHE act: " << dhe_active);

  bool m_send_all = true;
  bool m_send_roi = false;

  /// HLT frame ??? format still t.b.d. TODO
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_ONSEN_TRG << 27) | (m_trigger_nr & 0xFFFF));
  B2INFO("Trigger packet: $" << hex << m_run_nr_word1 << ", $" << m_run_nr_word2);
  if (isErrorIn(23)) {
    append_int32(0xDEAD8000);
  } else if (isErrorIn(25)) {
    append_int32(0xCAFE0000);// HLT HEADER, NO accepted flag
  } else {
    append_int32(0xCAFE8000 | (m_send_all ? 0x4000 : 0) | (m_send_roi ? 0x2000 : 0)); // HLT HEADER, accepted flag set
  }
  if (isErrorIn(26)) {
    append_int32(m_trigger_nr + 1);
  } else if (isErrorIn(36)) {
    append_int32(0x10000000 | m_trigger_nr); // HLT Trigger Nr with Hi work messed up
  } else {
    append_int32(m_trigger_nr); // HLT Trigger Nr
  }
  append_int16(m_run_nr_word2); // HLT Run NR etc
  append_int16(m_run_nr_word1); // HLT Run NR etc
  if (isErrorIn(24)) append_int32(0xDEAD0000);
  else append_int32(0xCAFE0000);// DATCON HEADER ...
  if (isErrorIn(45)) {
    // No Datcon
    append_int32(0x0); // trigger nr
    append_int32(0x0); // 2*16 trg tag
  } else {
    if (isErrorIn(22)) append_int32(m_trigger_nr + 1);
    else append_int32(m_trigger_nr);// DATCON Trigger Nr
    if (!isErrorIn(6)) {
      append_int16(m_run_nr_word2);  // DATCON Run NR etc
      append_int16(m_run_nr_word1);  // DATCON Run NR etc
    }
  }
  if (!isErrorIn(12)) add_frame_to_payload();
  if (isErrorIn(17)) {
    // double frame
    m_onsen_header.push_back(m_current_frame.size());
    m_onsen_payload.push_back(m_current_frame);
  }

  if (isErrorIn(30)) return; // empty event, no DHC frames

  /// DHC Start

  dhc_byte_count = 0;
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_DHC_START << 27) | ((dhc_id & 0xF) << 21) | ((dhe_active & 0x1F) << 16) |
               (m_trigger_nr & 0xFFFF));
  append_int16(m_trigger_nr >> 16);

  uint32_t mm = (unsigned int)((m_meta_time % 1000000000ull) * 0.127216 + 0.5);
  append_int16(((mm << 4) & 0xFFF0) | 0x1); // TT 11-0 | Type --- fill with something usefull TODO
  append_int16((mm >> 12) & 0xFFFF); // TT 27-12 ... not clear if completely filled by DHC
  append_int16((mm >> 28) & 0xFFFF); // TT 43-28 ... not clear if completely filled by DHC
  if (!isErrorIn(7)) {
    append_int16(m_run_nr_word1); // Run Nr 7-0 | Subrunnr 7-0
    append_int16(m_run_nr_word2);  // Exp NR 9-0 | Run Nr 13-8
  }
  if (!isErrorIn(13)) add_frame_to_payload();
  if (isErrorIn(18)) {
    // double frame
    m_onsen_header.push_back(m_current_frame.size());
    m_onsen_payload.push_back(m_current_frame);
  }

  // loop for each DHE in system
  // Run & TTT etc are zero until better idea

  if (isErrorIn(29)) dhe_active = 0; // dont send any DHE
  for (int i = 0; i < 5; i++) {
    if ((dhe_active & 0x1) or isErrorIn(28)) pack_dhe(dhe_ids[i], 0xF);
    dhe_active >>= 1;
  }

  /// lets copy the HLT/ROI frame

  //  start_frame();
  //  append_int32((EDHCFrameHeaderDataType::c_ONSEN_ROI<<27) | (m_trigger_nr & 0xFFFF));
  //  add_frame_to_payload();

  /// DHC End
  if (isErrorIn(31)) dhc_id += 1;
  if (isErrorIn(33)) dhc_byte_count += 4;
  unsigned int dlen = (dhc_byte_count / 4); // 32 bit words
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_DHC_END << 27) | ((dhc_id & 0xF) << 21) | (m_trigger_nr & 0xFFFF));
  append_int32(dlen); // 32 bit word count
  if (!isErrorIn(8)) append_int32(0x00000000);  // Error Flags
  if (!isErrorIn(14)) add_frame_to_payload();
  if (isErrorIn(19)) {
    // double frame
    m_onsen_header.push_back(m_current_frame.size());
    m_onsen_payload.push_back(m_current_frame);
  }
}

void PXDPackerErrModule::pack_dhe(int dhe_id, int dhp_active)
{
  if (isErrorIn(37)) dhp_active = 0; // mark as no DHP, but send them (see below)
  B2DEBUG(20, "PXD Packer Err --> pack_dhe ID " << dhe_id << " DHP act: " << dhp_active);
  // dhe_id is not dhe_id ...
  bool dhe_has_remapped = !m_InvertMapping; /// unless stated otherwise, DHH will not reformat coordinates

  if (m_InvertMapping) {
    // problem, we do not have an exact definition of if this bit is set in the new firmware and under which circumstances
    // and its not clear if we have to translate the coordinates back to "DHP" layout! (look up tabel etc!)
    B2FATAL("Inverse Mapping not implemented in Packer");
  }

  /// DHE Start
  dhe_byte_count = 0;
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_DHE_START << 27) | ((dhe_id & 0x3F) << 20) | ((dhp_active & 0xF) << 16) |
               (m_trigger_nr & 0xFFFF));
  if (isErrorIn(35)) append_int16(0x1000);
  else append_int16(m_trigger_nr >> 16); // Trigger Nr Hi
  if (!isErrorIn(11)) {
    append_int16(0x00000000);  // DHE Timer Lo
  }
  if (!isErrorIn(9)) {
    append_int16(0x00000000);  // DHE Time Hi
    append_int16(((m_trigger_dhp_framenr & 0x3F) << 10) |
                 (m_trigger_dhe_gate & 0xFF)); // Last DHP Frame Nr 15-10, Reserved 9-8, Trigger Offset 7-0
  }
  if (!isErrorIn(15)) add_frame_to_payload();
  if (isErrorIn(20)) {
    // double frame
    m_onsen_header.push_back(m_current_frame.size());
    m_onsen_payload.push_back(m_current_frame);
  }

  // Now we add some undefined things
  if (isErrorIn(46)) {
    start_frame();
    append_int32((EDHCFrameHeaderDataType::c_COMMODE << 27) | ((dhe_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
    add_frame_to_payload();
  }
  if (isErrorIn(47)) {
    start_frame();
    append_int32((EDHCFrameHeaderDataType::c_FCE_RAW << 27) | ((dhe_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
    add_frame_to_payload();
  }
  if (isErrorIn(48)) {
    start_frame();
    append_int32((EDHCFrameHeaderDataType::c_ONSEN_FCE << 27) | ((dhe_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
    add_frame_to_payload();
  }
  if (isErrorIn(49)) {
    start_frame();
    append_int32((EDHCFrameHeaderDataType::c_UNUSED_7 << 27) | ((dhe_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
    add_frame_to_payload();
    start_frame();
    append_int32((EDHCFrameHeaderDataType::c_UNUSED_8 << 27) | ((dhe_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
    add_frame_to_payload();
    start_frame();
    append_int32((EDHCFrameHeaderDataType::c_UNUSED_A << 27) | ((dhe_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
    add_frame_to_payload();
  }

// now prepare the data from one halfladder
// do the ROI selection??? optional...
// then loop for each DHP in system
// get active DHPs from a database?
// and pack data per halfladder.
// we fake the framenr and startframenr until we find some better solution

  if (dhp_active != 0) { /// is there any hardware switched on?
    // const int ladder_min_row = 0; Hardware counts from 0, only include if it does not.
    const int ladder_max_row = PACKER_NUM_ROWS - 1;
    // const int ladder_min_col = 0;
    const int ladder_max_col = PACKER_NUM_COLS - 1;

    /// clear pixelmap
    bzero(halfladder_pixmap, sizeof(halfladder_pixmap));

    /// refering to BelleII Note Nr 0010, the numbers run from ... to
    ///   unsigned int layer, ladder, sensor;
    ///   layer= vxdid.getLayerNumber();/// 1 ... 2
    ///   ladder= vxdid.getLadderNumber();/// 1 ... 8 and 1 ... 12
    ///   sensor= vxdid.getSensorNumber();/// 1 ... 2
    ///   dhe_id = ((layer-1)<<5) | ((ladder)<<1) | (sensor-1);
    unsigned short sensor, ladder, layer;
    sensor = (dhe_id & 0x1) + 1;
    ladder = (dhe_id & 0x1E) >> 1; // no +1
    layer = ((dhe_id & 0x20) >> 5) + 1;

    B2DEBUG(20, "pack_dhe: VxdId: " << VxdID(layer, ladder, sensor) << " " << (int)VxdID(layer, ladder, sensor));

    // Create some fixed hits (not too many, jut a few per ladder)
    for (auto j = 0; j < 10; j++) {
      unsigned int row, col;
      unsigned char charge;
      row = j * 23;
      col = j * 23;
      charge = 20 + j; // 0-255
      if (!dhe_has_remapped) {
        do_the_reverse_mapping(row, col, layer, sensor);
      }
      halfladder_pixmap[row][col] = charge;
    }

    if (isErrorIn(38)) dhp_active = 0; // no DHP data even if we expect it
    if (m_Clusterize) {
      B2FATAL("Clusterizer not supported in Packer");
    } else {
      for (int i = 0; i < 4; i++) {
        if ((dhp_active & 0x1) or isErrorIn(37)) {
          pack_dhp(i, dhe_id, dhe_has_remapped ? 1 : 0);
          /// The following lines "simulate" a full frame readout frame ... not for production yet!
//         if (m_trigger_nr == 0x11) {
//           pack_dhp_raw(i, dhe_id, false);
//           pack_dhp_raw(i, dhe_id, true);
//         }
        }
        dhp_active >>= 1;
      }
    }
  }

  /// DHE End
  if (isErrorIn(32)) dhe_id += 1;
  if (isErrorIn(34)) dhe_byte_count += 4;
  unsigned int dlen = (dhe_byte_count / 2); // 16 bit words
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_DHE_END << 27) | ((dhe_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
  append_int16(dlen & 0xFFFF); // 16 bit word count
  append_int16((dlen >> 16) & 0xFFFF); // 16 bit word count
  if (!isErrorIn(10)) append_int32(0x00000000);   // Error Flags
  if (!isErrorIn(16)) add_frame_to_payload();
  if (isErrorIn(21)) {
    // double frame
    m_onsen_header.push_back(m_current_frame.size());
    m_onsen_payload.push_back(m_current_frame);
  }
}

void PXDPackerErrModule::do_the_reverse_mapping(unsigned int& /*row*/, unsigned int& /*col*/, unsigned short /*layer*/,
                                                unsigned short /*sensor*/)
{
  B2FATAL("code needs to be written");
  // work to be done
  //
  // PXDMappingLookup::map_uv_to_rc_IF_OB(unsigned int& v_cellID, unsigned int& u_cellID, unsigned int& dhp_id, unsigned int dhe_ID)
  // PXDMappingLookup::map_uv_to_rc_IB_OF(unsigned int& v_cellID, unsigned int& u_cellID, unsigned int& dhp_id, unsigned int dhe_ID)
}

void PXDPackerErrModule::pack_dhp_raw(int chip_id, int dhe_id)
{
  B2DEBUG(20, "PXD Packer Err --> pack_dhp Raw Chip " << chip_id << " of DHE id: " << dhe_id);
  start_frame();
  /// DHP data Frame
  append_int32((EDHCFrameHeaderDataType::c_DHP_RAW << 27) | ((dhe_id & 0x3F) << 20) | ((chip_id & 0x03) << 16) |
               (m_trigger_nr & 0xFFFF));
  append_int32((EDHPFrameHeaderDataType::c_RAW << 29) | ((dhe_id & 0x3F) << 18) | ((chip_id & 0x03) << 16) |
               (m_trigger_dhp_framenr & 0xFFFF));

  int c1, c2;
  c1 = 64 * chip_id;
  c2 = c1 + 64;
  if (c2 >= PACKER_NUM_COLS) c2 = PACKER_NUM_COLS;

  // ADC data / memdump for pedestal calculation
  for (int row = 0; row < PACKER_NUM_ROWS; row++) {
    for (int col = c1; col < c2; col++) {
      append_int8(halfladder_pixmap[row][col]);
    }
    // unconnected drain lines -> 0
    for (int col = c2; col < c1 + 64; col++) {
      append_int8(0);
    }
  }

  add_frame_to_payload();
}

void PXDPackerErrModule::pack_dhp(int chip_id, int dhe_id, int dhe_has_remapped)
{
  B2DEBUG(20, "PXD Packer Err --> pack_dhp Chip " << chip_id << " of DHE id: " << dhe_id);
  // remark: chip_id != port most of the time ...
  bool empty = true;
  unsigned short last_rowstart = 0;
  bool error_done = false;

  if (dhe_has_remapped == 0) {
    // problem, we do not have an exact definition of if this bit is set in the new firmware and under which circumstances
    // and its not clear if we have to translate the coordinates back to "DHP" layout! (look up tabel etc!)
    assert(dhe_has_remapped == 0);
  }

  start_frame();
  /// DHP data Frame
  append_int32((EDHCFrameHeaderDataType::c_DHP_ZSD << 27) | ((dhe_id & 0x3F) << 20) | ((dhe_has_remapped & 0x1) << 19) | ((
                 chip_id & 0x03) << 16) | (m_trigger_nr & 0xFFFF));
  if (isErrorIn(39)) dhe_id = 0; // mess up dhe_id in DHP header
  if (isErrorIn(40)) chip_id = ~chip_id; // mess up chip id
  append_int32((EDHPFrameHeaderDataType::c_ZSD << 29) | ((dhe_id & 0x3F) << 18) | ((chip_id & 0x03) << 16) |
               (m_trigger_dhp_framenr & 0xFFFF));

  if (isErrorIn(41) or isErrorIn(42)) {
    halfladder_pixmap[PACKER_NUM_ROWS - 1][PACKER_NUM_COLS - 1] = 0x42; // make sure we have a hit
  }
  for (int row = 0; row < PACKER_NUM_ROWS; row++) { // should be variable
    bool rowstart;
    if (isErrorIn(43) && !error_done) rowstart = false;
    else rowstart = true;
    int c1, c2;
    c1 = 64 * chip_id;
    c2 = c1 + 64;
    if (c2 >= PACKER_NUM_COLS) c2 = PACKER_NUM_COLS;
    for (int col = c1; col < c2; col++) {
      if (halfladder_pixmap[row][col] != 0) {
        unsigned char charge;
        charge = halfladder_pixmap[row][col] & 0xFF; // before we make overflow error
        if (isErrorIn(41) and row == PACKER_NUM_ROWS - 1 and col == PACKER_NUM_COLS - 1 and !error_done) {
          row = PACKER_NUM_ROWS;
          rowstart = true; // force a new row
          error_done = true;
        }
        if (isErrorIn(42) and row == PACKER_NUM_ROWS - 1 and col == PACKER_NUM_COLS - 1 and !error_done) {
          col = PACKER_NUM_COLS; // masked out to modulo 64 anway
          error_done = true;
        }
        if (rowstart) {
          last_rowstart = ((row & 0x3FE) << (6 - 1)) | 0; // plus common mode 6 bits ... set to 0
          append_int16(last_rowstart);
          rowstart = false;
        }
        int colout = col;
        if (!isErrorIn(50)) append_int16(0x8000 | ((row & 0x1) << 14) | ((colout & 0x3F) << 8) | charge);
        empty = false;
      }
    }
  }
  if (!empty && (m_current_frame.size() & 0x3)) {
    B2DEBUG(20, "Repeat last rowstart to align to 32bit.");
    append_int16(last_rowstart);
  }


  if (empty) {
    B2DEBUG(20, "Found no data for halfladder! DHEID: " << dhe_id << " Chip: " << chip_id);
    // we DROP the frame, thus we have to correct DHE and DHC counters
    dhc_byte_count -= 8; // fixed size of Header
    dhe_byte_count -= 8; // fixed size of Header
    start_frame();
    /// Ghost Frame ... start frame overwrites frame info set above
    append_int32((EDHCFrameHeaderDataType::c_GHOST << 27) | ((dhe_id & 0x3F) << 20) | ((chip_id & 0x03) << 16) |
                 (m_trigger_nr & 0xFFFF));
  } else {
    //B2DEBUG(20,"Found data for halfladder DHEID: " << dhe_id << " Chip: " << chip_id);
  }
  add_frame_to_payload();

}

bool PXDPackerErrModule::isErrorIn(uint32_t enr)
{
  if (m_real_trigger_nr == enr) return true; // oder modulo, oder oder
  return false;
}
