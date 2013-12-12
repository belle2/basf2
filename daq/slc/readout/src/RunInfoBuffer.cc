#include "daq/slc/readout/RunInfoBuffer.h"

#include <daq/slc/base/Debugger.h>

#include <cstring>

using namespace Belle2;

size_t RunInfoBuffer::size() throw()
{
  return _mutex.size() + _cond.size() + sizeof(unsigned int) * 5;
}

bool RunInfoBuffer::open(const std::string& path)
{
  _path = path;
  if (!_memory.open(path, size())) {
    return false;
  }
  char* buf = (char*) _memory.map(0, size());
  if (buf == NULL) {
    return false;
  }
  _info = (unsigned int*)buf;
  buf += sizeof(unsigned int) * 5;
  _mutex = MMutex(buf);
  buf += _mutex.size();
  _cond = MCond(buf);
  return true;
}

bool RunInfoBuffer::init()
{
  if (_info == NULL) return false;
  _mutex.init();
  _cond.init();
  memset(_info, 0, sizeof(unsigned int) * 5);
  return true;
}

void RunInfoBuffer::clear()
{
  _mutex.lock();
  memset(_info, 0, sizeof(unsigned int) * 5);
  _mutex.unlock();
}

bool RunInfoBuffer::close()
{
  _memory.close();
  return true;
}

bool RunInfoBuffer::unlink()
{
  _memory.unlink();
  _memory.close();
  return true;
}

bool RunInfoBuffer::lock() throw()
{
  return _mutex.lock();
}

bool RunInfoBuffer::unlock() throw()
{
  return _mutex.unlock();
}

bool RunInfoBuffer::wait() throw()
{
  _mutex.lock();
  _cond.wait(_mutex);
  return _mutex.unlock();
}

bool RunInfoBuffer::wait(int time) throw()
{
  _mutex.lock();
  _cond.wait(_mutex, time, 0);
  return _mutex.unlock();
}

bool RunInfoBuffer::notify() throw()
{
  _mutex.lock();
  _cond.broadcast();
  return _mutex.unlock();
}
