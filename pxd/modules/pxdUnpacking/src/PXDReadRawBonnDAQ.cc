//+
// File : PXDReadRawBonnDAQ.cc
// Description : Module to Load Raw PXD Data from DHH network-dump file and store it as RawPXD in Data Store
// This is meant for lab use (standalone testing, debugging) without an event builder.
//
// Author : Bjoern Spruck
// Date : 02.05.2018
//-

#include <pxd/modules/pxdUnpacking/PXDReadRawBonnDAQ.h>
#include <boost/spirit/home/support/detail/endian.hpp>
#include <boost/crc.hpp>

using namespace std;
using namespace Belle2;
using namespace PXD;

using namespace boost::spirit::endian;

using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhe_crc_32_type;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDReadRawBonnDAQ)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDReadRawBonnDAQModule::PXDReadRawBonnDAQModule() : Module()
{
  fh = 0;
  m_msghandler = 0;
  //Set module properties
  setDescription("Read a BonnDAQ file and stores it as RawPXD in Data Store");
  //setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  //setPropertyFlags(c_Input);

  addParam("FileName", m_filename, "file name");
  addParam("RunNr", m_runNr, "run number", 0u);
  addParam("ExpNr", m_expNr, "exp number", 0u);
  m_nread = 0;
  m_compressionLevel = 0;
  m_buffer = new int[MAXEVTSIZE];

  B2DEBUG(0, "PXDReadRawBonnDAQModule: Constructor done.");
}


PXDReadRawBonnDAQModule::~PXDReadRawBonnDAQModule()
{
  delete[] m_buffer;
}

void PXDReadRawBonnDAQModule::initialize()
{
  // Open receiver sockets
//  m_recv = new EvtSocketSend(m_host, m_port);
  fh = fopen(m_filename.c_str(), "rb");
  if (fh) {
    B2INFO("Read BonnDAQ Data from " << m_filename);
  } else {
    B2ERROR("Could not open BonnDAQ Data: " << m_filename);
  }

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Register EvtMetaData
  m_eventMetaDataPtr.registerInDataStore(DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
  // Register RawPXD
  m_rawPXD.registerInDataStore(DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);

  B2DEBUG(0, "PXDReadRawBonnDAQModule: initialized.");
}

int PXDReadRawBonnDAQModule::read_data(char* data, size_t len)
{
  size_t l = 0;
  if (fh) l = fread(data, 1, len, fh);
  if (l != len) return 0;
  return l;
}

void PXDReadRawBonnDAQModule::endian_swapper(void* a, unsigned int len)
{
  // Quick and Dirty swapper for BonnDAQ
  ubig16_t* p;
  ulittle16_t* q;
  p = (ubig16_t*)a;
  q = (ulittle16_t*)a;
  len /= 2;
  for (unsigned int i = 0; i < len; i++, q++, p++) { *q = *p;}
}


int PXDReadRawBonnDAQModule::readOneEvent()
{
  unsigned int triggernr = 0xFFFFFFFF;

  struct EvtHeader {
    ulittle16_t size;
    ulittle16_t header;
  };

  char* data = (char*)m_buffer;
  struct EvtHeader* evt = (struct EvtHeader*)data;

  while (1) {
    ulittle32_t* data32 = (ulittle32_t*)data;
    ulittle16_t* data16 = (ulittle16_t*)data;
    // Read 8 bytes header (group)
    int br = read_data(data, 4);
    if (br <= 0) return br;
    B2DEBUG(1, "Header $" << std::hex << evt->header << " Chunk size " << std::dec << evt->size);
    if (evt->size <= 1) return 0;
    br = read_data(data + 4, evt->size * 4 - 4);
    if (br <= 0) return br;
    if (evt->header == 0xe230) {
      B2DEBUG(1, "File info " << std::hex << evt->header << " Events " << std::dec << data32[1]);
      continue;
    } else if (evt->header == 0xe100) {
      B2DEBUG(1, "Info Event " << std::hex << evt->header << " RunNr $" << std::hex << data32[1]);
      if (m_runNr == 0) m_runNr = data32[1]; // we assume it will not change within one file
      continue;
    } else if (evt->header == 0x0020) {
      B2DEBUG(1, "Run Event Group " << std::hex << evt->header << " Magic $" << std::hex << data32[1]);
      continue;
    } else if (evt->header == 0xbb00) {
      B2DEBUG(1, "Run Event " << std::hex << evt->header << " Magic $" << std::hex << data32[1]);
      continue;
    } else if (evt->header == 0x00a0) {
      int togo = evt->size;
      B2DEBUG(1, "Data Event Group " << std::hex << evt->header << " TriggerNr $" << std::hex << data32[1]);
      triggernr = data32[1];
      togo -= 2;
      data32 += 2;
      data16 += 4;
      while (togo > 2) {
        B2DEBUG(1, "TOGO: " << togo);
        B2DEBUG(1, " ............... " << std::hex << data32[0] << " TriggerNr $" << std::hex << data32[1]);
        if (triggernr != data32[1]) B2ERROR("Trigger Nr does not match!");
        B2DEBUG(1, " ............... " << std::hex << data32[2]);
        togo -= 2;
        data32 += 2;
        data16 += 4;
        if ((data32[0] & 0xFFFF0000) == 0xCAFE0000) {
          int frames = (data32[0] & 0x3FF);
          B2DEBUG(1, "Frames: " << frames);
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
            B2INFO(".... " << i << ": " << table16[i]);
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

          m_rawPXD.appendNew(m_onsen_header, m_onsen_payload);
        }
      }// while ...

      // Update EventMetaData
      B2INFO("Set Meta: Exp " << m_expNr << " Run " << m_runNr << " TrgNr " << triggernr);
      m_eventMetaDataPtr.create();
      m_eventMetaDataPtr->setExperiment(m_expNr);
      m_eventMetaDataPtr->setRun(m_runNr);
      m_eventMetaDataPtr->setEvent(triggernr);
      // we cannot recover time tag here. this would need further decoding from DHH header
      // m_eventMetaDataPtr->setTime((unsigned long long int)((time_tag_hi<<1) +(time_tag_mid &0x8000?1:0))*1000000000+(int)std::round(tt / 0.127216));

      return 1;
    } else {
      B2ERROR("Undefine Header $" << std::hex << evt->header);
      continue;
    }
    continue;

  }
  return 0;
}

void PXDReadRawBonnDAQModule::event()
{
  if (fh == 0) {
    B2ERROR("Unexpected close of dump file.");
    terminate();
    return;
  }

  // Get a record from socket
  int stat;
  do {
    stat = readOneEvent();
    if (stat <= 0) {
      /// End of File
      terminate();
      return;
    };
  } while (stat == 0);

  return;
}

void PXDReadRawBonnDAQModule::terminate()
{
  B2INFO("terminate called");
  if (fh) fclose(fh);
  fh = 0;
}

