#ifndef _Belle2_HistMemory_h
#define _Belle2_HistMemory_h

#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/EvtMessage.h>

#include <daq/slc/system/SharedMemory.h>
#include <daq/slc/system/MMutex.h>
#include <daq/slc/system/MCond.h>

#include <daq/slc/base/IOException.h>

#include <TH1.h>

#include <string>
#include <vector>

namespace Belle2 {

  class HistMemory {

  public:
    struct Header {
      unsigned int nbytes;
      unsigned int updateid;
      unsigned int timestamp;
    };

  public:
    static const unsigned int BUFFER_SIZE = 100000000; //100MB

  public:
    HistMemory() : m_path(), m_size(0),
      m_handler(0), m_body(NULL), m_buf(NULL), m_updateid(0) {}
    ~HistMemory() {}

  public:
    void open(const char* path, unsigned int size,
              const char* mode = "") throw(IOException);
    void init() throw(IOException);
    void serialize();
    std::vector<TH1*>& deserialize(Header* header = NULL);
    std::vector<TH1*>& get() { return m_hist; }
    TH1* add(TH1* h)
    {
      if (h == NULL) return NULL;
      m_hist.push_back(h);
      return h;
    }

  private:
    std::string m_path;
    unsigned int m_size;
    MsgHandler m_handler;
    int m_fd;
    char* m_body;
    char* m_buf;
    unsigned int m_updateid;
    SharedMemory m_memory;
    MMutex m_mutex;
    Header* m_header;
    std::vector<TH1*> m_hist;

  };

}

#endif
