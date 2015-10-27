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

  m_WfPacket = NULL;
  m_EvtPacket = NULL;
}

iTopRawConverterV2Module::~iTopRawConverterV2Module()
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
  int c = 0;
  //  int lastWindow = 0;
  int readoutWindow = 0;
  //  int carrier = 0;
  int asic = 0;
  int channel = 0;
  const int waveHeaderSize = 6;
  const int wavePacketSize = waveHeaderSize + WORDS_PER_WINDOW;
  const int eventPacketSize = 8;
  unsigned int eventPacket[eventPacketSize];
  unsigned int wavePacket[wavePacketSize];

  eventPacket[0] = 1001;
  eventPacket[1] = m_scrod;
  eventPacket[2] = 0;
  eventPacket[3] = m_evt_no;
  eventPacket[4] = 0;
  eventPacket[5] = 0;
  eventPacket[6] = 0;
  eventPacket[7] = 0;


  m_EvtPacket = new EventHeaderPacket(eventPacket,
                                      eventPacketSize);
  EventHeaderPacket* headerpkt = GetEvtHeaderPacket();
  m_evtheader_ptr.assign(headerpkt, true);


  wavePacket[0] = 1011;
  wavePacket[1] = m_scrod;


  while (1 == 1) {
    m_input_file.read((char*) m_temp_buffer, sizeof(packet_word_t)); //Grab one word and shift position (should make it per bit!!!)
    unsigned int* word = (unsigned int*) m_temp_buffer;
    unsigned int totalword = *word;
    if (totalword == 0x6c617374) {
      B2WARNING("End of file found.");
      return;
    }
    m_input_file.read((char*) m_temp_buffer, sizeof(packet_word_t));
    totalword--;//Grab one word and shift position (should make it per bit)

    word = (unsigned int*) m_temp_buffer;

    unsigned int event = (0x7FFFF) & (*word);
    //    unsigned int trigger_pattern = (0xF) & ((*word) >> 28);
    unsigned int windows = (0x1FF) & ((*word) >> 19);
    windows++;

    /*
    B2INFO("total words : "<<(totalword)
         <<"\theader: "<<std::hex<<std::setfill('0')<<std::setw(8)<<(*word)<<std::dec
         <<"\tevent : "<<event<<"\ttrig pattern: "
         <<trigger_pattern<<"\tnum windows: "<<windows);
    */

    while ((*word) != 0x6c617374) {
      m_input_file.read((char*) m_temp_buffer, sizeof(packet_word_t));
      totalword--;//Grab one word and shift position (should make it pssser bit!!!)
      word = (unsigned int*) m_temp_buffer;
      if ((*word) == 0x6c617374) break;
      m_carrier = ((*word) >> 30);
      asic = 0x3 & ((*word) >> 28);
      //      lastWindow = ((*word) >> 16) & 0x00FF;
      readoutWindow = 0x1FF & (*word);
      //      B2INFO("total words : "<<(totalword)<<"\tread : 0x"<<std::hex<<std::setfill('0')<<std::setw(8)<<(*word)<<std::dec);
      //      B2INFO("carrier: "<<m_carrier<<"\tasic: "<<asic<<"\tlastWrAddr: "<<lastWindow<<"\tconvertAddr: "<<readoutWindow);
      int row = m_carrier;
      int col = -1;
      for (channel = 0; channel < 8; channel++) {
        if (((asic == 2) && (channel < 4)) || ((asic == 3) && (channel > 3))) col = 3;
        else if (((asic == 2) && (channel > 3)) || ((asic == 3) && (channel < 4))) col = 2;
        else if (((asic == 0) && (channel < 4)) || ((asic == 1) && (channel > 3))) col = 1;
        else if (((asic == 0) && (channel > 3)) || ((asic == 1) && (channel < 4))) col = 0;
        wavePacket[2] = 0;
        wavePacket[3] = event;
        wavePacket[4] = 1;
        wavePacket[5] = (0x1FF & readoutWindow) + (channel << 9) + (row << 12) + (col << 14);
        wavePacket[6] = NPOINTS;

        for (int x = 0; x < NPOINTS / 2; x++) {
          m_input_file.read((char*) m_temp_buffer, sizeof(packet_word_t));
          totalword--;//Grab one word and shift position (should make it per bit!!!)
          word = (unsigned int*) m_temp_buffer;
          wavePacket[waveHeaderSize + x + 1] = (*word);
          //    B2INFO("\t0x"<<std::hex<<std::setfill('0')<<std::setw(8)<<(*word)<<std::dec);

        }

        //  B2INFO("readoutWindow: "<<readoutWindow<<"\tlastWindow: "<<lastWindow<<"\tasic: "<<asic<<"\tcarrier: "<<m_carrier<<"\tscrod: "<<m_scrod<<"\tmodule: "<<m_module<<"\tchannel: "<<channel<<"\tnumSamples: "<<wavePacket[6]<<"\tpacket[5]: "<<wavePacket[5]);
        m_WfPacket = new EventWaveformPacket(wavePacket,
                                             wavePacketSize);
        m_evtwaves_ptr.appendNew(EventWaveformPacket(*m_WfPacket));
      }

    }
    c++;
    m_prev_pos = m_input_file.tellg();
    m_input_file.read((char*) m_temp_buffer, sizeof(packet_word_t));
    totalword--;//Grab one word and shift position (should make it per bit!!!)
    word = (unsigned int*) m_temp_buffer;
    m_input_file.read((char*) m_temp_buffer, sizeof(packet_word_t));
    totalword--;//Grab one word and shift position (should make it per bit!!!)
    word = (unsigned int*) m_temp_buffer;
    unsigned int next_event = (0x7FFFF) & (*word);
    if (next_event != event) { // Last ASIC read out for this event...
      Rewind();
      break;
    }
    Rewind();

  }
  /*


  packet_type = FindNextPacket();
  c++; if(c==1000000) break;
  //    B2INFO("Processing packet with type "<<packet_type);
  if (packet_type == -1) {
    B2WARNING("Corrupt packet found.");
    //      break;
  } else if (packet_type == -11) {
    B2WARNING("End of file.");
    break;
  } else if (packet_type == 1||packet_type==0) {
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
  packet_type=-1;
  }
  */
  //  }
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

int iTopRawConverterV2Module::FindNextPacket()
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
  //  unsigned int word_value = *word;

  //  B2INFO("packet word: 0x"<<std::hex<<word_value);

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
  return -1;
}

