#ifndef StorageRBufferManager_h
#define StorageRBufferManager_h

#include <framework/pcore/RingBuffer.h>

#include <daq/slc/system/Mutex.h>

namespace Belle2 {

  class StorageRBufferManager {

  public:
    StorageRBufferManager(RingBuffer* buf = NULL)
      : m_rbuf(buf), m_serial(0) {}
    ~StorageRBufferManager() {
      if (m_rbuf != NULL) delete m_rbuf;
    }

  public:
    int read(char* buf);

  private:
    RingBuffer* m_rbuf;
    int m_serial;
    Mutex m_mutex;

  };

}

#endif
