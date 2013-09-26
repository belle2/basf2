#include "DataReceiverCallback.hh"

#include "RecieverManager.hh"
#include "EventBuilderManager.hh"

#include <nsm/RunStatus.hh>
#include <nsm/DataReceiverData.hh>

#include <node/DataReceiverNode.hh>

#include <system/Fork.hh>

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

using namespace B2DAQ;

DataReceiverCallback::DataReceiverCallback(DataReceiverNode* node, DataReceiverData* data)
  : RCCallback(node), _node(node), _data(data)
{
  _status = new RunStatus("RUN_STATUS");
  _buf_config = NULL;
  _buf_status = NULL;
}

DataReceiverCallback::~DataReceiverCallback() throw()
{
  delete _status;
}

bool DataReceiverCallback::boot() throw()
{
  while (!_status->isAvailable()) {
    try {
      _status->open();
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("TTD daemon : Failed to open run status. Waiting for 5 seconds..");
      sleep(5);
    }
  }
  try {
    if (_data != NULL) {
      _data->read(_node);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to access NSM data.");
    return false;
  }

  FILE* file = popen("cd /home/usr/tkonno/cdc;/home/xilinx/ise12.4/ISE/bin/lin/impact -batch cdc41b2l011-impact.cmd", "r");
  char str[102400];
  memset(str, '\0', 102400);
  fread(str, 1, 102400 - 1, file);
  pclose(file);
  std::string s = str;
  std::cout << s << std::endl;

  if (_buf_config == NULL) {
    _buf_config = openBuffer(4, "/ropc_config");
    if (_buf_config == NULL) {
      B2DAQ::debug("Failed to open buffer for config");
      setReply("Failed to open buffer for config");
      return false;
    }
    memset(_buf_config, 0, sizeof(int) * 4);
  }
  if (_buf_status == NULL) {
    _buf_status = openBuffer(4, "/ropc_status");
    if (_buf_status == NULL) {
      B2DAQ::debug("Failed to open buffer for status");
      setReply("Failed to open buffer for status");
      return false;
    }
    memset(_buf_status, 0, sizeof(int) * 4);
  }

  return true;
}

bool DataReceiverCallback::reboot() throw()
{
  return boot();
}

bool DataReceiverCallback::load() throw()
{
  try {
    if (_data != NULL) {
      _data->read(_node);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to access NSM data.");
    return false;
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
  system("killall eb0");
  _fork_v[0].cancel();
  _fork_v[1].cancel();
  _fork_v[0] = Fork(new EventBuilderManager(_node));
  _fork_v[1] = Fork(new RecieverManager(_node));
  return true;
}

bool DataReceiverCallback::reload() throw()
{
  return load();
}

bool DataReceiverCallback::start() throw()
{
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

bool DataReceiverCallback::stop() throw()
{
  memset(_buf_config, 0, sizeof(int) * 4);
  return true;
}

bool DataReceiverCallback::resume() throw()
{
  return true;
}

bool DataReceiverCallback::pause() throw()
{
  return true;
}

bool DataReceiverCallback::recover() throw()
{
  return true;
}

bool DataReceiverCallback::abort() throw()
{
  return true;
}

int* DataReceiverCallback::openBuffer(size_t count, const char* path) throw()
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

