#include "COPPERCallback.hh"

#include "SenderManager.hh"
#include "ProcessListener.hh"

#include <system/Fork.hh>
#include <system/PThread.hh>

#include <node/COPPERNode.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <iostream>

using namespace B2DAQ;

COPPERCallback::COPPERCallback(COPPERNode* node, NSMData* data)
  : RCCallback(node), _node(node), _data(data)
{
  for (int slot = 0; slot < 4; slot++) {
    _hslbcon_v[slot].setHSLB(slot, node->getHSLB(slot));
  }
  _status = new RunStatus("RUN_STATUS");
  _buf_config = NULL;
  _buf_status = NULL;
}

COPPERCallback::~COPPERCallback() throw()
{
  delete _status;
}

bool COPPERCallback::boot() throw()
{
  while (!_status->isAvailable()) {
    try {
      _status->open();
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("TTD daemon : Failed to open run status. Waiting for 5 seconds..");
      sleep(5);
    }
  }
  if (_data != NULL) {
    try {
      _data->read(_node);
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("Failed to access to NSM data, %s", e.what());
      return false;
    }
  }

  FILE* file = popen("${B2SLC_PATH}/cprcontrold/ttrx/bootrx ${B2SLC_PATH}/cprcontrold/ttrx/tt4r009.bit", "r");
  //FILE* file = popen("/home/usr/tkonno/b2slc/cprcontrold/ttrx/bootrx /home/usr/tkonno/b2slc/cprcontrold/ttrx/tt4r009.bit", "r");
  char str[1024];
  memset(str, '\0', 1024);
  fread(str, 1, 1024 - 1, file);
  pclose(file);
  std::string s = str;
  std::cout << s << std::endl;

  for (int slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].boot()) {
      B2DAQ::debug("Failed to boot HSLB:%c", (char)(slot + 'a'));
      setReply(B2DAQ::form("Failed to boot HSLB:%c", (char)(slot + 'a')));
      return false;
    }
  }
  if (_buf_config == NULL) {
    _buf_config = openBuffer(4, "/cpr_config");
    if (_buf_config == NULL) {
      B2DAQ::debug("Failed to open buffer for config");
      setReply("Failed to open buffer for config");
      return false;
    }
    memset(_buf_config, 0, sizeof(int) * 4);
  }
  if (_buf_status == NULL) {
    _buf_status = openBuffer(4, "/cpr_status");
    if (_buf_status == NULL) {
      B2DAQ::debug("Failed to open buffer for status");
      setReply("Failed to open buffer for status");
      return false;
    }
    memset(_buf_status, 0, sizeof(int) * 4);
  }
  return true;
}

bool COPPERCallback::reboot() throw()
{
  for (int slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].reboot()) {
      setReply(B2DAQ::form("Failed to reboot HSLB:%c", (char)(slot + 'a')));
      return false;
    }
  }
  return true;
}

bool COPPERCallback::load() throw()
{
  if (_data != NULL) {
    try {
      _data->read(_node);
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("Failed to access to NSM data %s", e.what());
      return false;
    }
  }
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].load()) {
      B2DAQ::debug("Failed to load HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  if (_status != NULL) {
    try {
      _status->read(NULL);
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("Failed to access to Run status %s", e.what());
      return false;
    }
  }
  system("killall basf2");
  _fork.cancel();
  _fork = Fork(new SenderManager(_node));
  PThread(new ProcessListener(this, _fork));
  return true;
}

bool COPPERCallback::reload() throw()
{
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].reload()) {
      B2DAQ::debug("Failed to reload HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  load();
  return true;
}

bool COPPERCallback::start() throw()
{
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].start()) {
      B2DAQ::debug("Failed to start HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  if (_status != NULL) {
    try {
      _status->read(NULL);
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("Failed to access to Run status %s", e.what());
      return false;
    }
  }
  int input[4] = {
    1,//RUNNING,
    (int)_status->getExpNumber(),
    (int)_status->getRunNumber(),
    0
  };
  memcpy(_buf_config, input, sizeof(int) * 4);
  return true;
}

bool COPPERCallback::stop() throw()
{
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].stop()) {
      B2DAQ::debug("Failed to stop HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  memset(_buf_config, 0, sizeof(int) * 4);
  return true;
}

bool COPPERCallback::resume() throw()
{
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].resume()) {
      B2DAQ::debug("Failed to resume HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  return true;
}

bool COPPERCallback::pause() throw()
{
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].pause()) {
      B2DAQ::debug("Failed to pause HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  return true;
}

bool COPPERCallback::recover() throw()
{
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].recover()) {
      B2DAQ::debug("Failed to recover HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  return true;
}

bool COPPERCallback::abort() throw()
{
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].abort()) {
      B2DAQ::debug("Failed to abort HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  return true;
}

int* COPPERCallback::openBuffer(size_t count, const char* path) throw()
{
  const int size = sizeof(int) * count;
  const int offset = 0;
  errno = 0;
  int fd = shm_open(path, O_CREAT | O_EXCL | O_RDWR, 0666);
  if (fd < 0) {
    if (errno != EEXIST) {
      perror("shm_open1");
      return NULL;
    }
    fd = shm_open(path, O_RDWR, 0666);
    if (fd < 0) {
      perror("shm_open2");
      return NULL;
    }
  }
  ftruncate(fd, size);
  return (int*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
}
