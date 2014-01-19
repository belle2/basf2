#ifndef Belle2_SharedEventBuffer_h
#define Belle2_SharedEventBuffer_h

#include "daq/storage/storage_info.h"

#include "daq/slc/readout/SharedMemory.h"
#include "daq/slc/readout/MMutex.h"
#include "daq/slc/readout/MCond.h"

namespace Belle2 {

  class SharedEventBuffer {

  private:
    static const int MAX_BUFFERS = 1000;

  public:
    SharedEventBuffer() {
      _buf = new int* [MAX_BUFFERS];
      _nword = 0;
    }
    ~SharedEventBuffer() {
      delete [] _buf;
    }

  public:
    size_t size() throw();
    bool open(const std::string& nodename,
              size_t nreserved = 0,
              bool recreate = false);
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
    storage_info* getInfo() throw() { return _info; }
    char* getPtr() throw() { return _ptr; }
    int write(const void* buf, size_t nword);
    int read(void* buf);

  private:
    std::string _path;
    SharedMemory _memory;
    char* _ptr;
    MMutex _mutex;
    MCond _cond;
    storage_info* _info;
    int* _i_read;
    int* _i_write;
    int* _ready;
    int** _buf;
    size_t _nword;

  };

}

#endif
