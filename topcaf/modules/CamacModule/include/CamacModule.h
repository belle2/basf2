#ifndef CamacModule_H
#define CamacModule_H



#define WORDS_IN_CAMAC 118
#define EVENT_HEADER                      (0x12345678)
//Bit masks for the CC USB integer mode readout
#define MASK_HEADER_WORD_NUMBER_OF_EVENTS (0x000003FF)
#define MASK_HEADER_WATCHDOG_BUFFER_BIT   (0x00008000)
#define MASK_HEADER_SCALER_BUFFER_BIT     (0x00004000)
#define MASK_Q_RESPONSE                   (0x02000000)
#define MASK_X_RESPONSE                   (0x01000000)
#define MASK_DATA                         (0x00FFFFFF)
#define SLOT_END_MARKER                   (0xDDDD)
#define CRATE_END_MARKER                  (0xFFFF)
#define N_SLOTS (17)


#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>

#include <fstream>
#include <string>

#include <topcaf/dataobjects/CamacData.h>

namespace Belle2 {

  typedef unsigned int packet_word_t;

  int ReadUnixTimeAndSize(std::ifstream& fin, int& unix_time_sec, int& unix_time_msec, int& size);

//Class used to process raw events
  class CamacModule : public Module {
  public:

    CamacModule();
    ~CamacModule();

    void initialize();
    void beginRun();
    void event();
    void terminate();

    int LoadCamacFile();
    int FindNextPacket(int evtno_req = -1);

  private:
    std::string m_in_filename;
    std::ifstream m_input_file;
    char temp_buffer[sizeof(packet_word_t) * 1280];
    int m_current_pos;

    int unix_time_sec, unix_time_msec;
    StoreObjPtr<CamacData> m_camac_data;

    int AlignEvtno(int evtno_req, int& evtno_read);
    //  std::map< unsigned int, CamacData* > m_evt2camac;
    std::multimap<int, unsigned short> m_3377wmap;

    short m_c3377nhit;
    short m_c3377tdc[1024];
    short m_c3377lt[1024];
    short m_c3377ch[1024];

    double m_ftswTDCtiming;
    int m_ftswWord, m_ftswSlot;
    unsigned int m_crateID;
  };
}
#endif
