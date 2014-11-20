#include <topcaf/modules/iTopRawConverterModule/iTopRawConverterModule.h>

#include <iostream>

using namespace Belle2;

REG_MODULE(iTopRawConverter)

//Constructor
iTopRawConverterModule::iTopRawConverterModule() : Module()
{
  setDescription("This module is used to upack the raw data from the testbeam and crt data");
  addParam("InputFileName", m_input_filename, "Raw input filename");
  addParam("InputDirectory", m_input_directory, "Raw input file directory");

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
  std::string m_input_fileandpath = m_input_directory + m_input_filename;

  LoadRawFile(m_input_fileandpath.c_str());

  //output

  m_evtheader_ptr.registerInDataStore();
  m_evtwaves_ptr.registerInDataStore();
  m_filedata_ptr.registerInDataStore();



}

void iTopRawConverterModule::beginRun()
{
  m_filedata_ptr.create();
  m_filedata_ptr->set(m_input_directory, m_input_filename);
}

void iTopRawConverterModule::event()
{

  //output
  m_evtheader_ptr.create();
  //  m_evtheader_ptr.clear();
  //  m_evtwaves_ptr.create();
  m_evtwaves_ptr.clear();

  int packet_type = 0;
  while (packet_type > -1) {
    packet_type = FindNextPacket();

    if (packet_type == -1) {
      B2ERROR("Corrupt packet found.");
      break;
    } else if (packet_type == -11) {
      B2WARNING("End of file.");
      break;
    } else if (packet_type == 1) {
      EventHeaderPacket* headerpkt = GetEvtHeaderPacket();
      if (m_evtheader_ptr->GetPacketType() == -1)
        m_evtheader_ptr.assign(headerpkt, true);
      else if (headerpkt->GetEventNumber() != m_evtheader_ptr->GetEventNumber()) {
        // Found next event.  Rewind and break.
        Rewind();
        //  B2INFO(" itop event " << m_evtheader_ptr->GetEventNumber() << " converted with " << m_evtwaves_ptr.getEntries() << " waveform packets.");
        break;
      }
    } else if (packet_type == 2) {
      //    B2INFO("Found Waveform Packet Type: " << packet_type);
      //      m_evtwaves_ptr.appendNew(GetWaveformPacket());
      m_evtwaves_ptr.appendNew(EventWaveformPacket(*m_WfPacket));
      //    B2INFO(" GetChannelID(): " <<   m_evtwave_ptr-> GetChannelID());
    }
  }

}

//Load file
int iTopRawConverterModule::LoadRawFile(const char* argc)
{
  m_input_file.open(argc, std::ios::binary);
  if (!m_input_file) {
    B2ERROR("Couldn't open input file: " << argc);
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
      B2WARNING("Large payload: " << ndata << " ... skipping!");
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

