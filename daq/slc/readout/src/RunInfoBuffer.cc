#include "daq/slc/readout/RunInfoBuffer.h"

#include <cstring>

using namespace Belle2;

size_t RunInfoBuffer::size() throw()
{
  return _mutex.size() + _cond.size() + sizeof(run_info);
}

bool RunInfoBuffer::open(const std::string& path)
{
  try {
    _path = path;
    _memory.open(path, size());
    char* buf = (char*) _memory.map(0, size());
    _mutex = MMutex(buf);
    buf += _mutex.size();
    _cond = MCond(buf);
    buf += _cond.size();
    _info = (run_info*)buf;
  } catch (const IOException& e) {
    return false;
  }
  return true;
}

bool RunInfoBuffer::init()
{
  try {
    _mutex.init();
    _cond.init();
    memset(_info, 0, sizeof(run_info));
  } catch (const IOException& e) {
    return false;
  }
  return true;
}

void RunInfoBuffer::clear()
{
  _mutex.lock();
  memset(_info, 0, sizeof(run_info));
  _mutex.unlock();
}

bool RunInfoBuffer::close()
{
  return _memory.close();
}

bool RunInfoBuffer::unlink()
{
  _memory.unlink();
  return _memory.close();
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
