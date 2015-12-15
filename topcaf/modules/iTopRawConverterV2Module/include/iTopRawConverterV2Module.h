#ifndef iTopRawConverterV2_H
#define iTopRawConverterV2_H

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

//Class used to process raw events
  class iTopRawConverterV2Module : public Module {
  public:
    typedef unsigned int packet_word_t;

    iTopRawConverterV2Module();
    ~iTopRawConverterV2Module();

//Module function (only to be used when running as a module!!!)
    void initialize();
    void beginRun();
    void event();

//Original function
    int LoadRawFile(const char* argc);
    EventHeaderPacket* GetEvtHeaderPacket();
    EventWaveformPacket* GetWaveformPacket();
    void Rewind() {m_input_file.seekg(m_prev_pos);}

  private:
    std::string m_input_filename;
    std::string m_input_directory;
    std::string m_input_fileandpath;
    std::ifstream m_input_file;
    std::string m_tmp;
    int m_current_pos, m_prev_pos;
    int m_carrier, m_scrod, m_module, m_evt_no;
    EventHeaderPacket* m_EvtPacket;
    EventWaveformPacket* m_WfPacket;
    StoreObjPtr<EventHeaderPacket>   m_evtheader_ptr;
    StoreArray<EventWaveformPacket>  m_evtwaves_ptr;
    StoreObjPtr<topFileMetaData>     m_filedata_ptr;
  };
}
#endif
