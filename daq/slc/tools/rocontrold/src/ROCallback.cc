#include "ROCallback.h"

#include "ProcessListener.h"
#include "RecieverManager.h"
#include "EventBuilderManager.h"

#include "system/Fork.h"

#include "base/Debugger.h"
#include "base/StringUtil.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <cstring>
#include <cstdio>

using namespace Belle2;

ROCallback::ROCallback(NSMNode* node)
  : RCCallback(node), _node(node)
{
  _buf_config = NULL;
  _buf_status = NULL;
}

ROCallback::~ROCallback() throw()
{
}

bool ROCallback::boot() throw()
{
  if (_buf_config == NULL) {
    _buf_config = openBuffer(4, "/ropc_config");
    if (_buf_config == NULL) {
      Belle2::debug("Failed to open buffer for config");
      setReply("Failed to open buffer for config");
      return false;
    }
    memset(_buf_config, 0, sizeof(int) * 4);
  }
  if (_buf_status == NULL) {
    _buf_status = openBuffer(4, "/ropc_status");
    if (_buf_status == NULL) {
      Belle2::debug("Failed to open buffer for status");
      setReply("Failed to open buffer for status");
      return false;
    }
    memset(_buf_status, 0, sizeof(int) * 4);
  }
  return true;
}

bool ROCallback::load() throw()
{
  system("killall basf2");
  system("killall eb0");
  //_fork_v[0].cancel();
  _fork_v[1].cancel();
  _fork_v[0] = Fork(new EventBuilderManager(_node));
  _fork_v[1] = Fork(new RecieverManager(_node));
  _thread_v[0] = PThread(new ProcessListener(this, _fork_v[0], "Event builder 0"));
  _thread_v[1] = PThread(new ProcessListener(this, _fork_v[1], "Serializer"));
  return true;
}

bool ROCallback::start() throw()
{
  int input[4] = {
    1,//RUNNING,
    (int)getMessage().getParam(0),//ExpNumber
    (int)getMessage().getParam(1),//RunNumber
    0
  };
  if (_buf_config != NULL)
    memcpy(_buf_config, input, sizeof(int) * 4);
  return true;
}

bool ROCallback::stop() throw()
{
  if (_buf_config != NULL)
    memset(_buf_config, 0, sizeof(int) * 4);
  system("killall eb0");
  return true;
}

bool ROCallback::resume() throw()
{
  return true;
}

bool ROCallback::pause() throw()
{
  return true;
}

bool ROCallback::abort() throw()
{
  if (_buf_config != NULL)
    memset(_buf_config, 0, sizeof(int) * 4);
  _fork_v[0].cancel();
  _fork_v[1].cancel();
  return true;
}

int* ROCallback::openBuffer(size_t count, const char* path) throw()
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

