#include <topcaf/modules/iTopRawConverterModule/inc/iTopRawConverterModule.h>
#include <iostream>

using namespace Belle2;

REG_MODULE(iTopRawConverter)

//Constructor
iTopRawConverterModule::iTopRawConverterModule() : Module()
{
  setDescription("This module is used to upack the raw data from the testbeam and crt data");
  addParam("InputFileName", m_input_filename, "Raw input filename");

  m_WfPacket = NULL;
  m_EvtPacket = NULL;
}

iTopRawConverterModule::~iTopRawConverterModule()
{
  if (m_WfPacket != NULL)
    delete m_WfPacket;
  if (m_EvtPacket != NULL)
    delete m_EvtPacket;

  m_input_file.close();
}

/////////////////////
// Module function //
/////////////////////
void iTopRawConverterModule::initialize()
{
  LoadRawFile(m_input_filename.c_str());

  //output
  m_evtheader_ptr.registerInDataStore();
  m_evtwave_ptr.registerInDataStore();

}

void iTopRawConverterModule::event()
{

  //output
  m_evtheader_ptr.create();
  m_evtwave_ptr.create();

  int packet_type = FindNextPacket();
  if (packet_type == 2) {
    std::cout << "Founds Waveform Packet Type: " << packet_type << std::endl;
    m_evtwave_ptr.assign(GetWaveformPacket(), true);
    std::cout << " GetChannelID(): " <<   m_evtwave_ptr-> GetChannelID() << std::endl;
  }

}

//Load file
int iTopRawConverterModule::LoadRawFile(const char* argc)
{
  m_input_file.open(argc, std::ios::binary);
  if (!m_input_file) {
    std::cout << "Couldn't open input file: " << argc << std::endl;
    return -9;
  }  else {
    return 0;
  }
  m_current_pos = m_input_file.tellg();
}


EventHeaderPacket* iTopRawConverterModule::GetEvtHeaderPacket()
{
  EventHeaderPacket* ret = new EventHeaderPacket(*m_EvtPacket);
  return ret;
}

EventWaveformPacket* iTopRawConverterModule::GetWaveformPacket()
{
  EventWaveformPacket* ret = new EventWaveformPacket(*m_WfPacket);
  return ret;
}

int iTopRawConverterModule::FindNextPacket()
{
  //Erase previous packet object
  if (m_WfPacket != NULL) {
    delete m_WfPacket;
    m_WfPacket = NULL;
  }

  if (m_EvtPacket != NULL) {
    delete m_EvtPacket;
    m_EvtPacket = NULL;
  }

  //Check file and if not at End-Of-File
  if (!m_input_file || m_input_file.peek() == EOF) {
    return -11;
  }
  //
  int type = -1;
  m_prev_pos = m_input_file.tellg();

  m_input_file.read((char*) m_temp_buffer, sizeof(packet_word_t)); //Grab one word and shift position (should make it per bit!!!)
  // int pos = m_input_file.tellg();

  unsigned int* word = (unsigned int*) m_temp_buffer;

  //Check if PACKET_HEADER
  if (*word == PACKET_HEADER) {
    //Grab next work which should be the length
    m_input_file.read((char*) m_temp_buffer, sizeof(packet_word_t));

    unsigned int packet_length = *((unsigned int*)(m_temp_buffer));

    //Grab the rest of the packet
    int ndata = (packet_length - 1) * sizeof(packet_word_t);
    if (ndata > 1280) {
      std::cout << "Large payload: " << ndata << " ... skipping!" << std::endl;
      return -1;
    }

    char temp_buffer1[ndata];
    m_input_file.read((char*) temp_buffer1, ndata);
    unsigned int* packet_type = (unsigned int*)(&temp_buffer1[0]);

    if (*packet_type == PACKET_TYPE_EVENT) {
      type = 1;
      m_EvtPacket = new EventHeaderPacket((unsigned int*) temp_buffer1,
                                          ndata / sizeof(packet_word_t));
    } else if (*packet_type == PACKET_TYPE_WAVEFORM) {
      type = 2;
      m_WfPacket = new EventWaveformPacket((unsigned int*) temp_buffer1,
                                           ndata / sizeof(packet_word_t));
    }

    //Do check sum
    unsigned int this_checksum = 0;
    this_checksum += PACKET_HEADER;
    this_checksum += packet_length;
    for (unsigned int i = 0; i < (packet_length - 1); i++) {
      this_checksum += *((unsigned int*)(&temp_buffer1[i * sizeof(packet_word_t)]));
    }

    m_input_file.read((char*) m_temp_buffer, sizeof(packet_word_t));
    unsigned int* checksum = (unsigned int*)(m_temp_buffer);
    if (this_checksum == (*checksum)) {
      return type;
    } else {
      return -1;
    }
  }
  // End of PACKET_HEADER check
  return 0;
}

