#ifndef Belle2_SharedEventBuffer_h
#define Belle2_SharedEventBuffer_h

#include "daq/storage/storage_info.h"

#include "daq/slc/readout/SharedMemory.h"
//#include "daq/slc/readout/MRWLock.h"
#include "daq/slc/readout/MMutex.h"
#include "daq/slc/readout/MCond.h"

namespace Belle2 {

  class SharedEventBuffer {

  public:
    struct Header {
      unsigned int expno;
      unsigned int runno;
      unsigned int subno;
      unsigned int count_in;
      unsigned int count_out;
      unsigned long long nword_in;
      unsigned long long nword_out;
      unsigned short nwriter;
      unsigned short nreader;
    };

  public:
    SharedEventBuffer() {
      m_buf = NULL;
      m_nword = 0;
    }
    ~SharedEventBuffer() {
      if (m_buf != NULL) m_memory.close();
    }

  public:
    size_t size() throw();
    bool open(const std::string& nodename,
              size_t nword, bool recreate = false);
    bool init();
    bool close();
    bool unlink();
    bool lock() throw();
    //bool rdlock() throw();
    //bool wrlock() throw();
    bool unlock() throw();
    bool wait() throw();
    bool wait(int time) throw();
    bool notify() throw();
    void clear();

  public:
    const std::string getPath() const throw() { return m_path; }
    Header* getHeader() throw() { return m_header; }
    int* getBuffer() throw() { return m_buf; }
    bool isWritable(int nword) throw();
    bool isReadable(int nword) throw();
    unsigned int write(const int* buf, unsigned int nword,
                       bool fouce, unsigned int serial = 0);
    unsigned int read(int* buf, bool fouce, Header* hdr = NULL);

  private:
    std::string m_path;
    SharedMemory m_memory;
    //MRWLock m_lock;
    MMutex m_mutex;
    MCond m_cond;
    Header* m_header;
    int* m_buf;
    unsigned int m_nword;

  };

}

#endif
