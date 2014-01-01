#ifndef StorageWorker_h
#define StorageWorker_h

#include "daq/storage/modules/DataStorePackage.h"

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <queue>

namespace Belle2 {

  class StorageRBufferManager;

  class StorageWorker {

  public:
    static void lock() { g_mutex.lock(); }
    static void unlock() { g_mutex.unlock(); }
    static void wait() { g_cond.wait(g_mutex); }
    static void notify() { g_cond.broadcast(); }
    static std::queue<DataStorePackage*>& getQueue() {
      return g_package_q;
    }

  private:
    static Mutex g_mutex;
    static Cond g_cond;
    static unsigned int g_serial;
    static std::queue<DataStorePackage*> g_package_q;

  public:
    StorageWorker(StorageRBufferManager* buf = NULL, int level = 0)
      : m_buf(buf), m_compressionLevel(level) {}
    ~StorageWorker() {}

  public:
    void run();

  private:
    StorageRBufferManager* m_buf;
    int m_compressionLevel;

  };

}

#endif
