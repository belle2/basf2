#include "daq/storage/modules/StorageWorker.h"
#include "daq/storage/modules/DataStorePackage.h"
#include "daq/storage/modules/StorageRBufferManager.h"

#include "daq/storage/BinData.h"

#include <framework/pcore/MsgHandler.h>

using namespace Belle2;

Mutex StorageWorker::g_mutex;
Cond StorageWorker::g_cond;
unsigned int StorageWorker::g_serial = 0;
std::queue<DataStorePackage*> StorageWorker::g_package_q;

void StorageWorker::run()
{
  if (m_buf == NULL) return;
  char* evbuf = new char [10000000];
  BinData* data = new BinData();
  data->setBuffer(evbuf);
  MsgHandler* msghandler = new MsgHandler(m_compressionLevel);
  while (true) {
    unsigned int serial = m_buf->read(evbuf);
    DataStorePackage* package = new DataStorePackage(serial);
    package->decode(*msghandler, *data);
    g_mutex.lock();
    while (true) {
      if (g_package_q.size() < 500000 &&
          g_serial == serial - 1) {
        g_package_q.push(package);
        g_serial++;
        g_cond.broadcast();
        break;
      }
      g_cond.wait(g_mutex);
    }
    g_mutex.unlock();
  }
}
