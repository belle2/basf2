/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : PXDReadRawBonnDAQMatched.cc
// Description : Module to Load Raw PXD Data from DHH network-dump file and store it as RawPXD in Data Store
// This is meant for lab use (standalone testing, debugging) without an event builder.
// This module is able to read the BonnDAQ data unordered, e.g. scan for the current event number
//
// Author : Bjoern Spruck
// Date : 16.06.2019
//-

#include <pxd/modules/pxdUnpacking/PXDReadRawBonnDAQMatched.h>
#include <boost/endian/arithmetic.hpp>
#include <boost/crc.hpp>

using namespace std;
using namespace Belle2;
using namespace PXD;

using ulittle16_t = boost::endian::little_uint16_t;
using ulittle32_t = boost::endian::little_uint32_t;
using ubig16_t = boost::endian::big_uint16_t;
using ubig32_t = boost::endian::big_uint32_t;

using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhe_crc_32_type;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDReadRawBonnDAQMatched)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDReadRawBonnDAQMatchedModule::PXDReadRawBonnDAQMatchedModule() : Module()
{
  fh = 0;
  //Set module properties
  setDescription("Read a BonnDAQ form file, match to current event and stores it as RawPXD in Data Store");
  //setPropertyFlags( c_ParallelProcessingCertified); // can not be run in parallel!

  addParam("RawPXDsName", m_RawPXDsName, "The name of the StoreArray of RawPXDs to be written", std::string(""));

  addParam("FileName", m_filename, "file name");
  m_buffer = new int[MAXEVTSIZE];

  B2DEBUG(29, "PXDReadRawBonnDAQMatchedModule: Constructor done.");
}


PXDReadRawBonnDAQMatchedModule::~PXDReadRawBonnDAQMatchedModule()
{
  delete[] m_buffer;
}

void PXDReadRawBonnDAQMatchedModule::initialize()
{
  // Open File
  fh = fopen(m_filename.c_str(), "rb");
  if (fh) {
    B2INFO("Read BonnDAQ Data from " << m_filename);
  } else {
    B2ERROR("Could not open BonnDAQ Data: " << m_filename);
  }

  // Ask EvtMetaData
  m_eventMetaDataPtr.isRequired();
  // Register RawPXD
  m_rawPXD.registerInDataStore(m_RawPXDsName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);

  B2DEBUG(29, "PXDReadRawBonnDAQMatchedModule: initialized.");
}

int PXDReadRawBonnDAQMatchedModule::read_data(char* data, size_t len)
{
  size_t l = 0;
  if (fh) l = fread(data, 1, len, fh);
  if (l != len) return 0;
  return l;
}

int PXDReadRawBonnDAQMatchedModule::readOneEvent(unsigned int& rettriggernr)
{
  unsigned int triggernr = 0xFFFFFFFF;

  auto current_offset = ftell(fh);
  if (current_offset > m_last_offset) m_last_offset = current_offset;

  struct EvtHeader {
    ulittle16_t size;
    ulittle16_t header;
    unsigned int get_size(void) { return (unsigned int) size + (((unsigned int)(header & 0x000F)) << 16);};
    unsigned int get_size_group(void) { return (unsigned int) size + (((unsigned int)((header & 0x003F) ^ 0x0020)) << 16);};
    unsigned int get_header12(void) { return (header & 0xFFF0);};
    unsigned int get_header10(void) { return (header & 0xFFC0);};
    unsigned int get_header8(void) { return (header & 0xFF00);};
  };

  char* data = (char*)m_buffer;
  struct EvtHeader* evt = (struct EvtHeader*)data;

  while (1) {
    ulittle32_t* data32 = (ulittle32_t*)data;
    ulittle16_t* data16 = (ulittle16_t*)data;
    // Read 8 bytes header (group)
    int br = read_data(data, 4);
    if (br <= 0) return br;
    unsigned int chunk_size = 0;
    if (evt->get_header8() == 0) {
      B2DEBUG(29, "Group Header $" << std::hex << evt->get_header10() << " Chunk size " << std::dec << evt->get_size());
      chunk_size = evt->get_size_group();
    } else {
      B2DEBUG(29, "Header $" << std::hex << evt->get_header12() << " Chunk size " << std::dec << evt->get_size());
      chunk_size = evt->get_size();
    }
    if (chunk_size <= 1) return 0;
    br = read_data(data + 4, chunk_size * 4 - 4);
    if (br <= 0) return br;
    if (evt->get_header12() == 0xe230) {
      B2DEBUG(29, "File info " << std::hex << evt->get_header12() << " Events " << std::dec << data32[1]);
      continue;
    } else if (evt->get_header12() == 0xe100) {
      B2DEBUG(29, "Info Event " << std::hex << evt->get_header12() << " RunNr $" << std::hex << data32[1]);
      // if (m_runNr == 0) m_runNr = data32[1]; // we assume it will not change within one file
      continue;
    } else if (evt->get_header10() == 0x0000) { // war 0x0020
      B2DEBUG(29, "Run Event Group " << std::hex << evt->get_header10() << " Magic $" << std::hex << data32[1]);
      continue;
    } else if (evt->get_header12() == 0xbb00) {
      B2DEBUG(29, "Run Event " << std::hex << evt->get_header12() << " Magic $" << std::hex << data32[1]);
      continue;
    } else if (evt->get_header10() == 0x0080) { // war 0x00A0
      int togo = chunk_size;
      B2DEBUG(29, "Data Event Group " << std::hex << evt->get_header10() << " TriggerNr $" << std::hex << data32[1]);
      triggernr = data32[1];
      togo -= 2;
      data32 += 2;
      data16 += 4;
      while (togo > 2) {
        B2DEBUG(29, "TOGO: " << togo);
        B2DEBUG(29, " ............... " << std::hex << data32[0] << " TriggerNr $" << std::hex << data32[1]);
        if (triggernr != data32[1]) B2ERROR("Trigger Nr does not match!");
        B2DEBUG(29, " ............... " << std::hex << data32[2]);
        togo -= 2;
        data32 += 2;
        data16 += 4;
        if ((data32[0] & 0xFFFF0000) == 0xCAFE0000) {
          int frames = (data32[0] & 0x3FF);
          B2DEBUG(29, "Frames: " << frames);
          int size = 0;
          bool nocrc = (data32[0] & 0x8000) != 0;

          if ((data32[0] & 0x4000) == 0) B2FATAL("large data fields not supported");

          togo--;
          data32++;
          data16 += 2;
          /** For one DHC event, we utilize one header (writing out, beware of endianess!) */
          std::vector <unsigned int> m_onsen_header;

          /** For one DHC event, we utilize one payload for all DHE/DHP frames */
          std::vector <std::vector <unsigned char>> m_onsen_payload;
          int offset = ((frames + 1) & ~1);

          ulittle16_t* table16 = data16;
          if (!nocrc) {
            togo--; // jump over TOC CRC
            data32++;
            data16 += 2;
          }

          for (int i = 0; i < frames; i++) {
            // B2INFO(".... " << i << ": " << table16[i]);
            size += table16[i];

            /** For current processed frames */
            std::vector <unsigned char> m_current_frame;

            for (int j = 0; j < (int)table16[i] * 2; j++) {
              unsigned short w = data16[offset++];
              m_current_frame.push_back((unsigned char)(w >> 8));
              m_current_frame.push_back((unsigned char)(w));
            }

            if (nocrc) { // recalculate
              dhe_crc_32_type current_crc;
              current_crc.process_bytes(m_current_frame.data(), m_current_frame.size());
              unsigned int w = current_crc.checksum();
              m_current_frame.push_back((unsigned char)(w >> 24));
              m_current_frame.push_back((unsigned char)(w >> 16));
              m_current_frame.push_back((unsigned char)(w >> 8));
              m_current_frame.push_back((unsigned char)(w));
            }

            m_onsen_header.push_back(m_current_frame.size());
            m_onsen_payload.push_back(m_current_frame);
          }
          togo -= ((frames + 1) & ~1) / 2 + size;
          data32 += ((frames + 1) & ~1) / 2 + size;
          data16 += ((frames + 1) & ~1) + size * 2;

          if (nocrc) {
            togo--;
            data32++;
            data16 += 2;
          }

          if (rettriggernr == triggernr) m_rawPXD.appendNew(m_onsen_header, m_onsen_payload);
        }
      }// while ...

      rettriggernr = triggernr;

      m_event_offset[triggernr] = current_offset;
      current_offset = ftell(fh);
      if (current_offset > m_last_offset) m_last_offset = current_offset;

      return 1;
    } else {
      B2ERROR("Undefine Header $" << std::hex << evt->get_header12());
      continue;
    }
    continue;

  }
  rettriggernr = 0xFFFFFFFF;

  return 0;
}

void PXDReadRawBonnDAQMatchedModule::event()
{
  if (fh == 0) {
    B2ERROR("Unexpected close of bonndaq file.");
    terminate();
    return;
  }

  auto triggernr = m_eventMetaDataPtr->getEvent();

  // Check if event has an offset, if not skip until end of checked area
  auto offset = m_event_offset[triggernr];
  if (offset == 0) offset = m_last_offset;
  fseek(fh, offset, SEEK_SET);
  // Get a record from file
  auto tnr = triggernr;
  do {
    // Search for triggernr
    tnr = triggernr; // set again as it is the return value
    int stat = readOneEvent(tnr);
    if (stat <= 0) {
      // End of File
      break;
    };
  } while (tnr != triggernr); // found and filled, else continue

  // set return value to true only if matching event was found
  setReturnValue(tnr == triggernr);

  return;
}

void PXDReadRawBonnDAQMatchedModule::terminate()
{
  if (fh) fclose(fh);
  fh = 0;
}
