#ifndef iTopRawConverterV3_H
#define iTopRawConverterV3_H

#include <framework/core/Module.h>

#include <fstream>
#include <iostream>

#include <topcaf/dataobjects/EventHeaderPacket.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <topcaf/dataobjects/topFileMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {

#define PACKET_HEADER (0x00004042) //hex:BELLE2
#define PACKET_TYPE_EVENT (0x65766e74) //ascii: "evnt"
#define PACKET_TYPE_WAVEFORM (0x77617665) //ascii: "wave"
#define PACKET_LAST (0x6c617374) //ascii: "last"
#define NPOINTS 64
#define WORDS_PER_WINDOW 256
#define COPPER_HEADER 0x7F7F
#define COPPER_FOOTER 0x7FFF0006
#define HSLB_B2L_HEADER 0xFFAA

  // struct for copper header
  typedef struct {
    unsigned int num_words;
    unsigned char block_words;
    unsigned char version;
    unsigned short CPR_hdr;  // should be 0x7F7F
    unsigned int exprun;
    unsigned int event;
    unsigned short trig_type;
    unsigned short TT_ctime;
    unsigned int TT_utime;
    unsigned int node_id;
    unsigned int B2L1;  // Temporary!
    unsigned int offset_block1;
    unsigned int offset_block2;
    unsigned int offset_block3;
    unsigned int offset_block4;
  } CPR_hdr;

// struct for HLSB header
  typedef struct {
    unsigned short HSLB_tag;
    unsigned short B2L_hdr;  // should be 0xFFAA
  } HSLB_hdr;

  typedef struct {
    unsigned int word1;
    unsigned int CPR_ftr;  // should be 0x7FFF0006
  } CPR_ftr;

  typedef struct {
//unsigned int TT_ctime;  // removed in PreRawCOPPER format
//unsigned int TT_type;    // removed in PreRawCOPPER format
//unsigned short TT_tag;  // replaced in PreRawCOPPER format by crc16 error count...?
    unsigned short B2L_crc16;
    unsigned short B2L_crc16_error_cnt;  // added in PocketDAQ, only one of TT_tag/B2L_crc16_error_cnt is valid
  } B2L_ftr;

//Class used to process raw events
  class iTopRawConverterV3Module : public Module {


  public:
    typedef unsigned int packet_word_t;
    iTopRawConverterV3Module();
    ~iTopRawConverterV3Module();

//Module function (only to be used when running as a module!!!)
    void initialize();
    void terminate();
    void beginRun();
    void event();

//Original function
    int LoadRawFile(const char* argc);
    EventHeaderPacket* GetEvtHeaderPacket();
    EventWaveformPacket* GetWaveformPacket();
    void Rewind() {m_input_file.seekg(m_prev_pos);}

  private:
    packet_word_t readWordUnique(packet_word_t prev_word);
    packet_word_t swap_endianess(packet_word_t x);

    std::string m_input_filename;
    std::string m_input_directory;
    bool m_forceTrigger0xF;
    std::string m_input_fileandpath;
    std::ifstream m_input_file;
    std::string m_tmp;
    int m_current_pos, m_prev_pos;
    int m_carrier, m_scrod, m_evt_no;
    EventHeaderPacket* m_EvtPacket;
    EventWaveformPacket* m_WfPacket;
    StoreObjPtr<EventHeaderPacket>   m_evtheader_ptr;
    StoreArray<EventWaveformPacket>  m_evtwaves_ptr;
    StoreObjPtr<topFileMetaData>     m_filedata_ptr;

    CPR_hdr cpr_hdr;
    HSLB_hdr hslb_hdr;
    B2L_ftr b2l_ftr;
    CPR_ftr cpr_ftr;
  };
}
#endif
