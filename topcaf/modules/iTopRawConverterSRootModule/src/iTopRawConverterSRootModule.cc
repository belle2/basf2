#include <topcaf/modules/iTopRawConverterSRootModule/iTopRawConverterSRootModule.h>
#include <iomanip>

using namespace Belle2;
using namespace std;

REG_MODULE(iTopRawConverterSRoot)

// We just need a wrapper class to be able to use the int* in a streambuf
// inspired by http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
// and http://stackoverflow.com/questions/7781898/get-an-istream-from-a-char
class intbuf : public streambuf {
public:
  intbuf(int* begin, size_t size)
  {
    char* start = reinterpret_cast<char*>(begin);
    char* end = reinterpret_cast<char*>(begin + size);
    setg(start, start, end);
  }
  int_type underflow()
  {
    return gptr() == egptr()
           ? traits_type::eof()
           : traits_type::to_int_type(*gptr());
  }
};

//Constructor
iTopRawConverterSRootModule::iTopRawConverterSRootModule() : Module()
{
  setDescription("This module is used to upack the raw data from the testbeam and crt data");

  m_WfPacket = nullptr;
  m_EvtPacket = nullptr;
}

iTopRawConverterSRootModule::~iTopRawConverterSRootModule()
{
  m_WfPacket = nullptr;
  m_EvtPacket = nullptr;
}

/////////////////////
// Module function //
/////////////////////
void
iTopRawConverterSRootModule::initialize()
{
  //output
  m_evtheader_ptr.registerInDataStore();
  m_evtwaves_ptr.registerInDataStore();
  // m_filedata_ptr.registerInDataStore();
}

void
iTopRawConverterSRootModule::beginRun()
{
  // m_filedata_ptr.create();
  // m_filedata_ptr->set(m_input_directory, m_input_filename);
  m_evt_no = 0;
}

void
iTopRawConverterSRootModule::terminate()
{
}


int
iTopRawConverterSRootModule::parseData(istream& in)
{
  //
  // //output
  // m_evtheader_ptr.create();
  // //  m_evtheader_ptr.clear();
  // //  m_evtwaves_ptr.create();
  // m_evtwaves_ptr.clear();
  // m_evt_no++;
  //
  // //  int packet_type = 0;
  // //  int lastWindow = 0;
  int readoutWindow = 0;
  // //  int carrier = 0;
  int channel = 0;
  const int WAVE_HEADER_SIZE = 6;
  const int WAVE_PACKET_SIZE = WAVE_HEADER_SIZE + WORDS_PER_WINDOW;
  const int EVENT_PACKET_SIZE = 8;
  packet_word_t eventPacket[EVENT_PACKET_SIZE];
  packet_word_t wavePacket[WAVE_PACKET_SIZE];

  packet_word_t word;
  int prev_pos = 0;
  in.read(reinterpret_cast<char*>(&cpr_hdr), sizeof(cpr_hdr));
  B2DEBUG(1, "cpr_hdr\n\tformat: 0x" << hex << (int)cpr_hdr.version
          << "\n\tblock words: 0x" << (int)cpr_hdr.block_words
          << "\n\trun: 0x" << cpr_hdr.exprun
          << "\n\tevent: " << cpr_hdr.event << " (" << dec << cpr_hdr.event << ")"
          << "\n\theader (0x7f7f): 0x" << hex << cpr_hdr.CPR_hdr << dec);

  if (cpr_hdr.CPR_hdr != 0x7f7f) {
    unsigned short x = 0;
    int c = 0;
    while (x != 0x7f7f) {
      in.read(reinterpret_cast<char*>(&x), sizeof(unsigned short));
      B2INFO("c: " << c << "X: 0x" << hex << x << dec);
      c++;
      if (c > 1e5) {
        B2FATAL("Could not recover from data corruption... giving up.");
      }
    }
    prev_pos = in.tellg();
    in.seekg(prev_pos - 2 * sizeof(int));
    in.read((char*)&cpr_hdr, sizeof(cpr_hdr));
    B2DEBUG(1, "cpr_hdr\n\tformat: 0x" << hex << (int)cpr_hdr.version
            << "\n\tblock words: 0x" << (int)cpr_hdr.block_words
            << "\n\trun: 0x" << cpr_hdr.exprun
            << "\n\tevent: " << cpr_hdr.event << " (" << dec << cpr_hdr.event << ")"
            << "\n\theader (0x7f7f): 0x" << hex << cpr_hdr.CPR_hdr
            << "\n\ttrig_type: 0x" << cpr_hdr.trig_type
            << "\n\tTT_ctime: 0x" << cpr_hdr.TT_ctime
            << "\n\tTT_utime: 0x" << cpr_hdr.TT_utime
            << "\n\tnode_id: 0x" << cpr_hdr.node_id
            << "\n\tB2L1: 0x" << cpr_hdr.B2L1
            << "\n\toffset_block1: 0x" << cpr_hdr.offset_block1
            << "\n\toffset_block2: 0x" << cpr_hdr.offset_block2
            << "\n\toffset_block3: 0x" << cpr_hdr.offset_block3
            << "\n\toffset_block4: 0x" << cpr_hdr.offset_block4
            << dec);

  }

  // // identify HSLB buffer sizes
  // // determine number of HSLBs in the data stream
  unsigned short asic, win, hslb, maxhslb;
  int bufsize[4];
  bufsize[0] = cpr_hdr.offset_block2 - cpr_hdr.offset_block1;
  bufsize[1] = cpr_hdr.offset_block3 - cpr_hdr.offset_block2;
  bufsize[2] = cpr_hdr.offset_block4 - cpr_hdr.offset_block3;
  bufsize[3] = cpr_hdr.num_words - cpr_hdr.block_words - cpr_hdr.offset_block4 + 125; // not quite right?
  //
  // // determine number of HSLBs in the data stream
  if (!bufsize[0])    {maxhslb = 0;}
  else if (!bufsize[1])  {maxhslb = 1;}
  else if (!bufsize[2])  {maxhslb = 2;}
  else if (!bufsize[3])  {maxhslb = 3;}
  else          {maxhslb = 4;}
  B2DEBUG(1, "max hslb: " << maxhslb);
  //
  //
  eventPacket[0] = 1001;
  eventPacket[1] = m_scrod;
  eventPacket[2] = cpr_hdr.version;
  eventPacket[3] = m_evt_no;
  eventPacket[4] = cpr_hdr.event;
  eventPacket[5] = 0;
  eventPacket[6] = cpr_hdr.block_words;
  eventPacket[7] = cpr_hdr.exprun;

  EventHeaderPacket* evtPacket = new EventHeaderPacket(eventPacket,
                                                       EVENT_PACKET_SIZE);
  m_evtheader_ptr.assign(evtPacket, true);
  wavePacket[0] = 1011;
  for (hslb = 0; hslb < maxhslb; hslb++) {
    in.read(reinterpret_cast<char*>(&hslb_hdr), sizeof(hslb_hdr));
    if (hslb_hdr.B2L_hdr != HSLB_B2L_HEADER) {
      B2WARNING("Bad HSLB header... 0x" << hex << hslb_hdr.B2L_hdr << " ... aborting event");
      m_evtheader_ptr->SetFlag(1000);
      break;
    }
    in.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
    unsigned int eventSize = word;
    in.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
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
      prev_pos = in.tellg();
      packet_word_t word1 = readWordUnique(in, word);
      int scrod1 = (word1 >> 9) & 0x7F; // should be a scrod id...
      packet_word_t word2 = readWordUnique(in, word1);
      int scrod2 = (word2 >> 9) & 0x7F; // should be a scrod id if really a trig pattern =0, otherwise should be data...
      if (scrod1 != scrod2) { // Then this looks like data, so guess it is older data and 0x0 actually is 0xF...
        trigPattern = 0xF;
        B2DEBUG(1, "Found older data, changing trigger pattern... word1: 0x"
                << std::hex << word1 << "\t word2: " << word2 << std::dec);
      }
      in.seekg(prev_pos);
    }

    for (win = 0; win <= numWindows ; win++) {
      word = readWordUnique(in, word);

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
          wavePacket[2] = 0;
          wavePacket[3] = m_evt_no;
          wavePacket[4] = 1;
          wavePacket[5] = (0x1FF & readoutWindow) + (channel << 9) + (row << 12) + (col << 14);
          wavePacket[6] = NPOINTS;

          for (int x = 0; x < NPOINTS / 2; x++) {
            if (!repeatedCheck) {
              word = readWordUnique(in, word);
              repeatedCheck = true;
            } else {
              in.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
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
        in.read(reinterpret_cast<char*>(&x1), sizeof(unsigned short));
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
    while (word != PACKET_LAST && in) {
      in.read(reinterpret_cast<char*>(&word), sizeof(packet_word_t));
      word = swap_endianess(word);
      B2DEBUG(1, "footer: 0x" << std::hex << word << std::dec);

      if (word != PACKET_LAST) {
        B2WARNING("Bad window footer (" << std::hex << word << "!=" << PACKET_LAST << std::dec << ")... marking event " << m_evt_no << ".");
        m_evtheader_ptr->SetFlag(1002);
        if (l > 30)  break;
        l++;
      }
    }
    in.read((char*)&b2l_ftr, sizeof(b2l_ftr));
    if (not in) {
      B2WARNING("File ended prematurely. Marking event as corrupt.");
      m_evtheader_ptr->SetFlag(1003);
      return -1;
    }
    B2DEBUG(1, "B2L footer ... B2L_crc16_errs: 0x" << std::hex << b2l_ftr.B2L_crc16_error_cnt << " ... B2L_crc16: 0x" <<
            b2l_ftr.B2L_crc16 << std::dec);
  }

  in.read((char*)&cpr_ftr, sizeof(cpr_ftr));
  if (not in) {
    B2WARNING("File ended prematurely. Marking event as corrupt.");
    m_evtheader_ptr->SetFlag(1003);
    return -2;
  }
  B2DEBUG(1, "CPR footer ... word1: 0x" << std::hex << cpr_ftr.word1 << " ... CPR-ftr: 0x" << cpr_ftr.CPR_ftr);
  if (cpr_ftr.CPR_ftr != COPPER_FOOTER) {
    B2WARNING("Bad copper footer found.  Marking event as corrupt.");
    m_evtheader_ptr->SetFlag(1003);
  }
  prev_pos = in.tellg();
  return 0;
}

void
iTopRawConverterSRootModule::event()
{

  StoreArray<RawDataBlock> raw_dblkarray;

  size_t array_entries = raw_dblkarray.getEntries();
  // B2DEBUG(1, "Number of RAW entries: " << array_entries);
  // There should only be one entry per "event"
  for (size_t i = 0; i < array_entries; i++) {
    RawDataBlock* raw = raw_dblkarray[i];
    //   B2DEBUG(1, "Number of RAW Entries: " << raw->GetNumEntries());
    //   B2DEBUG(1, "Number of RAW Events: " << raw->GetNumEvents());
    //   B2DEBUG(1, "Number of RAW Nodes: " << raw->GetNumNodes());
    // There should only be one "event" in the block
    int* data = raw->GetWholeBuffer();
    size_t nWords = raw->TotalBufNwords();
    intbuf buffer(data, nWords);
    istream in(&buffer);
    parseData(in);
  }
}


EventHeaderPacket*
iTopRawConverterSRootModule::GetEvtHeaderPacket()
{
  EventHeaderPacket* ret = new EventHeaderPacket(*m_EvtPacket);
  return ret;
}

EventWaveformPacket*
iTopRawConverterSRootModule::GetWaveformPacket()
{
  EventWaveformPacket* ret = new EventWaveformPacket(*m_WfPacket);
  return ret;
}

iTopRawConverterSRootModule::packet_word_t
iTopRawConverterSRootModule::readWordUnique(istream& in, packet_word_t prev_word)
{
  packet_word_t new_word;
  in.read(reinterpret_cast<char*>(&new_word), sizeof(packet_word_t));
  new_word = swap_endianess(new_word);
  while ((new_word == prev_word) && (new_word)) {
    B2DEBUG(1, "Repeated word found...");
    m_evtheader_ptr->SetFlag(901);
    prev_word = new_word;
    in.read(reinterpret_cast<char*>(&new_word), sizeof(packet_word_t));
    new_word = swap_endianess(new_word);
  }
  return new_word;
}

iTopRawConverterSRootModule::packet_word_t
iTopRawConverterSRootModule::swap_endianess(packet_word_t x)
{
  return ((((x) & (0xFF000000)) >> 24) |
          (((x) & (0x00FF0000)) >>  8) |
          (((x) & (0x0000FF00)) <<  8) |
          (((x) & (0x000000FF)) << 24));
}
