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
    static DataStorePackage* getQueue() { return g_package_q; }
    static unsigned int getQueueIndex() { return g_package_i; }
    static void setQueueIndex(unsigned int i) { g_package_i = i; }

  public:
    const static unsigned int MAX_QUEUES = 40000;

  private:
    static Mutex g_mutex;
    static Cond g_cond;
    static unsigned int g_serial;
    static DataStorePackage* g_package_q;
    static unsigned int g_package_i;

  public:
    StorageWorker(StorageRBufferManager* buf = NULL, int level = 0);
    ~StorageWorker() {}

  public:
    void run();

  private:
    StorageRBufferManager* m_buf;
    int m_compressionLevel;

  };

}

#endif
