/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdUnpacking/PXDPackerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

#include <boost/spirit/home/support/detail/endian.hpp>

// DHP modes are the same as for DHE envelope
#define DHP_FRAME_HEADER_DATA_TYPE_RAW  0x0
#define DHP_FRAME_HEADER_DATA_TYPE_ZSD  0x5

// DHE like before, but now 4 bits
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW     0x0
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD     0x5
#define DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW     0x1
#define DHC_FRAME_HEADER_DATA_TYPE_COMMODE     0x6
#define DHC_FRAME_HEADER_DATA_TYPE_GHOST       0x2
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_START   0x3
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_END     0x4
// DHC envelope, new
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_START  0xB
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_END    0xC
// Onsen processed data, new
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP     0xD
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE     0x9
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI     0xF
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG     0xE
// Free IDs are 0x7 0x8 0xA

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDPacker)

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
  addParam("dhe_to_dhc", m_dhe_to_dhc,  "DHE to DHC mapping (DHC_ID, DHE1, DHE2, ..., DHE5) ; -1 disable port");

}

void PXDPackerModule::initialize()
{
  B2INFO("PXD Packer --> Init");
  //Register output collections
  m_storeRaws.registerInDataStore(m_RawPXDsName);
  storeDigits.required(m_PXDDigitsName);

  m_packed_events = 0;

  /// read in the mapping for ONSEN->DHC->DHE->DHP
  /// until now ONSEN->DHC is not needed yet (might be based on event numbers per event)
  /// DHE->DHP is only defined by port number/active mask ... not implemented yet.
  for (auto& it : m_dhe_to_dhc) {
    bool flag;
    int dhc_id;
    B2INFO("PXD Packer --> DHC/DHE");
    flag = false;
    if (it.size() != 6) {
      /// means [ 1 2 3 4 5 -1 ] DHC 1 has DHE 2,3,4,5 on port 0-3 and nothing on port 4
      B2WARNING("PXD Packer --> DHC/DHE maps 1 dhc to 5 dhe (1+5 values), but I found " << it.size());
    }
    for (auto& it2 : it) {
      if (flag) {
        int v;
        v = it2;
        B2INFO("PXD Packer --> ... DHE " << it2);
        if (it2 < -1 || it2 >= 64) {
          if (it2 != -1) B2ERROR("PXD Packer --> DHC id " << it2 << " is out of range (0-64 or -1)! disable channel.");
          v = -1;
        }
//         if (v > 0) dhe_mapto_dhc[v] = dhc_id;
        m_dhc_mapto_dhe[dhc_id].push_back(v);
      } else {
        dhc_id = it2;
        B2INFO("PXD Packer --> DHC .. " << it2);
        if (dhc_id < 0 || dhc_id >= 16) {
          B2ERROR("PXD Packer --> DHC id " << it2 << " is out of range (0-15)! skip");
          break;
        }
      }
      flag = true;
    }
  }
  B2INFO("PXD Packer --> DHC/DHE done");

//   for (auto & it : m_dhe_mapto_dhc) {
//     B2INFO("PXD Packer --> DHE " << it.first << " connects to DHC " << it.second);
//   }

  for (auto& it : m_dhc_mapto_dhe) {
    int port = 0;
    B2INFO("PXD Packer --> DHC " << it.first);
    for (auto& it2 : it.second) {
      B2INFO("PXD Packer --> .. connects to DHE " << it2 << " port " << port);
      port++;
    }
  }

}

void PXDPackerModule::terminate()
{
}

void PXDPackerModule::event()
{
  StoreObjPtr<EventMetaData> evtPtr;

  B2INFO("PXD Packer --> Event");

//   B2ERROR("Test : " << evtPtr->getEvent() << ","  << evtPtr->getRun() << "," << evtPtr->getSubrun() << "," << evtPtr->getExperiment() << "," << evtPtr->getTime() << " ==");

  int nDigis = storeDigits.getEntries();

  B2INFO("PXD Packer --> Nr of Digis: " << nDigis);

  startOfVxdID.clear();

  VxdID lastVxdId = -1; /// invalid ... force to set first itertor/index
  /// We assume the Digits are sorted by VxdID (P.K. says they are)
  /// Thie saves some iterating lateron
  for (auto it = storeDigits.begin() ; it != storeDigits.end(); it++) {
    VxdID currentVxdId;
    currentVxdId = it->getSensorID();
    currentVxdId.setSegmentNumber(0);
    if (currentVxdId != lastVxdId) {
      // do something...
      lastVxdId = currentVxdId;
      B2INFO("VxdId: " << currentVxdId << " " << (int)currentVxdId);
      {
        unsigned int layer, ladder, sensor, segment, dhe_id;
        layer = currentVxdId.getLayerNumber();/// 1 ... 2
        ladder = currentVxdId.getLadderNumber();/// 1 ... 8 and 1 ... 12
        sensor = currentVxdId.getSensorNumber();/// 1 ... 2
        segment = currentVxdId.getSegmentNumber();// Frame nr?
        dhe_id = ((layer - 1) << 5) | ((ladder) << 1) | (sensor - 1);
        B2INFO("Layer: " << layer << " Ladder " << ladder << " Sensor " << sensor << " Segment(Frame) " << segment << " =>DHEID: " <<
               dhe_id);
      }

      startOfVxdID[currentVxdId] = std::distance(storeDigits.begin(), it);
      B2INFO("Offset : " << startOfVxdID[currentVxdId]);
    }
  }

  m_trigger_nr = evtPtr->getEvent();// m_packed_events;
  m_run_nr_word1 = ((evtPtr->getRun() & 0xFF) << 8) | (evtPtr->getSubrun() & 0xFF);
  m_run_nr_word2 = ((evtPtr->getExperiment() & 0x3FF) << 6) | ((evtPtr->getRun() >> 8) & 0x3F);
  pack_event();
  m_packed_events++;
}

void PXDPackerModule::endian_swap_frame(unsigned short* dataptr, int len)
{
  boost::spirit::endian::ubig16_t* p = (boost::spirit::endian::ubig16_t*)dataptr;

  /// swap endianess of all shorts in frame BUT not the CRC (2 shorts)
  for (int i = 0; i < len / 2 - 2; i++) {
    p[i] = dataptr[i];// Endian Swap! (it doesnt matter if you swap from little to big or vice versa)
  }
}

void PXDPackerModule::pack_event(void)
{
  int dhe_ids[5] = {0, 0, 0, 0, 0};
  B2INFO("PXD Packer --> pack_event");

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
}

void PXDPackerModule::append_int16(unsigned short w)
{
  m_current_frame.push_back((unsigned char)(w >> 8));
  m_current_frame.push_back((unsigned char)(w));
}

void PXDPackerModule::append_int32(unsigned int w)
{
  m_current_frame.push_back((unsigned char)(w >> 24));
  m_current_frame.push_back((unsigned char)(w >> 16));
  m_current_frame.push_back((unsigned char)(w >> 8));
  m_current_frame.push_back((unsigned char)(w));
}

void PXDPackerModule::start_frame(void)
{
  m_current_frame.clear();
}

void PXDPackerModule::pack_dhc(int dhc_id, int dhe_active, int* dhe_ids)
{
  B2INFO("PXD Packer --> pack_dhc ID " << dhc_id << " DHE act: " << dhe_active);

  /// HLT frame ??? format still t.b.d. TODO
  start_frame();
  append_int32((DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG << 27) | (m_trigger_nr & 0xFFFF));
  append_int32(0xCAFE0000);// HLT HEADER
  append_int32(m_trigger_nr); // HLT Trigger Nr
  append_int32(m_run_nr_word1); // HLT Run NR etc
  append_int32(0xCAFE0000);// DATCON HEADER ... do we want it here? t.b.d.
  append_int32(m_trigger_nr); // DATCON Trigger Nr
  append_int32(m_run_nr_word1); // DATCON Run NR etc
  add_frame_to_payload();

  /// DHC Start

  start_frame();
  append_int32((DHC_FRAME_HEADER_DATA_TYPE_DHC_START << 27) | ((dhc_id & 0xF) << 21) | ((dhe_active & 0x1F) << 16) |
               (m_trigger_nr & 0xFFFF));
  append_int16(m_trigger_nr >> 16);
  append_int16(0x00000000); // TT 11-0 | Type --- fill with something usefull TODO
  append_int16(0x00000000); // TT 27-12
  append_int16(0x00000000); // TT 43-28
  append_int16(m_run_nr_word1); // Run Nr 7-0 | Subrunnr 7-0
  append_int16(m_run_nr_word2); // Exp NR 9-0 | Run Nr 13-8
  add_frame_to_payload();

  // loop for each DHE in system
  // we use events_packed as event number until we get it from somewhere else
  // Run & TTT etc are zero until better idea

  for (int i = 0; i < 5; i++) {
    if (dhe_active & 0x1) pack_dhe(dhe_ids[i], 0xF);
    dhe_active >>= 1;
  }

  /// lets copy the HLT/ROI frame

  //  start_frame();
  //  append_int32((DHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI<<27) | (m_trigger_nr & 0xFFFF));
  //  add_frame_to_payload();

  /// DHC End

  start_frame();
  append_int32((DHC_FRAME_HEADER_DATA_TYPE_DHC_END << 27) | ((dhc_id & 0xF) << 21) | (m_trigger_nr & 0xFFFF));
  append_int32(0x00000000); // 16 bit word count
  append_int32(0x00000000); // Error Flags
  add_frame_to_payload();

}

void PXDPackerModule::pack_dhe(int dhe_id, int dhp_active)
{
  B2INFO("PXD Packer --> pack_dhe ID " << dhe_id << " DHP act: " << dhp_active);
  // dhe_id is not dhe_id ...
  int dhe_reformat = dhe_id & 0x1; /// up/downstream to check dhe_reformat flag

  /// DHE Start
  start_frame();
  append_int32((DHC_FRAME_HEADER_DATA_TYPE_DHE_START << 27) | ((dhe_id & 0x3F) << 20) | ((dhp_active & 0xF) << 16) |
               (m_trigger_nr & 0xFFFF));
  append_int16(m_trigger_nr >> 16); // Trigger Nr Hi
  append_int16(0x00000000);  // DHE Timer Lo
  append_int16(0x00000000);  // DHE Time Hi
  append_int16(0x00000000);  // Last DHP Frame Nr 5-0, Trigger Offset 9-0
  add_frame_to_payload();

// now prepare the data from one halfladder
// do the ROI selection??? optional...
// then loop for each DHP in system
// get active DHPs from a database?
// and pack data per halfladder.
// we fake the framenr and startframenr until we find some better solution

  if (dhp_active != 0) { /// is there any hardware switched on?
    unsigned int ladder_min_row = 0; /// get them from database
    unsigned int ladder_max_row = 767;
    unsigned int ladder_min_col = 0;
    unsigned int ladder_max_col = 250;

    /// clear pixelmap
    bzero(halfladder_pixmap, sizeof(halfladder_pixmap));

    VxdID currentVxdId = 0;
    {
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
    }
    B2INFO("pack_dhe: VxdId: " << currentVxdId << " " << (int)currentVxdId);

    {
      auto it = storeDigits.begin();
      B2INFO("Advance: " << startOfVxdID[currentVxdId]);
      advance(it, startOfVxdID[currentVxdId]);
      for (; it != storeDigits.end(); it++) {
        auto id = it->getSensorID();
        id.setSegmentNumber(0);
        if (currentVxdId != id) break; /// another sensor starts
        /// Fill pixel to pixelmap
        {
          unsigned int row, col;
          row = it->getUCellID(); // it->getVCellID(); funny, did somebody changed U and V???
          col = it->getVCellID(); // it->getUCellID();
          if (row < ladder_min_row || row > ladder_max_row || col < ladder_min_col || col > ladder_max_col) {
            B2ERROR("ROW/COL out of range col: " << col << " row: " << row);
          } else {
            // fill ADC ... convert float to unsigned char ... and how about common mode?
            halfladder_pixmap[row][col] = (unsigned char) it->getCharge(); // scaling??
          }
        }
      }
    }

    for (int i = 0; i < 4; i++) {
      if (dhp_active & 0x1) {
        pack_dhp(i, dhe_id, dhe_reformat);
        if (m_trigger_nr == 0x11) {
          pack_dhp_raw(i, dhe_id, false);
          pack_dhp_raw(i, dhe_id, true);
        }
      }
      dhp_active >>= 1;
    }
  }

  /// DHE End
  start_frame();
  append_int32((DHC_FRAME_HEADER_DATA_TYPE_DHE_END << 27) | ((dhe_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
  append_int32(0x00000000);  // 16 bit word count
  append_int32(0x00000000);  // Error Flags
  add_frame_to_payload();
}

void PXDPackerModule::pack_dhp_raw(int chip_id, int dhe_id, bool adcpedestal)
{
  B2INFO("PXD Packer --> pack_dhp Raw Chip " << chip_id << " of DHE id: " << dhe_id << " Mode " << adcpedestal);
  start_frame();
  /// DHP data Frame
  append_int32((DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW << 27) | ((dhe_id & 0x3F) << 20) | ((chip_id & 0x03) << 16) |
               (m_trigger_nr & 0xFFFF));
  append_int32((DHP_FRAME_HEADER_DATA_TYPE_RAW << 29) | ((dhe_id & 0x3F) << 18) | ((chip_id & 0x03) << 16) | (0 & 0xFFFF));

  int c1, c2;
  c1 = 64 * chip_id;
  c2 = c1 + 64;
  if (c2 >= PACKER_NUM_COLS) c2 = PACKER_NUM_COLS;

  if (adcpedestal) {
    for (int row = 0; row < PACKER_NUM_ROWS; row++) {
      for (int col = c1; col < c2; col++) {
        append_int16((halfladder_pixmap[row][col] << 8) | ((row + col) & 0xFF));
      }
      for (int col = c2; col < c1 + 64; col++) {
        append_int16(0);
      }
    }
    for (int row = PACKER_NUM_ROWS; row < 1024; row++) {
      for (int col = 0; col < 64; col++) {
        append_int16(0);
      }
    }
  } else {
    for (int row = 0; row < PACKER_NUM_ROWS; row++) {
      for (int col = c1; col < c2; col++) {
        append_int8(halfladder_pixmap[row][col]);
      }
      for (int col = c2; col < c1 + 64; col++) {
        append_int8(0);
      }
    }
    for (int row = PACKER_NUM_ROWS; row < 1024; row++) {
      for (int col = 0; col < 64; col++) {
        append_int8(0);
      }
    }
  }
  add_frame_to_payload();
}

void PXDPackerModule::pack_dhp(int chip_id, int dhe_id, int dhe_reformat)
{
  B2INFO("PXD Packer --> pack_dhp Chip " << chip_id << " of DHE id: " << dhe_id);
  // remark: chip_id != port most of the time ...
  bool empty = true;
  unsigned short last_rowstart = 0;
  unsigned short frame_id = 0; // to be set TODO

  start_frame();
  /// DHP data Frame
  append_int32((DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD << 27) | ((dhe_id & 0x3F) << 20) | ((dhe_reformat & 0x1) << 19) | ((
                 chip_id & 0x03) << 16) | (m_trigger_nr & 0xFFFF));
  append_int32((DHP_FRAME_HEADER_DATA_TYPE_ZSD << 29) | ((dhe_id & 0x3F) << 18) | ((chip_id & 0x03) << 16) | (frame_id & 0xFFFF));
  for (int row = 0; row < PACKER_NUM_ROWS; row++) { // should be variable
    bool rowstart;
    rowstart = true;
    int c1, c2;
    c1 = 64 * chip_id;
    c2 = c1 + 64;
    if (c2 >= PACKER_NUM_COLS) c2 = PACKER_NUM_COLS;
    for (int col = c1; col < c2; col++) {
      if (halfladder_pixmap[row][col] != 0) {
        if (rowstart) {
          last_rowstart = ((row & 0x3FE) << (6 - 1)) | 0; // plus common mode 6 bits ... set to 0
          append_int16(last_rowstart);
          rowstart = false;
        }
        int colout = col;
        if (dhe_reformat == 0) colout ^= 0x3C ; /// 0->60 61 62 63 4->56 57 58 59 ...
        append_int16(0x8000 | ((row & 0x1) << 14) | ((colout & 0x3F) << 8) | (halfladder_pixmap[row][col] & 0xFF));
        empty = false;
      }
    }
  }
  if (!empty && (m_current_frame.size() & 0x3)) {
    B2INFO("Repeat last rowstart to align to 32bit.");
    append_int16(last_rowstart);
  }


  if (empty) {
    B2INFO("Found no data for halfladder! DHEID: " << dhe_id << " Chip: " << chip_id);
    start_frame();
    /// Ghost Frame ... start frame overwrites frame info set above
    append_int32((DHC_FRAME_HEADER_DATA_TYPE_GHOST << 27) | ((dhe_id & 0x3F) << 20) | ((chip_id & 0x03) << 16) |
                 (m_trigger_nr & 0xFFFF));
  } else {
    //B2INFO("Found data for halfladder DHEID: " << dhe_id << " Chip: " << chip_id);
  }
  add_frame_to_payload();

}
