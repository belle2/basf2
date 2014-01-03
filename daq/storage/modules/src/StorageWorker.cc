#include "daq/storage/modules/StorageWorker.h"
#include "daq/storage/modules/DataStorePackage.h"
#include "daq/storage/modules/StorageRBufferManager.h"

#include "daq/storage/BinData.h"

#include <framework/pcore/MsgHandler.h>
#include <iostream>

using namespace Belle2;

Mutex StorageWorker::g_mutex;
Cond StorageWorker::g_cond;
unsigned int StorageWorker::g_serial = 0;
DataStorePackage* StorageWorker::g_package_q = NULL;
unsigned int StorageWorker::g_package_i;

StorageWorker::StorageWorker(StorageRBufferManager* buf, int level)
  : m_buf(buf), m_compressionLevel(level)
{
  if (g_package_q == NULL) {
    g_package_q = new DataStorePackage[MAX_QUEUES];
    g_package_i = 0;
  }
}

void StorageWorker::run()
{
  if (m_buf == NULL) return;
  char* evbuf = new char [10000000];
  BinData data;
  data.setBuffer(evbuf);
  MsgHandler msghandler(m_compressionLevel);
  DataStorePackage package;
  while (true) {
    unsigned int serial = m_buf->read(evbuf);
    package.setSerial(serial);
    //printf("%d %d\n", serial, data.getEventNumber());
    package.decode(msghandler, data);
    g_mutex.lock();
    while (true) {
      if (g_package_i < MAX_QUEUES &&
          g_serial == serial - 1) {
        g_package_q[g_package_i].copy(package);
        g_package_i++;
        g_serial++;
        g_cond.broadcast();
        break;
      }
      g_cond.wait(g_mutex);
    }
    g_mutex.unlock();
  }
}
