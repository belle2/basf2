#include "PackageManager.h"

#include <system/Time.h>

using namespace Belle2;

PackageManager::PackageManager(NSMData* data, AbstractMonitor* monitor)
{
  _monitor = monitor;
  _data = data;
  _available = false;
  _lock.init();
}

PackageManager::~PackageManager() throw()
{
  if (_monitor != NULL) delete _monitor;
}

void PackageManager::clear()
{
  _lock.wrlock();
  _available = false;
  _lock.unlock();
}

void PackageManager::init()
{
  _lock.wrlock();
  if (_monitor != NULL) {
    _monitor->init();
    _serializer.allocate(_monitor->getPackage());
    _serializer.update();
    _available = true;
  } else {
    _available = false;
  }
  _lock.unlock();
}

void PackageManager::update()
{
  _lock.wrlock();
  if (_monitor != NULL) {
    _monitor->getPackage()->incrementUpdateId();
    _monitor->getPackage()->setUpdateTime(Time().getSecond());
    if (_data->isAvailable()) _monitor->update(_data);
  }
  _serializer.update();
  _lock.unlock();
}

void PackageManager::setAvailable(bool available)
{
  _lock.wrlock();
  _available = available;
  _lock.unlock();
}

bool PackageManager::isAvailable()
{
  _lock.wrlock();
  bool available = _available;
  _lock.unlock();
  return available;
}

char* PackageManager::createConfig(size_t& buf_size)
{
  _lock.rdlock();
  char* buf = NULL;
  if (_available) {
    buf = _serializer.getConfig().createBuffer(buf_size);
  }
  _lock.unlock();
  return buf;
}

char* PackageManager::createContents(size_t& buf_size)
{
  _lock.rdlock();
  char* buf = NULL;
  if (_available) {
    buf = _serializer.getContents().createBuffer(buf_size);
  }
  _lock.unlock();
  return buf;
}

char* PackageManager::createContentsAll(size_t& buf_size)
{
  _lock.rdlock();
  char* buf = NULL;
  if (_available) {
    buf = _serializer.getContentsAll().createBuffer(buf_size);
  }
  _lock.unlock();
  return buf;
}

size_t PackageManager::copyConfig(char* buf, size_t size)
{
  _lock.rdlock();
  size_t count = 0;
  if (_available) {
    count = _serializer.getConfig().copy(buf, size);
  }
  _lock.unlock();
  return count;
}

size_t PackageManager::copyContents(char* buf, size_t size, int& update_id)
{
  _lock.rdlock();
  size_t count = 0;
  if (_available && update_id != _monitor->getPackage()->getUpdateId()) {
    update_id = _monitor->getPackage()->getUpdateId();
    count = _serializer.getContents().copy(buf, size);
  }
  _lock.unlock();
  return count;
}

size_t PackageManager::copyContentsAll(char* buf, size_t size, int& update_id)
{
  _lock.rdlock();
  size_t count = 0;
  if (_available &&
      update_id != _monitor->getPackage()->getUpdateId()) {
    update_id = _monitor->getPackage()->getUpdateId();
    count = _serializer.getContentsAll().copy(buf, size);
  }
  _lock.unlock();
  return count;
}

const std::string PackageManager::createXML()
{
  _lock.rdlock();
  std::string xml = "";
  if (_available) {
    xml = _monitor->getRootPanel()->toXML();
  }
  _lock.unlock();
  return xml;
}

int PackageManager::getUpdateId()
{
  _lock.rdlock();
  int id = 0;
  if (_monitor != NULL) {
    id = _monitor->getPackage()->getUpdateId();
  }
  _lock.unlock();
  return id;
}

std::string PackageManager::getName()
{
  _lock.rdlock();
  std::string name = "";
  if (_monitor != NULL) {
    name = _monitor->getPackage()->getName();
  }
  _lock.unlock();
  return name;
}
