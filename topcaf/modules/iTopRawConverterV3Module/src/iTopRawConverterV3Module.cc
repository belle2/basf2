#include <topcaf/modules/iTopRawConverterV3Module/iTopRawConverterV3Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <iostream>
#include <iomanip>

using namespace Belle2;

REG_MODULE(iTopRawConverterV3)



//Constructor
iTopRawConverterV3Module::iTopRawConverterV3Module() : Module()
{
  setDescription("This module is used to upack the raw data from the testbeam and crt data");
  addParam("inputFileName", m_input_filename, "Raw input filename");
  addParam("inputDirectory", m_input_directory, "Raw input file directory");

  m_WfPacket = nullptr;
  m_EvtPacket = nullptr;
}

iTopRawConverterV3Module::~iTopRawConverterV3Module()
{
  m_WfPacket = nullptr;
  m_EvtPacket = nullptr;
}

/////////////////////
// Module function //
/////////////////////
void iTopRawConverterV3Module::initialize()
{
  std::string m_input_fileandpath = m_input_directory + m_input_filename;
  LoadRawFile(m_input_fileandpath.c_str());

  //output
  m_evtheader_ptr.registerInDataStore();
  m_evtwaves_ptr.registerInDataStore();
  m_filedata_ptr.registerInDataStore();
  // data store objects registration
  StoreObjPtr<EventMetaData>::registerPersistent();
}

void iTopRawConverterV3Module::beginRun()
{
  m_filedata_ptr.create();
  m_filedata_ptr->set(m_input_directory, m_input_filename);
  m_evt_no = 0;
}

void iTopRawConverterV3Module::terminate()
{
  m_input_file.close();
}

void iTopRawConverterV3Module::event()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  evtMetaData.create();
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
  int channel = 0;
  const int WAVE_HEADER_SIZE = 6;
  const int WAVE_PACKET_SIZE = WAVE_HEADER_SIZE + WORDS_PER_WINDOW;
  const int EVENT_PACKET_SIZE = 8;
  packet_word_t eventPacket[EVENT_PACKET_SIZE];
  packet_word_t wavePacket[WAVE_PACKET_SIZE];

  packet_word_t word;


  if (not m_input_file) {
    B2WARNING("Reached end of file...");
    evtMetaData->setEndOfData(); // stop event processing
    return;
  }

  m_input_file.read((char*)&cpr_hdr, sizeof(cpr_hdr));
  B2DEBUG(1, "cpr_hdr\n\tformat: 0x" << std::hex << (int)cpr_hdr.version
          << "\n\tblock words: 0x" << (int)cpr_hdr.block_words
          << "\n\trun: 0x" << cpr_hdr.exprun
          << "\n\tevent: " << cpr_hdr.event << " (" << std::dec << cpr_hdr.event << ")"
          << "\n\theader (0x7f7f): 0x" << std::hex << cpr_hdr.CPR_hdr << std::dec);

  if (cpr_hdr.CPR_hdr != 0x7f7f) {
    unsigned short x = 0;
    int c = 0;
    while (x != 0x7f7f) {
      m_input_file.read(reinterpret_cast<char*>(&x), sizeof(unsigned short));
      //      B2INFO("c: "<<c<<"X: 0x"<<std::hex<<x<<std::dec);
      c++;
      if (c > 1e5) {
        B2FATAL("Could not recover from data corruption... giving up.");
      }
    }
    m_prev_pos = m_input_file.tellg();
    m_input_file.seekg(m_prev_pos - 2 * sizeof(int));
    m_input_file.read((char*)&cpr_hdr, sizeof(cpr_hdr));
    B2DEBUG(1, "cpr_hdr\n\tformat: 0x" << std::hex << (int)cpr_hdr.version
            << "\n\tblock words: 0x" << (int)cpr_hdr.block_words
            << "\n\trun: 0x" << cpr_hdr.exprun
            << "\n\tevent: " << cpr_hdr.event << " (" << std::dec << cpr_hdr.event << ")"
            << "\n\theader (0x7f7f): 0x" << std::hex << cpr_hdr.CPR_hdr
            << "\n\ttrig_type: 0x" << cpr_hdr.trig_type
            << "\n\tTT_ctime: 0x" << cpr_hdr.TT_ctime
            << "\n\tTT_utime: 0x" << cpr_hdr.TT_utime
            << "\n\tnode_id: 0x" << cpr_hdr.node_id
            << "\n\tB2L1: 0x" << cpr_hdr.B2L1
            << "\n\toffset_block1: 0x" << cpr_hdr.offset_block1
            << "\n\toffset_block2: 0x" << cpr_hdr.offset_block2
            << "\n\toffset_block3: 0x" << cpr_hdr.offset_block3
            << "\n\toffset_block4: 0x" << cpr_hdr.offset_block4
            << std::dec);

  }

  // identify HSLB buffer sizes
  // determine number of HSLBs in the data stream
  unsigned short asic, win, hslb, maxhslb;
  int bufsize[4];
  bufsize[0] = cpr_hdr.offset_block2 - cpr_hdr.offset_block1;
  bufsize[1] = cpr_hdr.offset_block3 - cpr_hdr.offset_block2;
  bufsize[2] = cpr_hdr.offset_block4 - cpr_hdr.offset_block3;
  bufsize[3] = cpr_hdr.num_words - cpr_hdr.block_words - cpr_hdr.offset_block4 + 125; // not quite right?

  // determine number of HSLBs in the data stream
  if (!bufsize[0])    maxhslb = 0;
  else if (!bufsize[1])  maxhslb = 1;
  else if (!bufsize[2])  maxhslb = 2;
  else if (!bufsize[3])  maxhslb = 3;
  else          maxhslb = 4;
  B2DEBUG(1, "max hslb: " << maxhslb);


  eventPacket[0] = 1001;
  eventPacket[1] = m_scrod;
  eventPacket[2] = cpr_hdr.version;
  eventPacket[3] = m_evt_no;
  eventPacket[4] = cpr_hdr.event;
  eventPacket[5] = 0;
  eventPacket[6] = cpr_hdr.block_words;
  eventPacket[7] = cpr_hdr.exprun;

  m_EvtPacket = new EventHeaderPacket(eventPacket,
                                      EVENT_PACKET_SIZE);
  m_evtheader_ptr.assign(m_EvtPacket, true);
  wavePacket[0] = 1011;

  for (hslb = 0; hslb < maxhslb; hslb++) {

    m_input_file.read((char*)&hslb_hdr, sizeof(hslb_hdr));
    if (hslb_hdr.B2L_hdr != HSLB_B2L_HEADER) {
      B2WARNING("Bad HSLB header... 0x" << std::hex << hslb_hdr.B2L_hdr << " ... aborting event");
      m_evtheader_ptr->SetFlag(1000);
      break;
    }
    m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
    unsigned int eventSize = word;
    m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
    word = swap_endianess(word);
    unsigned int trigPattern = (word >> 28) & 0xF;
    unsigned int numWindows  = (word >> 19) & 0x1FF;
    unsigned int timeStamp   = (word) & 0x7FFFF;
    B2DEBUG(1, "FEE header...(" << std::hex << word << std::dec << ") eventSize: " << eventSize << "\ttrigPattern: 0x" << std::hex <<
            trigPattern
            << "\tnumWindows: 0x" << numWindows
            << "\ttimestamp: 0x" << timeStamp << std::dec);

    bool repeatedCheck = false;

    // Check to see if this looks like selective readout or not...
    if (trigPattern == 0x0) {
      m_prev_pos = m_input_file.tellg();
      packet_word_t word1 = readWordUnique(word);
      int scrod1 = (word1 >> 9) & 0x7F; // should be a scrod id...
      packet_word_t word2 = readWordUnique(word1);
      int scrod2 = (word2 >> 9) & 0x7F; // should be a scrod id if really a trig pattern =0, otherwise should be data...
      if (scrod1 != scrod2) { // Then this looks like data, so guess it is older data and 0x0 actually is 0xF...
        trigPattern = 0xF;
        B2DEBUG(1, "Found older data, changing trigger pattern... word1: 0x"
                << std::hex << word1 << "\t word2: " << word2 << std::dec);
      }
      m_input_file.seekg(m_prev_pos);
    }

    for (win = 0; win <= numWindows ; win++) {
      word = readWordUnique(word);

      m_carrier = (word >> 30) & 0x3;
      asic = (word >> 28) & 0x3;
      unsigned int lastWrAddr = (word >> 16) & 0xFF;
      m_scrod = (word >> 9) & 0x7F;
      readoutWindow = (word) & 0x1FF;

      B2DEBUG(1, "window header (0x" << std::hex << word << std::dec << "... carrier: " << m_carrier << "\tirsx: " << asic
              << "\tlastWrAddr: 0x" << std::hex << lastWrAddr
              << "\tscrod id: 0x" << m_scrod << " (" << std::dec << m_scrod << std::hex << ")"
              << "\tconvAddr: 0x" << readoutWindow << std::dec);


      int row = m_carrier;
      int col = -1;
      for (channel = 0; channel < 8; channel++) {
        int halfChannel = channel / 2;
        if ((trigPattern & (0x1 << halfChannel)) > 0) {
          if (((asic == 2) && (channel < 4)) || ((asic == 3) && (channel > 3))) {
            col = 3;
          } else if (((asic == 2) && (channel > 3)) || ((asic == 3) && (channel < 4))) {
            col = 2;
          } else if (((asic == 0) && (channel < 4)) || ((asic == 1) && (channel > 3))) {
            col = 1;
          } else if (((asic == 0) && (channel > 3)) || ((asic == 1) && (channel < 4))) {
            col = 0;
          }
          B2DEBUG(1, "trigPattern: " << trigPattern << "\tchannel: " << channel << "\tcol: " << col);

          wavePacket[1] = m_scrod;
          wavePacket[2] = lastWrAddr;
          wavePacket[3] = m_evt_no;
          wavePacket[4] = 1;
          wavePacket[5] = (0x1FF & readoutWindow) + (channel << 9) + (row << 12) + (col << 14);
          wavePacket[6] = NPOINTS;

          for (int x = 0; x < NPOINTS / 2; x++) {
            if (!repeatedCheck) {
              word = readWordUnique(word);
              repeatedCheck = true;
            } else {
              m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
              word = swap_endianess(word);
            }
            wavePacket[WAVE_HEADER_SIZE + x + 1] = word;
            B2DEBUG(1, "data[" << x << "]: 0x" << std::hex << word << std::dec);
            if ((word & 0xF000F000) > 0) {
              B2WARNING("data out of range... event " << m_evt_no);
              m_evtheader_ptr->SetFlag(9999);
            }
          }
          m_WfPacket = m_evtwaves_ptr.appendNew(EventWaveformPacket(wavePacket, WAVE_PACKET_SIZE));
          m_WfPacket->SetASIC(asic);
        }
      }

    }
    int l = 0;
    if ((m_evtheader_ptr->GetEventFlag()) >= (9999)) {
      // must read a short at a time to get back on track...
      unsigned short x1, x2;

      while (word != PACKET_LAST && l < 30) {
        x2 = x1;
        m_input_file.read(reinterpret_cast<char*>(&x1), sizeof(unsigned short));
        word = ((x1 << 16) & 0xFFFF0000) + x2;
        B2DEBUG(1, "evt no: " << m_evt_no << "\tl: " << l << "\tx1: 0x" << std::hex << x1 << std::dec);
        if (x1 == 0x616c) {
          word = PACKET_LAST;
        }
        l++;
      }
      m_evtheader_ptr->SetFlag(1005);
    }
    // read ahead
    while (word != PACKET_LAST && m_input_file) {
      m_input_file.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
      word = swap_endianess(word);
      B2DEBUG(1, "footer: 0x" << std::hex << word << std::dec);

      if (word != PACKET_LAST) {
        B2WARNING("Bad window footer (" << std::hex << word << "!=" << PACKET_LAST << std::dec << ")... marking event " << m_evt_no << ".");
        m_evtheader_ptr->SetFlag(1002);
        if (l > 30)  break;
        l++;
      }
    }
    m_input_file.read((char*)&b2l_ftr, sizeof(b2l_ftr));
    if (not m_input_file) {
      B2WARNING("File ended prematurely. Marking event as corrupt.");
      m_evtheader_ptr->SetFlag(1003);
      return;
    }
    B2DEBUG(1, "B2L footer ... B2L_crc16_errs: 0x" << std::hex << b2l_ftr.B2L_crc16_error_cnt << " ... B2L_crc16: 0x" <<
            b2l_ftr.B2L_crc16 << std::dec);

  }

  m_input_file.read((char*)&cpr_ftr, sizeof(cpr_ftr));
  if (not m_input_file) {
    B2WARNING("File ended prematurely. Marking event as corrupt.");
    m_evtheader_ptr->SetFlag(1003);
    evtMetaData->setEndOfData(); // stop event processing
    return;
  }
  B2DEBUG(1, "CPR footer ... word1: 0x" << std::hex << cpr_ftr.word1 << " ... CPR-ftr: 0x" << cpr_ftr.CPR_ftr);
  if (cpr_ftr.CPR_ftr != COPPER_FOOTER) {
    B2WARNING("Bad copper footer found.  Marking event as corrupt.");
    m_evtheader_ptr->SetFlag(1003);
  }
  m_prev_pos = m_input_file.tellg();

}

//Load file
int iTopRawConverterV3Module::LoadRawFile(const char* argc)
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


EventHeaderPacket* iTopRawConverterV3Module::GetEvtHeaderPacket()
{
  EventHeaderPacket* ret = new EventHeaderPacket(*m_EvtPacket);
  return ret;
}

EventWaveformPacket* iTopRawConverterV3Module::GetWaveformPacket()
{
  EventWaveformPacket* ret = new EventWaveformPacket(*m_WfPacket);
  return ret;
}

iTopRawConverterV3Module::packet_word_t iTopRawConverterV3Module::readWordUnique(packet_word_t prev_word)
{

  packet_word_t new_word;

  m_input_file.read(reinterpret_cast<char*>(&new_word), sizeof(packet_word_t));
  new_word = swap_endianess(new_word);


  while ((new_word == prev_word) && (new_word)) {
    //    B2WARNING("Repeated word found....("<<std::hex<<new_word<<std::dec<<")");
    B2DEBUG(1, "Repeated word found...");
    //    m_evtheader_ptr->SetFlag(901);
    prev_word = new_word;
    m_input_file.read(reinterpret_cast<char*>(&new_word), sizeof(packet_word_t));
    new_word = swap_endianess(new_word);

  }

  return new_word;
}

iTopRawConverterV3Module::packet_word_t iTopRawConverterV3Module::swap_endianess(packet_word_t x)
{
  return ((((x) & (0xFF000000)) >> 24) |
          (((x) & (0x00FF0000)) >>  8) |
          (((x) & (0x0000FF00)) <<  8) |
          (((x) & (0x000000FF)) << 24));
}
