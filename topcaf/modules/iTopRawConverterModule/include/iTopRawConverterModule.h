#ifndef iTopRawConverter_H
#define iTopRawConverter_H

#include <framework/core/Module.h>

#include <fstream>
#include <iostream>

#include <topcaf/dataobjects/EventHeaderPacket.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <topcaf/dataobjects/topFileMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {

#define PACKET_HEADER (0x00BE11E2) //hex:BELLE2
#define PACKET_TYPE_EVENT (0x65766e74) //ascii:"evnt"
#define PACKET_TYPE_WAVEFORM (0x77617665) //ascii:"wave"
//Class used to process raw events
  class iTopRawConverterModule : public Module {
  public:
    typedef unsigned int packet_word_t;

    iTopRawConverterModule();
    ~iTopRawConverterModule();

//Module function (only to be used when running as a module!!!)
    void initialize();
    void beginRun();
    void event();

//Original function
    int LoadRawFile(const char* argc);
    int FindNextPacket();
    EventHeaderPacket* GetEvtHeaderPacket();
    EventWaveformPacket* GetWaveformPacket();
    void Rewind() {m_input_file.seekg(m_prev_pos);}

  private:
    std::string m_input_filename;
    std::string m_input_directory;
    std::string m_input_fileandpath;
    std::ifstream m_input_file;
    std::string m_tmp;
    unsigned int m_temp_buffer[1280];
    int m_current_pos, m_prev_pos;
    EventHeaderPacket* m_EvtPacket;
    EventWaveformPacket* m_WfPacket;
    StoreObjPtr<EventHeaderPacket>   m_evtheader_ptr;
    StoreArray<EventWaveformPacket>  m_evtwaves_ptr;
    StoreObjPtr<topFileMetaData>     m_filedata_ptr;
  };
}
#endif
