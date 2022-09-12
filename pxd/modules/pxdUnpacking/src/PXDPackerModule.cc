/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/unpacking/PXDRawDataDefinitions.h>
#include <pxd/modules/pxdUnpacking/PXDPackerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <boost/crc.hpp>
#include <boost/algorithm/clamp.hpp>

#include <pxd/unpacking/PXDMappingLookup.h>


#include <TRandom.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDPacker);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhe_crc_32_type;

///******************************************************************
///*********************** Main packer code *************************
///******************************************************************

PXDPackerModule::PXDPackerModule() :
  Module(),
  m_storeRaws()
{
  m_trigger_nr = 0;
  m_packed_events = 0;
  m_run_nr_word1 = 0;
  m_run_nr_word2 = 0;
  //Set module properties
  setDescription("Pack PXD Hits to raw data object");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("PXDDigitsName", m_PXDDigitsName, "The name of the StoreArray of PXDDigits to be processed", std::string(""));
  addParam("RawPXDsName", m_RawPXDsName, "The name of the StoreArray of generated RawPXDs", std::string(""));
  addParam("InjectionBGTimingName", m_InjectionBGTimingName, "The name of the StoreObj for Injection Timing", std::string(""));
  addParam("dhe_to_dhc", m_dhe_to_dhc,  "DHE to DHC mapping (DHC_ID, DHE1, DHE2, ..., DHE5) ; -1 disable port");
  addParam("InvertMapping",  m_InvertMapping, "Use invers mapping to DHP row/col instead of \"remapped\" coordinates", false);
  addParam("Clusterize",  m_Clusterize, "Use clusterizer (FCE format)", false);
  addParam("overrideFirmwareVersion", m_overrideFirmwareVersion, "Overwrite Firmware Version from DB with this value", 0);

}

void PXDPackerModule::initialize()
{
  // Register output collections
  m_storeRaws.registerInDataStore(m_RawPXDsName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
  m_storeDigits.isRequired(m_PXDDigitsName);
  m_storeInjectionBGTiming.isOptional(m_InjectionBGTimingName);

  m_packed_events = 0;

  B2DEBUG(27, "Clusterizer is " << m_Clusterize);
  B2DEBUG(27, "InvertMapping is " << m_InvertMapping);

  /// read in the mapping for ONSEN->DHC->DHE->DHP
  /// until now ONSEN->DHC is not needed yet (might be based on event numbers per event)
  /// DHE->DHP is only defined by port number/active mask ... not implemented yet.
  for (auto& it : m_dhe_to_dhc) {
    bool flag;
    int dhc_id;
    B2DEBUG(27, "PXD Packer --> DHC/DHE");
    flag = false;
    if (it.size() != 6) {
      /// means [ 1 2 3 4 5 -1 ] DHC 1 has DHE 2,3,4,5 on port 0-3 and nothing on port 4
      B2WARNING("PXD Packer --> DHC/DHE maps 1 dhc to 5 dhe (1+5 values), but I found " << it.size());
    }
    for (auto& it2 : it) {
      if (flag) {
        int v;
        v = it2;
        B2DEBUG(27, "PXD Packer --> ... DHE " << it2);
        if (it2 < -1 || it2 >= 64) {
          if (it2 != -1) B2ERROR("PXD Packer --> DHC id " << it2 << " is out of range (0-64 or -1)! disable channel.");
          v = -1;
        }
//         if (v > 0) dhe_mapto_dhc[v] = dhc_id;
        m_dhc_mapto_dhe[dhc_id].push_back(v);
      } else {
        dhc_id = it2;
        B2DEBUG(27, "PXD Packer --> DHC .. " << it2);
        if (dhc_id < 0 || dhc_id >= 16) {
          B2ERROR("PXD Packer --> DHC id " << it2 << " is out of range (0-15)! skip");
          break;
        }
      }
      flag = true;
    }
  }
  B2DEBUG(27, "PXD Packer --> DHC/DHE done");

//   for (auto & it : m_dhe_mapto_dhc) {
//     B2DEBUG(27, "PXD Packer --> DHE " << it.first << " connects to DHC " << it.second);
//   }

  for (auto& it : m_dhc_mapto_dhe) {
    int port = 0;
    B2DEBUG(27, "PXD Packer --> DHC " << it.first);
    for (auto& it2 : it.second) {
      B2DEBUG(27, "PXD Packer --> .. connects to DHE " << it2 << " port " << port);
      port++;
    }
  }

}

void PXDPackerModule::beginRun()
{
  if (m_overrideFirmwareVersion == 0) {
    if (m_firmwareFromDB.isValid()) m_firmware = (*m_firmwareFromDB).getDHHFirmwareVersion();
    else {
      B2WARNING("Could not read PXD Firmware version from db, assume default (new) firmware");
      m_firmware = 10;
    }
  } else {
    m_firmware = m_overrideFirmwareVersion;
  }
}

void PXDPackerModule::terminate()
{
}

void PXDPackerModule::event()
{
  StoreObjPtr<EventMetaData> evtPtr;

  // First, throw the dices for a few event-wise properties
  // optional, use simulated trigger information to calculate it

  m_trigger_dhp_framenr = gRandom->Integer(0x10000);
  // we use the very same m_trigger_dhp_framenr in DHE and DHP header, no second DHP frame is generated anyway
  // (-> like in the new firmware)

  if (m_storeInjectionBGTiming.isValid()) {
    m_trigger_dhe_gate = m_storeInjectionBGTiming->getTriggerGate();
  } else {
    m_trigger_dhe_gate = gRandom->Integer(192);
  }

  int nDigis = m_storeDigits.getEntries();

  B2DEBUG(27, "PXD Packer --> Nr of Digis: " << nDigis);

  startOfVxdID.clear();

  VxdID lastVxdId = -1; /// invalid ... force to set first iterator/index
  /// We assume the Digits are sorted by VxdID (P.K. says they are)
  /// This saves some iterating lateron
  for (auto it = m_storeDigits.begin() ; it != m_storeDigits.end(); ++it) {
    VxdID currentVxdId;
    currentVxdId = it->getSensorID();
    currentVxdId.setSegmentNumber(0);
    if (currentVxdId != lastVxdId) {
      // do something...
      lastVxdId = currentVxdId;
      B2DEBUG(27, "VxdId: " << currentVxdId << " " << (int)currentVxdId);
      {
        unsigned int layer, ladder, sensor, segment, dhe_id;
        layer = currentVxdId.getLayerNumber();/// 1 ... 2
        ladder = currentVxdId.getLadderNumber();/// 1 ... 8 and 1 ... 12
        sensor = currentVxdId.getSensorNumber();/// 1 ... 2
        segment = currentVxdId.getSegmentNumber();// Frame nr?
        dhe_id = ((layer - 1) << 5) | ((ladder) << 1) | (sensor - 1);
        B2DEBUG(27, "Layer: " << layer << " Ladder " << ladder << " Sensor " << sensor << " Segment(Frame) " << segment << " =>DHEID: " <<
                dhe_id);
      }

      if (startOfVxdID.count(currentVxdId) > 0) B2FATAL("PXD Digits are not sorted by VxdID!");
      startOfVxdID[currentVxdId] = std::distance(m_storeDigits.begin(), it);
      B2DEBUG(27, "Offset : " << startOfVxdID[currentVxdId]);
    }
  }

  m_trigger_nr = evtPtr->getEvent();
  m_run_nr_word1 = ((evtPtr->getRun() & 0xFF) << 8) | (evtPtr->getSubrun() & 0xFF);
  m_run_nr_word2 = ((evtPtr->getExperiment() & 0x3FF) << 6) | ((evtPtr->getRun() >> 8) & 0x3F);
  m_meta_time = evtPtr->getTime();

  pack_event();
  m_packed_events++;
}


void PXDPackerModule::pack_event(void)
{
  int dhe_ids[5] = {0, 0, 0, 0, 0};
  B2DEBUG(27, "PXD Packer --> pack_event");

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

void PXDPackerModule::add_frame_to_payload(void)
{
  if (m_current_frame.size() & 0x3) {
    B2ERROR("Frame is not 32bit aligned!!! Unsupported by Unpacker!");
  }
  // checksum frame
  dhe_crc_32_type current_crc;
  current_crc.process_bytes(m_current_frame.data(), m_current_frame.size());
  append_int32(current_crc.checksum());

  // and add it
  m_onsen_header.push_back(m_current_frame.size());
  m_onsen_payload.push_back(m_current_frame);
}

void PXDPackerModule::append_int8(unsigned char w)
{
  m_current_frame.push_back(w);
  dhe_byte_count++;
  dhc_byte_count++;
}

void PXDPackerModule::append_int16(unsigned short w)
{
  m_current_frame.push_back((unsigned char)(w >> 8));
  m_current_frame.push_back((unsigned char)(w));
  dhe_byte_count += 2;
  dhc_byte_count += 2;
}

void PXDPackerModule::append_int32(unsigned int w)
{
  m_current_frame.push_back((unsigned char)(w >> 24));
  m_current_frame.push_back((unsigned char)(w >> 16));
  m_current_frame.push_back((unsigned char)(w >> 8));
  m_current_frame.push_back((unsigned char)(w));
  dhe_byte_count += 4;
  dhc_byte_count += 4;
}

void PXDPackerModule::start_frame(void)
{
  m_current_frame.clear();
}

void PXDPackerModule::pack_dhc(int dhc_id, int dhe_active, int* dhe_ids)
{
  B2DEBUG(27, "PXD Packer --> pack_dhc ID " << dhc_id << " DHE act: " << dhe_active);

  /// HLT frame ??? format still t.b.d. TODO
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_ONSEN_TRG << 27) | (m_trigger_nr & 0xFFFF));
  append_int32(0xCAFE8000);// HLT HEADER, accepted flag set
  append_int32(m_trigger_nr); // HLT Trigger Nr
  append_int16(m_run_nr_word2); // Exp NR 9-0 | Run Nr 13-8
  append_int16(m_run_nr_word1); // Run Nr 7-0 | Subrunnr 7-0
  append_int32(0xCAFE0000);// DATCON HEADER ...
  append_int32(m_trigger_nr); // DATCON Trigger Nr
  append_int16(m_run_nr_word2); // Exp NR 9-0 | Run Nr 13-8
  append_int16(m_run_nr_word1); // Run Nr 7-0 | Subrunnr 7-0
  add_frame_to_payload();

  /// DHC Start

  dhc_byte_count = 0;
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_DHC_START << 27) | ((dhc_id & 0xF) << 21) | ((dhe_active & 0x1F) << 16) |
               (m_trigger_nr & 0xFFFF));
  append_int16(m_trigger_nr >> 16);

  uint32_t mm = (unsigned int)std::round((m_meta_time % 1000000000ull) * 0.127216); // in 127MHz Ticks
  uint32_t ss = (unsigned int)(m_meta_time / 1000000000ull) ; // in seconds
  append_int16(((mm << 4) & 0xFFF0) | 0x1); // TT 11-0 | Type --- fill with something usefull TODO
  append_int16(((mm >> 12) & 0x7FFF) | ((ss & 1) ? 0x8000 : 0x0)); // TT 27-12 ... not clear if completely filled by DHC
  append_int16((ss >> 1) & 0xFFFF); // TT 43-28 ... not clear if completely filled by DHC
  append_int16(m_run_nr_word1); // Run Nr 7-0 | Subrunnr 7-0
  append_int16(m_run_nr_word2); // Exp NR 9-0 | Run Nr 13-8
  add_frame_to_payload();

  // loop for each DHE in system
  // Run & TTT etc are zero until better idea

  for (int i = 0; i < 5; i++) {
    if (dhe_active & 0x1) pack_dhe(dhe_ids[i], 0xF);
    dhe_active >>= 1;
  }

  /// optional: copy the HLT/ROI frame, see no big reason to emulate it. TODO

  //  start_frame();
  //  append_int32((EDHCFrameHeaderDataType::c_ONSEN_ROI<<27) | (m_trigger_nr & 0xFFFF));
  ///  loop over ROIs and add for the DHEs in this DHC
  //  add_frame_to_payload();

  /// DHC End
  unsigned int dlen = (dhc_byte_count / 4); // 32 bit words
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_DHC_END << 27) | ((dhc_id & 0xF) << 21) | (m_trigger_nr & 0xFFFF));
  append_int32(dlen); // 32 bit word count
  append_int32(0x00000000); // Error Flags
  add_frame_to_payload();

}

void PXDPackerModule::pack_dhe(int dhe_id, int dhp_active)
{
  B2DEBUG(27, "PXD Packer --> pack_dhe ID " << dhe_id << " DHP act: " << dhp_active);
  // dhe_id is not dhe_id ...
  bool dhe_has_remapped = !m_InvertMapping; /// unless stated otherwise, DHH will not reformat coordinates

  if (m_InvertMapping) {
    // problem, we do not have an exact definition of if this bit is set in the new firmware and under which circumstances
    // and its not clear if we have to translate the coordinates back to "DHP" layout! (look up tabel etc!)
    // this code has never been completed as pxd cluster format will anyway need mapping in firmware
    B2FATAL("Inverse Mapping not implemented in Packer");
  }

  /// DHE Start
  dhe_byte_count = 0;
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_DHE_START << 27) | ((dhe_id & 0x3F) << 20) | ((dhp_active & 0xF) << 16) |
               (m_trigger_nr & 0xFFFF));
  append_int16(m_trigger_nr >> 16); // Trigger Nr Hi
  append_int16(0x00000000);  // DHE Timer Lo
  append_int16(0x00000000);  // DHE Time Hi
  append_int16(((m_trigger_dhp_framenr & 0x3F) << 10) |
               (m_trigger_dhe_gate & 0xFF)); // Last DHP Frame Nr 15-10, Reserved 9-8, Trigger Offset 7-0
  add_frame_to_payload();

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

    VxdID currentVxdId = 0;
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
    currentVxdId = VxdID(layer, ladder, sensor);

    B2DEBUG(27, "pack_dhe: VxdId: " << currentVxdId << " " << (int)currentVxdId);

    {
      auto it = m_storeDigits.begin();
      B2DEBUG(27, "Advance: " << startOfVxdID[currentVxdId]);
      advance(it, startOfVxdID[currentVxdId]);
      for (; it != m_storeDigits.end(); ++it) {
        auto id = it->getSensorID();
        id.setSegmentNumber(0);
        if (currentVxdId != id) break; /// another sensor starts
        /// Fill pixel to pixelmap
        {
          unsigned int row, col;
          row = it->getVCellID();// hardware starts counting at 0!
          col = it->getUCellID();// U/V cell ID DO NOT follow Belle2 Note, thus no -1
          if (row > ladder_max_row || col > ladder_max_col) {
            B2ERROR("U/V out of range U: " << col << " V: " << row);
          } else {
            // fill ADC ... convert float to unsigned char, clamp to 0 - 255 , no scaling ... and how about common mode?
            B2DEBUG(26, "Pixel: V: " << row << ", U: " << col << ", Ch " << it->getCharge());
            if (!dhe_has_remapped) {
              do_the_reverse_mapping(row, col, layer, sensor);
            }
            halfladder_pixmap[row][col] = (unsigned char) boost::algorithm::clamp(lrint(it->getCharge()), 0, 255);
          }
        }
      }
    }

    if (m_Clusterize) {
      B2FATAL("Clusterizer not supported in Packer");
    } else {
      for (int i = 0; i < 4; i++) {
        if (dhp_active & 0x1) {
          pack_dhp(i, dhe_id, dhe_has_remapped ? 1 : 0);
          /// The following lines "simulate" a full frame readout frame ... not for production yet!
          /// pedestals taking frames are normally not transfered to Belle2 DAQ (unless decided otherwise)
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
  unsigned int dlen = (dhe_byte_count / 2); // 16 bit words
  start_frame();
  append_int32((EDHCFrameHeaderDataType::c_DHE_END << 27) | ((dhe_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
  append_int16(dlen & 0xFFFF); // 16 bit word count
  append_int16((dlen >> 16) & 0xFFFF); // 16 bit word count
  append_int32(0x00000000);  // Error Flags
  add_frame_to_payload();
}

void PXDPackerModule::do_the_reverse_mapping(unsigned int& /*row*/, unsigned int& /*col*/, unsigned short /*layer*/,
                                             unsigned short /*sensor*/)
{
  B2FATAL("code needs to be written");
  // work to be done
  // I suspect this code will never be written.
  //
  // PXDMappingLookup::map_uv_to_rc_IF_OB(unsigned int& v_cellID, unsigned int& u_cellID, unsigned int& dhp_id, unsigned int dhe_ID)
  // PXDMappingLookup::map_uv_to_rc_IB_OF(unsigned int& v_cellID, unsigned int& u_cellID, unsigned int& dhp_id, unsigned int dhe_ID)
}

void PXDPackerModule::pack_dhp_raw(int chip_id, int dhe_id)
{
  B2FATAL("This code needs to be checked agains new firmware");
  B2DEBUG(27, "PXD Packer --> pack_dhp Raw Chip " << chip_id << " of DHE id: " << dhe_id);
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

void PXDPackerModule::pack_dhp(int chip_id, int dhe_id, int dhe_has_remapped, int startrow)
{
  B2DEBUG(27, "PXD Packer --> pack_dhp Chip " << chip_id << " of DHE id: " << dhe_id);
  // remark: chip_id != port most of the time ...
  bool empty = true;
  unsigned short last_rowstart = 0;

  if (dhe_has_remapped == 0) {
    // problem, we do not have an exact definition of if this bit is set in the new firmware and under which circumstances
    // and its not clear if we have to translate the coordinates back to "DHP" layout! (look up tabel etc!)
    B2FATAL("dhe_has_remapped == 0");
  }

  start_frame();
  /// DHP data Frame
  append_int32((EDHCFrameHeaderDataType::c_DHP_ZSD << 27) | ((dhe_id & 0x3F) << 20) | ((dhe_has_remapped & 0x1) << 19) | ((
                 chip_id & 0x03) << 16) | (m_trigger_nr & 0xFFFF));
  append_int32((EDHPFrameHeaderDataType::c_ZSD << 29) | ((dhe_id & 0x3F) << 18) | ((chip_id & 0x03) << 16) |
               (m_trigger_dhp_framenr & 0xFFFF));

  int c1, c2;
  c1 = 64 * chip_id;
  c2 = c1 + 64;
  if (c2 >= PACKER_NUM_COLS) c2 = PACKER_NUM_COLS;
  for (int rr = startrow; rr < startrow + PACKER_NUM_ROWS; rr++) {
    int row = (rr % PACKER_NUM_ROWS); // warp around
    /// we do not create a second frame (thats what the old firmware would have been doing)
    /// thus the behaviour is like in the new firmware, but old unpacker can handle that
    /// we do not support some "extra" readout gates before trigger and/or at the end of the full frame
    bool rowstart = true;
    for (int col = c1; col < c2; col++) {
      if (halfladder_pixmap[row][col] != 0) {
        // Attention, we decided to (mis)use ADC=0 values as truncation marker in future firmware! (discussion 15.1.2021)
        // ADC cut for sim should have been done already
        B2DEBUG(26, "Pixel: ROW: " << row << ", COL: " << col << ", Ch " << (int)halfladder_pixmap[row][col]);
        if (rowstart) {
          last_rowstart = ((row & 0x3FE) << (6 - 1)) | 0; // plus common mode 6 bits ... set to 0
          append_int16(last_rowstart);
          rowstart = false;
        }
        int colout = col;
        append_int16(0x8000 | ((row & 0x1) << 14) | ((colout & 0x3F) << 8) | (halfladder_pixmap[row][col] & 0xFF));
        empty = false;
      }
    }
  }
  if (!empty && (m_current_frame.size() & 0x3)) {
    B2DEBUG(27, "Repeat last rowstart to align to 32bit.");
    append_int16(last_rowstart);
  }

  if (empty) {
    B2DEBUG(27, "Found no data for halfladder! DHEID: " << dhe_id << " Chip: " << chip_id);
    /// This behaviour has changed in the overlapping trigger firmware.
    /// Ghost frames are ALWAYS an indication for a data flow/daq problem
    /// need to be consistent for simulation!
    if (m_firmware < 10) {
      // behaviour of old firmware is not upward comaptible, but for simulation (=Packing) the new behaviour
      // would work even for the old unpacking, even so not according to firmware documentation
      // -> maybe we remove this code?
      // we DROP the frame, thus we have to correct DHE and DHC counters
      dhc_byte_count -= 8; // fixed size of Header
      dhe_byte_count -= 8; // fixed size of Header
      start_frame();
      /// Ghost Frame ... start frame overwrites frame info set above
      append_int32((EDHCFrameHeaderDataType::c_GHOST << 27) | ((dhe_id & 0x3F) << 20) | ((chip_id & 0x03) << 16) |
                   (m_trigger_nr & 0xFFFF));
    }
  }
  add_frame_to_payload();

}
