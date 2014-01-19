#include "daq/storage/SharedEventBuffer.h"

#include <daq/slc/base/Debugger.h>
#include <daq/slc/system/LogFile.h>

#include <cstring>

using namespace Belle2;

size_t SharedEventBuffer::size() throw()
{
  return _mutex.size() + _cond.size() + sizeof(storage_info) +
         sizeof(int) * (2 + (_nword * MAX_BUFFERS + 1));
}

bool SharedEventBuffer::open(const std::string& nodename,
                             size_t nword, bool recreate)
{
  _nword = nword;
  std::string username = getenv("USER");
  _path = "/storage_info_" + username + "_" + nodename;
  Belle2::debug("%s", _path.c_str());
  if (recreate) SharedMemory::unlink(_path);
  if (!_memory.open(_path, size())) {
    perror("shm_open");
    LogFile::fatal("Failed to open %s", _path.c_str());
    return false;
  }
  char* buf = _ptr = (char*) _memory.map(0, size());
  if (buf == NULL) {
    return false;
  }
  _mutex = MMutex(buf);
  buf += _mutex.size();
  _cond = MCond(buf);
  buf += _cond.size();
  _info = (storage_info*)buf;
  buf += sizeof(storage_info);
  _i_write = (int*)buf;
  buf += sizeof(int);
  _i_read = (int*)buf;
  buf += sizeof(int);
  _ready = (int*)buf;
  buf += sizeof(int) * MAX_BUFFERS;
  for (int i = 0; i < MAX_BUFFERS; i++) {
    _buf[i] = (int*)buf;
    buf += sizeof(int) * _nword;
  }
  return true;
}

bool SharedEventBuffer::init()
{
  if (_ptr == NULL) return false;
  _mutex.init();
  _cond.init();
  memset(_ptr + (_mutex.size() + _cond.size()), 0,
         size() - (_mutex.size() + _cond.size()));
  return true;
}

void SharedEventBuffer::clear()
{
  if (_ptr == NULL) return;
  _mutex.lock();
  memset(_ptr + (_mutex.size() + _cond.size()), 0,
         size() - (_mutex.size() + _cond.size()));
  _mutex.unlock();
}

bool SharedEventBuffer::close()
{
  _memory.close();
  return true;
}

bool SharedEventBuffer::unlink()
{
  _memory.unlink();
  _memory.close();
  return true;
}

bool SharedEventBuffer::lock() throw()
{
  if (_ptr == NULL) return false;
  return _mutex.lock();
}

bool SharedEventBuffer::unlock() throw()
{
  if (_ptr == NULL) return false;
  return _mutex.unlock();
}

bool SharedEventBuffer::wait() throw()
{
  if (_ptr == NULL) return false;
  return _cond.wait(_mutex);
}

bool SharedEventBuffer::wait(int time) throw()
{
  if (_ptr == NULL) return false;
  return _cond.wait(_mutex, time, 0);
}

bool SharedEventBuffer::notify() throw()
{
  if (_ptr == NULL) return false;
  return _cond.broadcast();
}

int SharedEventBuffer::write(const void* buf, size_t nword)
{
  if (_ptr == NULL) return 0;
  if (nword > _nword) return -1;
  _mutex.lock();
  if (*_i_write == MAX_BUFFERS) {
    *_i_write = 0;
  }
  int i = *_i_write;
  *_i_write = i + 1;
  while (_ready[i] != 0) {
    _cond.wait(_mutex);
  }
  _cond.broadcast();
  _ready[i] = 1;
  memcpy(_buf[i], buf, sizeof(size_t) * nword);
  _mutex.unlock();
  return nword;
}

int SharedEventBuffer::read(void* buf)
{
  if (_ptr == NULL) return 0;
  _mutex.lock();
  if (*_i_read == MAX_BUFFERS) {
    *_i_read = 0;
  }
  int i = *_i_read;
  *_i_read = i + 1;
  while (_ready[i] == 0) {
    _cond.wait(_mutex);
  }
  _cond.broadcast();
  size_t nword =  _buf[i][0];
  memcpy(buf, _buf[i], sizeof(size_t) * nword);
  _ready[i] = 0;
  _mutex.unlock();
  return nword;
}
