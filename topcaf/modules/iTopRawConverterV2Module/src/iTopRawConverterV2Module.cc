#include <topcaf/modules/iTopRawConverterV2Module/iTopRawConverterV2Module.h>

#include <iostream>
#include <iomanip>

using namespace Belle2;

REG_MODULE(iTopRawConverterV2)

//Constructor
iTopRawConverterV2Module::iTopRawConverterV2Module() : Module()
{
  setDescription("This module is used to upack the raw data from the testbeam and crt data");
  addParam("inputFileName", m_input_filename, "Raw input filename");
  addParam("inputDirectory", m_input_directory, "Raw input file directory");
  addParam("carrier", m_carrier, "Carrier read out.");
  addParam("scrod", m_scrod, "Scrod read out.");
  addParam("boardstack", m_module, "Boardstack read out.");

  m_WfPacket = nullptr;
  m_EvtPacket = nullptr;
}

iTopRawConverterV2Module::~iTopRawConverterV2Module()
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
void iTopRawConverterV2Module::initialize()
{
  std::string m_input_fileandpath = m_input_directory + m_input_filename;
  LoadRawFile(m_input_fileandpath.c_str());

  //output
  m_evtheader_ptr.registerInDataStore();
  m_evtwaves_ptr.registerInDataStore();
  m_filedata_ptr.registerInDataStore();
}

void iTopRawConverterV2Module::beginRun()
{
  m_filedata_ptr.create();
  m_filedata_ptr->set(m_input_directory, m_input_filename);
  m_evt_no = 0;
}

void iTopRawConverterV2Module::event()
{
  //output
  m_evtheader_ptr.create();
  //  m_evtheader_ptr.clear();
  //  m_evtwaves_ptr.create();
  m_evtwaves_ptr.clear();
  m_evt_no++;

  //  int packet_type = 0;
  //  int lastWindow = 0;
  int readoutWindow = 0;
  //  int carrier = 0;
  int asic = 0;
  int channel = 0;
  const int WAVE_HEADER_SIZE = 6;
  const int WAVE_PACKET_SIZE = WAVE_HEADER_SIZE + WORDS_PER_WINDOW;
  const int EVENT_PACKET_SIZE = 8;
  packet_word_t eventPacket[EVENT_PACKET_SIZE];
  packet_word_t wavePacket[WAVE_PACKET_SIZE];

  eventPacket[0] = 1001;
  eventPacket[1] = m_scrod;
  eventPacket[2] = 0;
  eventPacket[3] = m_evt_no;
  eventPacket[4] = 0;
  eventPacket[5] = 0;
  eventPacket[6] = 0;
  eventPacket[7] = 0;

  m_EvtPacket = new EventHeaderPacket(eventPacket,
                                      EVENT_PACKET_SIZE);
  EventHeaderPacket* headerpkt = GetEvtHeaderPacket();
  m_evtheader_ptr.assign(headerpkt, true);

  wavePacket[0] = 1011;
  wavePacket[1] = m_scrod;

  packet_word_t word;
  while (true) {
    m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
    if (word == PACKET_LAST) {
      B2WARNING("End of file found.");
      return;
    }
    m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));

    packet_word_t event = 0x7FFFF & word;

    while (word != PACKET_LAST) {
      m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
      if (word == PACKET_LAST) {
        break;
      }
      m_carrier = (word >> 30);
      asic = 0x3 & (word >> 28);
      readoutWindow = 0x1FF & word;
      int row = m_carrier;
      int col = -1;
      for (channel = 0; channel < 8; channel++) {
        if (((asic == 2) && (channel < 4)) || ((asic == 3) && (channel > 3))) {
          col = 3;
        } else if (((asic == 2) && (channel > 3)) || ((asic == 3) && (channel < 4))) {
          col = 2;
        } else if (((asic == 0) && (channel < 4)) || ((asic == 1) && (channel > 3))) {
          col = 1;
        } else if (((asic == 0) && (channel > 3)) || ((asic == 1) && (channel < 4))) {
          col = 0;
        }
        wavePacket[2] = 0;
        wavePacket[3] = event;
        wavePacket[4] = 1;
        wavePacket[5] = (0x1FF & readoutWindow) + (channel << 9) + (row << 12) + (col << 14);
        wavePacket[6] = NPOINTS;

        for (int x = 0; x < NPOINTS / 2; x++) {
          m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
          wavePacket[WAVE_HEADER_SIZE + x + 1] = word;
        }
        m_WfPacket = new EventWaveformPacket(wavePacket, WAVE_PACKET_SIZE);
        m_evtwaves_ptr.appendNew(EventWaveformPacket(*m_WfPacket));
      }
    }
    m_prev_pos = m_input_file.tellg();
    // read ahead
    m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
    m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
    packet_word_t next_event = (0x7FFFF) & word;
    Rewind();
    if (next_event != event) { // Last ASIC read out for this event...
      break;
    }
  }
}

//Load file
int iTopRawConverterV2Module::LoadRawFile(const char* argc)
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


EventHeaderPacket* iTopRawConverterV2Module::GetEvtHeaderPacket()
{
  EventHeaderPacket* ret = new EventHeaderPacket(*m_EvtPacket);
  return ret;
}

EventWaveformPacket* iTopRawConverterV2Module::GetWaveformPacket()
{
  EventWaveformPacket* ret = new EventWaveformPacket(*m_WfPacket);
  return ret;
}
