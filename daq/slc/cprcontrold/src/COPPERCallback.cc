#include "COPPERCallback.h"

#include "SenderManager.h"
#include "ProcessListener.h"

#include <system/Fork.h>
#include <system/PThread.h>

#include <base/COPPERNode.h>
#include <base/Debugger.h>
#include <base/StringUtil.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <iostream>

using namespace Belle2;

COPPERCallback::COPPERCallback(COPPERNode* node)
  : RCCallback(node), _node(node)
{
  _buf_config = NULL;
  _buf_status = NULL;
  _listener = NULL;
}

COPPERCallback::~COPPERCallback() throw()
{
}

bool COPPERCallback::boot() throw()
{
  if (_buf_config == NULL) {
    _buf_config = openBuffer(4, "/cpr_config");
    if (_buf_config == NULL) {
      Belle2::debug("[ERROR] Failed to open buffer /cpr_config");
      setReply("Failed to open buffer for config");
      return false;
    }
    memset(_buf_config, 0, sizeof(int) * 4);
  }
  if (_buf_status == NULL) {
    _buf_status = openBuffer(4, "/cpr_status");
    if (_buf_status == NULL) {
      Belle2::debug("[ERROR] Failed to open buffer /cpr_status");
      setReply("Failed to open buffer for status");
      return false;
    }
    memset(_buf_status, 0, sizeof(int) * 4);
  }
  for (int slot = 0; slot < 4; slot++) {
    _hslbcon_v[slot].setHSLB(slot, _node->getHSLB(slot));
  }
  /*
  FILE* file = popen("${B2SLC_PATH}/cprcontrold/ttrx/bootrx ${B2SLC_PATH}/cprcontrold/ttrx/tt4r009.bit", "r");
  char str[4096];
  memset(str, '\0', 4096);
  fread(str, 1, 4096 - 1, file);
  pclose(file);
  std::cout << str << std::endl;
  */

  for (int slot = 0; slot < 4; slot++) {
    if (!(_hslbcon_v[slot].reset() &&
          _hslbcon_v[slot].boot())) {
      Belle2::debug("[ERROR] Failed to boot HSLB:%c", (char)(slot + 'a'));
      setReply(Belle2::form("Failed to boot HSLB:%c", (char)(slot + 'a')));
      return false;
    }
  }

  return true;
}

bool COPPERCallback::load() throw()
{
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].load()) {
      Belle2::debug("[ERROR] Failed to load HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  if (_listener != NULL) {
    _listener->setRunning(false);
    _listener = NULL;
    _thread.cancel();
  }

  system("killall basf2");
  _fork.cancel();
  _buf_status[0] = 0;
  _fork = Fork(new SenderManager(_node));
  _listener = new ProcessListener(this, _fork);
  _thread = PThread(_listener);
  while (_buf_status[0] != 1) {
    sleep(1);
  }
  //sleep(3);
  return true;
}

bool COPPERCallback::start() throw()
{
  /*
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].start()) {
      Belle2::debug("Failed to start HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  */
  int input[4] = {
    1,//RUNNING,
    (int)getMessage().getParam(0),//ExpNumber
    (int)getMessage().getParam(1),//RunNumber
    0
  };
  memcpy(_buf_config, input, sizeof(int) * 4);
  return true;
}

bool COPPERCallback::stop() throw()
{
  /*
  for (size_t slot = 0; slot < 4; slot++) {
    if (!_hslbcon_v[slot].stop()) {
      Belle2::debug("Failed to stop HSLB:%c", (char)(slot + 'a'));
      return false;
    }
  }
  */
  if (_buf_config != NULL)
    memset(_buf_config, 0, sizeof(int) * 4);
  return true;
}

bool COPPERCallback::resume() throw()
{
  return true;
}

bool COPPERCallback::pause() throw()
{
  return true;
}

bool COPPERCallback::abort() throw()
{
  if (_buf_config != NULL)
    memset(_buf_config, 0, sizeof(int) * 4);
  _fork.cancel();
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
