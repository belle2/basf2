#ifndef Belle2_SharedEventBuffer_h
#define Belle2_SharedEventBuffer_h

#include "daq/storage/storage_info.h"

#include "daq/slc/readout/SharedMemory.h"
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
    };

  public:
    SharedEventBuffer() {
      _buf = NULL;
      _nword = 0;
    }
    ~SharedEventBuffer() {
      if (_buf != NULL) _memory.close();
    }

  public:
    size_t size() throw();
    bool open(const std::string& nodename,
              size_t nword, bool recreate = false);
    bool init();
    bool close();
    bool unlink();
    bool lock() throw();
    bool unlock() throw();
    bool wait() throw();
    bool wait(int time) throw();
    bool notify() throw();
    void clear();

  public:
    const std::string getPath() const throw() { return _path; }
    Header* getHeader() throw() { return _header; }
    int* getBuffer() throw() { return _buf; }
    bool isWritable(int nword) throw();
    bool isReadable(int nword) throw();
    unsigned int write(const int* buf, unsigned int nword, unsigned int serial = 0);
    unsigned int read(int* buf);

  private:
    std::string _path;
    SharedMemory _memory;
    MMutex _mutex;
    MCond _cond;
    Header* _header;
    int* _buf;
    unsigned int _nword;

  };

}

#endif
