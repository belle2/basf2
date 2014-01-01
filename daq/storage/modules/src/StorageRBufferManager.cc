#include "daq/storage/modules/StorageRBufferManager.h"

#include <unistd.h>

using namespace Belle2;

int StorageRBufferManager::read(char* buf)
{
  m_mutex.lock();
  int size = 0;
  while ((size = m_rbuf->remq((int*)buf)) == 0) {
    usleep(20);
  }
  int serial = ++m_serial;
  m_mutex.unlock();
  return serial;
}
