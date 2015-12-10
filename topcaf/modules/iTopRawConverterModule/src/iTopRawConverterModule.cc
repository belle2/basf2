#include <topcaf/modules/iTopRawConverterModule/iTopRawConverterModule.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace Belle2;

REG_MODULE(iTopRawConverter)

//Constructor
iTopRawConverterModule::iTopRawConverterModule() : Module()
{
  setDescription("This module is used to upack the raw data from the testbeam and crt data");
  addParam("inputFileName", m_input_filename, "Raw input filename");
  addParam("inputDirectory", m_input_directory, "Raw input file directory");

  m_WfPacket = nullptr;
  m_EvtPacket = nullptr;
}

iTopRawConverterModule::~iTopRawConverterModule()
{
  if (m_WfPacket != nullptr)
    delete m_WfPacket;
  if (m_EvtPacket != nullptr)
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
  if (m_WfPacket != nullptr) {
    delete m_WfPacket;
    m_WfPacket = nullptr;
  }

  if (m_EvtPacket != nullptr) {
    delete m_EvtPacket;
    m_EvtPacket = nullptr;
  }

  //Check file and if not at End-Of-File
  if (!m_input_file || m_input_file.peek() == EOF) {
    return -11;
  }
  //
  int type = -1;
  m_prev_pos = m_input_file.tellg();

  vector<packet_word_t> buffer(2);
  m_input_file.read(reinterpret_cast<char*>(buffer.data()),
                    2 * sizeof(packet_word_t)); //Grab two words: Header and length and shift position

  packet_word_t word = buffer.at(0);
  if (word == PACKET_HEADER) {
    packet_word_t packet_length = buffer.at(1);

    //Grab the rest of the packet
    int ndata = (packet_length - 1) * sizeof(packet_word_t);
    if (ndata > 1280) {
      B2WARNING("Large payload: " << ndata << " ... skipping!");
      return -1;
    }

    vector<packet_word_t> data(ndata / sizeof(packet_word_t) + 1);
    m_input_file.read(reinterpret_cast<char*>(data.data()), ndata);
    packet_word_t packet_type = data.at(0);

    if (packet_type == PACKET_TYPE_EVENT) {
      type = 1;
      m_EvtPacket = new EventHeaderPacket(data.data(),
                                          ndata / sizeof(packet_word_t));
    } else if (packet_type == PACKET_TYPE_WAVEFORM) {
      type = 2;
      m_WfPacket = new EventWaveformPacket(data.data(),
                                           ndata / sizeof(packet_word_t));
    }

    //Do check sum
    packet_word_t this_checksum = 0;
    this_checksum += PACKET_HEADER;
    this_checksum += packet_length;
    for (unsigned int i = 0; i < (packet_length - 1); i++) {
      this_checksum += data.at(i);
    }

    m_input_file.read(reinterpret_cast<char*>(buffer.data()), sizeof(packet_word_t));
    packet_word_t checksum = buffer.at(0);
    if (this_checksum == checksum) {
      return type;
    } else {
      return -1;
    }
  }
  // End of PACKET_HEADER check
  return 0;
}
