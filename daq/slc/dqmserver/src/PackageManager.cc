#include "PackageManager.h"

#include "HistoFileReader.h"

#include <dqm/HistoManager.h>

#include <system/Time.h>

using namespace Belle2;

PackageManager::PackageManager(HistoManager* manager)
{
  _manager = manager;
  _reader = NULL;
  _pack = NULL;
  _available = false;
  _lock.init();
}

PackageManager::~PackageManager() throw()
{
  if (_manager != NULL) delete _manager;
  if (_reader != NULL) delete _reader;
  if (_pack != NULL) delete _pack;
}

void PackageManager::clear()
{
  _lock.wrlock();
  _available = false;
  if (_reader != NULL) delete _reader;
  _reader = NULL;
  if (_pack != NULL) delete _pack;
  _pack = NULL;
  if (_panel != NULL) delete _panel;
  _panel = NULL;
  _lock.unlock();
}

bool PackageManager::init()
{
  _lock.wrlock();
  _reader = new HistoFileReader(_map_path.c_str());
  bool ready = _reader->isReady();
  if (ready) {
    _pack = createPackage();
    _panel = createRootPanel();
    _serializer.allocate(_pack);
    _serializer.update();
    _available = true;
  }
  _lock.unlock();
  return ready;
}

void PackageManager::update()
{
  _lock.wrlock();
  _reader->update(_pack);
  _pack->incrementUpdateId();
  _pack->setUpdateTime(Belle2::Time().getSecond());
  analyze();
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
  if (_available && update_id != _pack->getUpdateId()) {
    update_id = _pack->getUpdateId();
    count = _serializer.getContents().copy(buf, size);
  }
  _lock.unlock();
  return count;
}

size_t PackageManager::copyContentsAll(char* buf, size_t size, int& update_id)
{
  _lock.rdlock();
  size_t count = 0;
  if (_available && update_id != _pack->getUpdateId()) {
    update_id = _pack->getUpdateId();
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
    xml = _panel->toXML();
  }
  _lock.unlock();
  return xml;
}

int PackageManager::getUpdateId()
{
  _lock.rdlock();
  int id = _pack->getUpdateId();
  _lock.unlock();
  return id;
}

std::string PackageManager::getName()
{
  _lock.rdlock();
  std::string name = _pack->getName();
  _lock.unlock();
  return name;
}

HistoPackage* PackageManager::createPackage()
{
  return _manager->createPackage(_reader->getHists());
}

RootPanel* PackageManager::createRootPanel()
{
  return _manager->createRootPanel(_reader->getHists());
}

void PackageManager::analyze()
{
  _manager->analyze(_reader->getHists());
}
