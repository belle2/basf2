#include "daq/slc/apps/PackageManager.h"

#include <daq/slc/system/Time.h>
#include <daq/slc/base/Debugger.h>

using namespace Belle2;

PackageManager::PackageManager(MonitorPackage* monitor)
{
  _monitor = monitor;
  _available = false;
  _updateid = -1;
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

bool PackageManager::init()
{
  _lock.wrlock();
  _monitor->init();
  _serializer.allocate(getPackage());
  _serializer.update();
  _available = true;
  _updateid = -1;
  _lock.unlock();
  return true;
}

bool PackageManager::update()
{
  _lock.wrlock();
  getPackage()->setUpdateTime(Belle2::Time().getSecond());
  bool updated = _monitor->update();
  if (updated) {
    getPackage()->incrementUpdateId();
    _serializer.update();
  }
  _lock.unlock();
  return updated;
}

void PackageManager::reset()
{
  _lock.wrlock();
  getPackage()->reset();
  getPackage()->setUpdateId(0);
  getPackage()->setUpdateTime(Belle2::Time().getSecond());
  _monitor->update();
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
  if (_available && update_id != getPackage()->getUpdateId()) {
    update_id = getPackage()->getUpdateId();
    count = _serializer.getContents().copy(buf, size);
  }
  _lock.unlock();
  return count;
}

size_t PackageManager::copyContentsAll(char* buf, size_t size, int& update_id)
{
  _lock.rdlock();
  size_t count = 0;
  if (_available && update_id != getPackage()->getUpdateId()) {
    update_id = getPackage()->getUpdateId();
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
    xml = getPanel()->toXML();
  }
  _lock.unlock();
  return xml;
}

int PackageManager::getUpdateId()
{
  _lock.rdlock();
  int id = getPackage()->getUpdateId();
  _lock.unlock();
  return id;
}

std::string PackageManager::getName()
{
  _lock.rdlock();
  std::string name = getPackage()->getName();
  _lock.unlock();
  return name;
}

