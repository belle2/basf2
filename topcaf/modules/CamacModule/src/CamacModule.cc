//
// version of camacConverter used for 2014 KEK cosmic ray test data
//
//   LeCroy 2228A in slot 6, T0 (FTSW) data in channel 5
//

#include <topcaf/modules/CamacModule/CamacModule.h>

#include <topcaf/dataobjects/EventHeaderPacket.h>

#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace Belle2;

REG_MODULE(Camac);
//Constructor
CamacModule::CamacModule() : Module(), m_c3377nhit(0)
{

  setDescription("This module is used to read auxliary CAMAC data for itop test beams.  Almost always needed for older versions of the electronics to get the FTSW timing.");

  addParam("InputFilename", m_in_filename, "The input .cmc file to be parsed.");
  addParam("CrateID", m_crateID, "The crate containing the FTSW TDC.");
  addParam("FTSW_TDC_Timing", m_ftswTDCtiming, "The timing of the FTSW TDC counts in nanoseconds.", 0.045056);
  addParam("FTSWslot", m_ftswSlot, "The slot number of the TDC containing the FTSW signal.");
  addParam("FTSWword", m_ftswWord, "The word number of the FTSW time.");

}

//Destructor
CamacModule::~CamacModule() {}

void CamacModule::initialize()
{

  std::memset(m_c3377tdc, 0, sizeof(m_c3377tdc));
  std::memset(m_c3377lt, 0, sizeof(m_c3377lt));
  std::memset(m_c3377ch, 0, sizeof(m_c3377ch));

  m_camac_data.registerInDataStore();

  LoadCamacFile();

}

void CamacModule::beginRun()
{

}

void CamacModule::event()
{
  m_camac_data.create();

  StoreObjPtr<EventHeaderPacket> evtheader_ptr;
  evtheader_ptr.isRequired();
  if (evtheader_ptr) {

    FindNextPacket();
    evtheader_ptr->SetFTSW((m_camac_data->GetRawTDC()*m_ftswTDCtiming));

  }
}

void CamacModule::terminate()
{
}

//Load file
int CamacModule::LoadCamacFile()
{
  m_input_file.open(m_in_filename.c_str());
  if (!m_input_file) {
    B2ERROR("Couldn't open input file: " << m_in_filename);
    return -9;
  }  else {
    return 0;
  }
  m_current_pos = m_input_file.tellg();
}

int CamacModule::FindNextPacket(int evtno_req)
{

  //Check file and if not at End-Of-File
  if (!m_input_file || m_input_file.peek() == EOF) {
    m_camac_data->SetRawTDC(-9);
    m_camac_data->SetEventNumber(-9);

    return -11;
  }

  // unsigned int long_word = 0;
  // unsigned short int short_word = 0;
  // unsigned short int words_this_event = 0;
  int start_position = m_input_file.tellg();
  static int next_position = start_position;
  static int last_position = next_position;
  // bool empty_flag[2] = {false,false};
  // int size_in_bytes;
  const int N_crates = 1;


  for (int i = 0; i < N_crates; ++i) {

    /*
    int start_position_this_crate = m_input_file.tellg();

    B2INFO("-------------------");
    B2INFO("Reading crate " << i);
    B2INFO("-------------------");
    B2INFO("Current position in file : " << start_position_this_crate);
    B2INFO("Previous event started at: " << last_position);
    B2INFO("Next position            : " << next_position);
    */

    if (next_position != 0) {
      m_input_file.seekg(next_position);
    }
    last_position = next_position;
    //std::cout << "Time: " << unix_time_sec << "." << unix_time_msec << std::endl;
    //std::cout << "Size: " << size_in_bytes << std::endl;


    unsigned int crate_id;
    m_input_file.read((char*) &crate_id, sizeof(unsigned int));    //Crate ID
    //    B2INFO("Value: " << std::hex << crate_id << std::dec << "/" << crate_id);
    if (crate_id == m_crateID) {
      //std::cout << "Crate ID: " << std::hex << crate_id << std::dec << "/" << crate_id << std::endl;
      //Read header
      int evt_num;
      m_input_file.read((char*) &evt_num, sizeof(int));    //Event Number
      if (evtno_req > -1 && AlignEvtno(evtno_req, evt_num)) {
        m_input_file.seekg(last_position);
        return -7;
      }

      unsigned int time_sec;
      m_input_file.read((char*) &time_sec, sizeof(unsigned int));    //Time seconds
      unsigned int time_msec;
      m_input_file.read((char*) &time_msec, sizeof(unsigned int));    //Time mseconds
      unsigned int evt_nbytes;
      m_input_file.read((char*) &evt_nbytes, sizeof(unsigned int));    //Number of bytes for event
      unsigned short usb_buffer_nevt;
      m_input_file.read((char*) &usb_buffer_nevt, sizeof(unsigned short));    //Number of events in USB buffer
      unsigned short evt_nwords;
      m_input_file.read((char*) &evt_nwords, sizeof(unsigned short));    //Number of words in event
      //      B2INFO("CamacCnv::EvtNum " << evt_num);
      m_camac_data->SetEventNumber(evt_num);

      ///////////////////////////////////////////
      // SLOT FORMAT - Loop over slots/modules //
      ///////////////////////////////////////////
      unsigned int test_position = 0;
      unsigned short previous_position, current_slot;
      unsigned short word_count = 0;
      bool isFirst = true;
      //unsigned short our_tdc=0;
      //unsigned short their_tdc=0;
      while (test_position != crate_id) {
        unsigned short word;
        m_input_file.read((char*) &word, sizeof(unsigned short));    //Slot number

        if (previous_position == 0xDDDD || isFirst) {
          current_slot = word;
          word_count = 0;
          isFirst = false;
        }

        // LeCroy 2228A - T0
        if (current_slot == m_ftswSlot) {
          //word_count++;
          //  std::cout << "## evt/word#" << word_count << "/pre: " << evt_num << "/"
          //                << word << "(" << std::hex << word << ")" << std::dec << "/"
          //              << previous_position << "(" << std::hex << previous_position << ")" << std::dec
          //            << std::endl;
          //printf("2228A:  [%2d] %d 0x%04x\n", word_count, word, word);


          if (word_count == m_ftswWord) m_camac_data->SetRawTDC(word);
          //          switch (word_count) {

          //    case  ftswWord: m_camac_data->SetRawTDC(word);  break;

          //  case  3: m_camac_data->SetRawTDC(word);  break;
          //  case  5: m_camac_data->SetRawRF0TDC(word);  break;
          //  case  7: m_camac_data->SetRawRF1TDC(word);  break;
          //  case  9: m_camac_data->SetRawRF2TDC(word);  break;
          //  case 11: m_camac_data->SetRawRF3TDC(word);  break;
          //          }
        }

        if (word == 65535 && previous_position == 65535) {
          //std::cout << "Footer ... exiting?" << std::endl;
          break;
        }

        previous_position = word;
        word_count++;

      }
      //double tdc_diff=(int)(their_tdc-our_tdc);
      //m_camac_data->SetRawTDC(their_tdc);
      //m_camac_data->SetTheirTDC(their_tdc);

      //std::cout << "TDC OUR/THEIR: " << "/" << their_tdc << "-" << our_tdc << std::endl;
      //std::cout << "Out..." << std::endl;

      // std::cout << "Wordmap has size " << ftwmap.size() << std::endl;


      return 1;

      /*
        m_input_file.seekg(next_position);

        unsigned short slot_num;
        m_input_file.read( (char *) &slot_num, sizeof(unsigned short));  //Slot number

        std::cout << "Crate ID: " << std::hex << crate_id << std::dec << "/" << crate_id << std::endl;
        std::cout << "Event #: " << evt_num << std::endl;
        std::cout << "Slot #: " << slot_num << std::endl;
        m_camac_data->SetEventNumber(evt_num);

        for (int s = 1; s <= N_SLOTS; ++s) { //CAMAC slots start from 1... I'm getting tired of this.  =P
        unsigned short slot;
        m_input_file.read( (char *) &slot, sizeof(unsigned short));  //Slot number
        std::cout << "word #" << s << ": " << slot
        << "(" << std::hex << slot << ")" << std::dec << std::endl;
        if(s==3){
        //std::cout << "Slot #" << s << ": " << slot
        //<< "(" << std::hex << slot << ")" << std::dec << std::endl;
        m_camac_data->SetRawTDC(slot);
        }
        }
        unsigned int term;
        m_input_file.read( (char *) &term, sizeof(unsigned int));  //Number of bytes for event
        }
      */

    }
  }


  return 1;
}

int CamacModule::AlignEvtno(int evtno_req, int& evtno_read)
{
  if (evtno_read < evtno_req) {
    unsigned int crate_id;
    unsigned int time_sec;
    unsigned int time_msec;
    unsigned int evt_nbytes;
    unsigned short usb_buffer_nevt;
    unsigned short evt_nwords;

    while (evtno_read < evtno_req) {
      m_input_file.read((char*) &time_sec, sizeof(unsigned int));
      m_input_file.read((char*) &time_msec, sizeof(unsigned int));
      m_input_file.read((char*) &evt_nbytes, sizeof(unsigned int));
      m_input_file.read((char*) &usb_buffer_nevt, sizeof(unsigned short));
      m_input_file.read((char*) &evt_nwords, sizeof(unsigned short));

      unsigned long pos = m_input_file.tellg();
      m_input_file.seekg(pos + evt_nbytes - 4);

      m_input_file.read((char*) &crate_id, sizeof(unsigned int));
      if (crate_id == 13369927) {
        m_input_file.read((char*) &evtno_read, sizeof(unsigned int));
      }
    }
  }

  // If the event number read back from the .cmc file was less than the event
  // number passed in, we skip events until we find an event number that is
  // equal or greater than the requested event number.
  if (evtno_req == evtno_read)
    return 0;

  else
    std::cerr << "CamacModule::FindNextPacket: event " << evtno_req
              << " requested, but CMC stream has advanced to event "
              << evtno_read << std::endl;
  return -1;

}


//Utils?
int ReadUnixTimeAndSize(std::ifstream& fin, int& unix_time_sec, int& unix_time_msec, int& size)
{
  //1. Unix time in seconds (as long int)
  fin.read((char*) &unix_time_sec     , sizeof(unix_time_sec));
  //2. Unix time in milliseconds (as long int)
  fin.read((char*) &unix_time_msec, sizeof(unix_time_msec));
  printf("DEBUG: Unix time: %d.%d\n", unix_time_sec, unix_time_msec);
  //3. Readout size in bytes (as int)
  fin.read((char*) &size, sizeof(size));
  if (fin) {
    return 0;
  } else {
    return 1;
  }
}
